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

// Client contract: the 335 target WDT (tile index) layout the native loader consumes. The target reads
// the full MPHD flag word and each MAIN entry flag byte WITHOUT masking, so it depends on the exact bit
// layout below. Source.hpp declares the modern high bits cleared to reach this shape.
namespace wraith::structure::wdt
{
    // FourCC packed big-endian to match the stored chunk magics.
    constexpr uint32_t CC(char a, char b, char c, char d)
    {
        return (uint32_t(uint8_t(a)) << 24) | (uint32_t(uint8_t(b)) << 16) |
               (uint32_t(uint8_t(c)) << 8) | uint32_t(uint8_t(d));
    }
    constexpr uint32_t MVER = CC('M','V','E','R'); // version

    // MPHD flags word and the MAIN tile grid the target consumes.
    constexpr uint32_t kFlagsOffset = 0x14;           // MPHD flags word
    constexpr uint32_t kMainOffset  = 0x3C;           // first MAIN entry
    constexpr uint32_t kMainBytes   = 64u * 64u * 8u; // 64x64 entries, 8 bytes each
    constexpr uint32_t kMainEntryStride = 8;

    // MPHD flag bits the target reader understands (low 5 bits).
    constexpr uint32_t kMphdFlagMask    = 0x1F;
    constexpr uint32_t kMphdFlagBigAlpha = 0x4; // big-alpha terrain

    // MAIN entry flag bit the target keeps (has-adt); all other entry bits are cleared.
    constexpr uint8_t kMainHasAdt = 0x1;

    struct Client
    {
        static constexpr uint32_t kSourceVersion = 335;

        // WDT version value in MVER.
        static constexpr uint32_t kMverValue = 18;

        static constexpr uint32_t kMagicMPHD = 0x4D504844; // 'MPHD'
    };
}
