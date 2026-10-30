#include "Server.h"

int main() {
    SimpleTcpServer server(8080); // 8080 포트 사용
    if (server.Start()) {
        server.Run();
    }
    return 0;
}