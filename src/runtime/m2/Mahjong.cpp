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

#include "Mahjong.hpp"

#include "Logger.hpp"

// Mahjong: the physics runtime. Parses the model's physics description, runs a sequential-impulse solver,
// and writes solved transforms into the live bone palette.
//
// BLOCKED: depends on the bone-palette view<->world correction (runtime/m2/Bone) and a physics-description
// reader that are not landed here. No self-contained behavior to install yet, so Install() is inert.
namespace wraith::runtime::mahjong
{
    void Install()
    {
        WLOG_INFO("M2/mahjong: physics solver not yet active");
    }
}
