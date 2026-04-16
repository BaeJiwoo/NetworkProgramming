/*
* =====================================================================
* IOCPServer와 서비스의 중간점
* 받은 메시지를 Packet처리 스레드로 보낸다.
* =====================================================================
*/

#pragma once

#include "IOCPServer.h"
#include <mutex>
#include "Packet.h"
#include "PacketManager.h"
#include <deque>

class EchoServer : public IOCPServer
{
public:
	virtual void Run(const UINT16 maxclient_) override {
		auto sendPacketFunc = [&](UINT32 clientIndex_, UINT16 packetSize, char* pSendPacket) {
			SendMsg(clientIndex_, pSendPacket, packetSize);
			};

		m_pPacketManager = std::make_unique<PacketManager>();
		m_pPacketManager->Init();
		m_pPacketManager->Run();	


		StartServer(maxclient_);
	}

	virtual void End() override {
		/*
		* TODO: 서비스 종료 로직 구현
		* IOCPServer.h 호출해서 수행함.
		*/

		DestroyThread();
		mIsPacketProcessRun = false;
		if (mPacketProcessThread.joinable()) mPacketProcessThread.join();


	}

	virtual void OnConnect(const UINT32 clientIndex_) override
	{
		printf("[OnConnect] 클라이언트: Index(%d)\n", clientIndex_);
	}

	virtual void OnClose(const UINT32 clientIndex_) override
	{
		printf("[OnClose] 클라이언트: Index(%d)\n", clientIndex_);
	}

	virtual void OnSend(const UINT32 clientIndex_, const UINT32 size_, char* pData_) override {

	}

	virtual void OnReceive(const UINT32 clientIndex_, const UINT32 size_, char* pData_) override
	{
		printf("[OnReceive] 클라이언트: Message(%s) Index(%d), dataSize(%d)\n", pData_, clientIndex_, size_);
		/*
		* TODO: 입력 받고 큐에 넣는 로직 구현
		* - 파라미터를 큐에 삽입(여기서 삽입된 데이터를 패킷 처리 스레드가 처리함)
		* - 큐 삽입할 떄, 스레드 동기화 필요
		*/
		PacketData packetData;
		packetData.Set(clientIndex_, size_, pData_);
		std::lock_guard<std::mutex>  lock(mMutex);
		mPacketDataQueue.push_back(packetData);
	}
	

private:
	


	// Properties
	std::thread mPacketProcessThread;
	std::deque<PacketData> mPacketDataQueue;
	bool mIsPacketProcessRun = true;
	std::unique_ptr<PacketManager> m_pPacketManager;

	std::mutex mMutex;
};