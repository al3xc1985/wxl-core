// Data-table subsystem entry point. Tables load lazily on first access; this manager tracks the live
// tables so they can be unloaded/reloaded in bulk.
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

#include <vector>

// Data-table subsystem entry point. Tables load lazily on first access (so they do not depend on when
// the archives are mounted); this manager only tracks the live tables so they can be unloaded/reloaded
// in bulk.
namespace wraith::features::db2
{
    void Install();

    // Register a table so UnloadAll/ReloadAll reach it. A Definition singleton calls this once.
    void Track(DB2File* table, const char* fileName);

    void UnloadAll();
    void ReloadAll();
}
