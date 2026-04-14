#pragma once
#include "Define.h"
#include <stdio.h>

//클라이언트 정보를 담기위한 구조체
//TODO: 클라이언트 조작과 IOCPServer분리
class stClientInfo
{
public:

	stClientInfo()
	{
		ZeroMemory(&m_stRecvOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&m_stSendOverlappedEx, sizeof(stOverlappedEx));
		m_socketClient = INVALID_SOCKET;
	}

	void Init(const UINT32 index, const HANDLE hIOCP_) {
		mIndex = index;
		mIOCPHandle = hIOCP_;
	}

	INT32 GetIndex() { return this->mIndex; }
	char* GetRecvBuf() { return this->mRecvBuf; }
	SOCKET* GetSocketClient() { return &m_socketClient; }
	UINT64 GetLatestCloseTimeSec() { return mLatestCloseTimeSec; }

	bool IsConnected() { return m_socketClient == INVALID_SOCKET ? false : true; }
	bool OnConnect(const HANDLE hIOCP, SOCKET socket_) {
		m_socketClient = socket_;

		//Clear();

		if (BindIOCompletionPort(hIOCP) == false) {
			return false;
		}

		return BindRecv();
	}

	bool BindIOCompletionPort(const HANDLE hIOCPHandle)
	{
		auto hIOCP = CreateIoCompletionPort((HANDLE)m_socketClient
			, hIOCPHandle
			, (ULONG_PTR)(this), 0);

		if (NULL == hIOCP || hIOCPHandle != hIOCP)
		{
			printf("[에러] CreateIoCompletionPort()함수 실패: %d\n", GetLastError());
			return false;
		}

		return true;
	}

