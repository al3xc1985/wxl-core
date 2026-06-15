// Live engine M2 object views shared by the runtime M2 fixups.
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

#include <cstddef>
#include <cstdint>

#include "structure/M2/Client.hpp"
#include "structure/M2/Source.hpp"

// DLL-only. Views of the engine's in-memory model objects once a model is loaded (not the on-disk
// format). The on-disk record structs (M2Header / M2Batch / M2SkinSection / ...) are shared with the
// host contract via structure/M2/Client.hpp. These views read the LIVE engine objects whose fields the
// runtime fixups touch.
namespace wraith::runtime::m2
{
    // The engine's parsed skin profile, hung off the model object. This is the LIVE in-memory object,
    // NOT the on-disk skin: the parse prepends a 4-byte leading field, so every array sits 4 bytes higher
    // than the file layout. Array offsets are raw pointers by the time the finalize/draw paths read them.
    // vertexLookup[i] -> global vertex index. indices[] are global local-vertex indices (into
    // vertexLookup), in [section.vertexStart, +vertexCount). bones[i] (uint8[4]) is per local vertex and
    // holds LOCAL indices into the section's boneCombos slice; these are the on-GPU vertex bone indices
    // the palette is addressed by.
#pragma pack(push, 1)
    struct M2SkinProfile
    {
        uint32_t                          _lead;        // 0x00  parse-prepended leading field
        uint32_t                          vertexCount;  // 0x04
        uint16_t*                         vertexLookup; // 0x08
        uint32_t                          indexCount;   // 0x0C
        uint16_t*                         indices;      // 0x10
        uint32_t                          boneCount;    // 0x14  (= vertexCount; bones[] is per local vertex)
        uint8_t*                          bones;        // 0x18  (uint8[4] per local vertex; local slice indices)
        uint32_t                          submeshCount; // 0x1C
        wraith::structure::m2::M2SkinSection* submeshes; // 0x20
        uint32_t                          batchCount;   // 0x24  (texunits)
        wraith::structure::m2::M2Batch*   batches;      // 0x28
        uint32_t                          boneCountMax; // 0x2C  per-draw bone budget seed
    };
#pragma pack(pop)

    static_assert(offsetof(M2SkinProfile, vertexCount) == 0x04, "skin.vertexCount");
    static_assert(offsetof(M2SkinProfile, vertexLookup) == 0x08, "skin.vertexLookup");
    static_assert(offsetof(M2SkinProfile, indexCount) == 0x0C, "skin.indexCount");
    static_assert(offsetof(M2SkinProfile, indices) == 0x10, "skin.indices");
    static_assert(offsetof(M2SkinProfile, bones) == 0x18, "skin.bones");
    static_assert(offsetof(M2SkinProfile, submeshCount) == 0x1C, "submeshCount");
    static_assert(offsetof(M2SkinProfile, submeshes) == 0x20, "submeshes");
    static_assert(offsetof(M2SkinProfile, batchCount) == 0x24, "batchCount");
    static_assert(offsetof(M2SkinProfile, batches) == 0x28, "batches");
    static_assert(offsetof(M2SkinProfile, boneCountMax) == 0x2C, "boneCountMax");
    static_assert(sizeof(M2SkinProfile) == 0x30, "M2SkinProfile");

    // The engine's loaded model object (the model asset, shared by all its instances).
    struct ModelView
    {
        uint8_t* base = nullptr;
        explicit ModelView(void* p) : base(static_cast<uint8_t*>(p)) {}

        const char* name() const { return reinterpret_cast<const char*>(base + 0x3C); } // inline path string
        wraith::structure::m2::M2Header* fileData() const
        {
            return *reinterpret_cast<wraith::structure::m2::M2Header**>(base + 0x150);
        }
        uint32_t fileSize() const { return *reinterpret_cast<uint32_t*>(base + 0x16C); }
        M2SkinProfile* skin() const { return *reinterpret_cast<M2SkinProfile**>(base + 0x170); }
    };

    // The live render instance (one per visible model). Its bone palette + transform are animated each
    // frame; the draw context points to it.
    struct InstanceView
    {
        uint8_t* base = nullptr;
        explicit InstanceView(void* p) : base(static_cast<uint8_t*>(p)) {}

        void* model() const { return *reinterpret_cast<void**>(base + 0x2C); }            // shared model asset
        const float* bonePalette() const { return *reinterpret_cast<const float**>(base + 0x98); } // live, row-major
    };

    // The per-batch render context the color-body draw receives as 'this'.
    struct M2DrawContext
    {
        uint8_t* base = nullptr;
        explicit M2DrawContext(void* p) : base(static_cast<uint8_t*>(p)) {}

        void* instance() const { return *reinterpret_cast<void**>(base + 0x60); } // the instance being drawn
        void* material() const { return *reinterpret_cast<void**>(base + 0x98); } // live material set by caller
    };
}
