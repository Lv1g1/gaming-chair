#include "Server.h"

Server::Server() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
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
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    std::cout << "Server is listening for incoming connections..." << std::endl;

    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            exit(1);
        }

        std::thread clientThread(&Server::handleClient, this, clientSocket);
        clientThread.detach();
    }
}

std::string Server::readFile(const std::string& filename) {
    std::ifstream file(filename);
    std::ostringstream oss;
    if (file.is_open()) {
        oss << file.rdbuf();
        file.close();
    }
    return oss.str();
}

std::string Server::extractPostData(const std::string& request) {
    std::string postData;
    size_t pos = request.find("\r\n\r\n");
    if (pos != std::string::npos) {
        postData = request.substr(pos + 4);
    }
    return postData;
}

void Server::handleClient(SOCKET clientSocket) {
    char buffer[4096];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        std::cout << "Received from client:\n" << buffer << std::endl;

        std::string httpRequest(buffer);
        if (httpRequest.find("POST") == 0) {
            std::string postData = extractPostData(httpRequest);
            std::cout << "POST Data:\n" << postData << std::endl;

            std::string httpResponse = "HTTP/1.1 200 OK\r\n";
            httpResponse += "Content-Type: text/html\r\n";
            httpResponse += "\r\n";
            httpResponse += "<html><body><h1>POST Request Received</h1></body></html>";
            send(clientSocket, httpResponse.c_str(), httpResponse.length(), 0);
        }
        else {
            std::string httpResponse = "HTTP/1.1 200 OK\r\n";
            httpResponse += "Content-Type: text/html\r\n";
            httpResponse += "\r\n";
            httpResponse += readFile("index.html");
            send(clientSocket, httpResponse.c_str(), httpResponse.length(), 0);
        }
    }

    closesocket(clientSocket);
}
