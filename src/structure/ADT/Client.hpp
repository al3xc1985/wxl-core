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

// Client contract: the 335 target ADT layout the native loader consumes. The target reads ONE monolithic
// tile (MVER|MHDR|MCIN[256]|map chunks|MCNK[256]); modern tiles are split into root + _tex0 + _obj0 and
// merged into this shape. These are the chunk tags, the MHDR/MCIN/MCNK field maps, and the per-MCNK
// sub-chunk layout. Source.hpp declares the modern additions and field deltas.
namespace wraith::structure::adt
{
    // FourCC packed big-endian to match the stored chunk magics.
    constexpr uint32_t CC(char a, char b, char c, char d)
    {
        return (uint32_t(uint8_t(a)) << 24) | (uint32_t(uint8_t(b)) << 16) |
               (uint32_t(uint8_t(c)) << 8) | uint32_t(uint8_t(d));
    }

    // Root chunks the merged tile consumes, in roughly emitted order.
    constexpr uint32_t MVER = CC('M','V','E','R'); // version
    constexpr uint32_t MHDR = CC('M','H','D','R'); // header (offset directory)
    constexpr uint32_t MCIN = CC('M','C','I','N'); // 256 MCNK index entries
    constexpr uint32_t MTEX = CC('M','T','E','X'); // texture name blob
    constexpr uint32_t MTXF = CC('M','T','X','F'); // texture flags
    constexpr uint32_t MMDX = CC('M','M','D','X'); // doodad model names
    constexpr uint32_t MMID = CC('M','M','I','D'); // doodad name offsets
    constexpr uint32_t MWMO = CC('M','W','M','O'); // map object names
    constexpr uint32_t MWID = CC('M','W','I','D'); // map object name offsets
    constexpr uint32_t MDDF = CC('M','D','D','F'); // doodad placements
    constexpr uint32_t MODF = CC('M','O','D','F'); // map object placements
    constexpr uint32_t MFBO = CC('M','F','B','O'); // flight box
    constexpr uint32_t MH2O = CC('M','H','2','O'); // liquid
    constexpr uint32_t MCNK = CC('M','C','N','K'); // terrain chunk

    // MCNK sub-chunks.
    constexpr uint32_t MCVT = CC('M','C','V','T'); // height map
    constexpr uint32_t MCCV = CC('M','C','C','V'); // vertex colors
    constexpr uint32_t MCNR = CC('M','C','N','R'); // normals
    constexpr uint32_t MCLY = CC('M','C','L','Y'); // texture layers
    constexpr uint32_t MCRF = CC('M','C','R','F'); // doodad + object refs (merged)
    constexpr uint32_t MCSH = CC('M','C','S','H'); // shadow map
    constexpr uint32_t MCAL = CC('M','C','A','L'); // alpha maps
    constexpr uint32_t MCSE = CC('M','C','S','E'); // sound emitters

    // Tile grid is 256 (16x16) MCNK chunks.
    constexpr uint32_t kMcnkCount = 256;

    // MHDR offset directory (relative to MHDR.data), 0x40 bytes total.
    constexpr uint32_t kMhdrSize       = 0x40;
    constexpr uint32_t kMhdrFlagsOff   = 0x00; // flags (bit 0x1 = MFBO present)
    constexpr uint32_t kMhdrMcinOff    = 0x04;
    constexpr uint32_t kMhdrMtexOff    = 0x08;
    constexpr uint32_t kMhdrMmdxOff    = 0x0C;
    constexpr uint32_t kMhdrMmidOff    = 0x10;
    constexpr uint32_t kMhdrMwmoOff    = 0x14;
    constexpr uint32_t kMhdrMwidOff    = 0x18;
    constexpr uint32_t kMhdrMddfOff    = 0x1C;
    constexpr uint32_t kMhdrModfOff    = 0x20;
    constexpr uint32_t kMhdrMfboOff    = 0x24;
    constexpr uint32_t kMhdrMh2oOff    = 0x28;
    constexpr uint32_t kMhdrMtxfOff    = 0x2C;

    // MCIN entry: 256 entries of 0x10 (offset, size, flags, asyncId).
    constexpr uint32_t kMcinEntryStride = 0x10;
    constexpr uint32_t kMcinOffsetOff   = 0x0;
    constexpr uint32_t kMcinSizeOff     = 0x4;

    // MCNK fixed header (0x80) field map.
    constexpr uint32_t kMcnkHeaderSize   = 0x80;
    constexpr uint32_t kMcnkFlagsOff     = 0x00; // flags (low 16 bits kept; bit 0x10000 = hi-res holes)
    constexpr uint32_t kMcnkIndexXOff    = 0x04;
    constexpr uint32_t kMcnkIndexYOff    = 0x08;
    constexpr uint32_t kMcnkNLayersOff   = 0x0C;
    constexpr uint32_t kMcnkNDoodadOff   = 0x10;
    constexpr uint32_t kMcnkOfsMcvtOff   = 0x14;
    constexpr uint32_t kMcnkOfsMcnrOff   = 0x18;
    constexpr uint32_t kMcnkOfsMclyOff   = 0x1C;
    constexpr uint32_t kMcnkOfsMcrfOff   = 0x20;
    constexpr uint32_t kMcnkOfsMcalOff   = 0x24;
    constexpr uint32_t kMcnkSizeMcalOff  = 0x28;
    constexpr uint32_t kMcnkOfsMcshOff   = 0x2C;
    constexpr uint32_t kMcnkSizeMcshOff  = 0x30;
    constexpr uint32_t kMcnkAreaIdOff    = 0x34;
    constexpr uint32_t kMcnkNObjRefsOff  = 0x38;
    constexpr uint32_t kMcnkHolesLoOff   = 0x3C; // 16-bit lo-res hole mask (4x4)
    constexpr uint32_t kMcnkOfsMcseOff   = 0x58;
    constexpr uint32_t kMcnkNSndEmitOff  = 0x5C;
    constexpr uint32_t kMcnkOfsMccvOff   = 0x74;
    constexpr uint32_t kMcnkHolesHiLoOff = 0x78; // hi-res 8x8 hole mask low dword
    constexpr uint32_t kMcnkHolesHiHiOff = 0x7C; // hi-res 8x8 hole mask high dword

    // MCNR normals payload: 448 bytes emitted, size field declared as 435.
    constexpr uint32_t kMcnrPayloadBytes = 448;
    constexpr uint32_t kMcnrSizeField    = 435;

    // MCLY layer record: 0x10 stride, 4 fields. Target caps to 4 layers.
    constexpr uint32_t kMclyStride       = 0x10;
    constexpr uint32_t kMclyTextureIdOff = 0x00;
    constexpr uint32_t kMclyFlagsOff     = 0x04;
    constexpr uint32_t kMclyOfsAlphaOff  = 0x08;
    constexpr uint32_t kMclyGroundOff    = 0x0C; // GroundEffectTexture id
    constexpr uint32_t kMclyMaxLayers    = 4;

    // MCSE sound emitter record stride.
    constexpr uint32_t kMcseStride = 0x1C;

    // MDDF placement record (0x24) flag word offset.
    constexpr uint32_t kMddfStride     = 36;
    constexpr uint32_t kMddfFlagsOff   = 0x22;

    // MODF placement record (0x40) flag word offset.
    constexpr uint32_t kModfStride     = 64;
    constexpr uint32_t kModfFlagsOff   = 0x38;

    struct Client
    {
        static constexpr uint32_t kSourceVersion = 335;

        // ADT version value in MVER.
        static constexpr uint32_t kMverValue = 18;
    };
}
