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

#include <d3d12.h>

// The d3d9.dll proxy exports. The proxy carries the d3d9 entry points (Direct3DCreate9/Ex, declared by
// the system d3d9.h) and these Wraith-private exports that hand the shared D3D12 device + queue (the ones
// On12 runs on) to Wraith.dll so its D3D12 backend renders on the same queue as the engine.
extern "C"
{
    __declspec(dllimport) ID3D12Device*       WraithD3D12Device();
    __declspec(dllimport) ID3D12CommandQueue* WraithD3D12Queue();
    __declspec(dllimport) void                WraithD3D12DrainDebug();
}
