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

#include "Anim.hpp"

#include "Hook.hpp"
#include "Logger.hpp"
#include "M2.hpp"

#include <windows.h>
#include <cstdint>
#include <cstring>

namespace off = wraith::offsets::game::m2;

// A Source external animation file is a single chunk [u32 magic][u32 size][payload], where the payload is
// byte-for-byte the target external-animation layout and size == filesize-8. The native loader resolves
// track offsets from the loaded buffer base, but the Source stores them relative to the chunk data
// (file+8), so the 8-byte header shifts every keyframe and animations read garbage. At read-completion,
// before the rebase, the header is stripped in place (payload moved back 8 bytes, size -= 8) so offsets
// line up. Files without the chunk magic are already raw and left untouched.
namespace wraith::runtime::anim
{
    namespace
    {
        // Source external-animation chunk magic ('A''F''M''2', little-endian) and its header size.
        constexpr uint32_t kSourceAnimMagic = 0x324D4641;
        constexpr uint32_t kSourceAnimHeaderSize = 8;

        off::M2_AnimLoadCompleteFn g_animLoadCompleteOriginal = nullptr;

        // The I/O record holds the just-read buffer and its size: the exact base and size the native
        // rebase consumes. The buffer was sized to the full Source file, so stripping the 8-byte header
        // leaves it large enough; every offset the model declares lands inside the payload.
        void StripHeader(void* record)
        {
            auto* recBytes = static_cast<uint8_t*>(record);
            auto* buffer = *reinterpret_cast<uint8_t**>(recBytes + off::kOffRecordBuffer);
            auto* sizeField = reinterpret_cast<uint32_t*>(recBytes + off::kOffRecordSize);
            if (!buffer || *sizeField < kSourceAnimHeaderSize) return;
            if (*reinterpret_cast<uint32_t*>(buffer) != kSourceAnimMagic) return; // already raw

            uint32_t payloadSize = *sizeField - kSourceAnimHeaderSize;
            memmove(buffer, buffer + kSourceAnimHeaderSize, payloadSize);
            *sizeField = payloadSize;
        }

        // SEH-guarded: derefs the I/O record and raw file bytes; a malformed file must not crash the loader.
        void StripHeaderGuarded(void* record)
        {
            if (!record) return;
            __try { StripHeader(record); }
            __except (EXCEPTION_EXECUTE_HANDLER) {}
        }

        // Read-completion: the buffer is filled but not yet offset-rebased. Strip a Source chunk header in
        // place so the native rebase treats the payload as raw, then proceed.
        void __cdecl AnimLoadCompleteDetour(void* node)
        {
            void* record = *reinterpret_cast<void**>(static_cast<uint8_t*>(node) + off::kOffNodeRecord);
            StripHeaderGuarded(record);
            g_animLoadCompleteOriginal(node);
        }
    }

    void Install()
    {
        hook::Install("M2::AnimLoadComplete", off::kAnimLoadComplete, &AnimLoadCompleteDetour,
                      reinterpret_cast<void**>(&g_animLoadCompleteOriginal));
        WLOG_INFO("M2/anim: installed");
    }
}
