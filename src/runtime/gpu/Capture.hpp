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

#include <d3d9.h>

// Captures the engine's D3D9 device by intercepting IDirect3D9::CreateDevice on the MAIN thread
namespace wraith::d3d12::capture
{
    // Called at each EndScene, on the main thread, after the device is captured. device = engine device.
    using FrameFn = void (*)(IDirect3DDevice9* device);

    // Wrap the real factory so CreateDevice/CreateDeviceEx are intercepted; returns a forwarding wrapper to
    // hand back to the engine in place of the real IDirect3D9/Ex.
    IDirect3D9*   Wrap(IDirect3D9* real);
    IDirect3D9Ex* WrapEx(IDirect3D9Ex* real);

    // Register the per-frame callback (call before the engine creates its device).
    void OnFrame(FrameFn fn);

    // The captured engine device, or null until the engine has called CreateDevice.
    IDirect3DDevice9* Device();
}
