// Terrain tile/chunk lookups, the tile-slot grid, and runtime in-memory chunk field offsets.
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

// INTERNAL to the core. Terrain tile/chunk lookups, the tile-slot grid, and runtime in-memory chunk
// field offsets. Modules never include this; they use wxl::game / wxl::events.
namespace wxl::offsets::game::adt
{
    // --- lookups ---
    // Chunk lookup (pos) -> runtime chunk object, or null when that chunk is not parsed yet. A non-null
    // result means the chunk heightmap + collision are resident.
    constexpr uintptr_t kGetChunk = 0x007B49C0;
    // CMapChunk::Build (this=CMapChunk in ECX): turns one raw MCNK into a live chunk (sub-chunk pointers,
    // bbox, texture-layer units, ref spawn). The "a terrain chunk was built" point, distinct from the
    // per-frame terrain draw.
    constexpr uintptr_t kChunkBuild = 0x007C64B0;
    // Near-tile placed-object counter (chunk, &progress, total) -> count of placed-object children still
    // loading that overlap the chunk box.
    constexpr uintptr_t kNearObjectCount = 0x007B50B0;

    // --- tile-slot grid ---
    // Tile-slot grid base: a 64x64 array of tile-area pointers (stride 4). Slot index is X-major
    // (tileX * 64 + tileY).
    constexpr uintptr_t kTileSlots   = 0x00CE48D0;
    constexpr uint32_t  kTileGridDim = 64;   // tiles per axis
    constexpr size_t    kTileSlotStride = 0x04;
    // Detailed/streaming-path selector (u32).
    constexpr uintptr_t kStreamingPathSelector = 0x00CE0494;

    // --- tile-area object fields ---
    constexpr size_t kOffTileAsyncRead = 0x70; // non-zero while the tile root read is in flight
    constexpr size_t kOffTileFileBuffer = 0x80; // non-zero once the tile file buffer is allocated

    // --- runtime chunk object fields ---
    constexpr size_t kOffChunkNodeLayerCount = 0x09; // draw-node layer count
    // CMapChunk -> MCNK 128-byte data header (= raw MCNK ptr + 8-byte tag). The authoritative texture-layer
    // count (SMChunk.nLayers, 0..4) lives at header + 0x0C.
    constexpr size_t kOffChunkMcnkHeader = 0x110;
    constexpr size_t kOffMcnkNLayers     = 0x0C;

    // --- signatures ---
    // Chunk lookup (pos on stack) -> chunk object.
    using Map_GetChunkFn = void*(__cdecl*)(float* pos);
    // Near-object counter (chunk, progressOut, total) -> count.
    using Map_NearObjectCountFn = int(__cdecl*)(void* chunk, int* progressOut, int total);
    // CMapChunk::Build: native this-in-ECX (__thiscall, ret 8). Declared __fastcall with a dummy EDX so the
    // trampoline routes the chunk into the this-register and keeps the two stack args.
    using Map_ChunkBuildFn = void(__fastcall*)(void* chunk, void* edx, void* rawMcnk, int param2);
}
