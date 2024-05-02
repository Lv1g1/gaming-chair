#pragma once

#include <iostream>
#include <WinSock2.h>
#include <fstream>
#include <sstream>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

class Server {
public:
    Server();
    ~Server();

    void start(int port);

private:
    std::string readFile(const std::string& filename);
    std::string extractPostData(const std::string& request);
    void handleClient(SOCKET clientSocket);

    SOCKET serverSocket;
};