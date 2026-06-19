// Map-doodad (placed M2) runtime object fields and the per-chunk doodad list layout.
// Copyright (C) 2026 WarcraftXL
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

// INTERNAL to the core. The runtime placed-doodad object (one per map M2 placement) and the per-chunk
// list that holds them. The placement transform lives here, not on the shared render model. The chunk
// lookup itself is in offsets/game/ADT.hpp (kGetChunk). Modules never include this; they use wxl::game.
namespace wxl::offsets::game::doodad
{
    // --- spawn ---
    // Build a CMapDoodad from an MDDF placement (modelName, MDDF entry, tile origin). Returns the new
    // CMapDoodad* in EAX. The "a placed doodad was created" point.
    constexpr uintptr_t kSpawnFromMDDF = 0x007BECD0;
    // __cdecl, 3 stack args, returns CMapDoodad*.
    using SpawnFromMDDFFn = void*(__cdecl*)(const char* modelName, void* mddf, void* tileOrigin);

    // --- placed-doodad object fields ---
    constexpr size_t kFlags = 0x0C; // 1 = normal placement

    // World position (C3Vector).
    constexpr size_t kPosX  = 0x6C;
    constexpr size_t kPosY  = 0x70;
    constexpr size_t kPosZ  = 0x74;
    constexpr size_t kScale = 0x78; // uniform scale

    // bbox min / sphere center / bbox max. WARNING: at spawn all three are set equal to the position (a
    // degenerate point), never the model's real extents and never recomputed. Not usable as a real box;
    // a real wireframe must transform the CM2Model local bounds by the live instance matrix.
    constexpr size_t kBBoxMinX = 0x38;
    constexpr size_t kBBoxMinY = 0x3C;
    constexpr size_t kBBoxMinZ = 0x40;
    constexpr size_t kCenterX  = 0x48;
    constexpr size_t kCenterY  = 0x4C;
    constexpr size_t kCenterZ  = 0x50;
    constexpr size_t kBBoxMaxX = 0x54;
    constexpr size_t kBBoxMaxY = 0x58;
    constexpr size_t kBBoxMaxZ = 0x5C;

    // Staging matrix (float[16], row-major) composed once at spawn. NOT what the renderer reads: the spawn
    // copies it once into the CM2 render instance (kInstWorldMatrix), then never touches it again. Writing
    // here does NOT move the model; kept only as the editor-facing source of truth.
    constexpr size_t kWorldMatrix      = 0xD8;
    constexpr size_t kWorldMatrixTransX = 0x108;
    constexpr size_t kWorldMatrixTransY = 0x10C;
    constexpr size_t kWorldMatrixTransZ = 0x110;

    // --- CM2 render instance (the object the renderer actually draws) ---
    // doodad+0x34 -> CM2 instance. The live world matrix the renderer multiplies every frame lives on the
    // instance, not the doodad. To move/rotate/scale a placed doodad you write kInstWorldMatrix.
    constexpr size_t kInstance        = 0x34; // doodad -> CM2 render instance (0 mid async-load)
    constexpr size_t kInstWorldMatrix = 0xB4; // float[16] row-major, READ every frame, never rewritten
    constexpr size_t kInstTransX      = 0xE4; // translation row of the live matrix (= world X/Y/Z)
    constexpr size_t kInstTransY      = 0xE8;
    constexpr size_t kInstTransZ      = 0xEC;
    constexpr size_t kInstModel       = 0x2C; // instance -> CM2Model cache node

    // --- CM2Model cache node (holds the file path + the parsed MD20 header) ---
    constexpr size_t kModelFullPath = 0x3C;  // inline NUL-terminated normalized path (take address)
    constexpr size_t kModelFileName = 0x140; // char* to the bare filename (points into the +0x3C buffer)
    constexpr size_t kModelHeader   = 0x150; // ptr to the parsed MD20 header blob (the local-bounds source)

    // --- MD20 header (H = *(model+0x150)): model-LOCAL bounding box ---
    // Transform these by the instance matrix (kInstWorldMatrix) to get the real world box of a placement.
    constexpr size_t kHdrBBoxMinX = 0xA0; // C3Vector local AABB min
    constexpr size_t kHdrBBoxMinY = 0xA4;
    constexpr size_t kHdrBBoxMinZ = 0xA8;
    constexpr size_t kHdrBBoxMaxX = 0xAC; // C3Vector local AABB max
    constexpr size_t kHdrBBoxMaxY = 0xB0;
    constexpr size_t kHdrBBoxMaxZ = 0xB4;

    // --- per-chunk doodad list ---
    constexpr size_t kChunkDoodadLinkOff = 0xC4;
    constexpr size_t kChunkDoodadHead    = 0xCC;
    constexpr size_t kNodeDoodad         = 0x04;
}
