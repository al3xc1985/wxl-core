// WDT translator: rewrite modern WDT flags/MAID into the 335 WDT shape.
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
#include <span>
#include <vector>

// Client.hpp / Source.hpp declare this format's two layouts (335 target / modern superset). The
// transform keeps its own local constants for now; consuming the declarations is a later no-op pass.

// HOST-only. WDT translator: rewrite modern WDT flags/MAID into the 335 WDT shape. Pure bytes -> 335 bytes. Free functions, span-based, concrete.
// The transform gates on chunk/field presence. See structure/WMO for the worked example of the contract pattern.
namespace wraith::structure::wdt
{
    // Fix modern WDT flags + strip MAID to produce a 335 WDT.
    bool FixWdt(std::span<const uint8_t> in, std::vector<uint8_t>& out);
}
