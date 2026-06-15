// CharSections data table: appearance section texture references per race/sex.
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

namespace wraith::features::db2::defs
{
    struct CharSectionsRow
    {
        uint32_t id;
        uint32_t textureName[3];   // material/texture file references
        uint16_t flags;
        uint8_t  raceID;
        uint8_t  sexID;
        uint8_t  baseSection;
        uint8_t  variationIndex;
        uint8_t  colorIndex;
    };

    class CharSections : public DB2Table<CharSectionsRow>
    {
    public:
        static CharSections& Get()
        {
            static CharSections instance;
            if (!instance.IsLoaded())
            {
                instance.Load("CharSections.db2");
                Track(&instance, "CharSections.db2");
            }
            return instance;
        }

        // Stock-table override target. Native record = 10 columns, 0x28 bytes. The render path reads
        // rows through the consumer accessor (not the storage object directly), so the override targets
        // the accessor.
        const NativeOverride* Override() const override
        {
            namespace off = wraith::offsets::game::db2::charsectionsdef;
            static const NativeOverride o {
                "CharSections.dbc",
                off::kStorageObject,
                off::kRecordCount,
                off::kRecordData,
                10,           // native columns
                0x28,         // native row size
                off::kRecordLookup,
                off::kCacheBuilder,
                off::kCacheRoot,
            };
            return &o;
        }
    };
}
