// Mahjong physics runtime: solve impulses and write the LIVE M2 bone palette.
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

// DLL-only RUNTIME feature. Parses .phys, runs a sequential-impulse solver, writes solved
// transforms into the live bone palette. Ships as "Mahjong"; never the donor name.
namespace wraith::runtime::mahjong
{
    void Install();
}
