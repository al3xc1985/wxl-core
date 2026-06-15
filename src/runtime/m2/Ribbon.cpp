// Copyright (C) 2026 WraithEngine
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include "Ribbon.hpp"

#include "Hook.hpp"
#include "Logger.hpp"
#include "M2.hpp"
#include "Gx.hpp"

#include <windows.h>
#include <d3d9.h>
#include <cstdint>

namespace off = wraith::offsets::game::m2;
namespace gx  = wraith::offsets::engine::gx;

// A Source multi-texture ribbon layers 3 textures meant to combine as tex0*tex1*tex2*color*4. The native
// engine draws an N-texture ribbon as N sequential single-texture passes on sampler s0, which cannot
// reproduce that product. The product is expressible in fixed-function (MODULATE, MODULATE, MODULATE4X),
// so a 3+ layer ribbon is drawn in ONE pass: bind the 3 resolved textures to s0/s1/s2 and set the stage
// combine at the draw call. The engine's own transforms place the strip and its additive blend is already
// set, so there is no shader swap and no constant push - only the texture-stage combine changes.
namespace wraith::runtime::ribbon
{
    namespace
    {
        // Set while a Source multi-texture ribbon draw is in flight; the draw override applies the 3-stage
        // combine only then, and saves/restores stage state so other draws are unaffected.
        volatile bool g_ribbonModern = false;

        // The engine graphics-device object, and the render-API device pointer hung off it.
        void* GxDevice() { return *reinterpret_cast<void**>(gx::kDevicePtr); }
        IDirect3DDevice9* RenderDevice()
        {
            void* dev = GxDevice();
            if (!dev) return nullptr;
            return *reinterpret_cast<IDirect3DDevice9**>(static_cast<uint8_t*>(dev) + gx::kOffRenderDevice);
        }

        // Sampler selectors for the engine bind path (s0 = 0x15, consecutive).
        constexpr uint32_t kSelS1 = 0x16;
        constexpr uint32_t kSelS2 = 0x17;

        auto TexResolve  = reinterpret_cast<off::M2_TexResolveFn>(off::kTexResolve);
        auto SamplerBind = reinterpret_cast<off::M2_SamplerBindFn>(off::kSamplerBind);

