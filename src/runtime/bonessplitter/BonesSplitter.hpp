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
#include <vector>

#include "structure/M2/Client.hpp"
#include "runtime/m2/M2Runtime.hpp"

// DLL-only (runtime). BonesSplitter: partitions a drawn submesh whose per-draw bone palette exceeds
// the target client's SM3 ceiling into <=ceiling-bone sub-sections, rebuilding the skin geometry
// arrays and the header boneCombos. Public surface consumed by the model loader.
namespace wraith::runtime::bonessplitter
{
    // SM3 vertex-constant ceiling: the bone palette starts at register 31, 3 registers per matrix,
    // so (256 - 31) / 3 = 75 bones per draw. A drawn section above this is split; a finalize clamp is
    // the safety net when a split is skipped or impossible.
    constexpr uint16_t kMaxBonesPerDraw = 75;

    // One rebuilt sub-section plus the original submesh it came from.
    struct SplitSection { wraith::structure::m2::M2SkinSection section; uint16_t origSubmesh; };
    // The contiguous run of new sub-section indices one original submesh became.
    struct SplitRun { uint16_t first; uint16_t count; };

    // Greedy triangle bin-packer: partition any drawn submesh whose per-draw bone palette exceeds the
    // SM3 ceiling into <=75-bone sub-sections, each with its own compact boneCombos slice, deduplicated
    // vertex block (bones[] remapped to the slice), and global-indexed triangle block. The skin
    // geometry arrays and header.boneCombos are rebuilt into owned buffers. Returns false (no commit)
    // on any overflow / OOM / missing array, leaving the caller on the clamp path.
    bool SplitSubmeshes(wraith::structure::m2::M2Header* md, wraith::runtime::m2::M2SkinProfile* skin,
                        std::vector<SplitSection>& outSections, std::vector<SplitRun>& splitMap,
                        uint32_t& splitCount, const char* name);
}
