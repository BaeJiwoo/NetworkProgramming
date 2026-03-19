#include <stdio.h>
#include <WinSock2.h>
#include "..\..\error.h"
#include <Windows.h>
#pragma comment (lib, "ws2_32.lib")

#define MAXLINE 1024
#define PORT 9000

struct SocketInfo {
	OVERLAPPED overlapped;
	SOCKET fd;
	char buf[MAXLINE];
	int readn;
	int writen;
	WSABUF wsabuf;
};

HANDLE g_hHandle[20];
int main(int argc, char** argv) {
	WSADATA wsa;
	struct sockaddr_in addr;
	struct SocketInfo* sInfo;
	SOCKET listensock, clientsock;
	int readn;
	int addrlen;
	if (argc != 2) {
		printf("Usage : %s [port num]\n", argv[0]);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		exit_with_error("WSAStartup()");
	}

	if ((listensock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		exit_with_error("socket()");
	}

	memset(&addr, 0x00, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(listensock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		exit_with_error("bind()");
	}

	if (listen(listensock, 5) == SOCKET_ERROR) {
		exit_with_error("listen()");
	}

	CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	int Threadn = sysInfo.dwNumberOfProcessors * 2;
	DWORD threadid;
	for (int i = 0; i < Threadn; i++) {
		g_hHandle[i] = CreateThread(NULL, 0, Thread_func, 0, 0, &threadid);
	}

	while (1) {
		addrlen = sizeof(addr);
		clientsock = accept(listensock, (struct sockaddr*)&addr, &addrlen);
		if (clientsock == INVALID_SOCKET) {
			exit_with_error("accept()");
		}
		sInfo = (SocketInfo*)malloc(sizeof(struct SocketInfo));
		sInfo->fd = clientsock;
		sInfo->readn = 0;
		sInfo->writen = 0;
		sInfo->wsabuf.buf = sInfo->buf;
		sInfo->wsabuf.len = MAXLINE;

		CreateIoCompletionPort(clientsock, g_hcp, )
	}
}