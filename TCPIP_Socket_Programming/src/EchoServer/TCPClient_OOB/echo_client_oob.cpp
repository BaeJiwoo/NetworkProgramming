#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include "..\..\error.h"
#pragma comment(lib, "ws2_32.lib")
char* SERVERIP = (char*)"0:0:0:0:0:0:0:1";
#define SERVERPORT 9000
#define BUFSIZE 512


int main(int argc, char* argv[]) {
	int retval;
	WSADATA wsa;


	// 윈도우 소켓 이벤트 관련
	WSANETWORKEVENTS networkEvents;
	int eventIndex;
	int iOptVal = TRUE;
	int iOptLen = sizeof(int);
	char buf[BUFSIZE + 1];

	WSAEVENT sEvent;


	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;


	SOCKET clientsock = socket(AF_INET6, SOCK_STREAM, 0);
	if (clientsock == INVALID_SOCKET) {
		exit_with_error("socket()");
	}

	// connect()
	struct sockaddr_in6 serveraddr;
	memset(&serveraddr, 0x00, sizeof(sockaddr_in6));
	serveraddr.sin6_family = AF_INET6;
	serveraddr.sin6_port = htons(SERVERPORT);
	inet_pton(AF_INET6, SERVERIP, &serveraddr.sin6_addr);
	retval = connect(clientsock, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr_in6));
	if (retval == SOCKET_ERROR) {
		exit_with_error("connect()");
	}

	sEvent = WSACreateEvent();
	if (WSAEventSelect(clientsock, sEvent, FD_READ | FD_OOB) == SOCKET_ERROR) {
		WSACleanup();
		exit_with_error("WSAEventSelect()");
	}


	//데이터 읽기
	int len;

	while (1) {
		// [전송 파트] 먼저 사용자에게 입력을 받습니다.
		printf("\n보낼 데이터 : ");
		if (fgets(buf, BUFSIZE, stdin) == NULL) break;
		len = strlen(buf);
		buf[strcspn(buf, "\n")] = '\0';
		if (strlen(buf) == 0) continue;

		retval = send(clientsock, buf, (int)strlen(buf), 0);
		if (retval == SOCKET_ERROR) break;
		printf("[TCP클라이언트] %d바이트 전송 완료. 서버 응답 대기 중...\n", retval);


		eventIndex = WSAWaitForMultipleEvents(1, &sEvent,
			FALSE, WSA_INFINITE, FALSE);
		printf("READ DATA \n");
		if (eventIndex == WSA_WAIT_FAILED) {
			printf("EVENT Wait Failed\n");
		}
		if (WSAEnumNetworkEvents(clientsock, sEvent, &networkEvents) == SOCKET_ERROR) {
			printf("EVENT Type Error\n");
		}
		if (networkEvents.lNetworkEvents & FD_READ) {
			memset(buf, 0x00, BUFSIZE);
			retval = recv(clientsock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) break;
			else if (retval == 0) break;
			buf[retval] = '\0';
			printf("[TCP서버] %s %d바이트\n", buf, retval);
		}
		if (networkEvents.lNetworkEvents & FD_OOB) {
			memset(buf, 0x00, BUFSIZE);
			retval = recv(clientsock, buf, BUFSIZE, MSG_OOB);
			if (retval == SOCKET_ERROR) break;
			else if (retval == 0) break;
			buf[retval] = '\0';
			printf("[OOB] [TCP서버] %s %d바이트\n", buf, retval);
		}


		//printf("\n 보낼 데이터 : ");
		//if (fgets(buf, BUFSIZE + 1, stdin) == NULL) break;
		//size_t len = strlen(buf);
		//if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
		//if (strlen(buf) == 0) break;

		//retval = send(clientsock, buf, (int)strlen(buf), 0);
		//if (retval == SOCKET_ERROR) return 1;

		//printf("[TCP클라이언트] %d바이트 전송\n", retval);

		//// recieve
		//retval = recv(clientsock, buf, len, 0);
		//if (retval == SOCKET_ERROR) break;
		//else if (retval == 0) break;
		//buf[retval] = '\0';

		//// data print
		//printf("[TCP서버] %s %d바이트\n", buf, retval);
	}

	closesocket(clientsock);
	WSACleanup();
	return 0;
}