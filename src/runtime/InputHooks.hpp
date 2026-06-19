// Window-input detour that republishes window messages as the OnInput event.
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

// The core subclasses the client's top-level window ONCE and republishes every message as OnInput, so a
// module never has to hook the window itself. A subscriber that consumes a message (sets *handled) makes
// the core swallow it. Install once after the graphics device (hence the window) is up.
namespace wxl::runtime::input
{
    void Install();
}
