#include "ChatServer.h"
#include <string>
#include <iostream>

const UINT16 SERVER_PORT = 11021;
const UINT16 MAX_CLIENT = 100;

int main()
{
	EchoServer server;

	server.InitSocket();

	server.BindandListen(SERVER_PORT);

	server.Run(MAX_CLIENT);

	printf("Press Any Key ...\n");
	while (true)
	{
		std::string inputCmd;
		std::getline(std::cin, inputCmd);

		if (inputCmd == "quit")
		{
			break;
		}
	}

	server.End();
	return 0;
}
