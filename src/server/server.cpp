#include "Server.h"
#include "../settings.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

// Utility functions implementation

Color parseColor(const std::string& color) {
	// color = #rrggbb
	std::string r_str = color.substr(1, 2);
	std::string g_str = color.substr(3, 2);
	std::string b_str = color.substr(5, 2);

	int r = std::stoi(r_str, nullptr, 16);
	int g = std::stoi(g_str, nullptr, 16);
	int b = std::stoi(b_str, nullptr, 16);

	return { (uint8_t)r, (uint8_t)g, (uint8_t)b, 255 };
}

bool parseSettings(const std::string& postData, Settings &new_settings) {
	//-----------------------------141222225930487737921243636093
	//Content-Disposition: form-data; name="<name>"
	//
	//<value>
	//-----------------------------141222225930487737921243636093
	
	std::istringstream iss(postData);
	std::string line;
	while (std::getline(iss, line)) {
		if (line.find("name=") != std::string::npos) {
			std::string name = line.substr(line.find("name=") + 6);
			name = name.substr(0, name.find("\""));

			// skip the next line (empty line)
			std::getline(iss, line);

			// get the value
			std::getline(iss, line);
			
			// remove the \r at the end of the value
			line = line.substr(0, line.size() - 1);

			std::string value = line;		

			if (name == "DRAW_BOX") {
				new_settings.DRAW_BOX = value == "on";
			}
			else if (name == "DRAW_BONES") {
				new_settings.DRAW_BONES = value == "on";
			}
			else if (name == "DRAW_HEALTH_BAR") {
				new_settings.DRAW_HEALTH_BAR = value == "on";
			}
			else if (name == "DRAW_HEAD") {
				new_settings.DRAW_HEAD = value == "on";
			}
			else if (name == "DRAW_TEXT") {
				new_settings.DRAW_TEXT = value == "on";
			}
			else if (name == "DRAW_BOX_WHEN_SPOTTED") {
				new_settings.DRAW_BOX_WHEN_SPOTTED = value == "on";
			}
			else if (name == "DRAW_BONES_WHEN_SPOTTED") {
				new_settings.DRAW_BONES_WHEN_SPOTTED = value == "on";
			}
			else if (name == "DRAW_HEALTH_BAR_WHEN_SPOTTED") {
				new_settings.DRAW_HEALTH_BAR_WHEN_SPOTTED = value == "on";
			}
			else if (name == "DRAW_HEAD_WHEN_SPOTTED") {
				new_settings.DRAW_HEAD_WHEN_SPOTTED = value == "on";
			}
			else if (name == "DRAW_TEXT_WHEN_SPOTTED") {
				new_settings.DRAW_TEXT_WHEN_SPOTTED = value == "on";
			}
			else if (name == "AIMBOT_ENABLED") {
				new_settings.AIMBOT_ENABLED = value == "on";
			}
			else if (name == "AIMBOT_ENABLED_WHEN_NOT_SPOTTED") {
				new_settings.AIMBOT_ENABLED_WHEN_NOT_SPOTTED = value == "on";
			}
			else if (name == "BOX_HEIGHT") {
				new_settings.BOX_HEIGHT = std::stof(value);
			}
			else if (name == "BOX_HEIGHT_WIDTH_RATIO") {
				new_settings.BOX_HEIGHT_WIDTH_RATIO = std::stof(value);
			}
			else if (name == "BOX_COLOR") {
				new_settings.BOX_COLOR_struct = parseColor(value);
			}
			else if (name == "BOX_THICKNESS") {
				new_settings.BOX_THICKNESS = std::stof(value);
			}
			else if (name == "BONES_COLOR") {
				new_settings.BONES_COLOR_struct = parseColor(value);
			}
			else if (name == "BONES_THICKNESS") {
				new_settings.BONES_THICKNESS = std::stof(value);
			}
			else if (name == "HEALTH_BAR_THICKNESS") {
				new_settings.HEALTH_BAR_THICKNESS = std::stof(value);
			}
			else if (name == "HEAD_RADIUS") {
				new_settings.HEAD_RADIUS = std::stof(value);
			}
			else if (name == "HEAD_COLOR") {
				new_settings.HEAD_COLOR_struct = parseColor(value);
			}
			else if (name == "TEXT_COLOR") {
				new_settings.TEXT_COLOR_struct = parseColor(value);
			}
			else if (name == "TEXT_THICKNESS") {
				new_settings.TEXT_THICKNESS = std::stof(value);
			}
			/*else if (name == "AIMBOT_KEY") {
				AIMBOT_KEY = value == "VK_RBUTTON" ? VK_RBUTTON : VK_LBUTTON;
			}*/
			else if (name == "AIMBOT_FOV") {
				new_settings.AIMBOT_FOV = std::stof(value);
			}
			else if (name == "AIMBOT_DRAW_FOV") {
				new_settings.AIMBOT_DRAW_FOV = value == "on";
			}
			else if (name == "AIMBOT_FOV_COLOR") {
				new_settings.AIMBOT_FOV_COLOR_struct = parseColor(value);
			}
			else if (name == "AIMBOT_SMOOTH") {
				new_settings.AIMBOT_SMOOTH = std::stoi(value);
			}
			else if (name == "SERVER_PORT") {
				new_settings.SERVER_PORT = std::stoi(value);
			}
			else if (name == "DISPLAY_WIDTH") {
				new_settings.DISPLAY_WIDTH = std::stoi(value);
			}
			else if (name == "DISPLAY_HEIGHT") {
				new_settings.DISPLAY_HEIGHT = std::stoi(value);
			}
		}
	}
}

