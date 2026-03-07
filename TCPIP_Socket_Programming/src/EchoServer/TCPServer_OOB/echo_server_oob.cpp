#include <WS2tcpip.h>
#include <WinSock2.h>
#include <stdio.h>
#include "..\..\error.h"

#define PORT 9000
#define BUFSIZE 512


int main(int argc, char* argv[]) {
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		exit_with_error("WSAStartup()");
	}

	SOCKET listensock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (listensock == INVALID_SOCKET) {
		exit_with_error("socket()");
	}

	sockaddr_in6 serveraddr;
	memset(&serveraddr, 0x00, sizeof(serveraddr));
	serveraddr.sin6_addr = in6addr_any;
	serveraddr.sin6_family = AF_INET6;
	serveraddr.sin6_port = htons(PORT);

	retval = bind(listensock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		exit_with_error("bind()");
	}

	retval = listen(listensock, 5);
	if (retval == SOCKET_ERROR) {
		exit_with_error("listen()");
	}

	sockaddr_in6 clientaddr;
	memset(&clientaddr, 0x00, sizeof(clientaddr));
	int clientaddrlen = sizeof(clientaddr);
	char buf[BUFSIZE + 1];

	// message loop
	while (1) {
		// accept
		SOCKET clientsock = accept(listensock, (struct sockaddr*)&clientaddr, &clientaddrlen);
		if (clientsock == INVALID_SOCKET) {
			exit_with_error("accept()");
		}
		int i = 1;


		while (1) {
			// recieve from client
			retval = recv(clientsock, buf, sizeof(buf), 0);
			if (retval == SOCKET_ERROR) break;
			if (retval <= 0) break;
			buf[retval] = '\0';

			printf("message recieved : %s\n",buf);

			// send to client
			if (i % 3 == 0) {
				printf("!!! 3번째 메시지: 신호 전송 !!!\n");
				char signal = '!';
				send(clientsock, &signal, 1, MSG_OOB); // 1바이트 긴급 신호
				retval = send(clientsock, buf, (int)strlen(buf), 0); // 실제 데이터는 일반 전송
				if (retval == SOCKET_ERROR) break;
			}
			retval = send(clientsock, buf, (int)strlen(buf), 0);
			if (retval == SOCKET_ERROR) break;
			if (retval <= 0) break;

			printf("message sended : %s\n\n", buf);
			i++;
		}
	}
}