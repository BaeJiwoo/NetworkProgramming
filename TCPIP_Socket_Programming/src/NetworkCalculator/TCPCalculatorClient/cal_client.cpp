#include "..\cal.h"
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define PORT 3000
const char* serverip = "127.0.0.1";

int main() {
	int retval;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET clientsock;
	clientsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientsock == INVALID_SOCKET) return 1;

	sockaddr_in serveraddr;
	memset(&serveraddr, 0x00, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, serverip, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(PORT);


	retval = connect(clientsock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) return 1;

	sockaddr_in clientaddr;
	int addrlen = sizeof(clientaddr);


	struct cal_data rdata;
	memset(&rdata, 0x00, sizeof(rdata));
	for (;;)
	{
		/*printf("숫자 1 입력 : ");
		scanf_s("%d", &rdata.left_num);
		printf("연산자 입력 : ");
		scanf_s(" %c", &rdata.op, (unsigned int)sizeof(rdata.op));
		printf("숫자 2 입력 : ");
		scanf_s("%d", &rdata.right_num);*/
		char input[100];
		printf("\n[Calc] 수식을 입력하세요 (예: 10 + 20 / 종료: 엔터) \n>> ");

		if (fgets(input, sizeof(input), stdin) == NULL) break;

		if (input[0] == '\n') break;

		int count = sscanf_s(input, "%d %c %d",
			&rdata.left_num,
			&rdata.op, (unsigned int)sizeof(rdata.op),
			&rdata.right_num);

		if (count < 3) {
			printf("형식이 섹시하지 않네요. '숫자 연산자 숫자' 형식을 지켜주세요!\n");
			continue;
		}

		rdata.left_num = htonl(rdata.left_num);
		rdata.right_num = htonl(rdata.right_num);

		retval = send(clientsock, (char*)&rdata, sizeof(rdata), 0);
		if (retval == SOCKET_ERROR) break;
		
		retval = recv(clientsock, (char*)&rdata, sizeof(rdata), 0);
		if (retval == SOCKET_ERROR) break;
		if (retval <= 0) break;

		rdata.left_num = ntohl(rdata.left_num);
		rdata.right_num = ntohl(rdata.right_num);
		rdata.result = ntohl(rdata.result);
		rdata.error = ntohs(rdata.error);


		printf("Processed: %d %c %d = %d (Error: %d)\n",
			rdata.left_num, rdata.op, rdata.right_num, rdata.result, rdata.error);
	}
	closesocket(clientsock);

	WSACleanup();

	return 0;
}