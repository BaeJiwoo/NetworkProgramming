#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\..\error.h"

#pragma comment(lib, "ws2_32.lib")

#define MAXLINE 1024
#define PORT 9000

struct SOCKETINFO {
	SOCKET fd;
	char buf[MAXLINE];
	int readn;
	int writen;
};

WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
int EventTotal = 0;
struct SOCKETINFO* socketArray[WSA_MAXIMUM_WAIT_EVENTS];

// 家南 积己
int CreateSocketInfo(SOCKET s) {
	struct SOCKETINFO* sInfo;

	if ((EventArray[EventTotal] = WSACreateEvent()) == WSA_INVALID_EVENT) {
		printf("Event Failure\n");
		return -1;
	}

	sInfo = (struct SOCKETINFO*)malloc(sizeof(struct SOCKETINFO));
	if (sInfo == NULL) {
		printf("malloc() failed\n");
		return -1;
	}
	memset((void*)sInfo, 0x00, sizeof(struct SOCKETINFO));
	sInfo->fd = s;
	sInfo->readn = 0;
	sInfo->writen = 0;
	memset(sInfo->buf, 0x00, MAXLINE);
	socketArray[EventTotal] = sInfo;

	EventTotal++;
	return 1;

}

// 家南 昏力
void freeSocketInfo(int eventIndex) {
	struct SOCKETINFO* si = socketArray[eventIndex];
	int i;

	closesocket(si->fd);
	free((void*)si);
	if (WSACloseEvent(EventArray[eventIndex]) == TRUE) {
		printf("Event Close OK\n");
	}
	else {
		printf("Event Close Failure\n");
	}
	for (i = eventIndex; i < EventTotal; i++) {
		EventArray[i] = EventArray[i + 1];
		socketArray[i] = socketArray[i + 1];
	}
	EventTotal--;
}

int main(int argc, char** argv) {
	int retval;
	WSADATA wsa;
	SOCKET listensock, clientsock;
	WSANETWORKEVENTS networkEvents;
	char buf[MAXLINE];
	int eventIndex;
	int flags;

	struct SOCKETINFO* socketInfo;
	struct sockaddr_in serveraddr;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		exit_with_error("WSAStartup()");
	}


	listensock = socket(AF_INET, SOCK_STREAM, 0);
	if (listensock == INVALID_SOCKET) exit_with_error("socket()");

	memset(&serveraddr, 0x00, sizeof(sockaddr_in));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	retval = bind(listensock, (struct sockaddr*)&serveraddr, sizeof(sockaddr_in));
	if (retval == SOCKET_ERROR) exit_with_error("bind()");
	retval = listen(listensock, 5);
	if (retval == SOCKET_ERROR) exit_with_error("listen()");

	if (CreateSocketInfo(listensock) == -1) {
		return 1;
	}

	if (WSAEventSelect(listensock, EventArray[EventTotal - 1], FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR) {
		return 1;
	}

	while (1) {
		eventIndex = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE);
		if (eventIndex == WSA_WAIT_FAILED) {
			printf("Event Wait Failed\n");
		}

		if (WSAEnumNetworkEvents(socketArray[eventIndex - WSA_WAIT_EVENT_0]->fd,
			EventArray[eventIndex - WSA_WAIT_EVENT_0], &networkEvents) == SOCKET_ERROR) {
			printf("Event Type Error\n");
		}

		if (networkEvents.lNetworkEvents & FD_ACCEPT) {
			if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0) {
				break;
			}
			if ((clientsock = accept(socketArray[eventIndex -
				WSA_WAIT_EVENT_0]->fd, NULL, NULL)) == INVALID_SOCKET) {
				break;
			}
			if (EventTotal > WSA_MAXIMUM_WAIT_EVENTS) {
				printf("Too Many connections\n");
				closesocket(clientsock);
			}
			CreateSocketInfo(clientsock);

			if (WSAEventSelect(clientsock, EventArray[EventTotal - 1],
				FD_READ | FD_CLOSE) == SOCKET_ERROR) {
				return 1;
			}
		}
		if (networkEvents.lNetworkEvents & FD_READ) {
			flags = 0;
			memset(buf, 0x00, MAXLINE);
			socketInfo = socketArray[eventIndex - WSA_WAIT_EVENT_0];
			socketInfo->readn = recv(socketInfo->fd, socketInfo->buf,
				MAXLINE, 0);
			send(socketInfo->fd, socketInfo->buf, socketInfo->readn, 0);
		}
		if (networkEvents.lNetworkEvents & FD_CLOSE) {
			printf("Socket Close\n");
			freeSocketInfo(eventIndex - WSA_WAIT_EVENT_0);
		}
	}

}