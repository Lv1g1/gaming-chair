#include "Server.h"
#include "../settings.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace {

// Largest request we are willing to buffer (the settings form is a few KB).
constexpr size_t kMaxRequestBytes = 64 * 1024;

// A misbehaving client must not be able to block the (single threaded) server.
constexpr DWORD kClientTimeoutMs = 5000;

std::string trim(const std::string& text) {
	const size_t first = text.find_first_not_of(" \t\r\n");
	if (first == std::string::npos)
		return {};
	const size_t last = text.find_last_not_of(" \t\r\n");
	return text.substr(first, last - first + 1);
}

bool parseInt(const std::string& text, int minimum, int maximum, int& out) {
	const std::string value = trim(text);
	if (value.empty())
		return false;

	errno = 0;
	char* end = nullptr;
	const long parsed = std::strtol(value.c_str(), &end, 10);
	if (end == value.c_str() || *end != '\0' || errno == ERANGE || parsed < minimum || parsed > maximum)
		return false;

	out = static_cast<int>(parsed);
	return true;
}

bool parseFloat(const std::string& text, float minimum, float maximum, float& out) {
	const std::string value = trim(text);
	if (value.empty())
		return false;

	errno = 0;
	char* end = nullptr;
	const float parsed = std::strtof(value.c_str(), &end);
	if (end == value.c_str() || *end != '\0' || errno == ERANGE)
		return false;
	if (!std::isfinite(parsed) || parsed < minimum || parsed > maximum)
		return false;

	out = parsed;
	return true;
}

bool parseOnOff(const std::string& text, bool& out) {
	const std::string value = trim(text);
	if (value == "on" || value == "1" || value == "true" || value == "yes") {
		out = true;
		return true;
	}
	if (value.empty() || value == "off" || value == "0" || value == "false" || value == "no") {
		out = false;
		return true;
	}
	return false;
}

int hexDigit(char character) {
	if (character >= '0' && character <= '9')
		return character - '0';
	if (character >= 'a' && character <= 'f')
		return character - 'a' + 10;
	if (character >= 'A' && character <= 'F')
		return character - 'A' + 10;
	return -1;
}

bool parseColor(const std::string& text, Color& out) {
	std::string value = trim(text);
	if (!value.empty() && value.front() == '#')
		value.erase(0, 1);

	// #rgb is expanded to #rrggbb.
	if (value.size() == 3)
		value = { value[0], value[0], value[1], value[1], value[2], value[2] };
	if (value.size() != 6)
		return false;

	int digits[6];
	for (int i = 0; i < 6; ++i) {
		digits[i] = hexDigit(value[i]);
		if (digits[i] < 0)
			return false;
	}

	out = {
		static_cast<uint8_t>(digits[0] * 16 + digits[1]),
		static_cast<uint8_t>(digits[2] * 16 + digits[3]),
		static_cast<uint8_t>(digits[4] * 16 + digits[5]),
		255
	};
	return true;
}

std::string colorToString(const Color& color) {
	std::ostringstream stream;
	stream << "#" << std::hex << std::setfill('0') << std::setw(2)
		<< static_cast<int>(color.r)
		<< std::setw(2) << static_cast<int>(color.g)
		<< std::setw(2) << static_cast<int>(color.b);
	return stream.str();
}

bool readFile(const std::string& path, std::string& out) {
	std::ifstream file(path, std::ios::binary);
	if (!file)
		return false;

	std::ostringstream buffer;
	buffer << file.rdbuf();
	out = buffer.str();
	return !out.empty();
}

std::string insertValue(const std::string& content, const std::string& placeholder, const std::string& value) {
	std::string modified = content;
	const size_t position = modified.find(placeholder);
	if (position != std::string::npos)
		modified.replace(position, placeholder.length(), value);
	return modified;
}

