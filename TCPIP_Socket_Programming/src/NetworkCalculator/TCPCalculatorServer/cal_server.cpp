#include "..\cal.h"
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define PORT 3000

int main() {int retval;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET listensock, clientsock;
	listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listensock == INVALID_SOCKET) return 1;

	sockaddr_in serveraddr;
	memset(&serveraddr, 0x00, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(PORT);
	
	retval = bind(listensock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) return 1;

	retval = listen(listensock, 5);
	if (retval == SOCKET_ERROR) return 1;

	sockaddr_in clientaddr;
	int addrlen = sizeof(clientaddr);

	
	struct cal_data rdata;
	for (;;)
	{
		clientsock = accept(listensock, (struct sockaddr*)&clientaddr, &addrlen);
		if (clientsock == INVALID_SOCKET) break;
		for (;;) {
			retval = recv(clientsock, (char*)&rdata, sizeof(rdata), 0);
			if (retval == 0) break;
			if (retval == SOCKET_ERROR) break;

			rdata.left_num = ntohl(rdata.left_num);
			rdata.right_num = ntohl(rdata.right_num);

			calculate(&rdata);
			printf("Processed: %d %c %d = %d (Error: %d)\n",
				rdata.left_num, rdata.op, rdata.right_num, rdata.result, rdata.error);


			rdata.left_num = htonl(rdata.left_num);
			rdata.right_num = htonl(rdata.right_num);
			rdata.result = htonl(rdata.result);
			rdata.error = htons(rdata.error);

			retval = send(clientsock, (char*)&rdata, sizeof(rdata), 0);
			if (retval == SOCKET_ERROR) break;
		}


		closesocket(clientsock);
	}

	closesocket(listensock);
	WSACleanup();
	
	return 0;
	
}