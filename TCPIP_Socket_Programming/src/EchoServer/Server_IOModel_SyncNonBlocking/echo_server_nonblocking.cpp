#include <WinSock2.h>
#include <WS2tcpip.h>
#include "..\..\error.h"

#define MAX_PACKETLEN 256
#define nPort 9000

struct client_list {
	SOCKET	fd_num;
	int		is_connected;
};

int main(int argc, char* argv[]) {
	int retval;
	WSADATA wsa;
	SOCKET listensock, clientsock;
	struct sockaddr_in serveraddr, clientaddr;
	char recvBuf[MAX_PACKETLEN];
	int readn, writen;
	int addr_len;
	int errno;
	u_long iMode;
	unsigned int i, client_num = 0;
	struct client_list clientlist[1024];

	memset((void*)clientlist, 0x00, sizeof(client_list) * 1024);

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		exit_with_error("WSAStartup()");
	}
	listensock = socket(AF_INET, SOCK_STREAM, 0);
	if(listensock == INVALID_SOCKET) exit_with_error("socket()");

	memset(&serveraddr, 0x00, sizeof(sockaddr_in));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(nPort);
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	retval = bind(listensock, (struct sockaddr*)&serveraddr, sizeof(sockaddr_in));
	if(retval == SOCKET_ERROR) exit_with_error("bind()");
	retval = listen(listensock, 5);
	if (retval == SOCKET_ERROR) exit_with_error("listen()");

	ZeroMemory(&clientaddr, sizeof(struct sockaddr_in));

	iMode = 1;
	if (ioctlsocket(listensock, FIONBIO, &iMode) != 0) {
		exit_with_error("FIONBIO setting error");
	}

	while (1) {
		addr_len = sizeof(struct sockaddr_in);
		clientsock = accept(listensock, (struct sockaddr*)&clientaddr, &addr_len);
		if (clientsock == -1) {
			errno = WSAGetLastError();
			if (errno == WSAEWOULDBLOCK) {

			}
			else {
				printf("Accept Error\n");
			}
		}
		else {
			printf("Client FD Accept %d %d\n", client_num, clientsock);
			iMode = 1;
			ioctlsocket(clientsock, FIONBIO, &iMode);

			clientlist[client_num].fd_num = clientsock;
			clientlist[client_num].is_connected = 1;
			client_num++;
		}

		for (int i = 0; i < client_num; i++) {
			if (clientlist[i].is_connected != 1) continue;
			clientsock = clientlist[i].fd_num;
			readn = recv(clientsock, recvBuf, MAX_PACKETLEN, 0);
			errno = WSAGetLastError();
			if (readn > 0) {
				writen = send(clientsock, recvBuf, readn, 0);
			}
			else if (readn == 0) {
				printf("client close\n");
				closesocket(clientsock);
				clientlist[i].is_connected = 0;
			}
			else {
				if (errno != WSAEWOULDBLOCK) {
					printf("client socket error %d\n", clientsock);
					closesocket(clientsock);
					clientlist[i].is_connected = 0;
				}
			}
		}
	}

	closesocket(listensock);
	WSACleanup();
	return 0;
}