std::string insertSettings(const std::string& content) {
	// Snapshot: the main thread swaps the settings pointer while the server runs.
	const Settings current = get_settings_snapshot();

	std::string page = content;

	page = insertValue(page, "<!-- SERVER_PORT -->", std::to_string(current.SERVER_PORT));
	page = insertValue(page, "<!-- DISPLAY_WIDTH -->", std::to_string(current.DISPLAY_WIDTH));
	page = insertValue(page, "<!-- DISPLAY_HEIGHT -->", std::to_string(current.DISPLAY_HEIGHT));
	page = insertValue(page, "<!-- DRAW_BOX -->", current.DRAW_BOX ? "checked" : "");
	page = insertValue(page, "<!-- DRAW_BONES -->", current.DRAW_BONES ? "checked" : "");
	page = insertValue(page, "<!-- DRAW_HEALTH_BAR -->", current.DRAW_HEALTH_BAR ? "checked" : "");
	page = insertValue(page, "<!-- DRAW_HEAD -->", current.DRAW_HEAD ? "checked" : "");
	page = insertValue(page, "<!-- DRAW_TEXT -->", current.DRAW_TEXT ? "checked" : "");
	page = insertValue(page, "<!-- DRAW_BOX_WHEN_SPOTTED -->", current.DRAW_BOX_WHEN_SPOTTED ? "checked" : "");
	page = insertValue(page, "<!-- DRAW_BONES_WHEN_SPOTTED -->", current.DRAW_BONES_WHEN_SPOTTED ? "checked" : "");
	page = insertValue(page, "<!-- DRAW_HEALTH_BAR_WHEN_SPOTTED -->", current.DRAW_HEALTH_BAR_WHEN_SPOTTED ? "checked" : "");
	page = insertValue(page, "<!-- DRAW_HEAD_WHEN_SPOTTED -->", current.DRAW_HEAD_WHEN_SPOTTED ? "checked" : "");
	page = insertValue(page, "<!-- DRAW_TEXT_WHEN_SPOTTED -->", current.DRAW_TEXT_WHEN_SPOTTED ? "checked" : "");
	page = insertValue(page, "<!-- AIMBOT_ENABLED -->", current.AIMBOT_ENABLED ? "checked" : "");
	page = insertValue(page, "<!-- AIMBOT_ENABLED_WHEN_NOT_SPOTTED -->", current.AIMBOT_ENABLED_WHEN_NOT_SPOTTED ? "checked" : "");
	page = insertValue(page, "<!-- BOX_HEIGHT -->", std::to_string(current.BOX_HEIGHT));
	page = insertValue(page, "<!-- BOX_HEIGHT_WIDTH_RATIO -->", std::to_string(current.BOX_HEIGHT_WIDTH_RATIO));
	page = insertValue(page, "<!-- BOX_COLOR -->", colorToString(current.BOX_COLOR_struct));
	page = insertValue(page, "<!-- BOX_THICKNESS -->", std::to_string(current.BOX_THICKNESS));
	page = insertValue(page, "<!-- BONES_COLOR -->", colorToString(current.BONES_COLOR_struct));
	page = insertValue(page, "<!-- BONES_THICKNESS -->", std::to_string(current.BONES_THICKNESS));
	page = insertValue(page, "<!-- HEALTH_BAR_THICKNESS -->", std::to_string(current.HEALTH_BAR_THICKNESS));
	page = insertValue(page, "<!-- HEAD_RADIUS -->", std::to_string(current.HEAD_RADIUS));
	page = insertValue(page, "<!-- HEAD_COLOR -->", colorToString(current.HEAD_COLOR_struct));
	page = insertValue(page, "<!-- TEXT_COLOR -->", colorToString(current.TEXT_COLOR_struct));
	page = insertValue(page, "<!-- TEXT_THICKNESS -->", std::to_string(current.TEXT_THICKNESS));
	page = insertValue(page, "<!-- AIMBOT_FOV -->", std::to_string(current.AIMBOT_FOV));
	page = insertValue(page, "<!-- AIMBOT_FOV_COLOR -->", colorToString(current.AIMBOT_FOV_COLOR_struct));
	page = insertValue(page, "<!-- AIMBOT_DRAW_FOV -->", current.AIMBOT_DRAW_FOV ? "checked" : "");
	page = insertValue(page, "<!-- AIMBOT_SMOOTH -->", std::to_string(current.AIMBOT_SMOOTH));

	// Wallhack toggle key selection
	page = insertValue(page, "<!-- WALLHACK_KEY_L_SELECTED -->", current.WALLHACK_TOGGLE_KEY == 0x4C ? "selected" : "");
	page = insertValue(page, "<!-- WALLHACK_KEY_K_SELECTED -->", current.WALLHACK_TOGGLE_KEY == 0x4B ? "selected" : "");
	page = insertValue(page, "<!-- WALLHACK_KEY_F1_SELECTED -->", current.WALLHACK_TOGGLE_KEY == 0x70 ? "selected" : "");
	page = insertValue(page, "<!-- WALLHACK_KEY_F2_SELECTED -->", current.WALLHACK_TOGGLE_KEY == 0x71 ? "selected" : "");
	page = insertValue(page, "<!-- WALLHACK_KEY_HOME_SELECTED -->", current.WALLHACK_TOGGLE_KEY == 0x24 ? "selected" : "");
	page = insertValue(page, "<!-- WALLHACK_KEY_END_SELECTED -->", current.WALLHACK_TOGGLE_KEY == 0x23 ? "selected" : "");

	// Proximity alert settings
	page = insertValue(page, "<!-- PROXIMITY_ALERT_ENABLED -->", current.PROXIMITY_ALERT_ENABLED ? "checked" : "");
	page = insertValue(page, "<!-- PROXIMITY_ALERT_DRAW -->", current.PROXIMITY_ALERT_DRAW ? "checked" : "");
	page = insertValue(page, "<!-- PROXIMITY_ALERT_RADIUS -->", std::to_string(current.PROXIMITY_ALERT_RADIUS));
	page = insertValue(page, "<!-- PROXIMITY_ALERT_INTERVAL -->", std::to_string(current.PROXIMITY_ALERT_INTERVAL));
	page = insertValue(page, "<!-- PROXIMITY_ALERT_COLOR -->", colorToString(current.PROXIMITY_ALERT_COLOR_struct));

	return page;
}

