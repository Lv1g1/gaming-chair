#pragma once

namespace offsets {
    constexpr auto dwLocalPlayerPawn = 0x173D5A8; // quale offset? dwLocalPlayerController = 0x1915C08;
    constexpr auto dwEntityList = 0x18C9E78;
    constexpr auto dwViewMatrix = 0x192B2C0;
    
    constexpr auto m_vecViewOffset = 0xC58;
    constexpr auto m_hPlayerPawn = 0x7E4;
    constexpr auto m_iHealth = 0x334;
    constexpr auto m_iTeamNum = 0x3CB; // uint8
    constexpr auto m_lifeState = 0x338; // int32
    constexpr auto m_vecOrigin = 0x80; // CNetworkOriginCellCoordQuantizedVector
    constexpr auto m_vOldOrigin = 0x127C;
    constexpr auto m_bDormant = 0xE7; // bool

    constexpr auto m_pGameSceneNode = 0x318; // CGameSceneNode*
    constexpr auto m_CBodyComponent = 0x38; // CBodyComponent*
    constexpr auto m_skeletonInstance = 0x50; // CSkeletonInstance
    constexpr auto m_modelState = 0x160; // CModelState
    constexpr auto m_entitySpottedState = 0x2278; // EntitySpottedState_t
    constexpr auto m_bSpotted = 0x8; // bool
}