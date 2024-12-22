#pragma once

#include <WinSock2.h>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

class Server {
public:
    Server();
    ~Server();

    void start(int port);
	void stop();
	bool is_running();

private:
    void handleClient(SOCKET clientSocket);

    SOCKET serverSocket;
	bool running;
};