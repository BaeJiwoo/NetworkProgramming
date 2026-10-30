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
			SendMsg(clientIndex_, pSendPacket, (UINT32)packetSize);
			};

		m_pPacketManager = std::make_unique<PacketManager>();
		m_pPacketManager->SendPacketFunc = sendPacketFunc;
		m_pPacketManager->Init(maxclient_);
		m_pPacketManager->Run();	


		StartServer(maxclient_);
	}

	virtual void End() override {
		m_pPacketManager->End();

		DestroyThread();
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
		m_pPacketManager->ReceivePacketData(clientIndex_, size_, pData_);
	}
	

private:
	


	// Properties
	std::thread mPacketProcessThread;
	//std::deque<Pac.ketData> mPacketDataQueue;
	bool mIsPacketProcessRun = true;
	std::unique_ptr<PacketManager> m_pPacketManager;

	std::mutex mMutex;
};