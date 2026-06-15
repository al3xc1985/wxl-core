// Data-table subsystem: lazy table registry (unload/reload in bulk).
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

#include "DB2Mgr.hpp"

#include "Logger.hpp"

namespace wraith::features::db2
{
    namespace
    {
        struct Entry { DB2File* table; const char* name; };
        std::vector<Entry> g_tables;
    }

    void Install()
    {
        // Tables load lazily via the Definition singletons; this only marks the subsystem ready.
        WLOG_INFO("DB2: subsystem ready");
    }

    void Track(DB2File* table, const char* fileName)
    {
        for (const auto& e : g_tables) if (e.table == table) return;
        g_tables.push_back({ table, fileName });
    }

    void UnloadAll()
    {
        for (auto& e : g_tables) e.table->Unload();
    }

    void ReloadAll()
    {
        for (auto& e : g_tables)
        {
            e.table->Unload();
            e.table->Load(e.name);
        }
    }
}
