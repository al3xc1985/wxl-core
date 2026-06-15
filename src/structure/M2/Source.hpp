// Source contract: the modern additive-superset model layout.
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

#include "Client.hpp"

// HOST-only. The SOURCE contract: how a modern (chunked-container) model differs from the target
// Client layout. The inner model version does not name an expansion; a single inner version spans
// several builds, so the transform gates on the inner-version RANGE and on field/flag presence, never
// on a single version number. The transform compacts these deltas down to the Client contract.
namespace wraith::structure::m2
{
    // Chunked-container magic: the wrapper carrying the inner model as a sub-chunk.
    constexpr uint32_t kMagicMD21 = 0x3132444D; // 'MD21'

    // Inner-version range the Source path accepts. Anything in this range compacts to the Client.
    constexpr uint32_t kSourceVersionMin = 272;
    constexpr uint32_t kSourceVersionMax = 274;

    // A texunit shaderId >= this is a modern shader-effect index (decoded at runtime, not on the host).
    constexpr uint16_t kSourceShaderMin = 0x8000;

    // Particle-emitter stride. The default is the Client stride; the Source form is 16 bytes larger
    // (inner version > 271, or global_flags bit 0x200). The native de-relocator strides by the Client
    // value, so a Source body's emitters are slid down to the Client stride before the client parses.
    constexpr uint32_t kParticleStrideClient = 0x1dc;
    constexpr uint32_t kParticleStrideSource = 0x1ec;
    constexpr uint32_t kFlagParticleExt      = 0x200; // global_flags bit forcing the larger emitter

    // particle textureId at emitter+0x16. The Client reads it as a flat u16 index into header.textures.
    // The multi-texture form (emitter flag 0x10000) instead packs three 5-bit ids; read flat it overruns
    // the texture-handle table. Keep the first id, drop the rest.
    constexpr uint32_t kParticleTextureIdOff  = 0x16;
    constexpr uint32_t kParticleFlagMultiTex  = 0x10000;
    constexpr uint16_t kParticleTextureIdMask = 0x1F;

    // particle blendingType at emitter+0x28 (u8). The Client blend table is stride-7 (modes 0..6); the
    // Source BlendAdd (7) indexes out of range. BlendAdd = (ONE, INV_SRC_ALPHA) = the engine's mode 3,
    // so 7 maps to 3 exactly; any other mode > 7 falls back to Add (4).
    constexpr uint32_t kParticleBlendOff = 0x28;

    // Compressed gravity: a Source emitter with flags 0x800000 stores its gravity track keys as packed
    // {int8 x, int8 y, int16 z} direction+magnitude, not as plain floats. The Client predates this and
    // reads the 4 raw bytes as a float (can be NaN). The gravity track body is at emitter+0x84; its value
    // array {count,ofs} at +0x90/+0x94 (two-level: outer indexed by animation, inner = the 4-byte keys).
    constexpr uint32_t kParticleFlagCompressedGravity = 0x800000;
    constexpr uint32_t kParticleGravityValCountOff    = 0x90;
    constexpr uint32_t kParticleGravityValOfsOff      = 0x94;
    constexpr float    kParticleGravityMagUnit        = 0.04238648f; // yards per key-unit

    // Flipbook atlas: textureRows@+0x30 / textureCols@+0x32 (u16) subdivide the texture into rows*cols
    // cells. The head/tail cell tracks { times array; keys array } are at +0x13c / +0x14c (keys = int16
    // cell indices at block+0x8 count / +0xc ofs). The Source wraps the sampled cell by rows*cols; the
    // Client does not, so a cell >= cols maps to row >= 1 and samples off the atlas. Wrap the keys here.
    constexpr uint32_t kParticleTexRowsOff  = 0x30;
    constexpr uint32_t kParticleTexColsOff  = 0x32;
    constexpr uint32_t kParticleHeadCellOff = 0x13c;
    constexpr uint32_t kParticleTailCellOff = 0x14c;

    // Ribbon stride. The native ribbon de-relocator and the ribbon draw both stride by the Client value;
    // the Source tail (priorityPlane/ribbonColorIndex/textureTransformLookupIndex, 4 bytes) lands in the
    // Client layout's tail padding (0xac..0xb0), so the Source stride equals the Client stride. Both names
    // are kept explicit so a future version that grows the ribbon body only edits the Source value.
    constexpr uint32_t kRibbonStrideClient = 0xb0;
    constexpr uint32_t kRibbonStrideSource = 0xb0;

    // Max animation id the Client engine resolves. A Source sequence id above this needs a remap.
    constexpr uint16_t kClientMaxAnimId = 505;

#pragma pack(push, 1)

    // The chunked-container header: magic, then the inner model's byte size, then the inner model. The
    // inner model offsets are already self-relative, so de-chunking is a slide to the buffer start.
    struct ContainerHeader
    {
        uint32_t magic;     // 0x00 'MD21'
        uint32_t chunkSize; // 0x04 size of the inner model
    };

    // The Source camera dropped the explicit fov float and appended an FoV animation track. The 0x54
    // track body is identical to the Client camera and its sub-array offsets are model-relative, so it
    // compacts to the Client camera in place with no offset cascade.
    struct M2CameraSource
    {
        uint32_t     type;           // 0x00
        float        farClip;        // 0x04
        float        nearClip;       // 0x08
        M2CameraBody body;           // 0x0C
        uint8_t      fovTrack[0x14]; // 0x60  (appended FoV track)
    };

#pragma pack(pop)

    static_assert(sizeof(ContainerHeader) == 8, "ContainerHeader");
    static_assert(sizeof(M2CameraSource) == 0x74, "M2CameraSource");
}
