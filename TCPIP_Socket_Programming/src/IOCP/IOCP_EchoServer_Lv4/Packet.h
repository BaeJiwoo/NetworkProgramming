#pragma once
struct PacketData {
	UINT32 SessionIndex = 0;
	UINT32 DataSize = 0;
	char* pData = nullptr;
	void Set(PacketData& value) {
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