// Map override: merge the modern Map.db2 into the stock in-memory map storage in place.
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

#include "MapOverride.hpp"

#include "Definitions/Map.hpp"
#include "game/DB2.hpp" // offsets
#include "Logger.hpp"

#include <windows.h>
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

using namespace wraith;
namespace mapoff = wraith::offsets::game::db2::map;

namespace
{
    // In-memory compacted map record (0x48 bytes, 18 dwords) the engine builds from the on-disk row.
    // All members are 4 bytes in the 32-bit client.
#pragma pack(push, 1)
    struct InMemoryMapRecord
    {
        uint32_t    id;                 // +0x00
        const char* directory;          // +0x04
        uint32_t    instanceType;       // +0x08
        uint32_t    flags;              // +0x0c
        uint32_t    mapNameMask;        // +0x10  localized-name presence mask
        const char* mapName;            // +0x14
        uint32_t    areaTableID;        // +0x18
        const char* mapDescription0;    // +0x1c
        const char* mapDescription1;    // +0x20
        int32_t     loadingScreenID;    // +0x24
        float       minimapIconScale;   // +0x28
        int32_t     corpseMapID;        // +0x2c
        float       corpseX;            // +0x30
        float       corpseY;            // +0x34
        int32_t     timeOfDayOverride;  // +0x38
        uint32_t    expansionID;        // +0x3c
        uint32_t    raidOffset;         // +0x40
        uint32_t    maxPlayers;         // +0x44
    };
#pragma pack(pop)
    static_assert(sizeof(InMemoryMapRecord) == 0x48, "InMemoryMapRecord must be the 0x48 in-memory layout");

    template <typename T> T  Read(uintptr_t a)        { return *reinterpret_cast<T*>(a); }
    template <typename T> void Write(uintptr_t a, T v) { *reinterpret_cast<T*>(a) = v; }

    // Persistent backing for the rebuilt storage. The engine reads these for the process lifetime, so
    // they must outlive the rebuild; static + never resized after commit keeps every pointer stable.
    std::deque<std::string>        g_strPool;  // stable element addresses for our own strings
    std::vector<InMemoryMapRecord> g_records;
    std::vector<InMemoryMapRecord*> g_idTable;
    bool                           g_rebuilt = false;

    const char* Persist(const char* s)
    {
        g_strPool.emplace_back(s ? s : "");
        return g_strPool.back().c_str();
    }

    // Build a compact record from one Map.db2 row. Strings are copied into g_strPool (own lifetime).
    InMemoryMapRecord FromDb2(features::db2::defs::Map& db, const features::db2::defs::MapRow& r, uint32_t nameMask)
    {
        InMemoryMapRecord o{};
        o.id                = r.id;
        o.directory         = Persist(db.Str(static_cast<uint32_t>(r.directory)));
        o.instanceType      = r.instanceType;
        o.flags             = r.flags[0];                 // split flags in two dwords; low dword = classic flags
        o.mapNameMask       = nameMask;
        o.mapName           = Persist(db.Str(static_cast<uint32_t>(r.mapName)));
        o.areaTableID       = r.areaTableID;
        o.mapDescription0   = Persist(db.Str(static_cast<uint32_t>(r.mapDescription0)));
        o.mapDescription1   = Persist(db.Str(static_cast<uint32_t>(r.mapDescription1)));
        o.loadingScreenID   = r.loadingScreenID;
        o.minimapIconScale  = r.minimapIconScale;
        o.corpseMapID       = r.corpseMapID;
        o.corpseX           = r.corpse[0];
        o.corpseY           = r.corpse[1];
        o.timeOfDayOverride = r.timeOfDayOverride;
        o.expansionID       = r.expansionID;
        o.raidOffset        = 0;                          // Map.db2 carries no RaidOffset
        o.maxPlayers        = r.maxPlayers;
        return o;
    }

