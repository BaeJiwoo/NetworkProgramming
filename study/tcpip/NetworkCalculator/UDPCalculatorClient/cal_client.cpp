#include "..\cal.h"
#include "..\..\error.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#define PORT 9000

int main(int argc, char* argv[]) {
	int retval;

	char ip[16] = "127.0.0.1";
	if(argc >= 2) strcpy_s(ip, argv[1]);

	//printf("서버 IP 주소 : %s\n", ip);

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	SOCKET clientsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (clientsock == INVALID_SOCKET) {
		return 1;
	}

	sockaddr_in serveraddr;
	memset(&serveraddr, 0x00, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	if (inet_pton(AF_INET, ip, &serveraddr.sin_addr) != 1) {
		exit_with_error("IP 주소 형식이 올바르지 않습니다.\n입력값을 확인하세요.");
	}

	cal_data rdata;
	int serveraddrlen = sizeof(serveraddr);
	for (;;) {
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

		printf("메시지 전송을 시도해요.\n");
		retval = sendto(clientsock, (char*)&rdata, sizeof(rdata), 0, (struct sockaddr*)&serveraddr, serveraddrlen);
		if (retval == SOCKET_ERROR) break;

		retval = recvfrom(clientsock, (char*)&rdata, sizeof(rdata), 0, (struct sockaddr*)&serveraddr, &serveraddrlen);
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