std::string extractPostData(const std::string& request) {
	std::string postData;
	size_t pos = request.find("\r\n\r\n");
	if (pos != std::string::npos) {
		postData = request.substr(pos + 4);
	}
	return postData;
}

std::string readFile(const std::string& filename) {
	std::ifstream file(filename); // Open the file
	std::string content; // String to hold the file content
	std::string line; // Temporary string to read lines from the file

	if (file.is_open()) { // Check if the file is opened successfully
		while (getline(file, line)) { // Read each line of the file
			content += line + "\n"; // Append each line to the content string
		}
		file.close(); // Close the file
	}

	return content; // Return the content of the file
}

std::string colorToString(const Color& color) {
	std::ostringstream oss;
	oss << "#" << std::hex << std::setfill('0') << std::setw(2)
		<< static_cast<int>(color.r)
		<< std::setw(2) << static_cast<int>(color.g)
		<< std::setw(2) << static_cast<int>(color.b);
	return oss.str();
}

std::string insertValue(const std::string& content, const std::string& placeholder, const std::string& value) {
	std::string modifiedContent = content;
	size_t pos = modifiedContent.find(placeholder);
	if (pos != std::string::npos) {
		modifiedContent.replace(pos, placeholder.length(), value);
	}
	return modifiedContent;
}

std::string insertSettings(const std::string& content) {
	std::string modifiedContent = content;

	// Insert settings here
	modifiedContent = insertValue(modifiedContent, "<!-- SERVER_PORT -->", std::to_string(settings->SERVER_PORT));
	modifiedContent = insertValue(modifiedContent, "<!-- DISPLAY_WIDTH -->", std::to_string(settings->DISPLAY_WIDTH));
	modifiedContent = insertValue(modifiedContent, "<!-- DISPLAY_HEIGHT -->", std::to_string(settings->DISPLAY_HEIGHT));
	modifiedContent = insertValue(modifiedContent, "<!-- DRAW_BOX -->", settings->DRAW_BOX ? "checked" : "");
	modifiedContent = insertValue(modifiedContent, "<!-- DRAW_BONES -->", settings->DRAW_BONES ? "checked" : "");
	modifiedContent = insertValue(modifiedContent, "<!-- DRAW_HEALTH_BAR -->", settings->DRAW_HEALTH_BAR ? "checked" : "");
	modifiedContent = insertValue(modifiedContent, "<!-- DRAW_HEAD -->", settings->DRAW_HEAD ? "checked" : "");
	modifiedContent = insertValue(modifiedContent, "<!-- DRAW_TEXT -->", settings->DRAW_TEXT ? "checked" : "");
	modifiedContent = insertValue(modifiedContent, "<!-- DRAW_BOX_WHEN_SPOTTED -->", settings->DRAW_BOX_WHEN_SPOTTED ? "checked" : "");
	modifiedContent = insertValue(modifiedContent, "<!-- DRAW_BONES_WHEN_SPOTTED -->", settings->DRAW_BONES_WHEN_SPOTTED ? "checked" : "");
	modifiedContent = insertValue(modifiedContent, "<!-- DRAW_HEALTH_BAR_WHEN_SPOTTED -->", settings->DRAW_HEALTH_BAR_WHEN_SPOTTED ? "checked" : "");
	modifiedContent = insertValue(modifiedContent, "<!-- DRAW_HEAD_WHEN_SPOTTED -->", settings->DRAW_HEAD_WHEN_SPOTTED ? "checked" : "");
	modifiedContent = insertValue(modifiedContent, "<!-- DRAW_TEXT_WHEN_SPOTTED -->", settings->DRAW_TEXT_WHEN_SPOTTED ? "checked" : "");
	modifiedContent = insertValue(modifiedContent, "<!-- AIMBOT_ENABLED -->", settings->AIMBOT_ENABLED ? "checked" : "");
	modifiedContent = insertValue(modifiedContent, "<!-- AIMBOT_ENABLED_WHEN_NOT_SPOTTED -->", settings->AIMBOT_ENABLED_WHEN_NOT_SPOTTED ? "checked" : "");
	modifiedContent = insertValue(modifiedContent, "<!-- BOX_HEIGHT -->", std::to_string(settings->BOX_HEIGHT));
	modifiedContent = insertValue(modifiedContent, "<!-- BOX_HEIGHT_WIDTH_RATIO -->", std::to_string(settings->BOX_HEIGHT_WIDTH_RATIO));
	modifiedContent = insertValue(modifiedContent, "<!-- BOX_COLOR -->", colorToString(settings->BOX_COLOR_struct));
	modifiedContent = insertValue(modifiedContent, "<!-- BOX_THICKNESS -->", std::to_string(settings->BOX_THICKNESS));
	modifiedContent = insertValue(modifiedContent, "<!-- BONES_COLOR -->", colorToString(settings->BONES_COLOR_struct));
	modifiedContent = insertValue(modifiedContent, "<!-- BONES_THICKNESS -->", std::to_string(settings->BONES_THICKNESS));
	modifiedContent = insertValue(modifiedContent, "<!-- HEALTH_BAR_THICKNESS -->", std::to_string(settings->HEALTH_BAR_THICKNESS));
	modifiedContent = insertValue(modifiedContent, "<!-- HEAD_RADIUS -->", std::to_string(settings->HEAD_RADIUS));
	modifiedContent = insertValue(modifiedContent, "<!-- HEAD_COLOR -->", colorToString(settings->HEAD_COLOR_struct));
	modifiedContent = insertValue(modifiedContent, "<!-- TEXT_COLOR -->", colorToString(settings->TEXT_COLOR_struct));
	modifiedContent = insertValue(modifiedContent, "<!-- TEXT_THICKNESS -->", std::to_string(settings->TEXT_THICKNESS));
	modifiedContent = insertValue(modifiedContent, "<!-- AIMBOT_FOV -->", std::to_string(settings->AIMBOT_FOV));
	modifiedContent = insertValue(modifiedContent, "<!-- AIMBOT_FOV_COLOR -->", colorToString(settings->AIMBOT_FOV_COLOR_struct));
	modifiedContent = insertValue(modifiedContent, "<!-- AIMBOT_DRAW_FOV -->", settings->AIMBOT_DRAW_FOV ? "checked" : "");
	modifiedContent = insertValue(modifiedContent, "<!-- AIMBOT_SMOOTH -->", std::to_string(settings->AIMBOT_SMOOTH));

	return modifiedContent;
}

