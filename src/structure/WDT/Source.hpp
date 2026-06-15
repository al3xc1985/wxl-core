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

// Source contract: what the modern WDT encodes differently relative to the target shape in Client.hpp.
// The modern tile index sets MPHD and MAIN-entry bits the target reader does not mask; this header
// declares the modern bit normalizations. Shared tags live in Client.hpp.
namespace wraith::structure::wdt
{
    // Modern MPHD flag bit: height-texturing. When set it is folded into the target big-alpha bit before
    // the flag word is masked down to kMphdFlagMask.
    constexpr uint32_t kMphdFlagHeightTexturing = 0x80;

    // The modern MAIN entry carries extra runtime/modern bits beyond has-adt; all but kMainHasAdt are
    // cleared (mask the entry flag byte to kMainHasAdt).
    constexpr uint8_t kMainEntryKeep = kMainHasAdt;

    struct Source {};
}