void sendResponse(SOCKET clientSocket, int status, const char* reason,
	const std::string& contentType, const std::string& body) {
	std::ostringstream header;
	header << "HTTP/1.1 " << status << ' ' << reason << "\r\n"
		<< "Content-Type: " << contentType << "\r\n"
		<< "Content-Length: " << body.size() << "\r\n"
		<< "Connection: close\r\n"
		<< "Cache-Control: no-store\r\n"
		<< "Access-Control-Allow-Origin: *\r\n"
		<< "\r\n";

	const std::string response = header.str() + body;

	size_t sent = 0;
	while (sent < response.size()) {
		const int chunk = static_cast<int>(std::min<size_t>(response.size() - sent, 16 * 1024));
		const int written = send(clientSocket, response.data() + sent, chunk, 0);
		if (written <= 0)
			break;
		sent += static_cast<size_t>(written);
	}
}

void sendText(SOCKET clientSocket, int status, const char* reason, const std::string& text) {
	sendResponse(clientSocket, status, reason, "text/plain; charset=utf-8", text);
}

void sendHtml(SOCKET clientSocket, int status, const char* reason, const std::string& html) {
	sendResponse(clientSocket, status, reason, "text/html; charset=utf-8", html);
}

} // namespace

// Parses the multipart/form-data body produced by src/server/index.html.
//
// Values are parsed defensively: a malformed or empty field used to reach
// std::stof/std::stoi and throw out of the server thread, which terminated the
// whole application (and showed up in the browser as a failed request).
struct ParseResult {
	bool ok = true;
	std::string error;
};

