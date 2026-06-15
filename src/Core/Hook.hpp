// Named hooking over MinHook: install by name + address, enable in one batch.
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

// Named hooking over MinHook. A feature installs a hook by NAME and address; the original
// (trampoline) is returned through `original`.
namespace wraith::hook
{
    // Initialise the hooking engine once at startup.
    bool Init();

    // Install one detour. `name` is for logging. `target` is the engine function address,
    // `detour` is the replacement, `original` receives the trampoline.
    bool Install(const char* name, void* target, void* detour, void** original);

    inline bool Install(const char* name, uintptr_t target, void* detour, void** original)
    {
        return Install(name, reinterpret_cast<void*>(target), detour, original);
    }

    // Enable every installed hook. Call after all features have registered.
    bool EnableAll();
}
