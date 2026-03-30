/*
4 단계. 네트워크와 로직(패킷 or 요청) 처리 각각의 스레드로 분리하기
Send를 Recv와 다른 스레드에서 하기
send를 연속으로 보낼 수 있는 구조가 되어야 한다.


- Recv와 Send각각 스레드를 반반 만듦
	- recv: 버퍼 읽어서 send 수행
	- send: 버퍼 읽어서 recv 수행
	-> cp는 하나로?? 
*/
#pragma once
#include "Define.h"
#include <vector>
#include <thread>


class IOCPServer {
public:
	IOCPServer() {}
	~IOCPServer() {
		WSACleanup();
	}

	bool InitSocket() {
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
			printf("[ERROR] WSAStartup(): %d\n", WSAGetLastError());
			return false;
		}

		mListeningSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
		if (INVALID_SOCKET == mListeningSocket) {
			printf("[ERROR] socket(): %d\n", WSAGetLastError());
			return false;
		}

		printf("소켓 초기화 성공\n");
		return true;
	}

	bool BindandListen(UINT16 PORT) {
		mPort = PORT;
		SOCKADDR_IN serveraddr;
		memset(&serveraddr, 0x00, sizeof(SOCKADDR_IN));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(mPort);
		serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

		int nRet = bind(mListeningSocket, (struct sockaddr*)&serveraddr, sizeof(SOCKADDR_IN));
		if (SOCKET_ERROR == nRet) {
			printf("[ERROR] bind(): %d\n", WSAGetLastError());
			return false;
		}

		nRet = listen(mListeningSocket, 5);
		if (SOCKET_ERROR == nRet) {
			printf("[ERROR] listen(): %d\n", WSAGetLastError());
			return false;
		}

		printf("서버 등록 성공\n");
		return true;
	}

	bool StartServer(UINT32 MAXCLIENT) {

		CreateClient(MAXCLIENT);

		mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAX_WORKERTHREAD);
		if (NULL == mIOCPHandle) {
			printf("[ERROR] CreateIoCompletionPort(): %d\n", WSAGetLastError());
		}

		bool bRet = CreateWorkerThread();
		if (false == bRet) {
			printf("[ERROR] CreateWorkerThread(): %d\n", WSAGetLastError());
			return false;
		}

		bRet = CreateAccepterThread();
		if (false == bRet) {
			printf("[ERROR] CreateAccepterThread(): %d\n", WSAGetLastError());
			return false;
		}

		printf("Server Start...\n");
		return true;
	}

	void CreateClient(const UINT32 MAXCLIENT) {
		for (UINT32 i = 0; i < MAXCLIENT; ++i) {
			mClientInfos.emplace_back();

			mClientInfos[i].mIndex = i;
		}
	}

	void DestroyThread() {
		mIsWorkerRun = false;
		CloseHandle(mIOCPHandle);
		CloseHandle(mIOCPHandle);

		for (auto& worker : mRecvWorkerThreads) {
			if (worker.joinable())
				worker.join();
		}
		for (auto& worker : mSendWorkerThreads) {
			if (worker.joinable())
				worker.join();
		}

		mIsAccepterRun = false;
		closesocket(mListeningSocket);

		if (mAccepterThread.joinable())
			mAccepterThread.join();
	}


	virtual void End() {}
	virtual void Run(UINT16 uint16_maxClient){}

