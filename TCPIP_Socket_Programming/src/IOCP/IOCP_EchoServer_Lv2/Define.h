#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#define MAX_SOCKBUF 1024	//패킷 크기
#define MAX_WORKERTHREAD 8

enum class IOOperation {
	RECV,
	SEND
};

struct stOverlappedEx {
	WSAOVERLAPPED	m_wsaOverlapped;
	SOCKET			m_socketClient;
	WSABUF			m_wsaBuf;
	IOOperation		m_eOperation;
};

struct stClientInfo {
	SOCKET			m_socketClient;
	stOverlappedEx	m_stRecvOverlappedEx;
	stOverlappedEx	m_stSendOverlappedEx;
	char			m_szRecvBuf[MAX_SOCKBUF];
	char			m_szSendBuf[MAX_SOCKBUF];

	stClientInfo() {
		ZeroMemory(&m_stRecvOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&m_stSendOverlappedEx, sizeof(stOverlappedEx));
		m_socketClient = INVALID_SOCKET;
	}
};