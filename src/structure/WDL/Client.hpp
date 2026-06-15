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

// Client contract: the 335 target WDL (low-detail world map) chunk set the native loader consumes:
// MVER, the MAOF tile offset directory, per-tile MARE height blocks, and optional MAHO holes. The MAOF
// directory has 4096 entries (64x64 tiles), each a byte offset to that tile's MARE. Source.hpp declares
// the modern chunks dropped to reach this shape.
namespace wraith::structure::wdl
{
    // FourCC packed big-endian to match the stored chunk magics.
    constexpr uint32_t CC(char a, char b, char c, char d)
    {
        return (uint32_t(uint8_t(a)) << 24) | (uint32_t(uint8_t(b)) << 16) |
               (uint32_t(uint8_t(c)) << 8) | uint32_t(uint8_t(d));
    }
    constexpr uint32_t MVER = CC('M','V','E','R'); // version
    constexpr uint32_t MAOF = CC('M','A','O','F'); // tile offset directory (4096 entries)
    constexpr uint32_t MARE = CC('M','A','R','E'); // per-tile height block
    constexpr uint32_t MAHO = CC('M','A','H','O'); // per-tile holes

    // MAOF directory dimensions.
    constexpr uint32_t kMaofTileCount   = 4096;          // 64x64 tiles
    constexpr uint32_t kMaofEntryStride = 4;             // u32 byte offset per tile
    constexpr uint32_t kMaofBytes       = 4096u * 4u;    // full directory size

    struct Client
    {
        static constexpr uint32_t kSourceVersion = 335;

        // WDL version value in MVER.
        static constexpr uint32_t kMverValue = 18;
    };
}
