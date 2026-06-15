// Map data table: modern map set served over the stock map storage.
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

#include "DB2File.hpp"
#include "DB2Mgr.hpp"
#include "game/DB2.hpp" // offsets

#include <cstddef>

// Map.db2 (modern layout, WDC1, 21 fields, separate id list, fixed 64-byte records). Field order/widths
// taken from the file's field structure: 6 string columns, Flags[2] u32, MinimapIconScale float,
// Corpse[2] float, 7 shorts, 5 bytes. The decoder emits [id u32][fields in file order at native widths]
// with natural C alignment, so the row below mirrors the decoded record exactly (decoded size 68 bytes,
// asserted on load).
namespace wraith::features::db2::defs
{
    struct MapRow
    {
        uint32_t id;                  // +0x00  separate id list (FileDataID-independent map id)
        int32_t  directory;           // +0x04  string: world\maps\<Directory>
        int32_t  mapName;             // +0x08  string: displayed map name
        int32_t  mapDescription0;     // +0x0C  string: Horde description (battlegrounds)
        int32_t  mapDescription1;     // +0x10  string: Alliance description
        int32_t  pvpShortDescription; // +0x14  string: pvp objective (short)
        int32_t  pvpLongDescription;  // +0x18  string: pvp objective (long)
        uint32_t flags[2];            // +0x1C  map flags
        float    minimapIconScale;    // +0x24
        float    corpse[2];           // +0x28  ghost-mode entrance X/Y
        uint16_t areaTableID;         // +0x30
        int16_t  loadingScreenID;     // +0x32
        int16_t  corpseMapID;         // +0x34
        int16_t  timeOfDayOverride;   // +0x36
        int16_t  parentMapID;         // +0x38
        int16_t  cosmeticParentMapID; // +0x3A
        int16_t  windSettingsID;      // +0x3C
        uint8_t  instanceType;        // +0x3E  0 none, 1 party, 2 raid, 3 pvp, 4 arena
        uint8_t  mapType;             // +0x3F
        uint8_t  expansionID;         // +0x40
        uint8_t  maxPlayers;          // +0x41
        uint8_t  timeOffset;          // +0x42
    };

    static_assert(sizeof(MapRow) == 68, "MapRow must match the decoded Map.db2 record size");
    static_assert(offsetof(MapRow, flags) == 0x1C, "MapRow.flags offset");
    static_assert(offsetof(MapRow, areaTableID) == 0x30, "MapRow.areaTableID offset");
    static_assert(offsetof(MapRow, instanceType) == 0x3E, "MapRow.instanceType offset");

    class Map : public DB2Table<MapRow>
    {
    public:
        static Map& Get()
        {
            static Map instance;
            if (!instance.IsLoaded())
            {
                instance.Load("Map.db2");
                Track(&instance, "Map.db2");
            }
            return instance;
        }

        // world\maps\<Directory> name for a map id, or nullptr if absent.
        const char* Directory(int32_t mapId)
        {
            const auto* r = Find(mapId);
            return r ? Str(static_cast<uint32_t>(r->directory)) : nullptr;
        }

        // Displayed map name, or nullptr if absent.
        const char* Name(int32_t mapId)
        {
            const auto* r = Find(mapId);
            return r ? Str(static_cast<uint32_t>(r->mapName)) : nullptr;
        }

        // Stock map-storage override target.
        // The on-disk stock row is compacted by the engine into a 0x48-byte (18-dword) in-memory record
        // (localized strings collapsed to one locale-selected char*); the override rewrites that
        // in-memory recordData + idTable, not the on-disk image. Map has no consumer cache: every
        // consumer reads idTable[id-minId] inline, so rewriting the storage fields + idTable (and
        // minId/maxId) is sufficient.
        const NativeOverride* Override() const override
        {
            namespace off = wraith::offsets::game::db2::mapdef;
            static const NativeOverride o {
                "Map.dbc",
                off::kStorageObject,
                off::kRecordCount,
                off::kRecordData,
                66,           // native columns (on-disk)
                0x108,        // native row size (on-disk)
                off::kLoader,
                off::kPostLoadBuild,
                off::kIdTable,
            };
            return &o;
        }

        // Map id whose Directory matches (case-insensitive), or -1 if none.
        int32_t IdByDirectory(const char* dir)
        {
            if (!dir) return -1;
            for (uint32_t i = 0; i < RowCount(); ++i)
            {
                const auto* r = At(i);
                if (r && IEquals(Str(static_cast<uint32_t>(r->directory)), dir))
                    return static_cast<int32_t>(r->id);
            }
            return -1;
        }

    private:
        static char Lower(char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c; }

        static bool IEquals(const char* a, const char* b)
        {
            if (!a || !b) return false;
            for (; *a && *b; ++a, ++b)
                if (Lower(*a) != Lower(*b)) return false;
            return *a == *b;
        }
    };
}
