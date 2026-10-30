# 📑 IOCP 핵심 구조체 요약 노트 (Core Structures)

## 1. OVERLAPPED Structure
비동기 I/O의 '영수증'과 같습니다. 작업을 요청할 때 던지고, 완료될 때 다시 돌려받아 어떤 작업이 끝났는지 식별합니다.

```cpp
typedef struct _OVERLAPPED {
    ULONG_PTR Internal;       // [Internal] OS 전용 에러 코드 및 상태 저장
    ULONG_PTR InternalHigh;   // [Internal] 전송된 데이터의 바이트 수 저장
    union {
        struct {
            DWORD Offset;     // 파일 I/O 시 데이터를 읽기 시작할 위치 (Low)
            DWORD OffsetHigh; // 파일 I/O 시 데이터를 읽기 시작할 위치 (High)
        } DUMMYSTRUCTNAME;
        PVOID Pointer;        // 시스템 예약 (사용 금지)
    } DUMMYUNIONNAME;
    HANDLE hEvent;            // 이벤트 핸들 (IOCP에서는 보통 NULL로 설정)
} OVERLAPPED, *LPOVERLAPPED;
```

## 2. WSABUF Structure
비동기 입출력 함수(WSASend, WSARecv)에서 데이터를 담는 버퍼의 정보입니다.

```cpp
typedef struct _WSABUF {
    ULONG len;     // 버퍼의 크기 (Bytes)
    char  *buf;    // 실제 데이터 버퍼의 메모리 시작 주소
} WSABUF, *LPWSABUF;
```

## 3. SOCKADDR_IN Structure
네트워크 통신을 위한 IPv4 주소와 포트 번호를 담는 구조체입니다.

```cpp
typedef struct sockaddr_in {
    short          sin_family;   // 주소 체계 (AF_INET 고정)
    u_short        sin_port;     // 포트 번호 (htons()로 네트워크 바이트 정렬 필요)
    struct in_addr sin_addr;     // 4바이트 IP 주소 (in_addr 구조체)
    char           sin_zero[8];  // sockaddr 구조체와 크기를 맞추기 위한 패딩
} SOCKADDR_IN;
```

## 4. WSAPROTOCOL_INFO Structure
소켓의 하위 프로토콜 특성을 정의하며, 특정 프로토콜을 명시하여 소켓을 생성할 때 사용합니다.

```cpp
typedef struct _WSAPROTOCOL_INFOA {
    DWORD            dwServiceFlags1;
    DWORD            dwServiceFlags2;
    // ... (중략) ...
    int              iAddressFamily;  // 주소 체계 (AF_INET 등)
    int              iSocketType;     // SOCK_STREAM 등
    int              iProtocol;       // IPPROTO_TCP 등
    char             szProtocol[WSAPROTOCOL_LEN + 1]; // 프로토콜 이름
} WSAPROTOCOL_INFOA, *LPWSAPROTOCOL_INFOA;
```

## 5. TRANSMIT_FILE_BUFFERS Structure
`TransmitFile` 함수를 통해 파일을 전송할 때, 파일 데이터 전후에 보낼 데이터를 지정합니다.

```cpp
typedef struct _TRANSMIT_FILE_BUFFERS {
    PVOID Head;       // 파일 전송 전 전송할 데이터 (Header)
    DWORD HeadLength; // 헤더 버퍼의 길이
    PVOID Tail;       // 파일 전송 후 전송할 데이터 (Tail)
    DWORD TailLength; // 테일 버퍼의 길이
} TRANSMIT_FILE_BUFFERS, *PTRANSMIT_FILE_BUFFERS;
```

---
**💡 Study Note:**
- `OVERLAPPED` 구조체는 반드시 사용자 정의 구조체의 **첫 번째 멤버**여야 `GetQueuedCompletionStatus`에서 포인터를 그대로 캐스팅해 쓸 수 있어.
- `SOCKADDR_IN`의 `sin_port`는 반드시 `htons()`를 써서 네트워크 바이트 순서(Big-Endian)로 바꿔줘야 해!