ParseResult parseSettings(const std::string& postData, Settings& new_settings) {
	ParseResult result;

	std::istringstream stream(postData);
	std::string line;
	while (std::getline(stream, line)) {
		if (line.find("name=") == std::string::npos)
			continue;

		// Content-Disposition: form-data; name="<name>"
		const size_t name_begin = line.find("name=\"");
		if (name_begin == std::string::npos)
			continue;

		const size_t value_begin = name_begin + 6;
		const size_t name_end = line.find('"', value_begin);
		if (name_end == std::string::npos)
			continue;

		const std::string name = line.substr(value_begin, name_end - value_begin);

		// The value is separated from the part headers by an empty line.
		if (!std::getline(stream, line))
			break;
		if (!std::getline(stream, line))
			break;

		// Strip the CR of the CRLF pair, but never a real character: the value
		// can legitimately be empty (and must then fail validation instead of
		// slicing a digit away).
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		const std::string value = line;

		auto invalid = [&](const char* expected) {
			ParseResult failure;
			failure.ok = false;
			failure.error = "Invalid value for " + name + ": \"" + value + "\" (expected " + expected + ")";
			return failure;
		};

		if (name == "DRAW_BOX") {
			if (!parseOnOff(value, new_settings.DRAW_BOX)) return invalid("on/off");
		}
		else if (name == "DRAW_BONES") {
			if (!parseOnOff(value, new_settings.DRAW_BONES)) return invalid("on/off");
		}
		else if (name == "DRAW_HEALTH_BAR") {
			if (!parseOnOff(value, new_settings.DRAW_HEALTH_BAR)) return invalid("on/off");
		}
		else if (name == "DRAW_HEAD") {
			if (!parseOnOff(value, new_settings.DRAW_HEAD)) return invalid("on/off");
		}
		else if (name == "DRAW_TEXT") {
			if (!parseOnOff(value, new_settings.DRAW_TEXT)) return invalid("on/off");
		}
		else if (name == "DRAW_BOX_WHEN_SPOTTED") {
			if (!parseOnOff(value, new_settings.DRAW_BOX_WHEN_SPOTTED)) return invalid("on/off");
		}
		else if (name == "DRAW_BONES_WHEN_SPOTTED") {
			if (!parseOnOff(value, new_settings.DRAW_BONES_WHEN_SPOTTED)) return invalid("on/off");
		}
		else if (name == "DRAW_HEALTH_BAR_WHEN_SPOTTED") {
			if (!parseOnOff(value, new_settings.DRAW_HEALTH_BAR_WHEN_SPOTTED)) return invalid("on/off");
		}
		else if (name == "DRAW_HEAD_WHEN_SPOTTED") {
			if (!parseOnOff(value, new_settings.DRAW_HEAD_WHEN_SPOTTED)) return invalid("on/off");
		}
		else if (name == "DRAW_TEXT_WHEN_SPOTTED") {
			if (!parseOnOff(value, new_settings.DRAW_TEXT_WHEN_SPOTTED)) return invalid("on/off");
		}
		else if (name == "AIMBOT_ENABLED") {
			if (!parseOnOff(value, new_settings.AIMBOT_ENABLED)) return invalid("on/off");
		}
		else if (name == "AIMBOT_ENABLED_WHEN_NOT_SPOTTED") {
			if (!parseOnOff(value, new_settings.AIMBOT_ENABLED_WHEN_NOT_SPOTTED)) return invalid("on/off");
		}
		else if (name == "BOX_HEIGHT") {
			if (!parseFloat(value, 0.0f, 1000.0f, new_settings.BOX_HEIGHT)) return invalid("a number between 0 and 1000");
		}
		else if (name == "BOX_HEIGHT_WIDTH_RATIO") {
			if (!parseFloat(value, 0.0f, 10.0f, new_settings.BOX_HEIGHT_WIDTH_RATIO)) return invalid("a number between 0 and 10");
		}
		else if (name == "BOX_COLOR") {
			if (!parseColor(value, new_settings.BOX_COLOR_struct)) return invalid("a color such as #00AAFF");
		}
		else if (name == "BOX_THICKNESS") {
			if (!parseFloat(value, 0.0f, 100.0f, new_settings.BOX_THICKNESS)) return invalid("a number between 0 and 100");
		}
		else if (name == "BONES_COLOR") {
			if (!parseColor(value, new_settings.BONES_COLOR_struct)) return invalid("a color such as #00AAFF");
		}
		else if (name == "BONES_THICKNESS") {
			if (!parseFloat(value, 0.0f, 100.0f, new_settings.BONES_THICKNESS)) return invalid("a number between 0 and 100");
		}
		else if (name == "HEALTH_BAR_THICKNESS") {
			if (!parseFloat(value, 0.0f, 100.0f, new_settings.HEALTH_BAR_THICKNESS)) return invalid("a number between 0 and 100");
		}
		else if (name == "HEAD_RADIUS") {
			if (!parseFloat(value, 0.0f, 1000.0f, new_settings.HEAD_RADIUS)) return invalid("a number between 0 and 1000");
		}
		else if (name == "HEAD_COLOR") {
			if (!parseColor(value, new_settings.HEAD_COLOR_struct)) return invalid("a color such as #00AAFF");
		}
		else if (name == "TEXT_COLOR") {
			if (!parseColor(value, new_settings.TEXT_COLOR_struct)) return invalid("a color such as #00AAFF");
		}
		else if (name == "TEXT_THICKNESS") {
			if (!parseFloat(value, 0.0f, 100.0f, new_settings.TEXT_THICKNESS)) return invalid("a number between 0 and 100");
		}
		else if (name == "AIMBOT_FOV") {
			if (!parseFloat(value, 0.0f, 10000.0f, new_settings.AIMBOT_FOV)) return invalid("a number between 0 and 10000");
		}
		else if (name == "AIMBOT_DRAW_FOV") {
			if (!parseOnOff(value, new_settings.AIMBOT_DRAW_FOV)) return invalid("on/off");
		}
		else if (name == "AIMBOT_FOV_COLOR") {
			if (!parseColor(value, new_settings.AIMBOT_FOV_COLOR_struct)) return invalid("a color such as #00AAFF");
		}
		else if (name == "AIMBOT_SMOOTH") {
			if (!parseInt(value, 0, 1000, new_settings.AIMBOT_SMOOTH)) return invalid("an integer between 0 and 1000");
		}
		else if (name == "SERVER_PORT") {
			if (!parseInt(value, 1, 65535, new_settings.SERVER_PORT)) return invalid("a port between 1 and 65535");
		}
		else if (name == "DISPLAY_WIDTH") {
			if (!parseInt(value, 1, 10000, new_settings.DISPLAY_WIDTH)) return invalid("an integer between 1 and 10000");
		}
		else if (name == "DISPLAY_HEIGHT") {
			if (!parseInt(value, 1, 10000, new_settings.DISPLAY_HEIGHT)) return invalid("an integer between 1 and 10000");
		}
		else if (name == "WALLHACK_TOGGLE_KEY") {
			if (!parseInt(value, 1, 254, new_settings.WALLHACK_TOGGLE_KEY)) return invalid("a virtual-key code between 1 and 254");
		}
		else if (name == "PROXIMITY_ALERT_ENABLED") {
			if (!parseOnOff(value, new_settings.PROXIMITY_ALERT_ENABLED)) return invalid("on/off");
		}
		else if (name == "PROXIMITY_ALERT_DRAW") {
			if (!parseOnOff(value, new_settings.PROXIMITY_ALERT_DRAW)) return invalid("on/off");
		}
		else if (name == "PROXIMITY_ALERT_RADIUS") {
			if (!parseFloat(value, 0.0f, 10000.0f, new_settings.PROXIMITY_ALERT_RADIUS)) return invalid("a number between 0 and 10000");
		}
		else if (name == "PROXIMITY_ALERT_INTERVAL") {
			if (!parseFloat(value, 0.0f, 3600.0f, new_settings.PROXIMITY_ALERT_INTERVAL)) return invalid("a number between 0 and 3600");
		}
		else if (name == "PROXIMITY_ALERT_COLOR") {
			if (!parseColor(value, new_settings.PROXIMITY_ALERT_COLOR_struct)) return invalid("a color such as #00AAFF");
		}
	}

	return result;
}

