#pragma once
#pragma once
#pragma comment(lib, "ws2_32.lib")

#include "Define.h";
#include <Windows.h>
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <thread>


class IOCompletionPort {
public:
	IOCompletionPort(void) {};
	~IOCompletionPort(void) {
		WSACleanup();
	};

	bool InitSocket() {
		WSADATA wsa;
		int retVal = WSAStartup(MAKEWORD(2, 2), &wsa);
		if (0 != retVal) {
			printf("[ERROR] WSAStartup() error : %d", WSAGetLastError());
			return false;
		}

		mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
		if (INVALID_SOCKET == mListenSocket) {
			printf("[ERROR] WSASocket() error : %d", WSAGetLastError());
			return false;
		}

		printf("Socket Creation Colpleted...");
		return true;
	}
	void DestroyThread() {
		mIsWorkerRun = false;
		CloseHandle(m_IOCPHandle);

		for (auto& th : mIOWorkerThreads) {
			if (th.joinable()) th.join();
		}

		mIsAccepterRun = false;
		closesocket(mListenSocket);
		if (mAccepterThread.joinable()) mAccepterThread.join();
	}
	bool BindandListen(const int SERVER_PORT) {
		SOCKADDR_IN serveraddr;
		memset(&serveraddr, 0x00, sizeof(SOCKADDR_IN));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(SERVER_PORT);
		serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		int retVal = bind(mListenSocket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
		if (retVal == SOCKET_ERROR) {
			printf("[ERROR] bind() error : %d", WSAGetLastError());
			return false;
		}

		if (listen(mListenSocket, 5) == SOCKET_ERROR) {
			printf("[ERROR] listen() error : %d", WSAGetLastError());
			return false;
		}

		printf("Server Register Completed...");
		return true;
	}

	bool StartServer(const UINT32 maxClientCount) {
		CreateClient(maxClientCount);

		m_IOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAX_WORKERTHREAD);
		if (NULL == m_IOCPHandle) {
			printf("[ERROR] CreateIoCompletionPort() error : %d", WSAGetLastError());
			return false;
		}


		bool retVal = CreateWorkerThread();
		if (false == retVal) return false;

		retVal = CreateAcceptThread();
		if (false == retVal) return false;

		printf("서버 시작\n");
		return true;
	}



private:
	void CreateClient(const UINT32 maxClientCount) {
		for (UINT32 i = 0; i < maxClientCount; ++i)
		{
			mClientInfos.emplace_back();
		}
	}

	bool CreateWorkerThread() {
		unsigned int uiThreadId = 0;
		for (int i = 0; i < MAX_WORKERTHREAD; i++) {
			mIOWorkerThreads.emplace_back([this]() {WorkerThread(); });
		}

		printf("Worker Thread Started...\n");
		return true;
	}

	bool CreateAcceptThread() {
		mAccepterThread = std::thread([this]() { AccepterThread(); });

		printf("Accepter Thread Started...\n");
		return true;
	}

	stClientInfo* GetEmptyClientInfo() {
		for (auto& client : mClientInfos) {
			if (INVALID_SOCKET == client.m_socketClient) {
				return &client;
			}
		}

		return nullptr;
	}

	bool BindIOCompletionPort(stClientInfo* pClientInfo) {
		auto hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->m_socketClient,
			(HANDLE)m_IOCPHandle,
			(ULONG_PTR)(pClientInfo), 0);