	//WSARecv Overlapped I/O 작업을 시킨다.
	bool BindRecv()
	{
		DWORD dwFlag = 0;
		DWORD dwRecvNumBytes = 0;

		//Overlapped I/O을 위해 각 정보를 셋팅해 준다.
		m_stRecvOverlappedEx.m_wsaBuf.len = MAX_SOCKBUF;
		m_stRecvOverlappedEx.m_wsaBuf.buf = mRecvBuf;
		m_stRecvOverlappedEx.m_eOperation = IOOperation::RECV;

		int nRet = WSARecv(m_socketClient,
			&(m_stRecvOverlappedEx.m_wsaBuf),
			1,
			&dwRecvNumBytes,
			&dwFlag,
			(LPWSAOVERLAPPED) & (m_stRecvOverlappedEx),
			NULL);

		//socket_error이면 client socket이 끊어진걸로 처리한다.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[에러] WSARecv()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	


	


	void Close(bool bIsForce = false)
	{

		struct linger stLinger = { 0, 0 };	// SO_DONTLINGER로 설정

		// bIsForce가 true이면 SO_LINGER, timeout = 0으로 설정하여 강제 종료 시킨다. 주의 : 데이터 손실이 있을수 있음 
		if (true == bIsForce)
		{
			stLinger.l_onoff = 1;
		}

		//socketClose소켓의 데이터 송수신을 모두 중단 시킨다.
		shutdown(m_socketClient, SD_BOTH);

		//소켓 옵션을 설정한다.
		setsockopt(m_socketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

		//소켓 연결을 종료 시킨다. 
		closesocket(m_socketClient);

		m_socketClient = INVALID_SOCKET;

		mLatestCloseTimeSec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

	}

	// SendQueue 전송용 데이터 넣음 => 전송용 데이터라함은 WSASend에 필요한 데이터임. 현재 플젝에서는 stOverlappedEx이다.
	bool SendMsg(char* pMsg, UINT32 nLen)
	{
		// TODO: 먼저 stOverlappedEx 만들어서 mSendOverlappedQueue에 삽입 -> 큐 사이즈가 1이면 바로 SendIO수행 -> 아니면 true 반환
		auto pOverlappedEx = new stOverlappedEx();
		
		ZeroMemory(pOverlappedEx, sizeof(stOverlappedEx));
		pOverlappedEx->m_socketClient = m_socketClient;
		pOverlappedEx->m_wsaBuf.len = nLen;
		pOverlappedEx->m_wsaBuf.buf = new char[nLen];
		CopyMemory(pOverlappedEx->m_wsaBuf.buf, pMsg, nLen);
		pOverlappedEx->m_eOperation = IOOperation::SEND;

		std::lock_guard<std::mutex> guard(mSendLock);
		mSendOverlappedQueue.push(pOverlappedEx);
		
		if (mSendOverlappedQueue.size() == 1) {
			SendIO();
		}

		return true;
	}

	// 실질적으로 IO를 수행하는 부분임. One-Send 구현을 API 입출력과 연결되는 SendMsg()와 분리함.
	bool SendIO() {
		auto pOverlappedEx = mSendOverlappedQueue.front();

		DWORD dwRecvNumBytes = 0;
		int nRet = WSASend(
			m_socketClient,
			&(pOverlappedEx->m_wsaBuf),
			1,
			&dwRecvNumBytes,
			0,
			(LPWSAOVERLAPPED) pOverlappedEx,
			NULL);

		if (nRet == SOCKET_ERROR && (WSAGetLastError() != WSA_IO_PENDING)) {
			printf("[에러] WSASend()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	// Send완료 시, 수행될 코드. 동적할당 된 데이터를 해제한다.
	void SendCompleted(DWORD dwIoSize) {
		printf("[송신 완료] bytes : %d\n", dwIoSize);
		std::lock_guard<std::mutex> guard(mSendLock);
		
		delete[] mSendOverlappedQueue.front()->m_wsaBuf.buf;
		delete mSendOverlappedQueue.front();
		
		mSendOverlappedQueue.pop();
		
		if (mSendOverlappedQueue.empty() == false) {
			SendIO();
		}
	}

	bool PostAccept(SOCKET hListenSock) {
		printf_s("Post Accept. client Index: %d\n", mIndex);
		mLatestCloseTimeSec = UINT32_MAX;

		m_socketClient = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (INVALID_SOCKET == m_socketClient) {
			printf_s("[에러] WSASocket()\n");
			return false;
		}

		ZeroMemory(&mAcceptContext, sizeof(stOverlappedEx));

		DWORD bytes = 0;
		DWORD flags = 0;
		mAcceptContext.m_wsaBuf.buf = nullptr;
		mAcceptContext.m_wsaBuf.len = 0;
		mAcceptContext.m_eOperation = IOOperation::ACCEPT;
		mAcceptContext.m_SessionIndex = mIndex;

		if (FALSE == AcceptEx(hListenSock, m_socketClient, mAcceptBuf, 0,
			sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytes, (LPOVERLAPPED)&mAcceptContext)) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				printf_s("[에러] AcceptEx()\n");
				return false;
			}
		}


		return true;
	}

	bool AcceptCompletion() {
		printf_s("AcceptCompletion : SessionIndex(%d)\n", (int)m_socketClient);

		if (OnConnect(mIOCPHandle, m_socketClient) == false) {
			return false;
		}

		SOCKADDR_IN stClientAddr;
		int nAddrlen = sizeof(SOCKADDR_IN);
		char clientIP[32] = { 0, };
		inet_ntop(AF_INET, &(stClientAddr.sin_addr), clientIP, 32 - 1);
		printf("클라이언트 접속: IP(%s), SOCKET(%d)\n", clientIP, (int)m_socketClient);

		return true;
	}
	
private:
	//서버에서 인덱스
	INT32 mIndex = 0;
	
	//Cliet와 연결되는 소켓
	SOCKET m_socketClient;			
	
	//RECV Overlapped I/O작업을 위한 
	stOverlappedEx	mAcceptContext;
	stOverlappedEx	m_stRecvOverlappedEx;
	stOverlappedEx	m_stSendOverlappedEx;
	
	//데이터 버퍼
	char mRecvBuf[MAX_SOCKBUF]; 
	char mSendBuf[MAX_SOCKBUF];
	char mAcceptBuf[64];

	//핸들
	HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;

	//전송 중인 사이즈
	UINT64 mSendPos = 0;

	// 전송 큐
	std::queue<stOverlappedEx*> mSendOverlappedQueue;

	//send buffer lock용
	std::mutex mSendLock;


	// close time 확인
	UINT64 mLatestCloseTimeSec = 0;
};