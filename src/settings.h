#pragma once

// Display settings
constexpr auto DISPLAY_WIDTH = 1920;
constexpr auto DISPLAY_HEIGHT = 1080;

// Cheat Settings
constexpr auto WALLHACK_DRAW_BOX = 1;
constexpr auto WALLHACK_DRAW_BONES = 1;
constexpr auto WALLHACK_DRAW_HEALTH_BAR = 1;
constexpr auto WALLHACK_DRAW_HEAD = 1;
constexpr auto WALLHACK_DRAW_TEXT = 1;

constexpr auto WALLHACK_DRAW_BOX_WHEN_SPOTTED = 1;
constexpr auto WALLHACK_DRAW_BONES_WHEN_SPOTTED = 0;
constexpr auto WALLHACK_DRAW_HEALTH_BAR_WHEN_SPOTTED = 1;
constexpr auto WALLHACK_DRAW_HEAD_WHEN_SPOTTED = 0;
constexpr auto WALLHACK_DRAW_TEXT_WHEN_SPOTTED = 1;

constexpr auto AIMBOT_ENABLED = 1; // only aim when the player is spotted
constexpr auto AIMBOT_ENABLED_WHEN_NOT_SPOTTED = 0;

// Wallhack settings
constexpr auto WALLHACK_KEY = VK_F1; //Va aggiunta

constexpr auto WALLHACK_BOX_HEIGHT = 75.0f;
constexpr auto WALLHACK_BOX_HEIGHT_WIDTH_RATIO = 0.4f;
#define WALLHACK_BOX_COLOR 0, 0, 0, 255
constexpr auto WALLHACK_BOX_THICKNESS = 1.0f;

#define WALLHACK_BONES_COLOR 255, 0, 0, 255
constexpr auto WALLHACK_BONES_THICKNESS = 1.0f;

constexpr auto WALLHACK_HEALTH_BAR_THICKNESS = 2.0f;

constexpr auto WALLHACK_HEAD_RADIUS = 2.0f;
#define WALLHACK_HEAD_COLOR 0, 0, 255, 255

#define  WALLHACK_TEXT_COLOR 255, 255, 255, 255
constexpr auto WALLHACK_TEXT_THICKNESS = 2.0f;

// Aimbot settings
constexpr auto AIMBOT_KEY = VK_RBUTTON; //Va aggiunta

constexpr auto AIMBOT_FOV = 10.0f;

constexpr auto AIMBOT_DRAW_FOV = 1;
#define AIMBOT_DRAW_FOV_COLOR 255, 255, 255, 255

constexpr auto AIMBOT_SMOOTH = 0.1f;