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

#include "Client.hpp"

// Source contract: what the modern WMO ADDS or encodes differently relative to the target shape in
// Client.hpp. Shared tags (MVER, kept root/group chunks) live in Client.hpp; this header declares only
// the modern-only chunks the transform strips, the field deltas it normalizes, and the modern flag bits
// it clears. The transform is data-gated (keyed off chunk presence / field values), not versioned.
namespace wraith::structure::wmo
{
    // Modern-only root chunks the target loader has no slot for; stripped before the positional walk.
    constexpr uint32_t kGFID = FourCC('G', 'F', 'I', 'D'); // group FileDataIDs
    constexpr uint32_t kMOUV = FourCC('M', 'O', 'U', 'V'); // animated UV translations
    constexpr uint32_t kMODI = FourCC('M', 'O', 'D', 'I'); // doodad FileDataIDs
    constexpr uint32_t kMOSI = FourCC('M', 'O', 'S', 'I'); // skybox FileDataID

    // Modern shader id range above the target's 0..6; collapsed to the nearest family.
    constexpr uint32_t kModernShaderMin = 7;
    constexpr uint32_t kModernShaderMax = 22;

    // Modern shader id (0..22) -> nearest target 0..6 family. Index past the table degrades to 0.
    constexpr uint8_t kShaderRemap[23] = {
        0, 1, 2, 3, 4, 5, 6, // identity for the target range
        5,                   // 7  TwoLayerEnvMetal     -> EnvMetal
        6,                   // 8  TwoLayerTerrain      -> TwoLayerDiffuse
        0,                   // 9  DiffuseEmissive      -> Diffuse
        0, 0, 0, 0, 0,       // 10..14 unused           -> Diffuse
        6,                   // 15 TwoLayerDiffuseEmiss -> TwoLayerDiffuse
        0,                   // 16 Diffuse alias        -> Diffuse
        5,                   // 17 AdditiveMaskedEnvMet -> EnvMetal
        6,                   // 18 TwoLayerDiffuseMod2x -> TwoLayerDiffuse
        6,                   // 19 ..Mod2xNA            -> TwoLayerDiffuse
        6,                   // 20 ..DiffuseAlpha       -> TwoLayerDiffuse
        4,                   // 21 Lod impostor         -> Opaque
        0,                   // 22 Parallax             -> Diffuse
    };

    // Placeholder name written when a texture FileDataID does not resolve, keeping the material's MOTX
    // offset pointed at a valid NUL-terminated string.
    constexpr const char kFallbackTexture[] = "createcrappygreentexture.blp";

    // MOBA modern material-id relocation. When the flag byte at +0x16 has bit 0x2 set, the real material
    // id sits at +0x0A and the leading bytes overlap what the target reads as the i16 bounding box. The
    // transform moves the id to the target offset, clears the flag, and rebuilds the bounding box.
    constexpr uint32_t kMobaModernMatOff = 0x0A; // u8 modern material id
    constexpr uint32_t kMobaFlagOffset   = 0x16; // u8 relocation flag
    constexpr uint8_t  kMobaRelocFlag    = 0x02;

    // Every chunk-gating group flag bit plus the modern-only high bits, cleared before recomputing flags.
    constexpr uint32_t kGrpFlagClear =
        0x1u | 0x2u | 0x4u | 0x200u | 0x400u | 0x800u | 0x1000u | 0x20000u |
        0x1000000u | 0x2000000u | 0x4000000u | 0x8000000u | 0x10000000u | 0x20000000u | 0x40000000u | 0x80000000u;

    struct Source {};
}
