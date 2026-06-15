// World tick / load-gate / async-I/O entry addresses, globals, and their signatures.
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

// World tick / load-gate entries, async-I/O queue primitives, and the load-state globals.
namespace wraith::offsets::game::world
{
    // --- world tick / load gate ---
    // World tick + loading-screen synchronous drain (param): while the load-gate flag is set this runs
    // the drain that blocks dismissal until pending I/O and near-player objects are resident.
    constexpr uintptr_t kTick = 0x007B6B00;
    // Load-gate flag (u32): nonzero while the blocking drain runs.
    constexpr uintptr_t kLoadActive = 0x00ADFBC8;
    // Focus world position floats X/Y/Z (the center of the load box / player position).
    constexpr uintptr_t kFocusPosX = 0x00CD7778;
    constexpr uintptr_t kFocusPosY = 0x00CD777C;
    constexpr uintptr_t kFocusPosZ = 0x00CD7780;

    // --- async I/O queue primitives ---
    // Wait-all: blocks pumping the async queues until no async file read is pending.
    constexpr uintptr_t kAsyncWaitAll = 0x004BAE10;
    // Pending predicate: nonzero while any async file request still has outstanding work.
    constexpr uintptr_t kAsyncPending = 0x004BAD80;
    // Service the async queues one pump (called as (0, 0)).
    constexpr uintptr_t kAsyncServiceQueues = 0x004B9B20;

    // --- signatures ---
    // World tick + drain (param on stack).
    using World_TickFn = void(__cdecl*)(int param);
    // Async wait-all.
    using World_AsyncWaitAllFn = void(__cdecl*)();
    // Async pending predicate.
    using World_AsyncPendingFn = int(__cdecl*)();
    // Async service one pump (two args on stack).
    using World_AsyncServiceQueuesFn = void(__cdecl*)(int a, int b);
}
