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

#include <cstddef>
#include <cstdint>

// Empty-base typed view: wraps a raw engine object pointer and reads fields at named
// byte offsets (from offsets/game/<FORMAT>.hpp).
namespace wraith::core
{
    template <class Derived>
    struct View
    {
        uint8_t* base = nullptr;

        explicit View(void* p) : base(static_cast<uint8_t*>(p)) {}

        // Typed read of the field at byte offset `off`.
        template <class T>
        T& At(size_t off) const { return *reinterpret_cast<T*>(base + off); }
    };
}
