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
	}


	//WSASend Overlapped I/O작업을 시킨다.
	bool SendMsg(char* pMsg, UINT32 nLen)
	{
		auto sendOverlappedEx = new stOverlappedEx();
		ZeroMemory(sendOverlappedEx, sizeof(stOverlappedEx));
		sendOverlappedEx->m_wsaBuf.len = nLen;
		sendOverlappedEx->m_wsaBuf.buf = new char[nLen];
		CopyMemory(sendOverlappedEx->m_wsaBuf.buf, pMsg, nLen);
		sendOverlappedEx->m_eOperation = IOOperation::SEND;

		DWORD dwRecvNumBytes = 0;
		int nRet = WSASend(m_socketClient,
			&(m_stRecvOverlappedEx.m_wsaBuf),
			1,
			&dwRecvNumBytes,
			0,
			(LPWSAOVERLAPPED) (sendOverlappedEx),
			NULL);

		//socket_error이면 client socket이 끊어진걸로 처리한다.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[에러] WSASend()함수 실패 : %d\n", WSAGetLastError());
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
			
	}

private:
	INT32 mIndex = 0;
	SOCKET			m_socketClient;			//Cliet와 연결되는 소켓
	stOverlappedEx	m_stRecvOverlappedEx;	//RECV Overlapped I/O작업을 위한 변수
	char			mRecvBuf[MAX_SOCKBUF]; //데이터 버퍼
};