        using DipFn = HRESULT(STDMETHODCALLTYPE*)(IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
        DipFn g_origDip = nullptr;
        bool  g_dipPatched = false;

        HRESULT STDMETHODCALLTYPE HookDip(IDirect3DDevice9* dev, D3DPRIMITIVETYPE type, INT base, UINT minIdx,
                                          UINT numVerts, UINT startIdx, UINT primCount)
        {
            // Source multi-texture ribbon: all 3 textures are bound (s0 by the engine's loop, s1/s2
            // pre-bound by the draw hook). Apply tex0*tex1*tex2*color*4 in fixed-function. Stage state is
            // saved and restored so the next draw is unaffected; the additive frame blend is already set.
            if (g_ribbonModern)
            {
                DWORD s[4][4];
                for (DWORD st = 0; st < 4; ++st)
                {
                    dev->GetTextureStageState(st, D3DTSS_COLOROP,   &s[st][0]);
                    dev->GetTextureStageState(st, D3DTSS_COLORARG1, &s[st][1]);
                    dev->GetTextureStageState(st, D3DTSS_COLORARG2, &s[st][2]);
                    dev->GetTextureStageState(st, D3DTSS_ALPHAOP,   &s[st][3]);
                }

                const D3DTEXTUREOP op[3] = { D3DTOP_MODULATE, D3DTOP_MODULATE, D3DTOP_MODULATE4X };
                for (DWORD st = 0; st < 3; ++st)
                {
                    dev->SetTextureStageState(st, D3DTSS_COLOROP,   op[st]);
                    dev->SetTextureStageState(st, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                    dev->SetTextureStageState(st, D3DTSS_COLORARG2, st == 0 ? D3DTA_DIFFUSE : D3DTA_CURRENT);
                    dev->SetTextureStageState(st, D3DTSS_ALPHAOP,   op[st]);
                    dev->SetTextureStageState(st, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                    dev->SetTextureStageState(st, D3DTSS_ALPHAARG2, st == 0 ? D3DTA_DIFFUSE : D3DTA_CURRENT);
                }
                dev->SetTextureStageState(3, D3DTSS_COLOROP, D3DTOP_DISABLE);

                HRESULT hr = g_origDip(dev, type, base, minIdx, numVerts, startIdx, primCount);

                for (DWORD st = 0; st < 4; ++st)
                {
                    dev->SetTextureStageState(st, D3DTSS_COLOROP,   s[st][0]);
                    dev->SetTextureStageState(st, D3DTSS_COLORARG1, s[st][1]);
                    dev->SetTextureStageState(st, D3DTSS_COLORARG2, s[st][2]);
                    dev->SetTextureStageState(st, D3DTSS_ALPHAOP,   s[st][3]);
                }
                return hr;
            }
            return g_origDip(dev, type, base, minIdx, numVerts, startIdx, primCount);
        }

        // Patch this device's draw slot once. Each device has its own vtable, so patch the live engine
        // device the moment it exists.
        void PatchDip(IDirect3DDevice9* dev)
        {
            if (g_dipPatched || !dev) return;
            void** vt = *reinterpret_cast<void***>(dev);
            DWORD old = 0;
            VirtualProtect(&vt[gx::kVtDrawIndexedPrimitive], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
            g_origDip = reinterpret_cast<DipFn>(vt[gx::kVtDrawIndexedPrimitive]);
            vt[gx::kVtDrawIndexedPrimitive] = reinterpret_cast<void*>(&HookDip);
            VirtualProtect(&vt[gx::kVtDrawIndexedPrimitive], sizeof(void*), old, &old);
            g_dipPatched = true;
            WLOG_INFO("M2/ribbon: draw slot hooked on engine device %p", dev);
        }

        // Bind layers 1 and 2 to s1/s2 through the engine (the native draw loop only binds s0, so these
        // survive into the single pass). Only called with layerCount >= 3, so [1] and [2] are in bounds.
        bool BindExtraSamplers(void* gxDev, const uint8_t* emitter)
        {
            const void* const* arr =
                *reinterpret_cast<const void* const* const*>(emitter + off::kOffRibbonTexHandlePtr);
            if (!arr) return false;
            void* h1 = const_cast<void*>(arr[1]);
            void* h2 = const_cast<void*>(arr[2]);
            if (!h1 || !h2) return false;
            void* t1 = TexResolve(h1, 0, 0);
            void* t2 = TexResolve(h2, 0, 0);
            if (!t1 || !t2) return false;
            SamplerBind(gxDev, nullptr, kSelS1, t1);
            SamplerBind(gxDev, nullptr, kSelS2, t2);
            return true;
        }

        constexpr int kRibbonLoggedMax = 16;
        const void* g_ribbonLogged[kRibbonLoggedMax] = {};
        int         g_ribbonLoggedCount = 0;

        void LogRibbonOnce(const void* emitter, uint32_t layerCount)
        {
            for (int i = 0; i < g_ribbonLoggedCount; ++i)
                if (g_ribbonLogged[i] == emitter) return;
            if (g_ribbonLoggedCount >= kRibbonLoggedMax) return;
            g_ribbonLogged[g_ribbonLoggedCount++] = emitter;
            WLOG_INFO("M2/ribbon: emitter %p drawn as single-pass 3-texture combine (%u textures)",
                      emitter, layerCount);
        }

        off::M2_RibbonDrawFn g_ribbonDraw = nullptr;

        // ON: 3+ layer ribbons use the single-pass combine. OFF: native multi-pass (additive) ribbon.
        constexpr bool kRibbonMultiTex = true;

        // Ribbon emitter draw: native this-in-ECX. For a 3+ layer ribbon, pre-bind s1/s2, flag the draw so
        // the override applies the 3-stage combine, and clamp the layer count to 1 so the engine's own
        // draw runs exactly one pass; otherwise run the native N-pass draw untouched.
        int __fastcall HookRibbonDraw(void* self, void* edx, void* stateBlock)
        {
            g_ribbonModern = false;

            uint8_t* emitter = static_cast<uint8_t*>(self);
            bool bridged = false;
            uint32_t savedLayerCount = 0;
            uint32_t* layerCountPtr = nullptr;

            if (emitter)
            {
                __try
                {
                    layerCountPtr = reinterpret_cast<uint32_t*>(emitter + off::kOffRibbonLayerCount);
                    uint32_t layers = *layerCountPtr;
                    if (kRibbonMultiTex && layers >= 3)
                    {
                        void* gxDev = GxDevice();
                        if (gxDev)
                        {
                            PatchDip(RenderDevice());
                            if (BindExtraSamplers(gxDev, emitter))
                            {
                                g_ribbonModern = true;
                                savedLayerCount = layers;
                                *layerCountPtr = 1;
                                bridged = true;
                                LogRibbonOnce(emitter, layers);
                            }
                        }
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER) { bridged = false; g_ribbonModern = false; }
            }

            int r = g_ribbonDraw(self, edx, stateBlock);

            if (bridged && layerCountPtr)
                __try { *layerCountPtr = savedLayerCount; }
                __except (EXCEPTION_EXECUTE_HANDLER) {}
            g_ribbonModern = false;
            return r;
        }
    }

    void Install()
    {
        hook::Install("M2::RibbonDraw", off::kRibbonDraw, &HookRibbonDraw,
                      reinterpret_cast<void**>(&g_ribbonDraw));
        WLOG_INFO("M2/ribbon: multi-texture combine installed");
    }
}
