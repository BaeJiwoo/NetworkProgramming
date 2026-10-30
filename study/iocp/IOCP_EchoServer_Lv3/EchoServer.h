#pragma once

#include "IOCPServer.h"

class EchoServer : public IOCPServer {
	virtual void OnConnect(const UINT32 clientIndex_) override {
		printf("Client(%d) Connected\n", clientIndex_);
	}

	virtual void OnClose(const UINT32 clientIndex_) override {
		printf("ClientSocket(%d) Closed\n", clientIndex_);
	}

	virtual void OnReceive(const UINT32 clientIndex_) override {
		printf("Message Received From: %d\n", clientIndex_);
	}
};