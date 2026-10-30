#pragma once
#include "Define.h"
#include "FileManager.h"
#include<thread>
#include <vector>

class IOCPServer {
public:
	IOCPServer() {}
	~IOCPServer() {}

	bool InintSocket() {
		if (0 != WSAStartup(MAKEWORD(2, 2), &wsa)) {
			printf("[ERROR] WSAStartup() :%d\n", WSAGetLastError());
			return false;
		}

		mListenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == mListenSock) {
			printf("[ERROR] socket() :%d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	bool BindandListen(UINT16 PORT) {
		int retval;
		memset(&serveraddr, 0x00, sizeof(SOCKADDR_IN));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(PORT);
		serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

		retval = bind(mListenSock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) {
			printf("[ERROR] bind() :%d\n", WSAGetLastError());
			return false;
		}

		retval = listen(mListenSock, 5);
		if (retval == SOCKET_ERROR) {
			printf("[ERROR] listen() :%d\n", WSAGetLastError());
			return false;
		}



		return true;
	}

	bool StartServer(const UINT32 maxClient) {
		// CP 생성
		mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAX_WORKERTHREAD);
		if (NULL == mIOCPHandle) {
			printf("[ERROR] CreateIoCompletionPort() :%d\n", WSAGetLastError());
			return false;
		}

		// create clients
		CreateClient(maxClient);

		// create aceepter thread
		CreateAccepterThread();
		// create worker thread
		CreateWorkerThread();

		printf("Server Started.... :p\n");
		return true;
	}

	void DestroyThread() {
		mWorkerRunning = false;
		CloseHandle(mIOCPHandle);

		for (auto& worker : mWorkerThread) {
			if (worker.joinable())
				worker.join();
		}

		mAccepterRunning = false;
		closesocket(mListenSock);

		if (mAccepterThread.joinable())
			mAccepterThread.join();
	}


private:
	void CreateClient(const UINT32 maxClient) {
		for (int i = 0; i < maxClient; i++) {
			mClients.emplace_back();
			mClients[i].mIndex = i;
		}
	}

	void CreateWorkerThread() {
		for (int i = 0; i < MAX_WORKERTHREAD; i++) {
			mWorkerThread.emplace_back([this]() {WorkerThread(); });
		}
	}
	void CreateAccepterThread() {
		mAccepterThread = std::thread([this]() {AccepterThread(); });
	}

	bool BindRecv(stClientInfo* pClientInfo) {
		DWORD dwFlag = 0;
		DWORD dwRecvNumBytes = 0;

		// receive and send message
		pClientInfo->mRecvOverlapped.mWsaBuf.buf = pClientInfo->szRecvBuf;
		pClientInfo->mRecvOverlapped.mWsaBuf.len = MAXBUFSIZ;
		pClientInfo->mRecvOverlapped.mIOOperation = IOOperation::RECV;

		int nRet = WSARecv(pClientInfo->mClientSocket
			, &(pClientInfo->mRecvOverlapped.mWsaBuf)
			, 1
			, &dwRecvNumBytes
			, &dwFlag
			, (LPWSAOVERLAPPED) & (pClientInfo->mRecvOverlapped),
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

		pClientInfo->mSendOverlapped.mWsaBuf.buf = pClientInfo->szSendBuf;
		pClientInfo->mSendOverlapped.mWsaBuf.len = nLen;
		pClientInfo->mSendOverlapped.mIOOperation = IOOperation::SEND;


		int nRet = WSASend(pClientInfo->mClientSocket
			, &(pClientInfo->mSendOverlapped.mWsaBuf)
			, 1
			, &dwRecvNumBytes
			, 0
			, (LPWSAOVERLAPPED) & (pClientInfo->mSendOverlapped)
			, NULL);

		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
			printf("[ERROR] WSASend(): %d\n", WSAGetLastError());
			return false;
		}

		return true;
	}


