// M2 ribbon runtime: bind 3 textures s0/s1/s2, one-pass MODULATE combine via live D3D9.
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

// DLL-only RUNTIME feature. Touches live engine objects / the GPU. Never moves to host.
namespace wraith::runtime::ribbon
{
    // Register this feature's hooks. Called once at bootstrap (cold path).
    void Install();

}
