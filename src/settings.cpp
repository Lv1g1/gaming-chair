#include "settings.h"
#include <Windows.h>
#include <fstream>

char WORKING_DIR[256] = "C:\\Users\\jak\\source\\repos\\gaming-chair-gi2\\";

bool settings_changed = false;

Settings setting_stuct_1;
Settings setting_stuct_2;

Settings *settings = &setting_stuct_1;
Settings *unused_settings = &setting_stuct_2;


void print_color(const std::string& name, const Color& color) {
	std::cout << name << ": (R: " << static_cast<int>(color.r)
		<< ", G: " << static_cast<int>(color.g)
		<< ", B: " << static_cast<int>(color.b)
		<< ", A: " << static_cast<int>(color.a) << ")" << std::endl;
}

void CreateNewConsole()
{
	// Allocare una nuova console
	if (AllocConsole())
	{
		// Reindirizzare stdout a CONOUT$
		FILE* fpOut;
		freopen_s(&fpOut, "CONOUT$", "w", stdout);
		if (fpOut == nullptr)
		{
			std::cerr << "Errore nel reindirizzare stdout.\n";
		}

		// Reindirizzare stderr a CONOUT$
		FILE* fpErr;
		freopen_s(&fpErr, "CONOUT$", "w", stderr);
		if (fpErr == nullptr)
		{
			std::cerr << "Errore nel reindirizzare stderr.\n";
		}

		// (Opzionale) Reindirizzare stdin a CONIN$
		FILE* fpIn;
		freopen_s(&fpIn, "CONIN$", "r", stdin);
		if (fpIn == nullptr)
		{
			std::cerr << "Errore nel reindirizzare stdin.\n";
		}

		// Sincronizzare i buffer C++ con gli stream C
		std::ios::sync_with_stdio();
		std::cout.clear();
		std::cerr.clear();
	}
	else
	{
		// Allocazione della console fallita
		return;
	}
}
// Funzione per stampare i valori di Settings
void print_settings(const Settings& s) {
	// Server settings
	std::cout << "SERVER_PORT: " << s.SERVER_PORT << std::endl;

	// Display s
	std::cout << "DISPLAY_WIDTH: " << s.DISPLAY_WIDTH << std::endl;
	std::cout << "DISPLAY_HEIGHT: " << s.DISPLAY_HEIGHT << std::endl;

	// Cheat Settings
	std::cout << "DRAW_BOX: " << (s.DRAW_BOX ? "true" : "false") << std::endl;
	std::cout << "DRAW_BONES: " << (s.DRAW_BONES ? "true" : "false") << std::endl;
	std::cout << "DRAW_HEALTH_BAR: " << (s.DRAW_HEALTH_BAR ? "true" : "false") << std::endl;
	std::cout << "DRAW_HEAD: " << (s.DRAW_HEAD ? "true" : "false") << std::endl;
	std::cout << "DRAW_TEXT: " << (s.DRAW_TEXT ? "true" : "false") << std::endl;

	std::cout << "DRAW_BOX_WHEN_SPOTTED: " << (s.DRAW_BOX_WHEN_SPOTTED ? "true" : "false") << std::endl;
	std::cout << "DRAW_BONES_WHEN_SPOTTED: " << (s.DRAW_BONES_WHEN_SPOTTED ? "true" : "false") << std::endl;
	std::cout << "DRAW_HEALTH_BAR_WHEN_SPOTTED: " << (s.DRAW_HEALTH_BAR_WHEN_SPOTTED ? "true" : "false") << std::endl;
	std::cout << "DRAW_HEAD_WHEN_SPOTTED: " << (s.DRAW_HEAD_WHEN_SPOTTED ? "true" : "false") << std::endl;
	std::cout << "DRAW_TEXT_WHEN_SPOTTED: " << (s.DRAW_TEXT_WHEN_SPOTTED ? "true" : "false") << std::endl;

	std::cout << "AIMBOT_ENABLED: " << (s.AIMBOT_ENABLED ? "true" : "false") << std::endl;
	std::cout << "AIMBOT_ENABLED_WHEN_NOT_SPOTTED: " << (s.AIMBOT_ENABLED_WHEN_NOT_SPOTTED ? "true" : "false") << std::endl;

	// Wallhack s
	std::cout << "BOX_HEIGHT: " << s.BOX_HEIGHT << std::endl;
	std::cout << "BOX_HEIGHT_WIDTH_RATIO: " << s.BOX_HEIGHT_WIDTH_RATIO << std::endl;
	print_color("BOX_COLOR_struct", s.BOX_COLOR_struct);
	std::cout << "BOX_THICKNESS: " << s.BOX_THICKNESS << std::endl;

	print_color("BONES_COLOR_struct", s.BONES_COLOR_struct);
	std::cout << "BONES_THICKNESS: " << s.BONES_THICKNESS << std::endl;

	std::cout << "HEALTH_BAR_THICKNESS: " << s.HEALTH_BAR_THICKNESS << std::endl;

	std::cout << "HEAD_RADIUS: " << s.HEAD_RADIUS << std::endl;
	print_color("HEAD_COLOR_struct", s.HEAD_COLOR_struct);

	print_color("TEXT_COLOR_struct", s.TEXT_COLOR_struct);
	std::cout << "TEXT_THICKNESS: " << s.TEXT_THICKNESS << std::endl;

	// Aimbot s
	std::cout << "AIMBOT_FOV: " << s.AIMBOT_FOV << std::endl;

	std::cout << "AIMBOT_DRAW_FOV: " << (s.AIMBOT_DRAW_FOV ? "true" : "false") << std::endl;
	print_color("AIMBOT_FOV_COLOR_struct", s.AIMBOT_FOV_COLOR_struct);

	std::cout << "AIMBOT_SMOOTH: " << s.AIMBOT_SMOOTH << std::endl;
}