	void WorkerThread() {
		// cp 대기..
		FileManager fm(L"C:\\Server\\Data");
		DWORD dwIoSize;
		DWORD dwFlag;
		stClientInfo* pClientInfo;
		LPOVERLAPPED lpOverlapped = NULL;

		while (mWorkerRunning) {
			bool bSuccess =
				GetQueuedCompletionStatus(mIOCPHandle
					, &dwIoSize
					, (PULONG_PTR)&pClientInfo
					, &lpOverlapped
					, INFINITE);

			if (TRUE == bSuccess && 0 == dwIoSize && NULL == lpOverlapped) {
				mWorkerRunning = false;
				continue;
			}

			if (NULL == lpOverlapped) {
				continue;
			}

			if (FALSE == bSuccess || (0 == dwIoSize && TRUE == bSuccess)) {
				printf("socket(%d) disconnected\n", pClientInfo->mClientSocket);
			}

			auto pOverlapped = (stOverlappedEX*)lpOverlapped;
			if (IOOperation::RECV == pOverlapped->mIOOperation) {
				pClientInfo->szRecvBuf[dwIoSize] = '\0';
				if (strncmp(pClientInfo->szRecvBuf, "list", 4) == 0) {
					if (fm.GetSerializedFileListForNet(sendBuffer)) {
						bool bSuccess = SendMsg(pClientInfo, sendBuffer.data(), (int)sendBuffer.size());

						if (bSuccess) {
							printf("[INFO] 파일 리스트 전송 시작 (%d bytes)\n", (int)sendBuffer.size());
						}
					}
				}
				else if (strncmp(pClientInfo->szRecvBuf, "OK  ", 4) == 0) {
					//SendMsg(pClientInfo, pClientInfo->szRecvBuf, dwIoSize);
				}
				else if (strncmp(pClientInfo->szRecvBuf, "get ", 4) == 0) {
					std::wstring targetFile = Utf8ToWide(pClientInfo->szRecvBuf + 4);
					LONGLONG fileSize = fm.GetFileSize(targetFile);
					std::string sizeStr = std::to_string(fileSize);

					SendMsg(pClientInfo, (char*)sizeStr.c_str(), dwIoSize);
				}
				else if (strcmp(pClientInfo->szRecvBuf, "quit") == 0) {
					CloseSocket();
				}
				else {
					SendMsg(pClientInfo, pClientInfo->szRecvBuf, dwIoSize);
				}


				BindRecv(pClientInfo);
			}
			else if (IOOperation::SEND == pOverlapped->mIOOperation) {
				// 전송 내용 출력 후 recv기다리기...
				printf("[SEND] bytes : %d , msg : %s\n", dwIoSize, pClientInfo->szSendBuf);
			}
			else {
				printf("[Error]Exception Error from socket(%d)\n", pClientInfo->mClientSocket);
			}
		}
	}
	void CloseSocket() {
		//TODO:
	}


	void AccepterThread() {
		// accept대기...후 cp bind
		int ret;
		SOCKADDR_IN stClientAddr;

		int addrlen = sizeof(SOCKADDR_IN);
		while (mAccepterRunning) {
			stClientInfo* pClientInfo = GetEmptyClientInfo();
			if (NULL == pClientInfo) {
				printf("[ERROR] Client Full!\n");
				continue;
			}

			pClientInfo->mClientSocket = accept(mListenSock, (struct sockaddr*)&pClientInfo->mClientAddr, &addrlen);
			if (INVALID_SOCKET == pClientInfo->mClientSocket) {
				printf("[ERROR] accept() :%d\n", WSAGetLastError());
				continue;
			}
			ret = BindIOCompletionPort(pClientInfo);
			if (false == ret) {
				printf("[ERROR] BindIOCompletionPort()\n");
				continue;
			}

			//TODO: recv ...
			if (false == BindRecv(pClientInfo)) {
				printf("[ERROR] bindRecv()\n");
				return;
			}

			++mClientCnt;

		}
	}

	bool BindIOCompletionPort(stClientInfo* pClientInfo) {
		auto hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->mClientSocket
			, mIOCPHandle
			, (ULONG_PTR)pClientInfo, MAX_WORKERTHREAD);
		if (NULL == hIOCP && mIOCPHandle != hIOCP) {
			printf("[ERROR] CreateIoCompletionPort() :%d\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	stClientInfo* GetEmptyClientInfo() {
		for (auto& e : mClients) {
			if (e.mClientSocket == INVALID_SOCKET) {
				return &e;
			}
		}

		return NULL;
	}

	WSADATA wsa;
	SOCKET mListenSock;
	SOCKADDR_IN serveraddr;
	int retval;
	HANDLE mIOCPHandle = NULL;

	std::vector<stClientInfo> mClients;
	std::vector<std::thread> mWorkerThread;
	std::thread mAccepterThread;

	bool mWorkerRunning = true;
	bool mAccepterRunning = true;

	UINT32 mClientCnt = 0;


	std::vector<char> sendBuffer;
};