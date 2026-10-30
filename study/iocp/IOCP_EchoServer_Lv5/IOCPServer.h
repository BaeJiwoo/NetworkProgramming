//출처: 강정중님의 저서 '온라인 게임서버'에서
#pragma once
#pragma comment(lib, "ws2_32")
#include "Define.h"
#include "ClientInfo.h"
#include <thread>
#include <vector>

class IOCPServer
{
public:
	IOCPServer(void) {}

	virtual ~IOCPServer(void)
	{
		//윈속의 사용을 끝낸다.
		WSACleanup();
	}

	//소켓을 초기화하는 함수
	bool InitSocket()
	{
		WSADATA wsaData;

		int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (0 != nRet)
		{
			printf("[에러] WSAStartup()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		//연결지향형 TCP , Overlapped I/O 소켓을 생성
		mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

		if (INVALID_SOCKET == mListenSocket)
		{
			printf("[에러] socket()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		printf("소켓 초기화 성공\n");
		return true;
	}

	//서버의 주소정보를 소켓과 연결시키고 접속 요청을 받기 위해 소켓을 등록하는 함수
	bool BindandListen(int nBindPort)
	{
		SOCKADDR_IN		stServerAddr;
		stServerAddr.sin_family = AF_INET;
		stServerAddr.sin_port = htons(nBindPort); //서버 포트를 설정한다.		
		stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		int nRet = bind(mListenSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
		if (0 != nRet)
		{
			printf("[에러] bind()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		nRet = listen(mListenSocket, 5);
		if (0 != nRet)
		{
			printf("[에러] listen()함수 실패 : %d\n", WSAGetLastError());
			return false;
		}

		printf("서버 등록 성공..\n");
		return true;
	}

	bool StartServer(const UINT32 maxClientCount)
	{
		CreateClient(maxClientCount);

		mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MAX_WORKERTHREAD);
		if (NULL == mIOCPHandle)
		{
			printf("[에러] CreateIoCompletionPort()함수 실패: %d\n", GetLastError());
			return false;
		}

		bool bRet = CreateWokerThread();
		if (false == bRet) {
			return false;
		}

		bRet = CreateAccepterThread();
		if (false == bRet) {
			return false;
		}

		bRet = CreateSenderThread();
		if (bRet == false) {
			return false;
		}

		printf("서버 시작\n");
		return true;
	}

	//생성되어있는 쓰레드를 파괴한다.
	void DestroyThread()
	{
		mIsWorkerRun = false;
		CloseHandle(mIOCPHandle);

		for (auto& th : mIOWorkerThreads)
		{
			if (th.joinable())
			{
				th.join();
			}
		}

		mIsAccepterRun = false;
		closesocket(mListenSocket);

		if (mAccepterThread.joinable())
		{
			mAccepterThread.join();
		}

		mIsSenderRun = false;
		if (mSenderThread.joinable()) {
			mSenderThread.join();
		}
	}

	bool SendMsg(UINT32 u32ClientIndex, char* pMsg, UINT32 nLen)
	{
		stClientInfo* pClientInfo = GetClientInfo(u32ClientIndex);
		return pClientInfo->SendMsg(pMsg, nLen) == false;
	}
	stClientInfo* GetClientInfo(const UINT32 sessionIndex)
	{
		return mClientInfos[sessionIndex];
	}

	// 네트워크 이벤트를 처리할 함수들
	virtual void Run(const UINT16 maxclient_) {}
	virtual void End() {}
	virtual void OnConnect(const UINT32 clientIndex_) {}
	virtual void OnClose(const UINT32 clientIndex_) {}
	virtual void OnReceive(const UINT32 clientIndex_, const UINT32 size_, char* pData_) {}
	virtual void OnSend(const UINT32 clientIndex_, const UINT32 size_, char* pData_) {}

private:
	void CreateClient(const UINT32 maxClientCount)
	{
		for (UINT32 i = 0; i < maxClientCount; ++i)
		{
			auto client = new stClientInfo();
			client->Init(i);

			mClientInfos.push_back(client);
		}
	}

	bool CreateWokerThread()
	{
		unsigned int uiThreadId = 0;
		for (int i = 0; i < MAX_WORKERTHREAD; i++)
		{
			mIOWorkerThreads.emplace_back([this]() { WorkerThread(); });
		}

		printf("WokerThread 시작..\n");
		return true;
	}

	//accept요청을 처리하는 쓰레드 생성
	bool CreateAccepterThread()
	{
		mAccepterThread = std::thread([this]() { AccepterThread(); });

		printf("AccepterThread 시작..\n");
		return true;
	}

	bool CreateSenderThread() {
		mSenderThread = std::thread([this]() { SendThread();  });

		printf("SendThread 시작..\n");
		return true;
	}

	//사용하지 않는 클라이언트 정보 구조체를 반환한다.
	stClientInfo* GetEmptyClientInfo()
	{
		for (auto& client : mClientInfos)
		{
			if (!client->IsConnected())
			{
				return client;
			}
		}
		return nullptr;
	}








	void WorkerThread()
	{
		stClientInfo* pClientInfo = nullptr;
		BOOL bSuccess = TRUE;
		DWORD dwIoSize = 0;
		LPOVERLAPPED lpOverlapped = NULL;

		while (mIsWorkerRun)
		{
			bSuccess = GetQueuedCompletionStatus(mIOCPHandle,
				&dwIoSize,					// 실제로 전송된 바이트
				(PULONG_PTR)&pClientInfo,		// CompletionKey
				&lpOverlapped,				// Overlapped IO 객체
				INFINITE);					// 대기할 시간

			//사용자 쓰레드 종료 메세지 처리..
			if (TRUE == bSuccess && 0 == dwIoSize && NULL == lpOverlapped)
			{
				mIsWorkerRun = false;
				continue;
			}

			if (NULL == lpOverlapped)
			{
				continue;
			}

			//client가 접속을 끊었을때..			
			if (FALSE == bSuccess || (0 == dwIoSize && TRUE == bSuccess))
			{
				//printf("socket(%d) 접속 끊김\n", (int)pClientInfo->m_socketClient);
				CloseSocket(pClientInfo);
				continue;
			}


			auto pOverlappedEx = (stOverlappedEx*)lpOverlapped;

			//Overlapped I/O Recv작업 결과 뒤 처리
			if (IOOperation::RECV == pOverlappedEx->m_eOperation)
			{
				OnReceive(pClientInfo->GetIndex(), dwIoSize, pClientInfo->GetRecvBuf());
				//pClientInfo->mRecvBuf[dwIoSize] = '\0';
				//printf("[수신] bytes : %d , msg : %s\n", dwIoSize, pClientInfo->mRecvBuf);

				//클라이언트에 메세지를 에코한다.

				// TODO: 이 부분 수정 필요할 듯
				//SendMsg(pClientInfo, pClientInfo->mRecvBuf, dwIoSize);

				//BindRecv(pClientInfo);
				pClientInfo->BindRecv();
			}
			//Overlapped I/O Send작업 결과 뒤 처리
			else if (IOOperation::SEND == pOverlappedEx->m_eOperation)
			{
				//printf("[송신] bytes : %d , msg : %s\n", dwIoSize, pClientInfo->mSendBuf);
				//OnSend(pClientInfo->GetIndex(), dwIoSize, pClientInfo->GetRecvBuf());

				//delete[] pOverlappedEx->m_wsaBuf.buf;
				//delete pOverlappedEx;
				pClientInfo->SendCompleted(dwIoSize);
			}
			//예외 상황
			else
			{
				printf("socket(%d)에서 예외상황\n", (int)pClientInfo->GetSocketClient());
			}
		}
	}

	//사용자의 접속을 받는 쓰레드
	void AccepterThread()
	{
		SOCKADDR_IN		stClientAddr;
		int nAddrLen = sizeof(SOCKADDR_IN);

		while (mIsAccepterRun)
		{
			//접속을 받을 구조체의 인덱스를 얻어온다.
			stClientInfo* pClientInfo = GetEmptyClientInfo();
			if (NULL == pClientInfo)
			{
				printf("[에러] Client Full\n");
				return;
			}

			//클라이언트 접속 요청이 들어올 때까지 기다린다.

			// TODO: OnConnect로 연결 관리
			SOCKET newSocket = accept(mListenSocket, (SOCKADDR*)&stClientAddr, &nAddrLen);
			if (INVALID_SOCKET == newSocket)
			{
				continue;
			}

			if (pClientInfo->OnConnect(mIOCPHandle, newSocket) == false) {
				pClientInfo->Close(true);
			}
			//I/O Completion Port객체와 소켓을 연결시킨다.
			//bool bRet = BindIOCompletionPort(pClientInfo);
			//if (false == bRet)
			//{
			//	return;
			//}

			////Recv Overlapped I/O작업을 요청해 놓는다.
			//bRet = BindRecv(pClientInfo);
			//if (false == bRet)
			//{
			//	return;
			//}

			//char clientIP[32] = { 0, };
			//inet_ntop(AF_INET, &(stClientAddr.sin_addr), clientIP, 32 - 1);
			//printf("클라이언트 접속 : IP(%s) SOCKET(%d)\n", clientIP, (int)pClientInfo->m_socketClient);

			OnConnect(pClientInfo->GetIndex());

			//클라이언트 갯수 증가
			++mClientCnt;
		}
	}

	void SendThread() {
		while (mIsSenderRun) {
			for (auto e : mClientInfos) {
				if (!e->IsConnected()) continue;

				e->SendIO();
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(8));
		}
	}

	// TODO: ClientInfo가 해결
	//소켓의 연결을 종료 시킨다.
	void CloseSocket(stClientInfo* pClientInfo, bool bIsForce = false)
	{
		auto clientIndex = pClientInfo->GetIndex();

		pClientInfo->Close();

		OnClose(clientIndex);
	}



	//클라이언트 정보 저장 구조체
	std::vector<stClientInfo*> mClientInfos;

	//클라이언트의 접속을 받기위한 리슨 소켓
	SOCKET		mListenSocket = INVALID_SOCKET;

	//접속 되어있는 클라이언트 수
	int			mClientCnt = 0;

	//IO Worker 스레드
	std::vector<std::thread> mIOWorkerThreads;

	//Accept 스레드
	std::thread	mAccepterThread;

	//send thread
	std::thread mSenderThread;

	//CompletionPort객체 핸들
	HANDLE		mIOCPHandle = INVALID_HANDLE_VALUE;

	//작업 쓰레드 동작 플래그
	bool		mIsWorkerRun = true;

	//접속 쓰레드 동작 플래그
	bool		mIsAccepterRun = true;

	//send thread 동작 플래그
	bool		mIsSenderRun = true;

	//소켓 버퍼
	char		mSocketBuf[1024] = { 0, };
};