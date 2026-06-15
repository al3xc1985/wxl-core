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

// DLL-only (runtime/GPU). The SHARED multi-pass mechanism: given a list of passes, emit the draw
// calls and per-pass sampler/blend/env state. This is the one place the pass-emission machinery
// lives; each asset runtime (M2/ribbon, terrain/ADT, WMO) builds a pass list from ITS OWN data
// (M2 texture units/combiners, ADT MCLY layers + alpha maps, WMO materials/batches) and feeds it
// here. The adapters stay with their format (runtime/m2, runtime/terrain, runtime/wmo); only the
// mechanism is centralized, so it is never triplicated.
namespace wraith::runtime::multipass
{
    // One render pass: a texture stage plus the fixed-function/shader state to combine it with the
    // accumulated result. Exact fields are filled from the RE corpus during the runtime migration.
    struct Pass
    {
        uint32_t textureHandle; // bound texture for this pass
        uint32_t blendMode;     // combine mode against the previous pass
        uint32_t flags;         // env / alpha-test / two-sided, etc.
    };

    // Emit `count` passes for the geometry currently bound. The caller (a per-format adapter) owns
    // the geometry bind; this only drives the per-pass texture/state + draw. TODO: signature firms
    // up against the device wrapper in runtime/gpu.
    // void Emit(const Pass* passes, uint32_t count);
}
