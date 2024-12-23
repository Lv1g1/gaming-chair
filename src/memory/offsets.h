#pragma once

namespace offsets {
    constexpr auto dwLocalPlayerPawn = 0x1868CC8; // quale offset? dwLocalPlayerController = 0x1915C08;
    constexpr auto dwEntityList = 0x1A146E8;
    constexpr auto dwViewMatrix = 0x1A7F610;

    constexpr auto m_vecViewOffset = 0xCB0;
    constexpr auto m_hPlayerPawn = 0x80C;
    constexpr auto m_iHealth = 0x344;
    constexpr auto m_iTeamNum = 0x3E3; // uint8
    constexpr auto m_lifeState = 0x348; // int32
    constexpr auto m_vecOrigin = 0x88; // CNetworkOriginCellCoordQuantizedVector
    constexpr auto m_vOldOrigin = 0x1324;
    constexpr auto m_bDormant = 0xEF; // bool

    constexpr auto m_pGameSceneNode = 0x328; // CGameSceneNode*
    constexpr auto m_CBodyComponent = 0x38; // CBodyComponent*
    constexpr auto m_skeletonInstance = 0x50; // CSkeletonInstance
    constexpr auto m_modelState = 0x170; // CModelState
    constexpr auto m_entitySpottedState = 0x23D0; // EntitySpottedState_t
    constexpr auto m_bSpotted = 0x8; // bool
}