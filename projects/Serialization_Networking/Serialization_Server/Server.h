#pragma once
#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <vector>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#endif

class SimpleTcpServer {
public:
    SimpleTcpServer(int port);
    ~SimpleTcpServer();

    bool Start();                // 서버 시작
    void Stop();                 // 서버 중지
    void Run();                  // 루프 실행 (클라이언트 수락 및 에코)

private:
    int m_port;
    SOCKET m_listenSocket;
    bool m_isRunning;

    bool Initialize();           // 소켓 초기화 및 바인드
    void HandleClient(SOCKET clientSocket); // 클라이언트 통신 로직
};