#include "Server.h"

SimpleTcpServer::SimpleTcpServer(int port)
    : m_port(port), m_listenSocket(INVALID_SOCKET), m_isRunning(false) {
}

SimpleTcpServer::~SimpleTcpServer() {
    Stop();
}

bool SimpleTcpServer::Initialize() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return false;
#endif

    // 1. 소켓 생성 (IPv4, TCP)
    m_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenSocket == INVALID_SOCKET) return false;

    // 2. 주소 설정
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // 모든 인터페이스로부터 접속 허용
    serverAddr.sin_port = htons(static_cast<unsigned short>(m_port));

    // 3. 바인드 (포트 점유)
    if (bind(m_listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        Stop();
        return false;
    }

    // 4. 리스닝 (대기열 설정)
    if (listen(m_listenSocket, SOMAXCONN) == -1) {
        Stop();
        return false;
    }

    return true;
}

bool SimpleTcpServer::Start() {
    if (!Initialize()) {
        std::cerr << "서버 초기화 실패!" << std::endl;
        return false;
    }
    m_isRunning = true;
    std::cout << "서버가 " << m_port << " 포트에서 대기 중입니다..." << std::endl;
    return true;
}

void SimpleTcpServer::Run() {
    while (m_isRunning) {
        sockaddr_in clientAddr{};
        int clientSize = sizeof(clientAddr);

        // 5. 클라이언트 접속 수락 (Accept)
        SOCKET clientSocket = accept(m_listenSocket, (struct sockaddr*)&clientAddr, &clientSize);
        if (clientSocket != INVALID_SOCKET) {
            std::cout << "클라이언트 접속 성공!" << std::endl;
            HandleClient(clientSocket); // 실제 데이터 통신
        }
    }
}

void SimpleTcpServer::HandleClient(SOCKET clientSocket) {
    char buffer[1024];
    while (true) {
        // 6. 데이터 수신 (Receive)
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) break; // 연결 종료 또는 에러

        // 7. 데이터 송신 (Echo back)
        send(clientSocket, buffer, bytesReceived, 0);
    }

#ifdef _WIN32
    closesocket(clientSocket);
#else
    close(clientSocket);
#endif
    std::cout << "클라이언트 접속 종료." << std::endl;
}

void SimpleTcpServer::Stop() {
    m_isRunning = false;
    if (m_listenSocket != INVALID_SOCKET) {
#ifdef _WIN32
        closesocket(m_listenSocket);
        WSACleanup();
#else
        close(m_listenSocket);
#endif
        m_listenSocket = INVALID_SOCKET;
    }
}