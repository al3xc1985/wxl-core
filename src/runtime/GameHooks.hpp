// Game-logic detours that publish the non-render events (model load, doodad spawn, world lifecycle...).
// Copyright (C) 2026 WarcraftXL
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

// Function-entry detours into game logic, republished as events. Installs the MinHook detours; the
// caller runs hook::EnableAll() once after every installer. Emits OnModelLoad (and, as their RE lands,
// OnDoodadSpawn / OnWorldEnter / OnWorldLeave / OnTextureUpload / OnAdtChunkBuild).
namespace wxl::runtime::game
{
    void Install();
}
