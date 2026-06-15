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

#include "Bone.hpp"

#include "Logger.hpp"
#include "M2.hpp"

// Bone-palette view<->world correction on the live per-instance palette. The palette is bone->VIEW
// (camera-relative), so any world-space bone effect (physics, attachments) must convert between view and
// world before and after touching it. The correction hooks the per-instance palette build and operates on
// the live matrices the vertex shader consumes.
//
// BLOCKED: this is the structure<->runtime handoff for bone physics; it has no value until the physics
// solver feeds it solved transforms. No self-contained behavior to install yet, so Install() is inert.
namespace wraith::runtime::bone
{
    void Install()
    {
        WLOG_INFO("M2/bone: palette correction not yet active (awaits the physics handoff)");
    }
}
