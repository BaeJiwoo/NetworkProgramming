#include "UserManager.h"
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

	// RedisConnect

	mRedisManager = new RedisManager();

	std::string host = "172.18.175.153";
	uint16_t port = 6379;
	if (!mRedisManager->connect(host.c_str(), port)) {
		printf("Connect Error %s\n", mRedisManager->getErrorStr().c_str());
		std::this_thread::sleep_for(std::chrono::seconds(2));
		return -1;
	}
	else {
		printf("connect success\n");
	}

	return true;
}

void PacketManager::End() {
	mIsRunProcessThread = false;

	if (mProcessThread.joinable()) {
		mProcessThread.join();
	}
}

void PacketManager::ReceivePacketData(const UINT32 clientIndex_, const UINT32 size_, char* pData) {
	auto pUser = mUserManager->GetUserByConnIdx(clientIndex_);
	pUser->SetPacketData(size_, pData);

	EnquePacketData(clientIndex_);
}

void PacketManager::ProcessPacket()
{
	while (mIsRunProcessThread) {
		bool isIdle = true;

		if (auto packetData = DequePacketData(); packetData.PacketId > (UINT16)PACKET_ID::SYS_END) {
			isIdle = false;
			ProcessRecvPacket(packetData.ClientIndex, packetData.PacketId, packetData.DataSize, packetData.pDataPtr);
		}

		if (auto sysPacketData = DequeSystemPacketData(); sysPacketData.PacketId != 0) {
			isIdle = false;
			ProcessRecvPacket(sysPacketData.ClientIndex, sysPacketData.PacketId, sysPacketData.DataSize, sysPacketData.pDataPtr);

		}

		if (isIdle) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}

void PacketManager::EnquePacketData(const UINT32 clientIndex_) {
	std::lock_guard<std::mutex> guard(mLock);
	mIncomingPacketUserIndex.push_back(clientIndex_);
}

PacketInfo PacketManager::DequePacketData() {
	UINT32 userIndex = 0;

	{
		std::lock_guard<std::mutex> guard(mLock);
		if (mIncomingPacketUserIndex.empty()) {
			return PacketInfo();
		}

		userIndex = mIncomingPacketUserIndex.front();
		mIncomingPacketUserIndex.pop_front();
	}

	auto pUser = mUserManager->GetUserByConnIdx(userIndex);
	auto packetData = pUser->GetPacket(userIndex);
	packetData.ClientIndex = userIndex;
	return packetData;
}

void PacketManager::PushSystemPacket(PacketInfo packet_) {
	std::lock_guard<std::mutex> guard(mLock);
	mSystemPacketQueue.push_back(packet_);
}

PacketInfo PacketManager::DequeSystemPacketData() {
	std::lock_guard<std::mutex> guard(mLock);

	if (mSystemPacketQueue.empty()) {
		return PacketInfo();
	}

	auto sysPacket = mSystemPacketQueue.front();
	mSystemPacketQueue.pop_front();

	return sysPacket;
}

void PacketManager::ProcessRecvPacket(const UINT32 clientIndex_, const UINT16 packetId_, const UINT16 packetSize_, char* pPacket_) {
	auto iter = mRecvFunctionDictionary.find(packetId_);
	if (iter != mRecvFunctionDictionary.end()) {
		(this->*(iter->second))(clientIndex_, packetSize_, pPacket_);
	}
}


void PacketManager::ProcessUserConnect(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {
	printf("[ProcessUserConnect] clientIndex: %d\n", clientIndex_);
	auto pUser = mUserManager->GetUserByConnIdx(clientIndex_);
	pUser->Clear();
	// 로그인 로직 수행
}

void PacketManager::ProcessUserDisConnect(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {
	printf("[ProcessUserDisConnect] clientIndex: %d\n", clientIndex_);
	ClearConnectionInfo(clientIndex_);
}

void PacketManager::ProcessLogin(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_) {
	printf("ProcessLogin %d\n", clientIndex_);
	printf("[Message] %s\n", pPacket_);
	if (LOGIN_REQUEST_PACKET_SIZE != packetSize_){
		return;
	}
	
	auto pLoginReqPacket = reinterpret_cast<LOGIN_REQUEST_PACKET*>(pPacket_);

	auto pUserID = pLoginReqPacket->UserID;
	auto pUserPW = pLoginReqPacket->UserPW;
	printf("requested user id = %s, user pw = %s\n", pUserID, pUserPW);

	LOGIN_RESPONSE_PACKET loginResPacket;
	loginResPacket.PacketId = (UINT16)PACKET_ID::LOGIN_RESPONSE;
	loginResPacket.PacketLength = sizeof(LOGIN_RESPONSE_PACKET);

	if (mUserManager->GetCurrentUserCnt() >= mUserManager->GetMaxUserCnt()) {
		// 접속자 수가 최대 수를 차지해서 접속 불가
		loginResPacket.Result = (UINT16)ERROR_CODE::LOGIN_USER_USED_ALL_OBJ;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&loginResPacket);
		return;
	}

	// 이미 접속 중인 유저인지 확인, 접속된 유저라면 실패
	if (mUserManager->FindUserIndexByID(pLoginReqPacket->UserID) == -1) {
		loginResPacket.Result = (UINT16)ERROR_CODE::NONE;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&loginResPacket);
	}
	else {
		// 접속 중인 유저여서 실패 반환
		loginResPacket.Result = (UINT16)ERROR_CODE::LOGIN_USER_ALREADY;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&loginResPacket);
		return;
	}

	mRedisManager->checkuser(pUserID, pUserPW);
}

void PacketManager::ClearConnectionInfo(INT32 clientIndex_) {
	auto pReqUser = mUserManager->GetUserByConnIdx(clientIndex_);

	if (pReqUser->GetDomainState() != User::DOMAIN_STATE::NONE) {
		mUserManager->DeleteUserInfo(pReqUser);
	}
}