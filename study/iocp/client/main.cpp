#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_BUFFER 1024

int main(int argc, char* argv[]) {
    // 1. 명령줄 인수 체크
    if (argc < 3) {
        std::cout << "사용법: " << argv[0] << " <IP주소> <포트번호>" << std::endl;
        std::cout << "예시: " << argv[0] << " 127.0.0.1 9000" << std::endl;
        return 1;
    }

    const char* serverIp = argv[1];
    int serverPort = std::stoi(argv[2]);

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "Winsock 초기화 실패" << std::endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cout << "소켓 생성 실패: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons((unsigned short)serverPort);

    // IP 주소 변환
    if (inet_pton(AF_INET, serverIp, &serverAddr.sin_addr) != 1) {
        std::cout << "잘못된 IP 주소 형식입니다: " << serverIp << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "서버 접속 시도 중... [" << serverIp << ":" << serverPort << "]" << std::endl;

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "접속 실패! 에러 코드: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "서버에 연결되었습니다!" << std::endl;
    std::cout << "보낼 메시지를 입력하세요 (종료하려면 'quit' 입력)" << std::endl;

    char buffer[MAX_BUFFER];
    while (true) {
        std::string input;
        std::cout << ">> ";
        std::getline(std::cin, input);

        if (input == "quit") break;
        if (input.empty()) continue;

        // 데이터 송신
        if (send(clientSocket, input.c_str(), (int)input.length(), 0) == SOCKET_ERROR) {
            std::cout << "데이터 전송 중 연결이 끊겼습니다." << std::endl;
            break;
        }

        // 서버로부터 에코 수신
        ZeroMemory(buffer, MAX_BUFFER);
        int recvLen = recv(clientSocket, buffer, MAX_BUFFER - 1, 0);

        if (recvLen > 0) {
            std::cout << "<< Server Echo: " << buffer << " [" << recvLen << " bytes]" << std::endl;
        }
        else {
            std::cout << "서버와의 연결이 종료되었습니다." << std::endl;
            break;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}