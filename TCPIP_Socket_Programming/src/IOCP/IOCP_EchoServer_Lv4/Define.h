#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>

#define MAX_WORKERTHREAD 8
#define MAX_SOCKBUF 1024

enum class IOOperation {
	RECV,
	SEND
};

struct stOverlappedEx {
	WSAOVERLAPPED	m_wsaOveralpped;
	SOCKET			m_socketClient;
	WSABUF			m_wsabuf;
	IOOperation		m_eOperation;
};

struct stClientInfo {
	INT32			mIndex = 0;
	SOCKET			m_socketClient;

	stOverlappedEx	m_RecvOverlappedEx;
	stOverlappedEx	m_SendOverlappedEx;

	char			szRecvBuf[MAX_SOCKBUF];
	char			szSendBuf[MAX_SOCKBUF];

	stClientInfo() {
		ZeroMemory(&m_RecvOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&m_SendOverlappedEx, sizeof(stOverlappedEx));
		m_socketClient = INVALID_SOCKET;
	}
};