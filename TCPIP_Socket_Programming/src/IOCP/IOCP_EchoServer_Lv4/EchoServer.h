/*
* =====================================================================
* EchoServer은 IOCPServer에서 처리한 패킷을 실질적으로 처리하는 스레드임
* IOCPServer가 데이터 Recv Send만을 담당
* EchoServer는 그 Recv와 Send 시, 처리 방식을 담당함.
* Recv와 Send의 책임을 IOCPServer만 담당하고
* EchoServer가 그것을 이용한다고 생각하면 된다.
* =====================================================================
*/
#pragma once

#include "IOCPServer.h"
#include "Packet.h"
#include <deque>

class EchoServer : public IOCPServer
{
public:
	virtual void Run(const UINT16 maxclient_) override {
		StartServer(maxclient_);
		mPacketProcessThread = std::thread([this]() {ProcessPacket(); });
	}

	virtual void End() override {
		/*
		* TODO: 서비스 종료 로직 구현
		* IOCPServer.h 호출해서 수행함.
		*/
	}

	virtual void OnConnect(const UINT32 clientIndex_) override
	{
		printf("[OnConnect] 클라이언트: Index(%d)\n", clientIndex_);
	}

	virtual void OnClose(const UINT32 clientIndex_) override
	{
		printf("[OnClose] 클라이언트: Index(%d)\n", clientIndex_);
	}

	virtual void OnReceive(const UINT32 clientIndex_, const UINT32 size_, char* pData_) override
	{
		printf("[OnReceive] 클라이언트: Index(%d), dataSize(%d)\n", clientIndex_, size_);
		/*
		* TODO: 입력 받고 큐에 넣는 로직 구현
		* 파라미터를 큐에 삽입(여기서 삽입된 데이터를 패킷 처리 스레드가 처리함)
		*/
	}


private:
	void ProcessPacket() {
		/*
		TODO: 패킷처리 로직
		- queue에 데이터 들어올 때까지 기다림
		- Enqueue시, 복사하여 처리
		- Send로 데이터 전송함.(EchoServer라 처리할 게 이게 다임ㅇㅇ)
		*/

	}


	// Properties
	std::thread mPacketProcessThread;
	std::deque<Packet> mPacketQueue;
};