// WMO byte translator: read Source WMO, emit 335 WMO bytes.
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

#include "../Resolver.hpp"

// Client.hpp / Source.hpp declare this format's two layouts (335 target / modern superset). The
// transform keeps its own local constants for now; consuming the declarations is a later no-op pass.

// HOST-only. WMO is the Phase-1 pilot and the worked EXAMPLE of the contract pattern.
// Pure bytes -> bytes. Strips modern chunks, rebuilds MOTX from FDID resolution, collapses
// the shader family 7..22 -> 0..6, fixes MOBA, recomputes group flags. Output must be
// byte-identical to the prior DLL down-convert for identical input.
//
// Pipeline:
//   read via the Source contract -> emit via the Client contract, applying the source
//   fixup hooks -> target-shaped bytes
//
// Free functions, span-based, concrete. The transform gates on chunk/field presence.
namespace wraith::structure::wmo
{
    // Translate a WMO ROOT file to 335. rc resolves FileDataID texture references (modern
    // materials) to paths appended to the rebuilt MOTX blob. Returns false (serve raw)
    // when the source is already a target-shaped root needing no change.
    bool TranslateWmoRoot(std::span<const uint8_t> in, const ResolveCtx& rc,
                          std::vector<uint8_t>& out);

    // Translate one WMO GROUP file to 335. Returns false (serve raw) when the group is already
    // target-shaped (no modern sub-chunks, no flag/batch mismatch).
    bool TranslateWmoGroup(std::span<const uint8_t> in, std::vector<uint8_t>& out);
}
