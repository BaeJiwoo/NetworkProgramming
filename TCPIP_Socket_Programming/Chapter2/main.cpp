#include <iostream>
#include <winsock2.h>
#include "..\Common.h"
using namespace std;

int main(int argc, char* argv[]) {
	WSADATA wsa;
	if (WSAStartup(0x0212, &wsa) != 0) {
		err_quit("소켓생성에러");
		return 1;
	}
	printf("[알림] 윈속 초기화 성공\n");

	//SOCKET socket(
	//	int af,			// 주소 체계 지정
	//	int type,		// 소켓 타입 지정
	//	int protocol	// 사용할 프로토콜 지정
	//);
	SOCKET sock = socket(
		AF_INET6,			// 주소 체계 지정
		SOCK_STREAM,		// 소켓 타입 지정
		0	// 사용할 프로토콜 지정
	);

	if (sock == INVALID_SOCKET) err_quit("socket()");
	printf("[알림] 소켓 생성 성공\n");
	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}