private:
	bool BindRecv(stClientInfo* pClientInfo) {
		DWORD dwFlag = 0;
		DWORD dwRecvNumBytes = 0;

		// receive and send message
		pClientInfo->m_RecvOverlappedEx.m_wsabuf.buf = pClientInfo->szRecvBuf;
		pClientInfo->m_RecvOverlappedEx.m_wsabuf.len = MAX_SOCKBUF;
		pClientInfo->m_RecvOverlappedEx.m_eOperation = IOOperation::RECV;

		int nRet = WSARecv(pClientInfo->m_socketClient
			, &(pClientInfo->m_RecvOverlappedEx.m_wsabuf)
			, 1
			, &dwRecvNumBytes
			, &dwFlag
			, (LPWSAOVERLAPPED) & (pClientInfo->m_RecvOverlappedEx),
			NULL);

		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
			printf("[ERROR] WSARecv(): %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	bool SendMsg(stClientInfo* pClientInfo, char* pMsg, int nLen) {
		DWORD dwRecvNumBytes = 0;

		CopyMemory(pClientInfo->szSendBuf, pMsg, nLen);
		pClientInfo->szSendBuf[nLen] = '\0';

		pClientInfo->m_SendOverlappedEx.m_wsabuf.buf = pClientInfo->szSendBuf;
		pClientInfo->m_SendOverlappedEx.m_wsabuf.len = nLen;
		pClientInfo->m_SendOverlappedEx.m_eOperation = IOOperation::SEND;


		int nRet = WSASend(pClientInfo->m_socketClient
			, &(pClientInfo->m_SendOverlappedEx.m_wsabuf)
			, 1
			, &dwRecvNumBytes
			, 0
			, (LPWSAOVERLAPPED) & (pClientInfo->m_SendOverlappedEx)
			, NULL);

		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
			printf("[ERROR] WSASend(): %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	bool CreateWorkerThread() {
		unsigned int uiThreadId = 0;
		for (UINT16 i = 0; i < MAX_WORKERTHREAD/2; ++i) {
			mRecvWorkerThreads.emplace_back([this]() {WorkerRecvThread(); });
		}
		for (UINT16 i = 0; i < MAX_WORKERTHREAD/2; ++i) {
			mSendWorkerThreads.emplace_back([this]() {WorkerSendThread(); });
		}

		printf("WorkerThread Started...\n");
		return true;
	}

	stClientInfo* GetEmptyClientInfo() {
		for (auto& client : mClientInfos) {
			if (INVALID_SOCKET == client.m_socketClient)
				return &client;
		}
		return nullptr;
	}

	bool CreateAccepterThread() {
		mAccepterThread = std::thread([this]() {AccepterTrhead(); });
		printf("AccepterThread Started...\n");
		return true;
	}

	void WorkerRecvThread() {
		DWORD dwIoSize;
		stClientInfo* pClientInfo;
		LPOVERLAPPED lpOverlapped = NULL;

		while (mIsWorkerRun) {
			bool bSuccess = GetQueuedCompletionStatus(mIOCPHandle
				, &dwIoSize
				, (PULONG_PTR)&pClientInfo
				, &lpOverlapped,
				INFINITE);

			if (TRUE == bSuccess && 0 == dwIoSize && NULL == lpOverlapped) {
				mIsWorkerRun = false;
				CloseSocket(pClientInfo);
				continue;
			}

			if (NULL == lpOverlapped) {
				continue;
			}

			if (FALSE == bSuccess || (0 == dwIoSize && TRUE == bSuccess)) {
				printf("socket(%d) disconnected\n", pClientInfo->m_socketClient);
			}

			auto pOverlapped = (stOverlappedEx*)lpOverlapped;
			if (IOOperation::RECV == pOverlapped->m_eOperation) {
				pClientInfo->szRecvBuf[dwIoSize] = '\0';
				OnReceive(pClientInfo->m_socketClient);
				SendMsg(pClientInfo, pClientInfo->szRecvBuf, dwIoSize);

				//BindRecv(pClientInfo);
			}
			else if (pOverlapped->m_eOperation == IOOperation::SEND) {
				// 다시 큐에 삽입
				bSuccess = PostQueuedCompletionStatus(mIOCPHandle, dwIoSize, (ULONG_PTR)&pClientInfo, (LPOVERLAPPED)&lpOverlapped);
				continue;
			}
			else {
				printf("[Error]Exception Error from socket(%d)\n", pClientInfo->m_socketClient);
			}
		}
	}

	void WorkerSendThread() {
		DWORD dwIoSize;
		stClientInfo* pClientInfo;
		LPOVERLAPPED lpOverlapped = NULL;

		while (mIsWorkerRun) {
			bool bSuccess = GetQueuedCompletionStatus(mIOCPHandle,
				&dwIoSize
				, (PULONG_PTR)&pClientInfo
				, &lpOverlapped,
				INFINITE);

			if (TRUE == bSuccess && 0 == dwIoSize && NULL == lpOverlapped) {
				mIsWorkerRun = false;
				CloseSocket(pClientInfo);
				continue;
			}

			if (NULL == lpOverlapped) {
				continue;
			}

			if (FALSE == bSuccess || (0 == dwIoSize && TRUE == bSuccess)) {
				printf("socket(%d) disconnected\n", pClientInfo->m_socketClient);
			}

			auto pOverlapped = (stOverlappedEx*)lpOverlapped;
			if (IOOperation::SEND == pOverlapped->m_eOperation) {
				// 전송 내용 출력 후 recv기다리기...
				printf("[SEND] bytes : %d , msg : %s\n", dwIoSize, pClientInfo->szSendBuf);
				BindRecv(pClientInfo);

			}
			else if (pOverlapped->m_eOperation == IOOperation::RECV) {
				// 다시 큐에 삽입
				bSuccess = PostQueuedCompletionStatus(mIOCPHandle, dwIoSize, (ULONG_PTR)&pClientInfo, (LPOVERLAPPED)&lpOverlapped);
				continue;
			}
			else {
				printf("[Error]Exception Error from socket(%d)\n", pClientInfo->m_socketClient);
			}
		}
	}

	// Legacy Worker Thread
	/*
	void WorkerThread() {
		DWORD dwIoSize;
		stClientInfo* pClientInfo;
		LPOVERLAPPED lpOverlapped = NULL;
		while (mIsWorkerRun) {
			bool bSuccess = GetQueuedCompletionStatus(mIOCPHandle,
				&dwIoSize
				, (PULONG_PTR)&pClientInfo
				, &lpOverlapped,
				INFINITE);

			if (TRUE == bSuccess && 0 == dwIoSize && NULL == lpOverlapped) {
				mIsWorkerRun = false;
				CloseSocket(pClientInfo);
				continue;
			}

			if (NULL == lpOverlapped) {
				continue;
			}

			if (FALSE == bSuccess || (0 == dwIoSize && TRUE == bSuccess)) {
				printf("socket(%d) disconnected\n", pClientInfo->m_socketClient);
			}

			auto pOverlapped = (stOverlappedEx*)lpOverlapped;
			if (IOOperation::RECV == pOverlapped->m_eOperation) {
				pClientInfo->szRecvBuf[dwIoSize] = '\0';
				OnReceive(pClientInfo->m_socketClient);

				//SendMsg(pClientInfo, pClientInfo->szRecvBuf, dwIoSize);

				//BindRecv(pClientInfo);
			}
			else if (IOOperation::SEND == pOverlapped->m_eOperation) {
				// 전송 내용 출력 후 recv기다리기...
				//printf("[SEND] bytes : %d , msg : %s\n", dwIoSize, pClientInfo->szSendBuf);
				

			}
			else {
				printf("[Error]Exception Error from socket(%d)\n", pClientInfo->m_socketClient);
			}
		}
	}
	*/

	bool BindIOCompletionPort(stClientInfo* pClientInfo) {
		auto hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->m_socketClient
			, mIOCPHandle
			, (ULONG_PTR)(pClientInfo), 0);

		if (NULL == hIOCP || mIOCPHandle != hIOCP) {
			printf("[ERROR] CreateIoCompletionPort(): %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	void AccepterTrhead() {

		SOCKADDR_IN stClientAddr;
		int nAddrLen = sizeof(SOCKADDR_IN);

		while (mIsAccepterRun) {
			stClientInfo* pClientInfo = GetEmptyClientInfo();
			if (NULL == pClientInfo) {
				printf("[ERROR] Client Full!\n");
				return;
			}

			pClientInfo->m_socketClient = accept(mListeningSocket, (sockaddr*)&stClientAddr, &nAddrLen);
			if (INVALID_SOCKET == pClientInfo->m_socketClient) {
				printf("[ERROR] accept(): %d\n", WSAGetLastError());
			}

			// bind client socket with Completion Port
			bool bRet = BindIOCompletionPort(pClientInfo);
			if (false == bRet) {
				printf("[ERROR] BindIOCompletionPort()\n");
				return;
			}

			// request Recv Overlapped IO
			bRet = BindRecv(pClientInfo);
			if (false == bRet) {
				printf("[ERROR] bindRecv()\n");
				return;
			}

			OnConnect(pClientInfo->m_socketClient);
			++mClientCnt;
		}
	}

	void CloseSocket(stClientInfo* pClientInfo, bool bIsForce = false) {
		struct linger stLinger = { 0,0 };

		if (true == bIsForce) {
			stLinger.l_onoff = 1;
		}

		shutdown(pClientInfo->m_socketClient, SD_BOTH);

		setsockopt(pClientInfo->m_socketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

		OnClose(pClientInfo->m_socketClient);

		closesocket(pClientInfo->m_socketClient);

		pClientInfo->m_socketClient = INVALID_SOCKET;
	}

	virtual void OnConnect(const UINT32 clientIndex_) {

	}

	virtual void OnClose(const UINT32 clientIndex_) {

	}

	virtual void OnReceive(const UINT32 clientIndex_) {

	}


	WSADATA wsa;
	SOCKET mListeningSocket = INVALID_SOCKET;
	int mPort;
	std::vector<stClientInfo> mClientInfos;
	HANDLE mIOCPHandle = NULL;


	//std::vector<std::thread> mIOWorkerThreads;
	std::vector<std::thread> mRecvWorkerThreads;
	std::vector<std::thread> mSendWorkerThreads;
	std::thread mAccepterThread;

	bool mIsWorkerRun = true;
	bool mIsAccepterRun = true;

	int mClientCnt = 0;
};