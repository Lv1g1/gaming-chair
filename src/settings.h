#pragma once

// Color struct
#include <imgui/imgui.h>
#include <iostream>
typedef struct {
	uint8_t r, g, b, a;
} Color;

#define BOX_COLOR IM_COL32(settings->BOX_COLOR_struct.r, settings->BOX_COLOR_struct.g, settings->BOX_COLOR_struct.b, settings->BOX_COLOR_struct.a)
#define BONES_COLOR IM_COL32(settings->BONES_COLOR_struct.r, settings->BONES_COLOR_struct.g, settings->BONES_COLOR_struct.b, settings->BONES_COLOR_struct.a)
#define HEAD_COLOR IM_COL32(settings->HEAD_COLOR_struct.r, settings->HEAD_COLOR_struct.g, settings->HEAD_COLOR_struct.b, settings->HEAD_COLOR_struct.a)
#define TEXT_COLOR IM_COL32(settings->TEXT_COLOR_struct.r, settings->TEXT_COLOR_struct.g, settings->TEXT_COLOR_struct.b, settings->TEXT_COLOR_struct.a)
#define AIMBOT_FOV_COLOR IM_COL32(settings->AIMBOT_FOV_COLOR_struct.r, settings->AIMBOT_FOV_COLOR_struct.g, settings->AIMBOT_FOV_COLOR_struct.b, settings->AIMBOT_FOV_COLOR_struct.a)

typedef struct {
	// Server settings
	int SERVER_PORT;

    // Display settings
	int DISPLAY_WIDTH;
	int DISPLAY_HEIGHT;

	// Cheat Settings
	bool DRAW_BOX;
	bool DRAW_BONES;
	bool DRAW_HEALTH_BAR;
	bool DRAW_HEAD;
	bool DRAW_TEXT;

	bool DRAW_BOX_WHEN_SPOTTED;
	bool DRAW_BONES_WHEN_SPOTTED;
	bool DRAW_HEALTH_BAR_WHEN_SPOTTED;
	bool DRAW_HEAD_WHEN_SPOTTED;
	bool DRAW_TEXT_WHEN_SPOTTED;

	bool AIMBOT_ENABLED; // only aim when the player is spotted
	bool AIMBOT_ENABLED_WHEN_NOT_SPOTTED;

	// Wallhack settings
	float BOX_HEIGHT;
	float BOX_HEIGHT_WIDTH_RATIO;
	Color BOX_COLOR_struct;
	float BOX_THICKNESS;

	Color BONES_COLOR_struct;
	float BONES_THICKNESS;

	float HEALTH_BAR_THICKNESS;

	float HEAD_RADIUS;
	Color HEAD_COLOR_struct;

	Color TEXT_COLOR_struct;
	float TEXT_THICKNESS;

	// Aimbot settings
	float AIMBOT_FOV;

	bool AIMBOT_DRAW_FOV;
	Color AIMBOT_FOV_COLOR_struct;

	int AIMBOT_SMOOTH;
} Settings;


extern char WORKING_DIR[256];
extern Settings *settings;

bool validate_settings(Settings s);
void load_settings();
void save_settings();
bool modify_settings(Settings s);
void refresh_settings();