// Server class implementation

Server::Server() {
    running = false;

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        WSACleanup();
        exit(1);
    }
}

Server::~Server() {
    closesocket(serverSocket);
    WSACleanup();
}

void Server::start(int port) {
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    running = true;
	while (running) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            closesocket(serverSocket);
            WSACleanup();
            exit(1);
        }

        // std::thread clientThread(&Server::handleClient, this, clientSocket);
        // clientThread.detach();
		handleClient(clientSocket);
    }
}


void Server::handleClient(SOCKET clientSocket) {
    char buffer[9182];
	int length = 0;
	// Loop until all data is received or until an error occurs
	while (true) {
		int bytesReceived = recv(clientSocket, buffer + length, sizeof(buffer) - length - 1, 0);
		length += bytesReceived;
		if (bytesReceived == 0) {
			// Connection closed by the client
			break;
		}
		else {
			// Error occurred
			// Handle error or break out of the loop
			break;
		}
	}

	if (length > 0) {
		buffer[length] = '\0';
		std::string httpRequest(buffer);
		if (httpRequest.find("POST") == 0) {
			std::string postData = extractPostData(httpRequest);
			//OutputDebugStringA(postData.c_str());

			Settings new_settings = *settings;

			parseSettings(postData, new_settings);

			// Update settings
			if (modify_settings(new_settings)) {
				std::string httpResponse = "HTTP/1.1 200 OK\r\n";
				httpResponse += "Content-Type: text/html\r\n";
				httpResponse += "\r\n";
				httpResponse += "<html><body><h1>Settings Updated</h1></body></html>";
				send(clientSocket, httpResponse.c_str(), httpResponse.length(), 0);
			}
			else {
				std::string httpResponse = "HTTP/1.1 500 Internal Server Error\r\n";
				httpResponse += "Content-Type: text/html\r\n";
				httpResponse += "\r\n";
				httpResponse += "<html><body><h1>Invalid Settings</h1></body></html>";
				send(clientSocket, httpResponse.c_str(), httpResponse.length(), 0);
			}
		}
		else {
			std::string httpResponse = "HTTP/1.1 200 OK\r\n";
			httpResponse += "Content-Type: text/html\r\n";
			httpResponse += "\r\n";

			// paht = WORKING_DIR/src/server/index.html
			std::string path = WORKING_DIR;
			path += "src\\server\\index.html";

			std::string html = readFile(path);

			html = insertSettings(html);

			httpResponse += html;
			send(clientSocket, httpResponse.c_str(), httpResponse.length(), 0);
		}
	}

    closesocket(clientSocket);
}

void Server::stop() {
	running = false;
	
	// Connect to the server to unblock the accept call
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed.\n";
		return;
	}

	SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ConnectSocket == INVALID_SOCKET) {
		std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(12345);
	serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // Use loopback address

	if (connect(ConnectSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cerr << "Unable to connect to server: " << WSAGetLastError() << std::endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	closesocket(ConnectSocket);
}

bool Server::is_running() {
	return running;
}