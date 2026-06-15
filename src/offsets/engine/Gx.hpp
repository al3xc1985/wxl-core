// Graphics-device pointer, device-cache field offsets, and render-vtable slots.
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

#pragma once

#include <cstdint>
#include <cstddef>

// Graphics-device addresses, device-object field offsets, and render-API vtable slots.
namespace wraith::offsets::engine::gx
{
    // Pointer to the engine graphics-device object. The render-API device pointer lives at
    // (device + kOffRenderDevice).
    constexpr uintptr_t kDevicePtr        = 0x00C5DF88;
    constexpr size_t    kOffRenderDevice  = 0x397C; // render-API device pointer field

    // Cached render-target surfaces on the graphics-device object.
    constexpr size_t    kOffBackBuffer    = 0x3B3C; // cached back-buffer surface
    constexpr size_t    kOffDepthSurface  = 0x3B40; // cached world depth surface

    // Render-device vtable slots (standard layout). DrawIndexedPrimitive is the single primitive-draw
    // route hooked by the draw path; the shader slots create / set / release shader objects, and the
    // constant slots upload shader constant registers.
    constexpr int kVtRelease              = 2;   // shader/COM object release
    constexpr int kVtDrawIndexedPrimitive = 82;  // (type, baseVtx, minVtx, numVtx, startIdx, primCount)
    constexpr int kVtCreateVertexShader   = 91;  // (const dword* code, out vertexShader)
    constexpr int kVtSetVertexShader      = 92;  // (vertexShader)
    constexpr int kVtSetVertexShaderConst = 94;  // (startReg, const float*, vec4Count)
    constexpr int kVtCreatePixelShader    = 106; // (const dword* code, out pixelShader)
    constexpr int kVtSetPixelShader       = 107; // (pixelShader)
    constexpr int kVtSetPixelShaderConst  = 109; // (startReg, const float*, vec4Count)

    // Engine-internal shader-constant upload (the device's own constant path), addressed as a vtable
    // byte-offset on the graphics-device object. shaderType 0 = vertex, 4 = pixel.
    constexpr int kVtSetShaderConstant    = 0x118 / 4; // byte 0x118

    // Engine-internal shader-constant uploader: native this-in-ECX; declared with a dummy second
    // parameter so the trampoline keeps the trailing arguments on the stack.
    using Gx_SetShaderConstantFn = void(__fastcall*)(void* device, void* edx, uint32_t shaderType, uint32_t startReg,
                                                     const float* data, uint32_t vec4Count);
}
