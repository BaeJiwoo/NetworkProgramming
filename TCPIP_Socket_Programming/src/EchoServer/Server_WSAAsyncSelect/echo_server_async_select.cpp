#include <WinSock2.h>
#include <WS2tcpip.h>
#include<stdio.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")

#define WM_SOCKET WM_USER + 1
#define MAXLINE 1024
#define PORTNUM 9000

LRESULT CALLBACK WinProc(HWND hWnd, int uMsg, WPARAM wParam, LPARAM lParam);
SOCKET listensock;

void errMsg(char* msg) {
	MessageBoxA(NULL, msg, "Error!", MB_ICONEXCLAMATION | MB_OK);
	exit(0);
}

void makeSocket(HWND hWnd) {
	WSADATA wsaData;
	struct sockaddr_in addr;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	memset((void*)&addr, 0x00, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORTNUM);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	bind(listensock, (struct sockaddr*)&addr, sizeof(addr));
	listen(listensock, 5);
	WSAAsyncSelect(listensock, hWnd, WM_SOCKET,
		FD_ACCEPT | FD_CLOSE);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev,
	LPSTR lpCmdLine, int nCmdShow) {
	MSG msg;
	WNDCLASSEX wClass;
	HWND Windows;

	ZeroMemory(&wClass, sizeof(WNDCLASSEX));
	wClass
}