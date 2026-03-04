#include "..\cal.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#define PORT 9000

int main() {
	int retval;
	
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}
	
	SOCKET listensock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (listensock == INVALID_SOCKET) {
		return 1;
	}

	sockaddr_in serveraddr;
	memset(&serveraddr, 0x00, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	serveraddr.sin_addr.S_un.S_addr = INADDR_ANY;

	retval = bind(listensock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) return 1;

	sockaddr_in clientaddr;
	int clientaddrlen = sizeof(clientaddr);
	memset(&clientaddr, 0x00, sizeof(clientaddr));
	cal_data rdata;
	for (;;) {
		memset(&rdata, 0x00, sizeof(rdata));


		retval = recvfrom(listensock, (char*)&rdata, sizeof(rdata), 0, (struct sockaddr*) &clientaddr, &clientaddrlen);
		if (retval == SOCKET_ERROR) break;
		if (retval <= 0) break;

		rdata.left_num = ntohl(rdata.left_num);
		rdata.right_num = ntohl(rdata.right_num);

		calculate(&rdata);
		printf("Processed: %d %c %d = %d (Error: %d)\n",
			rdata.left_num, rdata.op, rdata.right_num, rdata.result, rdata.error);


		rdata.left_num = htonl(rdata.left_num);
		rdata.right_num = htonl(rdata.right_num);
		rdata.result = htonl(rdata.result);
		rdata.error = htons(rdata.error);

		retval = sendto(listensock, (char*)&rdata, sizeof(rdata), 0, (struct sockaddr*)&clientaddr, clientaddrlen);
		if (retval == SOCKET_ERROR) break;
		if (retval <= 0) break;
	}

	closesocket(listensock);

	WSACleanup();

	return 0;
}