// Server class implementation

Server::Server()
	: serverSocket(INVALID_SOCKET), port(0), running(false), stop_requested(false), wsa_started(false) {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "[server] WSAStartup failed\n";
		return;
	}
	wsa_started = true;

	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) {
		std::cerr << "[server] socket() failed: " << WSAGetLastError() << "\n";
	}
}

Server::~Server() {
	if (serverSocket != INVALID_SOCKET) {
		closesocket(serverSocket);
		serverSocket = INVALID_SOCKET;
	}

	if (wsa_started) {
		WSACleanup();
		wsa_started = false;
	}
}

void Server::start(int requested_port) {
	if (serverSocket == INVALID_SOCKET) {
		std::cerr << "[server] no socket, cannot start\n";
		return;
	}

	port = requested_port;

	// Allows restarting the tool without waiting for TIME_WAIT.
	BOOL reuse = TRUE;
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
		reinterpret_cast<const char*>(&reuse), sizeof(reuse));

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(static_cast<unsigned short>(port));

	if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
		// Killing the process here used to leave the user with a dead cheat and
		// no explanation when the port was already in use.
		std::cerr << "[server] bind failed on port " << port << ": " << WSAGetLastError()
			<< " (is another instance running?)\n";
		return;
	}

	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "[server] listen failed: " << WSAGetLastError() << "\n";
		return;
	}

	std::cout << "[server] listening on http://127.0.0.1:" << port << "/\n";

	// stop() may have been called while we were still setting up: honour it.
	if (stop_requested) {
		shutdown(serverSocket, SD_BOTH);
		closesocket(serverSocket);
		serverSocket = INVALID_SOCKET;
		return;
	}

	running = true;
	while (!stop_requested) {
		SOCKET clientSocket = accept(serverSocket, NULL, NULL);
		if (clientSocket == INVALID_SOCKET) {
			if (stop_requested)
				break;
			std::cerr << "[server] accept failed: " << WSAGetLastError() << "\n";
			break;
		}

		handleClient(clientSocket);
	}
	running = false;

	shutdown(serverSocket, SD_BOTH);
	closesocket(serverSocket);
	serverSocket = INVALID_SOCKET;
}

