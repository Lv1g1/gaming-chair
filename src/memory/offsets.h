#pragma once

#include <cstdint>

// Dynamic offset resolution.
//
// The offsets used to read game memory are no longer compile-time constants:
// they are runtime values that can be refreshed from a JSON source (the same
// approach used by "cs2-offsets" style projects) so the cheat survives game
// updates without recompiling.
//
// Usage:
//   - offsets::reset()   restores the compiled-in fallback values.
//   - offsets::resolve() loads offsets from a local file and/or a remote URL
//     and updates only the values that are found. Call it once at startup;
//     anything that cannot be resolved keeps its fallback value.
//
// The JSON source is an object of `"name": value` pairs where `value` can be a
// decimal number or a hex string:
//
//   {
//     "dwLocalPlayerPawn": 25581640,
//     "dwEntityList": "0x1A146E8",
//     "m_iHealth": 844
//   }
//
// Nested objects are also accepted (for example the offsets.json and
// client_dll.json produced by a2x/cs2-dumper), because the resolver simply
// looks for every known name as a JSON key.

namespace offsets {

    // --- Globals (dw*) ---
    extern uintptr_t dwLocalPlayerPawn;
    extern uintptr_t dwEntityList;
    extern uintptr_t dwViewMatrix;

    // --- Schema fields (m_*) ---
    extern uintptr_t m_vecViewOffset;
    extern uintptr_t m_hPlayerPawn;
    extern uintptr_t m_iHealth;
    extern uintptr_t m_iTeamNum;
    extern uintptr_t m_lifeState;
    extern uintptr_t m_vecOrigin;
    extern uintptr_t m_vOldOrigin;
    extern uintptr_t m_bDormant;

    extern uintptr_t m_pGameSceneNode;
    extern uintptr_t m_CBodyComponent;
    extern uintptr_t m_skeletonInstance;
    extern uintptr_t m_modelState;
    extern uintptr_t m_entitySpottedState;
    extern uintptr_t m_bSpotted;

    // --- Entity list layout / skeleton helpers ---
    // These are not schema fields: they are the hardcoded constants used to walk
    // the entity list and the bone array. They change with game updates too, so
    // they can be overridden from the JSON source instead of recompiling.
    extern uintptr_t entityListEntrySize;   // stride of one entity list entry (0x70)
    extern uintptr_t entityListChunkOffset; // chunk array offset in the entity system (0x10)
    extern uintptr_t entityListChunkSize;   // entities per chunk (512)
    extern uintptr_t modelStateBoneArray;   // CModelState field with the bone array pointer (0x80)

    // Local file looked up next to the executable, in the project root and in
    // the working directory.
    inline constexpr const char* OFFSETS_LOCAL_FILENAME = "offsets.json";

    // Remote endpoint. Override at build time, for example:
    //   /DOFFSETS_URL="https://raw.githubusercontent.com/<user>/<repo>/main/offsets.json"
    // Leave undefined to skip the remote fetch.
#ifndef OFFSETS_URL
#define OFFSETS_URL ""
#define OFFSETS_REMOTE_ENABLED 0
#else
#define OFFSETS_REMOTE_ENABLED 1
#endif

    // Restore every offset to its compiled-in fallback value.
    void reset() noexcept;

    // Resolve offsets from the local file and (optionally) the remote URL.
    // Returns true when at least one offset was resolved from a JSON source.
    bool resolve() noexcept;
}
