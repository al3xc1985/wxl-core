// Event bus: the readable hook surface modules subscribe to. The core owns the detours.
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

#include <cstdint>

// The core installs the dangerous detours (addresses, MinHook) and republishes them as named events.
// A module Subscribe()s; it never sees an address. A subscriber is a plain function
// pointer plus an opaque user, so Emit() walks a flat vector and is safe even for per-frame events
// (no std::function, no vtable on the path). Each event carries a typed POD args struct by const ptr.
namespace wxl::events
{
    enum class Event : uint32_t
    {
        OnModelLoad,     // a model finished loading and is parsed     (ModelLoadArgs)
        OnFrame,         // per-frame Present                          (FrameArgs)
        OnEndScene,      // end of the frame, before present           (EndSceneArgs)
        OnWorldRender,   // per-frame world draw pass                  (WorldRenderArgs)
        OnWorldRenderEnd,// world -> UI boundary, the post-fx slot     (WorldRenderEndArgs)
        OnM2BatchDraw,   // one M2 triangle batch is drawing           (M2BatchDrawArgs)
        OnInput,         // window input message (swallowable)         (InputArgs)
        OnAdtChunkBuild, // an ADT map chunk is being built            (AdtChunkArgs)
        OnTextureUpload, // a texture is about to upload to the device (TextureUploadArgs)
        OnDoodadSpawn,   // a placed map doodad (CMapDoodad) was built (DoodadSpawnArgs)
        OnWorldEnter,    // the world/map finished loading, in-world   (WorldEnterArgs)
        OnWorldLeave,    // the world/map is being torn down           (WorldLeaveArgs)
        Count
    };

    // Typed args, passed by const void* and reinterpreted by the subscriber for its event.
    struct ModelLoadArgs      { void* model; };
    struct FrameArgs          { void* device; };
    struct EndSceneArgs       { void* device; };
    struct WorldRenderArgs    { void* device; };
    struct WorldRenderEndArgs { void* device; };
    // One M2 triangle batch, fired just after the native draw with the same draw parameters, so a
    // subscriber can re-issue the draw (the vertex/index buffers are still bound).
    struct M2BatchDrawArgs
    {
        void*    device;
        void*    model;
        int      primType;
        int      baseVertex;
        uint32_t minIndex;
        uint32_t numVerts;
        uint32_t startIndex;
        uint32_t primCount;
    };
    // Window input. A subscriber that consumes the message sets `*handled = true`, which makes the core
    // swallow it (the game does not also react). `handled` is never null. Args are otherwise read-only.
    struct InputArgs         { uint32_t message; uintptr_t wparam; uintptr_t lparam; bool* handled; };
    struct AdtChunkArgs      { void* chunk; uint32_t layerCount; };
    struct TextureUploadArgs { void* texture; uint32_t width; uint32_t height; };
    struct DoodadSpawnArgs   { void* doodad; };  // CMapDoodad just built (read transform via wxl::game::doodad)
    struct WorldEnterArgs    { uint32_t mapId; };
    struct WorldLeaveArgs    { uint32_t mapId; };

    using Handler = void (*)(void* user, const void* args);

    // Subscribe a handler to an event (cold, at startup).
    void Subscribe(Event e, Handler handler, void* user);

    // Publish an event to its subscribers, in subscription order. Called by the core's detours.
    void Emit(Event e, const void* args);
}
