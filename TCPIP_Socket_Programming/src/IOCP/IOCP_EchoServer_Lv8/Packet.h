#pragma once
#include <Windows.h>

struct RawPacketData {
	UINT32 SessionIndex = 0;
	UINT32 DataSize = 0;
	char* pData = nullptr;
	void Set(RawPacketData& value) {
		SessionIndex = value.SessionIndex;
		DataSize = value.DataSize;
		pData = value.pData;
	}

	void Set(UINT32 sessionIndex_, UINT32 dataSize_, char* pData_) {
		SessionIndex = sessionIndex_;
		DataSize = dataSize_;
		pData = pData_;
	}
};


#pragma pack(push,1)
struct PACKET_HEADER
{
	UINT16 PacketLength;
	UINT16 PacketId;
	UINT8 Type; //압축여부 암호화여부 등 속성을 알아내는 값
};

const UINT32 PACKET_HEADER_LENGTH = sizeof(PACKET_HEADER);

struct PacketInfo {
	PACKET_ID PacketId;
	UINT32 DataSize;
	char* pDataPtr;
};

enum class PACKET_ID : UINT16 {
	//SYSTEM
	SYS_USER_CONNECT = 11,
	SYS_USER_DISCONNECT = 12,
	SYS_END = 30,

	//DB
	DB_END = 199,

	//Client
	LOGIN_REQUEST = 201,
	LOGIN_RESPONSE = 202,

	ROOM_ENTER_REQUEST = 206,
	ROOM_ENTER_RESPONSE = 207,

	ROOM_LEAVE_REQUEST = 215,
	ROOM_LEAVE_RESPONSE = 216,

	ROOM_CHAT_REQUEST = 221,
	ROOM_CHAT_RESPONSE = 222,
	ROOM_CHAT_NOTIFY = 223,
};