void Server::handleClient(SOCKET clientSocket) {
	// Never let a stalled client block the whole server.
	const DWORD timeout = kClientTimeoutMs;
	setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
	setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));

	// Read the request until the header block and the declared body are complete.
	std::string request;
	bool complete = false;
	char chunk[4096];

	while (request.size() < kMaxRequestBytes) {
		const int received = recv(clientSocket, chunk, static_cast<int>(sizeof(chunk)), 0);
		if (received <= 0)
			break;

		request.append(chunk, static_cast<size_t>(received));

		const size_t header_end = request.find("\r\n\r\n");
		if (header_end == std::string::npos)
			continue;

		std::string lower_headers = request.substr(0, header_end);
		std::transform(lower_headers.begin(), lower_headers.end(), lower_headers.begin(),
			[](unsigned char character) { return static_cast<char>(std::tolower(character)); });

		size_t content_length = 0;
		const size_t length_position = lower_headers.find("content-length:");
		if (length_position != std::string::npos) {
			const char* digits = request.c_str() + length_position + 15;
			content_length = static_cast<size_t>(std::strtoul(digits, nullptr, 10));
		}

		if (request.size() >= header_end + 4 + content_length) {
			complete = true;
			break;
		}
	}

	if (!complete) {
		sendText(clientSocket, 400, "Bad Request", "ERROR: incomplete or oversized request\n");
		shutdown(clientSocket, SD_SEND);
		closesocket(clientSocket);
		return;
	}

	const size_t header_end = request.find("\r\n\r\n");
	const std::string headers = request.substr(0, header_end);
	const std::string body = request.substr(header_end + 4);

	const size_t request_line_end = headers.find("\r\n");
	const std::string request_line = headers.substr(0, request_line_end);

	std::istringstream request_stream(request_line);
	std::string method;
	std::string path;
	std::string version;
	request_stream >> method >> path >> version;

	try {
		if (method == "OPTIONS") {
			// CORS preflight, needed when the page is opened from disk.
			sendResponse(clientSocket, 204, "No Content", "text/plain; charset=utf-8", "");
		}
		else if (method == "POST") {
			Settings new_settings = get_settings_snapshot();

			const ParseResult result = parseSettings(body, new_settings);
			if (!result.ok) {
				std::cerr << "[server] POST rejected: " << result.error << "\n";
				sendText(clientSocket, 400, "Bad Request", "ERROR: " + result.error + "\n");
			}
			else if (!modify_settings(new_settings)) {
				std::cerr << "[server] POST rejected by validation\n";
				sendText(clientSocket, 400, "Bad Request", "ERROR: settings failed validation\n");
			}
			else {
				std::cout << "[server] POST / -> 200, settings updated\n";
				sendText(clientSocket, 200, "OK", "OK: settings updated\n");
			}
		}
		else {
			const std::string page_path = std::string(WORKING_DIR) + "src\\server\\index.html";

			std::string page;
			if (!readFile(page_path, page)) {
				std::cerr << "[server] cannot read " << page_path << "\n";
				sendText(clientSocket, 500, "Internal Server Error",
					"ERROR: cannot read " + page_path + "\n");
			}
			else {
				sendHtml(clientSocket, 200, "OK", insertSettings(page));
			}
		}
	}
	catch (const std::exception& error) {
		// An unexpected failure must not take the whole application down.
		std::cerr << "[server] request failed: " << error.what() << "\n";
		sendText(clientSocket, 500, "Internal Server Error",
			std::string("ERROR: ") + error.what() + "\n");
	}
	catch (...) {
		std::cerr << "[server] request failed with an unknown error\n";
		sendText(clientSocket, 500, "Internal Server Error", "ERROR: unknown server error\n");
	}

	shutdown(clientSocket, SD_SEND);
	closesocket(clientSocket);
}

void Server::stop() {
	stop_requested = true;

	// Connect to ourselves to unblock a pending accept(). This is best effort:
	// if the server has not bound its port yet, start() will notice the stop
	// request and shut down on its own.
	SOCKET wake = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (wake == INVALID_SOCKET)
		return;

	sockaddr_in address{};
	address.sin_family = AF_INET;
	address.sin_port = htons(static_cast<unsigned short>(port));
	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	connect(wake, reinterpret_cast<SOCKADDR*>(&address), sizeof(address));
	closesocket(wake);
}

bool Server::is_running() {
	return running;
}
