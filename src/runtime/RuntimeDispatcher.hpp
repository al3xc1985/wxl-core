// DLL feature registry: a flat list of install entries, iterated at bootstrap.
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

// DLL-only. The Feature registry (the "FeatureRegistry"). Bootstrap iterates a FLAT
// static list of {name, Install} entries and calls each once at startup (cold). NO
// virtual Feature hierarchy, NO std::function on any hot path: each entry is a plain
// function pointer to a feature's install routine.
//
// Features depend only on core/ + offsets/, never on each other. These hook the LIVE
// engine / GPU; the byte-translation features live HOST-side in structure/.
namespace wraith::runtime
{
    // One feature install entry. Install() registers this feature's hooks (cold).
    struct FeatureEntry
    {
        const char* name;
        void (*Install)();
    };

    // The static feature table (defined in the .cpp). Order is install order.
    std::span<const FeatureEntry> Features();

    // Bootstrap: run every feature's Install(), then enable all hooks in one batch.
    void InstallAll();
}
