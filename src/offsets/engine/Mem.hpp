// Engine small-block heap allocator / free addresses and their signatures.
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

// Engine heap allocator / free entries. Both are callee-cleaned (4 args).
namespace wraith::offsets::engine::mem
{
    // Allocate a block: (size, fileName, line, flags) -> pointer (size is rounded up internally).
    constexpr uintptr_t kAlloc = 0x0076E540;
    // Free a block obtained from the allocator: (ptr, fileName, line, flags).
    constexpr uintptr_t kFree  = 0x0076E5A0;

    using Mem_AllocFn = void*(__stdcall*)(uint32_t size, const char* file, int line, uint32_t flags);
    using Mem_FreeFn  = void(__stdcall*)(void* ptr, const char* file, int line, uint32_t flags);
}
