// Map override: serves the modern Map.db2 over the stock in-memory map storage.
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

// Serves the modern Map.db2 over the stock map storage: after the engine builds its in-memory map
// storage (recordData + idTable + min/max), the storage is rebuilt from a merge of the stock rows and
// the modern Map.db2 rows (modern rows win on id collision), so every map-id consumer sees the modern
// map set. Client-only.
namespace wraith::features::db2
{
    void InstallMapOverride();
}
