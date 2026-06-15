// Archive file-I/O primitive addresses, their signatures, and the whole-file open flag.
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

// Archive file-I/O primitives. All callee-cleaned. Hooked to serve reads from the host.
namespace wraith::offsets::engine::io
{
    // Open (archiveOrNull, name, flags, &handle) -> nonzero, fills handle.
    constexpr uintptr_t kFileOpen  = 0x00424B50;
    // Second open entry: thin open-by-name (archiveOrNull, name, flags, &handle) -> bool.
    constexpr uintptr_t kFileOpen2 = 0x00422040;
    // Size (handle, &sizeHigh) -> file size low dword.
    constexpr uintptr_t kFileSize  = 0x004218C0;
    // Read (handle, dst, len, &read|0, 0, 0) -> nonzero.
    constexpr uintptr_t kFileRead  = 0x00422530;
    // Seek (handle, distLow, &distHigh, method) -> new position low; method 0=begin,1=current,2=end.
    constexpr uintptr_t kFileSeek  = 0x00421BB0;
    // Close (handle).
    constexpr uintptr_t kFileClose = 0x00422910;

    // Open flag: load the whole file into the handle buffer.
    constexpr uint32_t  kOpenWholeFile = 0x20000;

    using Storage_FileOpenFn  = int(__stdcall*)(void* archive, const char* name, uint32_t flags, void** out);
    using Storage_FileSizeFn  = uint32_t(__stdcall*)(void* handle, uint32_t* sizeHigh);
    using Storage_FileReadFn  = int(__stdcall*)(void* handle, void* dst, uint32_t len, uint32_t* read,
                                                void* ovl, uint32_t unk);
    using Storage_FileSeekFn  = uint32_t(__stdcall*)(void* handle, int32_t distLow, uint32_t* distHigh,
                                                     uint32_t method);
    using Storage_FileCloseFn = int(__stdcall*)(void* handle);
}
