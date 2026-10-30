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

	void Init(const UINT32 index) {
		mIndex = index;
	}

	INT32 GetIndex() { return this->mIndex; }
	char* GetRecvBuf() { return this->mRecvBuf; }
	SOCKET* GetSocketClient() { return &m_socketClient; }

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

	void SendCompleted(DWORD dwIoSize) {
		printf("[송신 완료] bytes : %d\n", dwIoSize);
		mIsSending = false;
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

	}

	//SendBuf에 데이터 넣음
	bool SendMsg(char* pMsg, UINT32 nLen)
	{
		std::lock_guard<std::mutex> guard(mSendLock);
		if (mSendPos + nLen > MAX_SOCKBUF) {
			mSendPos = 0;
		}

		auto pSendBuf = &mSendBuf[0];

		CopyMemory(pSendBuf, pMsg, nLen);
		mSendPos += nLen;
		return true;
	}

	bool SendIO() {
		if (mIsSending || mSendPos <= 0) {
			//전송 중인 경우
			return true;
		}
		std::lock_guard<std::mutex> guard(mSendLock);
		//auto sendOverlappedEx = new stOverlappedEx();
		//ZeroMemory(m_stSendOverlappedEx, sizeof(stOverlappedEx));
		mIsSending = true;
		CopyMemory(&mSendingBuf, &mSendBuf[0], mSendPos);
		m_stSendOverlappedEx.m_wsaBuf.len = mSendPos;
		m_stSendOverlappedEx.m_wsaBuf.buf = &mSendingBuf[0];
		m_stSendOverlappedEx.m_eOperation = IOOperation::SEND;

		DWORD dwRecvNumBytes = 0;
		int nRet = WSASend(m_socketClient,
			&(m_stSendOverlappedEx.m_wsaBuf),
			1,
			&dwRecvNumBytes,
			0,
			(LPWSAOVERLAPPED)(&m_stSendOverlappedEx),
			NULL);

		//socket_error이면 client socket이 끊어진걸로 처리한다.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[에러] WSASend()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}
		mSendPos = 0;
		return true;
	}
	
private:
	//서버에서 인덱스
	INT32 mIndex = 0;
	
	//Cliet와 연결되는 소켓
	SOCKET m_socketClient;			
	
	//RECV Overlapped I/O작업을 위한 변수
	stOverlappedEx	m_stRecvOverlappedEx;
	stOverlappedEx	m_stSendOverlappedEx;
	
	//데이터 버퍼
	char mRecvBuf[MAX_SOCKBUF]; 
	char mSendBuf[MAX_SOCKBUF];
	char mSendingBuf[MAX_SOCKBUF];

	//전송 중 확인
	bool mIsSending = false;


	//전송 중인 사이즈
	UINT64 mSendPos = 0;

	//send buffer lock용
	std::mutex mSendLock;
};