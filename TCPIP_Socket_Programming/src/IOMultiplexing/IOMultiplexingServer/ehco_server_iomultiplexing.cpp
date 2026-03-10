#include <WinSock2.h>
#include <WS2tcpip.h>
#include "..\..\error.h"
#include <stdio.h>

#define MAX_PACKETLEN 1024
#define nPort 3600

int main(int argc, char* argv[]) {
	int retval;
	WSADATA wsa;
	SOCKET listensock, clientsock;
	struct sockaddr_in serveraddr, clientaddr;
	char recvBuf[MAX_PACKETLEN];
	int readn, writen;
	int addr_len;
	int fd_num;

	SOCKET sockfd;
	unsigned int i = 0;
	
	fd_set readfds, allfds;
	
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		exit_with_error("WSAStartup()");
	}

	listensock = socket(AF_INET, SOCK_STREAM, 0);
	if (listensock == INVALID_SOCKET) {
		exit_with_error("socket()");
	}

	memset(&serveraddr, 0x00, sizeof(struct sockaddr_in));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(nPort);
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(listensock, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
		exit_with_error("bind()");
	}
	retval = listen(listensock, 5);
	if (retval == SOCKET_ERROR) {
		exit_with_error("listen()");
	}

	FD_ZERO(&readfds);
	FD_SET(listensock, &readfds);

	while (1) {
		allfds = readfds;
		//printf("allfds : %d\nreadfds : %d\n", allfds.fd_count, readfds.fd_count);

		fd_num = select(0, &allfds, NULL, NULL, NULL);
		if (FD_ISSET(listensock, &allfds)) {
			addr_len = sizeof(struct sockaddr_in);
			clientsock = accept(listensock, (struct sockaddr*)&clientaddr, &addr_len);
			if (clientsock == INVALID_SOCKET) {
				printf("accept Error\n");
				continue;
			}
			FD_SET(clientsock, &readfds);
			continue;
		}
		for (int i = 0; i < allfds.fd_count; i++) {
			if (allfds.fd_array[i] == listensock) continue;
			sockfd = allfds.fd_array[i];
			memset(recvBuf, 0x00, MAX_PACKETLEN);
			readn = recv(sockfd, recvBuf, MAX_PACKETLEN, 0);
			printf("recieved : (fdNumber)%d (message)%s\n", i, recvBuf);
			if (readn<=0) {
				closesocket(sockfd);
				FD_CLR(sockfd, &readfds);
				//i--;
			}
			else {
				writen = send(sockfd, recvBuf, readn, 0);
				printf("sended : (fdNumber)%d (message)%s\n\n", i, recvBuf);
			}
		}
	}

	closesocket(listensock);
	WSACleanup();
	return 0;
}