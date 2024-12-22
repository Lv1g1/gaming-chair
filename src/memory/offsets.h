#pragma once

namespace offsets {
    constexpr auto dwLocalPlayerPawn = 0x17C9468; // quale offset? dwLocalPlayerController = 0x1915C08;
    constexpr auto dwEntityList = 0x195C460;
    constexpr auto dwViewMatrix = 0x19BE600;

    constexpr auto m_vecViewOffset = 0xC50;
    constexpr auto m_hPlayerPawn = 0x7DC;
    constexpr auto m_iHealth = 0x324;
    constexpr auto m_iTeamNum = 0x3C3; // uint8
    constexpr auto m_lifeState = 0x328; // int32
    constexpr auto m_vecOrigin = 0x88; // CNetworkOriginCellCoordQuantizedVector
    constexpr auto m_vOldOrigin = 0x1274;
    constexpr auto m_bDormant = 0xEF; // bool

    constexpr auto m_pGameSceneNode = 0x308; // CGameSceneNode*
    constexpr auto m_CBodyComponent = 0x38; // CBodyComponent*
    constexpr auto m_skeletonInstance = 0x50; // CSkeletonInstance
    constexpr auto m_modelState = 0x170; // CModelState
    constexpr auto m_entitySpottedState = 0x2278; // EntitySpottedState_t
    constexpr auto m_bSpotted = 0x8; // bool
}