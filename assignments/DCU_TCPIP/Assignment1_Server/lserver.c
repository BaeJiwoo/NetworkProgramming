#include <WinSock2.h>
#include <stdio.h>
#include <ws2tcpip.h> 
#include <string.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVERPORT 9000
#define MAXBUFFER 512


int main(int argc, char* argv[]) {
	// 시간
	time_t now = time(NULL);
	char timestr[64]; // 시간 문자열을 담을 버퍼
	struct tm t;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	SOCKET listen_s = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_s == INVALID_SOCKET) return 1;


	struct sockaddr_in servaddr, clientaddr;
	memset(&servaddr, 0x00, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERVERPORT);
	servaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // INADDR_ANY는 0.0.0.0으로 모든 패킷을 다 받겠다는 뜻입니다.

	if (bind(listen_s, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) return 1;

	if (listen(listen_s, 5) == SOCKET_ERROR) return 1;

	int len, readn, written;
	SOCKET client_s;
	char recieveBuffer[MAXBUFFER + 1];
	while (1) {
		memset(&clientaddr, 0x00, sizeof(clientaddr));
		len = sizeof(struct sockaddr_in);

		client_s = accept(listen_s, (struct sockaddr*)&clientaddr, &len);
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("[TCP클라이언트] %s:%d 접속\n", addr, ntohs(clientaddr.sin_port));

		while (1) {
			readn = recv(client_s, recieveBuffer, MAXBUFFER, 0);
			if (readn <= 0) break;

			recieveBuffer[readn] = '\0';
			printf("[TCP클라이언트] %s\n", recieveBuffer);
			if (readn > 0) {
				// logic
				const char* result = NULL;

				if (strncmp(recieveBuffer, "TIME", 4) == 0) {
					//result = "show time";
					localtime_s(&t, &now);
					sprintf_s(timestr, sizeof(timestr), "%04d-%02d-%02d %02d:%02d:%02d",
						t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
					result = timestr;
				}
				else if (strncmp(recieveBuffer, "NAME", 4) == 0) {
					if (strlen(recieveBuffer) > 5) {
						const char* userName = recieveBuffer + 5;

						static char nameReply[64];
						sprintf_s(nameReply, sizeof(nameReply), "My name is %s Server", userName);
						result = nameReply;
					}
					else {
						result = "Please enter your name after 'NAME '";
					}
				}
				else if (strncmp(recieveBuffer, "QUIT", 4) == 0) {
					printf("클라이언트 요청으로 연결을 종료합니다.\n");
					break;
				}
				else {
					result = "Unknown Command";
				}
				int sendLen = (int)strlen(result);
				written = send(client_s, result, sendLen, 0);
				if (written == SOCKET_ERROR) break;
			}
		}
		closesocket(client_s);
		printf("[TCP클라이언트] %s:%d 종료\n", addr, ntohs(clientaddr.sin_port));
	}

	closesocket(listen_s);

	WSACleanup();
	return 0;
}