bool validate_settings(Settings s) { 
	//if (s.SERVER_PORT < 0 || s.SERVER_PORT > 65535) {
	//	return false;
	//}
	//if (s.DISPLAY_WIDTH < 0 || s.DISPLAY_WIDTH > 10000) {
	//	return false;
	//}
	//if (s.DISPLAY_HEIGHT < 0 || s.DISPLAY_HEIGHT > 10000) {
	//	return false;
	//}
	//if (s.BOX_HEIGHT < 0 || s.BOX_HEIGHT > 1000) {
	//	return false;
	//}
	//if (s.BOX_HEIGHT_WIDTH_RATIO < 0 || s.BOX_HEIGHT_WIDTH_RATIO > 1000) {
	//	return false;
	//}
	//if (s.BOX_THICKNESS < 0 || s.BOX_THICKNESS > 1000) {
	//	return false;
	//}
	//if (s.BONES_THICKNESS < 0 || s.BONES_THICKNESS > 1000) {
	//	return false;
	//}
	//if (s.HEALTH_BAR_THICKNESS < 0 || s.HEALTH_BAR_THICKNESS > 1000) {
	//	return false;
	//}
	//if (s.HEAD_RADIUS < 0 || s.HEAD_RADIUS > 1000) {
	//	return false;
	//}
	//if (s.TEXT_THICKNESS < 0 || s.TEXT_THICKNESS > 1000) {
	//	return false;
	//}
	//if (s.AIMBOT_FOV < 0 || s.AIMBOT_FOV > 1000) {
	//	return false;
	//}
	//if (s.AIMBOT_SMOOTH < 0 || s.AIMBOT_SMOOTH > 1000) {
	//	return false;
	//}
	//return true;
	CreateNewConsole();
	//print_settings(s);
	return true;

} // to do
#include <Windows.h>
void load_settings() {
	std::string path = WORKING_DIR;
	path += "src\\server\\settings.bin";

	char string_buf[4096];

	sprintf_s(string_buf, "Loading settings from: %s\n", path.c_str());

	OutputDebugStringA(string_buf);	

	// Open the file in binary mode
	std::ifstream inputFile(path);


	// Check if the file was opened successfully
	if (inputFile.is_open()) {
		// Read the struct object from the file or create if it does not exist
		inputFile.read(reinterpret_cast<char*>(settings), sizeof(Settings));


		// Close the file
		inputFile.close();

		// Check if the settings are valid
		if (validate_settings(*settings)) {
			char string_buf[4096];
			sprintf_s(string_buf, "Settings loaded successfully\n");
			OutputDebugStringA(string_buf);

			return;
		}
		else {
			char string_buf[4096];
			sprintf_s(string_buf, "Settings are invalid\n");

			OutputDebugStringA(string_buf);

		}
	}
	
	// If the file does not exist, use the default settings
	*settings = {
		// Server settings
		12345, // SERVER_PORT
		
		// Display settings
		1920, // DISPLAY_WIDTH
		1080, // DISPLAY_HEIGHT

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
	inputFile.close();
	save_settings();

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
		//print_settings(*unused_settings);
		return true;
	}
	return false;
}

void refresh_settings() {
	if (settings_changed) {
		Settings* temp = settings;
		settings = unused_settings;
		unused_settings = temp;
		print_settings(*settings);
		save_settings();
		settings_changed = false;
	}
}