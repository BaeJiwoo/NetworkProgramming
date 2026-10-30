#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <stdio.h>
#include "..\..\error.h"

#define PORT 9000
#define BUFSIZE 512
void HandleClient(SOCKET clientsock) {
	char buf[BUFSIZE + 1];
	int ret;
	while (1) {
		ret = recv(clientsock, buf, BUFSIZE, 0);
		buf[ret] = '\0';
		if (ret <= 0) break;
		printf("recieved : %s\n", buf);
		ret = send(clientsock, buf, ret, 0);
		if (ret <= 0) break;
		printf("sended : %s\n", buf);
	}
	closesocket(clientsock);
}

int main(int argc, char* argv[]) {
	int retval; 

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		exit_with_error("WSAStartup()");
	}
	

	SOCKET listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listensock == INVALID_SOCKET) exit_with_error("socket()");

	sockaddr_in serveraddr;
	memset(&serveraddr, 0x00, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	serveraddr.sin_addr.S_un.S_addr = INADDR_ANY;
	
	retval = bind(listensock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) exit_with_error("bind()");

	retval = listen(listensock, 5);
	if (retval == SOCKET_ERROR) exit_with_error("bind()");

	sockaddr_in clientaddr;
	memset(&clientaddr, 0x00, sizeof(serveraddr));
	int clientaddrlen = sizeof(clientaddr);
	while (1) {
		SOCKET clientsock = accept(listensock, (struct sockaddr*)&clientaddr, &clientaddrlen);
		if (clientsock == INVALID_SOCKET) exit_with_error("accept()"); 
		else {
			// fork() 대신 스레드 생성
			std::thread(HandleClient, clientsock).detach();
		}
	}

	closesocket(listensock);
	WSACleanup();
}