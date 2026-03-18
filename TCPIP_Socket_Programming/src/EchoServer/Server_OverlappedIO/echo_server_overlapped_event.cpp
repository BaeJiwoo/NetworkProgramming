#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

#define MAXLINE 1024
#define PORT 9000

struct SOCKETINFO {
    WSAOVERLAPPED overlapped;
    SOCKET socket;
    WSABUF dataBuf;
    char buf[MAXLINE];
};

struct SOCKETINFO* socketArray[WSA_MAXIMUM_WAIT_EVENTS];
WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
int EventTotal = 0;
CRITICAL_SECTION CriticalSection;

DWORD WINAPI ThreadProc(LPVOID argv);

int main(int argc, char** argv) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;
    InitializeCriticalSection(&CriticalSection);

    SOCKET listensock, clientsock;
    struct sockaddr_in addr;
    DWORD ThreadId;

    listensock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0x00, sizeof(addr));
    addr.sin_port = htons(PORT);
    addr.sin_family = AF_INET;
    addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    bind(listensock, (struct sockaddr*)&addr, sizeof(addr));
    listen(listensock, 5);

    // 스레드 생성
    CreateThread(NULL, 0, ThreadProc, NULL, 0, &ThreadId);

    printf("Server started on port %d...\n", PORT);

    while (1) {
        clientsock = accept(listensock, NULL, NULL);
        if (clientsock == INVALID_SOCKET) break;

        EnterCriticalSection(&CriticalSection);
        if (EventTotal >= WSA_MAXIMUM_WAIT_EVENTS) {
            closesocket(clientsock);
            LeaveCriticalSection(&CriticalSection);
            continue;
        }

        struct SOCKETINFO* sInfo = (struct SOCKETINFO*)malloc(sizeof(struct SOCKETINFO));
        memset(sInfo, 0, sizeof(struct SOCKETINFO));
        sInfo->socket = clientsock;
        sInfo->dataBuf.buf = sInfo->buf;
        sInfo->dataBuf.len = MAXLINE;
        sInfo->overlapped.hEvent = WSACreateEvent();

        EventArray[EventTotal] = sInfo->overlapped.hEvent;
        socketArray[EventTotal] = sInfo;

        DWORD flags = 0, recvBytes = 0;
        WSARecv(sInfo->socket, &sInfo->dataBuf, 1, &recvBytes, &flags, &sInfo->overlapped, NULL);

        EventTotal++;
        LeaveCriticalSection(&CriticalSection);
    }

    DeleteCriticalSection(&CriticalSection);
    WSACleanup();
    return 0;
}

DWORD WINAPI ThreadProc(LPVOID argv) {
    DWORD readn, flags;
    int index, i, pos;
    struct SOCKETINFO* si;

    while (1) {
        // EventTotal이 0이면 대기하지 않음
        if (EventTotal == 0) { Sleep(10); continue; }

        index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE);
        if (index == WSA_WAIT_FAILED) continue;

        pos = index - WSA_WAIT_EVENT_0;
        WSAResetEvent(EventArray[pos]);
        si = socketArray[pos];

        if (WSAGetOverlappedResult(si->socket, &si->overlapped, &readn, FALSE, &flags) == FALSE || readn == 0) {
            // 클라이언트 종료 처리
            EnterCriticalSection(&CriticalSection);
            closesocket(si->socket);
            WSACloseEvent(EventArray[pos]);
            free(si);

            // 중요: 배열 당기기 로직 수정
            for (i = pos; i < EventTotal - 1; i++) {
                EventArray[i] = EventArray[i + 1];
                socketArray[i] = socketArray[i + 1];
            }
            EventTotal--;
            LeaveCriticalSection(&CriticalSection);
            continue;
        }

        // 수신 데이터 에코 (strlen 대신 readn 사용)
        send(si->socket, si->buf, readn, 0);

        // 다음 수신 예약
        WSAEVENT hTemp = si->overlapped.hEvent;
        memset(&si->overlapped, 0, sizeof(WSAOVERLAPPED));
        si->overlapped.hEvent = hTemp;
        flags = 0;

        WSARecv(si->socket, &si->dataBuf, 1, &readn, &flags, &si->overlapped, NULL);
    }
    return 0;
}