#include "settings.h"

#include <fstream>

char WORKING_DIR[256] = "C:\\Users\\luigi\\source\\repos\\gaming chair\\";

bool settings_changed = false;

Settings setting_stuct_1;
Settings setting_stuct_2;

Settings *settings = &setting_stuct_1;
Settings *unused_settings = &setting_stuct_2;

bool validate_settings(Settings s) { return true; } // to do
#include <Windows.h>
void load_settings() {
	std::string path = WORKING_DIR;
	path += "src\\server\\settings.bin";

	OutputDebugStringA(path.c_str());

	// Open the file in binary mode
	std::ifstream inFile(path, std::ios::binary);

	// Check if the file was opened successfully
	if (inFile) {
		// Read the struct object from the file
		inFile.read(reinterpret_cast<char*>(settings), sizeof(Settings));

		// Close the file
		inFile.close();

		// Check if the settings are valid
		if (validate_settings(*settings)) {
			return;
		}
	}
	
	// If the file does not exist, use the default settings
	*settings = {
		// Server settings
		12345, // SERVER_PORT
		
		// Display settings
		800, // DISPLAY_WIDTH
		600, // DISPLAY_HEIGHT

		// Cheat Settings
		true, // DRAW_BOX
		true, // DRAW_BONES
		true, // DRAW_HEALTH_BAR
		true, // DRAW_HEAD
		true, // DRAW_TEXT
		
		true, // DRAW_BOX_WHEN_SPOTTED
		false, // DRAW_BONES_WHEN_SPOTTED
		true, // DRAW_HEALTH_BAR_WHEN_SPOTTED
		false, // DRAW_HEAD_WHEN_SPOTTED
		true, // DRAW_TEXT_WHEN_SPOTTED

		true, // AIMBOT_ENABLED
		false, // AIMBOT_ENABLED_WHEN_NOT_SPOTTED

		// Wallhack settings
		75.0f, // BOX_HEIGHT
		0.4f, // BOX_HEIGHT_WIDTH_RATIO
		{ 0, 0, 0, 255 }, // BOX_COLOR_struct
		1.0f, // BOX_THICKNESS

		{ 255, 0, 0, 255 }, // BONES_COLOR_struct
		1.0f, // BONES_THICKNESS

		2.0f, // HEALTH_BAR_THICKNESS

		2.0f, // HEAD_RADIUS
		{ 0, 0, 255, 255 }, // HEAD_COLOR_struct

		{ 255, 255, 255, 255 }, // TEXT_COLOR_struct
		2.0f, // TEXT_THICKNESS

		// Aimbot settings
		10.0f, // AIMBOT_FOV

		true, // AIMBOT_DRAW_FOV
		{ 255, 255, 255, 255 }, // AIMBOT_FOV_COLOR_struct

		5 // AIMBOT_SMOOTH
	};
}

void save_settings() {
	std::string path = WORKING_DIR;
	path += "src\\server\\settings.bin";

	// Open the file in binary mode or create it if it does not exist
	std::ofstream outFile(path, std::ios::binary);

	// Check if the file was opened successfully
	if (!outFile) {
		return;
	}

	// Write the struct object to the file
	outFile.write(reinterpret_cast<char*>(settings), sizeof(Settings));

	// Close the file
	outFile.close();
}

bool modify_settings(Settings s) {
	if (validate_settings(s)) {
		*unused_settings = s;
		settings_changed = true;
		return true;
	}
	return false;
}

void refresh_settings() {
	if (settings_changed) {
		Settings* temp = settings;
		settings = unused_settings;
		unused_settings = temp;
		settings_changed = false;
	}
}