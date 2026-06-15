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

#include "Particle.hpp"
#include "M2Runtime.hpp"

#include "Hook.hpp"
#include "Logger.hpp"
#include "M2.hpp"

#include <windows.h>
#include <cstdint>

namespace off = wraith::offsets::game::m2;
namespace sdk = wraith::structure::m2;

// Scope the lowered alpha-key reference (0.5) to Source models at draw time. Stock content keeps its
// vanilla cutoff.
//
// The shared per-batch alpha/material setter is called by BOTH the creature triangle path and the doodad
// path with the same draw context, so hooking it covers trees (doodads) and creatures in one place. It
// derives the alpha-key ref from the material blend mode and pushes it to the device. After the original
// runs we re-push 0.5 for Source mode-1 (alpha-key) batches, overwriting the same value. Mode 0 (test
// off) and mode >= 2 are left as set.
namespace wraith::runtime::particle
{
    namespace
    {
        off::M2_SetupBatchAlphaFn g_setupAlphaOriginal = nullptr;
        auto g_pushAlphaRef = reinterpret_cast<off::M2_PushAlphaRefFn>(off::kPushAlphaRef);

        // The Source alpha-key cutoff: the coverage midpoint, where Source leaf/foliage coverage-alpha sits.
        constexpr float kSourceAlphaKeyRef = 0.5f;

        // Material blend-mode field offset within a live material record.
        constexpr size_t kOffMaterialBlend = 0x02;

        // True if the batch's model is a Source model and its material is blend mode 1 (alpha key).
        bool IsSourceAlphaKeyBatch(void* ctx)
        {
            bool hit = false;
            __try
            {
                wraith::runtime::m2::M2DrawContext dc(ctx);
                void* instPtr = dc.instance();
                if (!instPtr) return false;
                wraith::runtime::m2::InstanceView inst(instPtr);
                void* modelPtr = inst.model();
                if (!modelPtr) return false;
                wraith::runtime::m2::ModelView model(modelPtr);
                sdk::M2Header* md = model.fileData();
                if (!md || md->magic != sdk::kMagicMD20 || md->version < sdk::kSourceVersionMin)
                    return false;

                void* mat = dc.material();
                if (!mat) return false;
                hit = (*reinterpret_cast<uint16_t*>(static_cast<char*>(mat) + kOffMaterialBlend) == 1);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) { hit = false; }
            return hit;
        }

        void __fastcall SetupAlphaDetour(void* ctx)
        {
            g_setupAlphaOriginal(ctx);
            if (IsSourceAlphaKeyBatch(ctx))
                g_pushAlphaRef(kSourceAlphaKeyRef);
        }
    }

    void Install()
    {
        hook::Install("M2::SetupBatchAlpha", off::kSetupBatchAlpha, &SetupAlphaDetour,
                      reinterpret_cast<void**>(&g_setupAlphaOriginal));
        WLOG_INFO("M2/particle: alpha-key scope installed (Source >= %u)", sdk::kSourceVersionMin);
    }
}
