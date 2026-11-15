#pragma once

#include <WinSock2.h>
#include <atomic>
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
	int port;
	std::atomic<bool> running;
	std::atomic<bool> stop_requested;
	bool wsa_started;
};
