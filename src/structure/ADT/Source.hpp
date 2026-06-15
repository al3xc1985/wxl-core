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

// Source contract: what the modern ADT ADDS or encodes differently relative to the target shape in
// Client.hpp. The modern tile is split across three sibling files; this header declares the sibling
// naming, the modern-only sub-chunks the merge consumes from the split files, and the field deltas the
// merge normalizes (placement flag masks, MCLY ground range). Shared tags live in Client.hpp.
namespace wraith::structure::adt
{
    // Split-file sibling suffixes the modern tile is read from (root has no suffix).
    constexpr const char kSiblingTex0[] = "_tex0";
    constexpr const char kSiblingObj0[] = "_obj0";

    // Texture-flag source chunks present in the _tex0 sibling.
    constexpr uint32_t MTXP = CC('M','T','X','P'); // modern texture params (MTXF derived from these)

    // Modern per-MCNK ref chunks in the _obj0 sibling; merged into the target MCRF.
    constexpr uint32_t MCRD = CC('M','C','R','D'); // doodad refs
    constexpr uint32_t MCRW = CC('M','C','R','W'); // map object refs

    // MTXP record stride (one entry per texture).
    constexpr uint32_t kMtxpStride = 0x10;

    // MDDF placement flag bits the target understands; modern high bits are masked off.
    constexpr uint16_t kMddfFlagMask = 0x7; // 0x1 | 0x2 | 0x4

    // MODF placement flag bits cleared (modern), plus the modern scale field zeroed (target padding).
    constexpr uint16_t kModfFlagClear = 0xC;  // clear 0x4 | 0x8
    constexpr uint32_t kModfScaleOff  = 0x3E; // modern scale; zeroed (padding in the target)

    // MCLY layer flags the target understands; modern high bits are masked off.
    constexpr uint32_t kMclyFlagMask = 0x7FF;

    // GroundEffectTexture ids above this are out of the target range and clamped to 0.
    constexpr uint32_t kGroundMax = 73186;

    struct Source {};
}