    // Merge stock map rows with the modern Map.db2 rows (modern wins on id) and rewrite the in-memory
    // storage. Runs once. Leaves the stock storage untouched on any inconsistency.
    void RebuildMapStorage()
    {
        if (g_rebuilt) return;
        if (Read<uint32_t>(mapoff::kStorageLoadedFlag) == 0) return; // map storage not built yet

        auto* stockData    = Read<InMemoryMapRecord*>(mapoff::kRecordData);
        auto* stockIdTable = Read<InMemoryMapRecord**>(mapoff::kIdTable);
        const int32_t stockMin = Read<int32_t>(mapoff::kMinId);
        const int32_t stockMax = Read<int32_t>(mapoff::kMaxId);
        if (!stockData || !stockIdTable || stockMax < stockMin) return;

        auto& db = features::db2::defs::Map::Get();
        if (!db.IsLoaded() || db.RowCount() == 0)
        {
            WLOG_WARN("Map override: Map.db2 not available, leaving stock map storage");
            return;
        }

        // Reuse the stock localized-name mask convention from the first stock record.
        uint32_t nameMask = 0;
        for (int32_t id = stockMin; id <= stockMax; ++id)
            if (auto* sp = stockIdTable[id - stockMin]) { nameMask = sp->mapNameMask; break; }

        const int32_t newMin = (db.MinId() < stockMin) ? db.MinId() : stockMin;
        const int32_t newMax = (db.MaxId() > stockMax) ? db.MaxId() : stockMax;
        const uint32_t span = static_cast<uint32_t>(newMax - newMin + 1);

        g_strPool.clear();
        g_records.clear();
        g_records.reserve(span); // upper bound; reserved once so push_back never reallocates after commit

        uint32_t fromDb2 = 0, fromStock = 0;
        for (int32_t id = newMin; id <= newMax; ++id)
        {
            if (const auto* our = db.Find(id))
            {
                g_records.push_back(FromDb2(db, *our, nameMask));
                ++fromDb2;
            }
            else if (id >= stockMin && id <= stockMax)
            {
                if (auto* sp = stockIdTable[id - stockMin]) // copy stock record (its string ptrs stay valid)
                {
                    g_records.push_back(*sp);
                    ++fromStock;
                }
            }
        }

        // Index by id only after g_records is final (addresses stable: reserved, no further growth).
        g_idTable.assign(span, nullptr);
        for (auto& rec : g_records)
            g_idTable[rec.id - newMin] = &rec;

        Write<uint32_t>(mapoff::kRecordCount, static_cast<uint32_t>(g_records.size()));
        Write<InMemoryMapRecord*>(mapoff::kRecordData, g_records.data());
        Write<int32_t>(mapoff::kMinId, newMin);
        Write<int32_t>(mapoff::kMaxId, newMax);
        Write<InMemoryMapRecord**>(mapoff::kIdTable, g_idTable.data());

        g_rebuilt = true;
        WLOG_INFO("Map override: served %u maps (id %d..%d) = %u from Map.db2 + %u from stock",
                  static_cast<uint32_t>(g_records.size()), newMin, newMax, fromDb2, fromStock);
    }

    // Wait for the engine to finish building the map storage, then rebuild once. The loader sets the
    // loaded flag only AFTER it has built the records + idTable, so observing the flag means the storage
    // is ready; on x86 (no load-load reorder) reading the flag set guarantees we then see the finished
    // idTable.
    DWORD WINAPI MapWatcher(LPVOID)
    {
        constexpr int kPollMs = 100;
        constexpr int kMaxWaitMs = 60000;
        for (int waited = 0; waited < kMaxWaitMs && Read<uint32_t>(mapoff::kStorageLoadedFlag) == 0; waited += kPollMs)
            Sleep(kPollMs);

        if (Read<uint32_t>(mapoff::kStorageLoadedFlag) == 0)
        {
            WLOG_WARN("Map override: map storage not built within timeout, stock kept");
            return 0;
        }

        __try { RebuildMapStorage(); }
        __except (EXCEPTION_EXECUTE_HANDLER) { WLOG_WARN("Map override: rebuild faulted, stock map storage kept"); }
        return 0;
    }
}

namespace wraith::features::db2
{
    void InstallMapOverride()
    {
        // No accessor funnel exists for the map storage; the override rewrites it in place once the
        // stock build completes. A watcher thread avoids hooking the loader (unknown arity).
        CloseHandle(CreateThread(nullptr, 0, &MapWatcher, nullptr, 0, nullptr));
        WLOG_INFO("DB2: Map override watcher started");
    }
}
