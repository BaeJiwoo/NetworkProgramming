#include "IOCPServer.h"

#define PORT 9000

int main() {
	IOCPServer server;
	server.InintSocket();

	server.BindandListen(9000);

	server.StartServer(100);
	
	printf("Type Anything to quit..\n");
	getchar();
	server.DestroyThread();
}