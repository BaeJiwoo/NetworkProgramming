
#include "PacketManager.h"

void PacketManager::Init(const UINT32 maxClient_) {
	mRecvFunctionDictionary = std::unordered_map<int, PROCESS_RECV_PACKET_FUNCTION>();

	mRecvFunctionDictionary[(int)PACKET_ID::SYS_USER_CONNECT] = &PacketManager::ProcessUserConnect;
	mRecvFunctionDictionary[(int)PACKET_ID::SYS_USER_DISCONNECT] = &PacketManager::ProcessUserDisConnect;

	mRecvFunctionDictionary[(int)PACKET_ID::LOGIN_REQUEST] = &PacketManager::ProcessLogin;

	CreateComponent(maxClient_);
}

void PacketManager::CreateComponent(const UINT32 maxClient_) {
	mUserManager = new UserManager;
	mUserManager->Init(maxClient_);
}

bool PacketManager::Run() {
	mIsRunProcessThread = true;
	mProcessThread = std::thread(
		[this]() {ProcessPacket(); }
	);

	return true;
}

void PacketManager::End() {
	mIsRunProcessThread = false;

	if (mProcessThread.joinable()) {
		mProcessThread.join();
	}
}

void PacketManager::ReceivePacketData(const UINT32 clientIndex_, const UINT32 size_, char* pData) {
	auto pUser = mUserManager
}