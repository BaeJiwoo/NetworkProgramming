#pragma once
#pragma comment(lib, "ws2_32.lib")
#include<iostream>
#include<WinSock2.h>
#include<WS2tcpip.h>

#define MAXBUFSIZ 1024
#define MAX_WORKERTHREAD 16

enum class IOOperation {
	RECV,
	SEND
};
struct stOverlappedEX {
	WSAOVERLAPPED mWSAOverlapped;
	WSABUF mWsaBuf;
	SOCKET mSocket;
	IOOperation mIOOperation;
};

struct stClientInfo {
	UINT32 mIndex;
	SOCKET mClientSocket = INVALID_SOCKET;
	SOCKADDR_IN mClientAddr;

	stOverlappedEX mRecvOverlapped;
	stOverlappedEX mSendOverlapped;

	char			szRecvBuf[MAXBUFSIZ];
	char			szSendBuf[MAXBUFSIZ];

	stClientInfo() {
		ZeroMemory(&mRecvOverlapped, sizeof(stOverlappedEX));
		ZeroMemory(&mSendOverlapped, sizeof(stOverlappedEX));
	}
};

std::wstring Utf8ToWide(const char* utf8Str) {
	if (!utf8Str) return L"";
	int size = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);
	if (size <= 0) return L"";

	std::wstring wstr(size, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, &wstr[0], size);

	// wstring 생성 시 할당된 마지막 null 제거
	if (wstr.back() == L'\0') wstr.resize(size - 1);
	return wstr;
}