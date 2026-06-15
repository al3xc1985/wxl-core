// Host translator registry: route a file name to its byte translator.
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
#include <string_view>

#include "ADT/AdtMerge.hpp"
#include "M2/M2Translate.hpp"
#include "WDL/WdlRebuild.hpp"
#include "WDT/WdtFixup.hpp"
#include "WMO/WmoTranslate.hpp"

// HOST-only. The byte-translation registry (the "TranslatorRegistry"). Pure bytes ->
// 335 bytes. This is a COLD router: it runs once per file open, picks a concrete
// free function by file name, and the function does the tight transform. NO virtual
// hierarchy, NO std::function in any hot path.
//
// Each format has two contracts: Client (the 335 target layout) and Source (the modern
// additive superset). The transform gates on chunk/field presence, not on a version id.
namespace wraith::structure
{
    // Classifies a requested name into a format (cold, once per open).
    enum class Format : uint32_t { Raw, Adt, Wdt, Wdl, Wmo, M2 };

    Format Classify(std::string_view name);

    // Translate `in` (source-version bytes for `name`) into 335 `out`. rc resolves FileDataID
    // references for formats that need it (e.g. WMO texture FDIDs); pass an empty ResolveCtx when
    // none is available. Returns false if the name is not handled or needs no change (caller serves
    // the raw `in` bytes).
    bool Dispatch(std::string_view name, std::span<const uint8_t> in,
                  const ResolveCtx& rc, std::vector<uint8_t>& out);
}
