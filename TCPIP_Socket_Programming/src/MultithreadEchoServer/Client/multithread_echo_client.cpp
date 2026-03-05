#include <WinSock2.h>
#include <WS2tcpip.h>
#include "..\..\error.h"
#include <stdio.h>

#define BUFSIZE 512

int main(int argc, char* argv[]) {
	int retval;

	// 기본값 설정
	const char* serverip = "127.0.0.1";
	int port = 9000;

	// 인수가 있을 경우 덮어쓰기
	if (argc >= 2) serverip = argv[1];
	if (argc >= 3) port = atoi(argv[2]);
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) exit_with_error("WSAStartup()");
	
	SOCKET clientsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(clientsock == INVALID_SOCKET) exit_with_error("socket()");

	sockaddr_in serveraddr;
	memset(&serveraddr, 0x00, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	inet_pton(AF_INET,serverip,&serveraddr.sin_addr);
	int serveraddrlen = sizeof(serveraddr);

	retval = connect(clientsock, (struct sockaddr*)&serveraddr, serveraddrlen);
	if(retval == SOCKET_ERROR) exit_with_error("connect()");

	char buf[BUFSIZE + 1];
	while (1) {
		fgets(buf, BUFSIZE + 1, stdin);
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = '\0';
		retval = send(clientsock, buf, strlen(buf) + 1, 0);
		if (retval <= 0) break;
		printf("sended : %s\n", buf);
		retval = recv(clientsock, buf, BUFSIZE, 0);
		if (retval <= 0) break;
		printf("recieved : %s\n", buf);
	}
	closesocket(clientsock);
	WSACleanup();
}