#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#pragma comment(lib, "ws2_32.lib")
char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE 512

int main(int argc, char* argv[]) {
	int retval;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;


	SOCKET clientsock = socket(AF_INET, SOCK_STREAM, 0);
	if (clientsock == INVALID_SOCKET) return 1;

	// connect()
	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVERPORT);
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	retval = connect(clientsock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) return 1;

	//데이터 읽기
	char buf[BUFSIZE + 1];
	int len;

	while (1) {
		printf("\n 보낼 데이터 : ");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL) break;
		len = (int)strlen(buf);
		if (buf[len - 1] == '\n') buf[len - 1] = '\0';
		if (strlen(buf) == 0) break;

		retval = send(clientsock, buf, (int)strlen(buf), 0);
		if (retval == SOCKET_ERROR) return 1;

		printf("[TCP클라이언트] %d바이트 전송\n", retval);

		// recieve
		retval = recv(clientsock, buf, len, 0);
		if (retval == SOCKET_ERROR) break;
		else if (retval == 0) break;

		// data print
		printf("[TCP서버] %s, %d바이트\n", buf, retval);
	}

	closesocket(clientsock);
	WSACleanup();
	return 0;
}