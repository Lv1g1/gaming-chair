#include "settings.h"
#include <Windows.h>
#include <cmath>
#include <cstring>
#include <fstream>
#include <mutex>
#include <string>

char WORKING_DIR[256] = "";

bool settings_changed = false;

Settings setting_stuct_1;
Settings setting_stuct_2;

Settings *settings = &setting_stuct_1;
Settings *unused_settings = &setting_stuct_2;

// Guards settings_changed and the settings/unused_settings swap: the HTTP
// server thread updates the configuration while the main loop applies it.
namespace {
	std::mutex settings_mutex;
}


void print_color(const std::string& name, const Color& color) {
	std::cout << name << ": (R: " << static_cast<int>(color.r)
		<< ", G: " << static_cast<int>(color.g)
		<< ", B: " << static_cast<int>(color.b)
		<< ", A: " << static_cast<int>(color.a) << ")" << std::endl;
}

void init_console()
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



static bool in_range(float value, float minimum, float maximum) {
	return std::isfinite(value) && value >= minimum && value <= maximum;
}

bool validate_settings(Settings s) {
	if (s.SERVER_PORT < 1 || s.SERVER_PORT > 65535) {
		return false;
	}
	if (s.DISPLAY_WIDTH < 1 || s.DISPLAY_WIDTH > 10000) {
		return false;
	}
	if (s.DISPLAY_HEIGHT < 1 || s.DISPLAY_HEIGHT > 10000) {
		return false;
	}
	if (!in_range(s.BOX_HEIGHT, 0.0f, 1000.0f)) {
		return false;
	}
	if (!in_range(s.BOX_HEIGHT_WIDTH_RATIO, 0.0f, 10.0f)) {
		return false;
	}
	if (!in_range(s.BOX_THICKNESS, 0.0f, 100.0f)) {
		return false;
	}
	if (!in_range(s.BONES_THICKNESS, 0.0f, 100.0f)) {
		return false;
	}
	if (!in_range(s.HEALTH_BAR_THICKNESS, 0.0f, 100.0f)) {
		return false;
	}
	if (!in_range(s.HEAD_RADIUS, 0.0f, 1000.0f)) {
		return false;
	}
	if (!in_range(s.TEXT_THICKNESS, 0.0f, 100.0f)) {
		return false;
	}
	if (!in_range(s.AIMBOT_FOV, 0.0f, 10000.0f)) {
		return false;
	}
	if (s.AIMBOT_SMOOTH < 0 || s.AIMBOT_SMOOTH > 1000) {
		return false;
	}
	if (s.WALLHACK_TOGGLE_KEY < 1 || s.WALLHACK_TOGGLE_KEY > 254) {
		return false;
	}
	if (!in_range(s.PROXIMITY_ALERT_RADIUS, 0.0f, 10000.0f)) {
		return false;
	}
	if (!in_range(s.PROXIMITY_ALERT_INTERVAL, 0.0f, 3600.0f)) {
		return false;
	}

	return true;
}

void init_working_dir() {
	char exe_path[MAX_PATH] = {};
	if (GetModuleFileNameA(nullptr, exe_path, MAX_PATH) == 0)
		return;

	std::string path = exe_path;

	// Directory containing the executable (ends with a backslash).
	size_t slash = path.find_last_of('\\');
	if (slash == std::string::npos)
		return;
	path = path.substr(0, slash + 1);   // e.g. "...\gaming-chair\build\"

	// The source tree (src\server\...) lives one level above the build output.
	if (!path.empty() && path.back() == '\\')
		path.pop_back();                // "...\gaming-chair\build"
	slash = path.find_last_of('\\');
	if (slash == std::string::npos)
		return;
	path = path.substr(0, slash + 1);   // "...\gaming-chair\"

	strncpy_s(WORKING_DIR, path.c_str(), _TRUNCATE);
}

// The box colour used to default to pure black, which is invisible on top of the
// game: "the boxes are not drawn" can simply mean "the boxes are black". Treat
// that value as "not configured" and fall back to a visible colour.
static void apply_box_colour_default() {
	if (settings->BOX_COLOR_struct.r == 0 && settings->BOX_COLOR_struct.g == 0 &&
		settings->BOX_COLOR_struct.b == 0) {
		settings->BOX_COLOR_struct = { 0, 255, 0, 255 };
		OutputDebugStringA("BOX_COLOR was the old invisible default, using green\n");
	}
}

#include <Windows.h>
void load_settings() {
	init_working_dir();

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

		const std::streamsize read_bytes = inputFile.gcount();

		// Close the file
		inputFile.close();

		// Accept the file only if it is exactly the size of the current Settings
		// struct (guards against loading a settings.bin written by an older build).
		if (read_bytes == sizeof(Settings) && validate_settings(*settings)) {
			char string_buf[4096];
			sprintf_s(string_buf, "Settings loaded successfully\n");
			OutputDebugStringA(string_buf);

			apply_box_colour_default();
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
		true, // DRAW_BONES_WHEN_SPOTTED
		true, // DRAW_HEALTH_BAR_WHEN_SPOTTED
		true, // DRAW_HEAD_WHEN_SPOTTED
		true, // DRAW_TEXT_WHEN_SPOTTED

		true, // AIMBOT_ENABLED
		true, // AIMBOT_ENABLED_WHEN_NOT_SPOTTED

		// Wallhack settings
		75.0f, // BOX_HEIGHT
		0.4f, // BOX_HEIGHT_WIDTH_RATIO
		{ 0, 255, 0, 255 }, // BOX_COLOR_struct (green: black is invisible on the game)
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

		5, // AIMBOT_SMOOTH

		// Wallhack toggle key (VK_L)
		76, // WALLHACK_TOGGLE_KEY

		// Proximity alert
		true, // PROXIMITY_ALERT_ENABLED
		true, // PROXIMITY_ALERT_DRAW
		400.0f, // PROXIMITY_ALERT_RADIUS
		3.5f, // PROXIMITY_ALERT_INTERVAL
		{ 255, 0, 255, 255 }, // PROXIMITY_ALERT_COLOR_struct

	};
	inputFile.close();
	apply_box_colour_default();
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
	std::lock_guard<std::mutex> lock(settings_mutex);

	if (!validate_settings(s)) {
		return false;
	}

	*unused_settings = s;
	settings_changed = true;
	return true;
}

void refresh_settings() {
	std::lock_guard<std::mutex> lock(settings_mutex);

	if (!settings_changed) {
		return;
	}

	Settings* temp = settings;
	settings = unused_settings;
	unused_settings = temp;
	print_settings(*settings);
	save_settings();
	settings_changed = false;
}

Settings get_settings_snapshot() {
	std::lock_guard<std::mutex> lock(settings_mutex);
	return *settings;
}