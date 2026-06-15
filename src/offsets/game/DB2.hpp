// Data-table runtime offsets: live-image addresses for the map-data override and the data-table
// definitions that replace stock data tables in place.
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

// Data-table runtime offsets. The map-data override rewrites the in-memory map storage built by the
// engine; the definition overrides target the stock storage object + accessor of each replaced table.
namespace wraith::offsets::game::db2
{
    // -------------------------------------------------------------------------
    // Map storage. The engine compacts each on-disk map row into a fixed-size in-memory record and
    // indexes it by (id - minId). The override merges modern map rows into this storage in place.
    // -------------------------------------------------------------------------
    namespace map
    {
        constexpr uintptr_t kStorageLoadedFlag = 0x00AD4164; // u32, set to 1 once the storage is built
        constexpr uintptr_t kRecordCount       = 0x00AD4168; // u32
        constexpr uintptr_t kMaxId             = 0x00AD416C; // i32
        constexpr uintptr_t kMinId             = 0x00AD4170; // i32
        constexpr uintptr_t kRecordData        = 0x00AD417C; // record array base
        constexpr uintptr_t kIdTable           = 0x00AD4180; // record* table, indexed by (id - minId)
    }

    // -------------------------------------------------------------------------
    // Stock data-table definition override targets. A definition declares the storage object + accessor
    // its table replaces; the install reads these. Fields not needed for a table are left 0.
    // -------------------------------------------------------------------------
    namespace mapdef
    {
        constexpr uintptr_t kStorageObject  = 0x00AD4160; // storage instance
        constexpr uintptr_t kRecordCount    = 0x00AD4168; // recordCount field
        constexpr uintptr_t kRecordData     = 0x00AD417C; // recordData pointer field
        constexpr uintptr_t kLoader         = 0x00648510; // on-disk loader (no accessor funnel exists)
        constexpr uintptr_t kPostLoadBuild  = 0x0065E020; // post-load builder (rebuilds min/max + idTable)
        constexpr uintptr_t kIdTable        = 0x00AD4180; // idTable consumers read inline
    }

    namespace charsectionsdef
    {
        constexpr uintptr_t kStorageObject  = 0x00AD332C; // storage instance
        constexpr uintptr_t kRecordCount    = 0x00AD3334; // recordCount field
        constexpr uintptr_t kRecordData     = 0x00AD3348; // recordData pointer field
        constexpr uintptr_t kRecordLookup   = 0x004F3BA0; // consumer accessor (hook point)
        constexpr uintptr_t kCacheBuilder   = 0x004F3DD0; // cache builder
        constexpr uintptr_t kCacheRoot      = 0x00B6B864; // consumer cache root
    }
}
