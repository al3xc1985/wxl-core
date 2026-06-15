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

#include "WdlRebuild.hpp"

// Rebuild a modern low-detail world map (.wdl) into the target client (335) chunk set: keep
// MVER/MAOF/MARE/MAHO, drop the modern ML*/MAOE chunks, and rebuild MAOF so each tile offset points at
// its MARE in the new layout. The 4096-entry MAOF (64x64 tiles) is the heightmap directory.
namespace wraith::structure::wdl
{
    namespace
    {
        constexpr uint32_t CC(char a, char b, char c, char d)
        {
            return (uint32_t(uint8_t(a)) << 24) | (uint32_t(uint8_t(b)) << 16) |
                   (uint32_t(uint8_t(c)) << 8) | uint32_t(uint8_t(d));
        }
        constexpr uint32_t MVER = CC('M','V','E','R');
        constexpr uint32_t MAOF = CC('M','A','O','F'); // tile offset directory (4096 entries)
        constexpr uint32_t MARE = CC('M','A','R','E'); // per-tile height block
        constexpr uint32_t MAHO = CC('M','A','H','O'); // per-tile holes

        uint32_t rd32(const uint8_t* p) { return p[0] | (p[1]<<8) | (p[2]<<16) | (uint32_t(p[3])<<24); }

        // Sequential output builder; offsets back-patched once the layout is known.
        struct Out
        {
            std::vector<uint8_t> d;
            uint32_t tell() const { return uint32_t(d.size()); }
            void u32(uint32_t v) { for (int i=0;i<4;++i) d.push_back(uint8_t(v>>(i*8))); }
            void raw(const uint8_t* p, uint32_t n) { if (n) d.insert(d.end(), p, p+n); }
            void patch32(uint32_t at, uint32_t v) { for (int i=0;i<4;++i) d[at+i]=uint8_t(v>>(i*8)); }
            uint32_t chunk(uint32_t magic, const uint8_t* data, uint32_t n)
            { uint32_t s=tell(); u32(magic); u32(n); if (data) raw(data,n); else d.insert(d.end(), n, 0); return s; }
        };

        bool find(const uint8_t* buf, uint32_t len, uint32_t magic, uint32_t& dataOff, uint32_t& size)
        {
            uint32_t o = 0;
            while (o + 8 <= len)
            {
                uint32_t m = rd32(buf+o), sz = rd32(buf+o+4);
                if (o + 8 + sz > len) break;
                if (m == magic) { dataOff = o+8; size = sz; return true; }
                o += 8 + sz;
            }
            return false;
        }
    }

    bool RebuildWdl(std::span<const uint8_t> in, std::vector<uint8_t>& out)
    {
        const uint8_t* b = in.data();
        const uint32_t n = static_cast<uint32_t>(in.size());
        if (n < 8 || rd32(b) != MVER) return false;
        uint32_t maofOff, maofSz;
        if (!find(b, n, MAOF, maofOff, maofSz) || maofSz < 4096*4) return false;

        Out o;
        { uint8_t v[4]={18,0,0,0}; o.chunk(MVER, v, 4); }
        uint32_t maof = o.chunk(MAOF, nullptr, 4096*4);
        uint32_t maofData = maof + 8;

        for (int i = 0; i < 4096; ++i)
        {
            uint32_t src = rd32(b + maofOff + i*4);
            if (!src || src + 8 > n || rd32(b+src) != MARE) continue;
            uint32_t mareSz = rd32(b + src + 4);
            if (src + 8 + mareSz > n) continue;
            o.patch32(maofData + i*4, o.tell());
            o.chunk(MARE, b + src + 8, mareSz);
            // After MARE: skip MAOE, copy MAHO if it belongs to this tile.
            uint32_t p = src + 8 + mareSz;
            for (int k = 0; k < 2 && p + 8 <= n; ++k)
            {
                uint32_t m = rd32(b+p), sz = rd32(b+p+4);
                if (p + 8 + sz > n || m == MARE) break;
                if (m == MAHO) { o.chunk(MAHO, b+p+8, sz); break; }
                p += 8 + sz; // skip MAOE / unknown
            }
        }

        out = std::move(o.d);
        return true;
    }
}
