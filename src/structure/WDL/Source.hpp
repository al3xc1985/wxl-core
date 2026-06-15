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

#include "Client.hpp"

// Source contract: what the modern WDL ADDS relative to the target shape in Client.hpp. The modern map
// carries extra chunks the target has no slot for; the rebuild keeps MVER/MAOF/MARE/MAHO and drops these.
// Shared tags live in Client.hpp.
namespace wraith::structure::wdl
{
    // Modern-only chunk dropped by the rebuild. It follows a MARE within a tile and is skipped while
    // scanning for that tile's MAHO.
    constexpr uint32_t MAOE = CC('M','A','O','E'); // per-tile outer edge data

    struct Source {};
}