		if (NULL == hIOCP || m_IOCPHandle != hIOCP) {
			printf("[ERROR] CreateIoCompletionPort() error : %d", WSAGetLastError());
			return false;
		}
		return true;
	}

	bool BindRecv(stClientInfo* pClientInfo) {
		DWORD dwFlag = 0;
		DWORD dwRecvNumBytes = 0;

		pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.len = MAX_SOCKBUF;
		pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.buf = pClientInfo->m_szRecvBuf;
		pClientInfo->m_stRecvOverlappedEx.m_eOperation = IOOperation::RECV;

		int nRet = WSARecv(pClientInfo->m_socketClient,
			&(pClientInfo->m_stRecvOverlappedEx.m_wsaBuf),
			1,
			&dwRecvNumBytes,
			&dwFlag,
			(LPWSAOVERLAPPED) & (pClientInfo->m_stRecvOverlappedEx),
			NULL);

		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
			printf("[ERROR] WSARecv() error : %d", WSAGetLastError());
			return false;
		}

		return true;
	}

	bool SendMsg(stClientInfo* pClientInfo, char* pMsg, int nLen) {
		DWORD dwRecvNumBytes = 0;

		CopyMemory(pClientInfo->m_szSendBuf, pMsg, nLen);
		pClientInfo->m_szSendBuf[nLen] = '\0';

		pClientInfo->m_stSendOverlappedEx.m_wsaBuf.len = nLen;
		pClientInfo->m_stSendOverlappedEx.m_wsaBuf.buf = pClientInfo->m_szSendBuf;
		pClientInfo->m_stSendOverlappedEx.m_eOperation = IOOperation::SEND;

		int nRet = WSASend(pClientInfo->m_socketClient,
			&(pClientInfo->m_stSendOverlappedEx.m_wsaBuf),
			1,
			&dwRecvNumBytes,
			0,
			(LPWSAOVERLAPPED) & (pClientInfo->m_stSendOverlappedEx),
			NULL);

		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
			printf("[ERROR] WSASend()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	void WorkerThread() {
		stClientInfo* pClientInfo = NULL;
		BOOL bSuccess = TRUE;
		DWORD dwIoSize = 0;
		LPOVERLAPPED lpOverlapped = NULL;

		while (mIsWorkerRun) {
			bSuccess = GetQueuedCompletionStatus(m_IOCPHandle,
				&dwIoSize,
				(PULONG_PTR)&pClientInfo,
				&lpOverlapped,
				INFINITE);

			if (TRUE == bSuccess && 0 == dwIoSize && NULL == lpOverlapped) {
				mIsWorkerRun = false;
				continue;
			}

			if (NULL == lpOverlapped) {
				continue;
			}

			if (FALSE == bSuccess || (0 == dwIoSize && TRUE == bSuccess)) {
				printf("socket(%d) 접속 끊김\n", (int)pClientInfo->m_socketClient);
				CloseSocket(pClientInfo);
				continue;
			}

			stOverlappedEx* pOverlappedEx = (stOverlappedEx*)lpOverlapped;

			if (IOOperation::RECV == pOverlappedEx->m_eOperation) {
				pClientInfo->m_szRecvBuf[dwIoSize] = NULL;
				printf("[RECV] bytes : %d , msg : %s\n", dwIoSize, pClientInfo->m_szRecvBuf);
				SendMsg(pClientInfo, pClientInfo->m_szRecvBuf, dwIoSize);
				BindRecv(pClientInfo);
			}
			else if (IOOperation::SEND == pOverlappedEx->m_eOperation) {
				printf("[SEND] bytes : %d , msg : %s\n", dwIoSize, pClientInfo->m_szSendBuf);
			}
			else {
				printf("socket(%d)에서 예외상황\n", (int)pClientInfo->m_socketClient);
			}
		}
	}

	void AccepterThread() {
		SOCKADDR_IN stClientAddr;
		int nAddrLen = sizeof(SOCKADDR_IN);
		while (mIsAccepterRun) {
			stClientInfo* pClientInfo = GetEmptyClientInfo();

			pClientInfo->m_socketClient = accept(mListenSocket, (struct sockaddr*)&stClientAddr, &nAddrLen);
			if (INVALID_SOCKET == pClientInfo->m_socketClient)
			{
				continue;
			}

			bool bRet = BindIOCompletionPort(pClientInfo);
			if (false == bRet) {
				return;
			}

			bRet = BindRecv(pClientInfo);
			if (false == bRet) {
				return;
			}

			char clientIp[32] = { 0, };
			inet_ntop(AF_INET, &(stClientAddr.sin_addr), clientIp, 32 - 1);
			printf("Client Joined : IP(%s) SOCKET(%d)\n", clientIp, (int)pClientInfo->m_socketClient);

			mClientCnt++;
		}
	}



	void CloseSocket(stClientInfo* pClientInfo, bool bIsForce = false) {
		struct linger stLinger = { 0,0 };
		if (true == bIsForce) {
			stLinger.l_onoff = 1;
		}

		shutdown(pClientInfo->m_socketClient, SD_BOTH);

		setsockopt(pClientInfo->m_socketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

		closesocket(pClientInfo->m_socketClient);
		pClientInfo->m_socketClient = INVALID_SOCKET;
	}


	// property
	SOCKET						mListenSocket = INVALID_SOCKET;
	std::vector<stClientInfo>	mClientInfos;
	HANDLE						m_IOCPHandle = INVALID_HANDLE_VALUE;
	std::vector<std::thread>	mIOWorkerThreads;
	std::thread					mAccepterThread;
	bool						mIsAccepterRun = true;
	bool						mIsWorkerRun = true;
	int							mClientCnt = 0;
};