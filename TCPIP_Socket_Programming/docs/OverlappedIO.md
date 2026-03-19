# 윈도우 중첩 입출력(Overlapped I/O) vs 리눅스 모델 비교 및 실무 가이드

이 문서는 윈도우 중첩 입출력 모델의 동작 원리, 리눅스 모델과의 메모리 관점 차이, 그리고 실제 `WSARecv` 사용법을 정리한 리포트입니다.

---

## 1. 모델 개요 및 철학의 차이

| 구분 | 윈도우 (Overlapped I/O) | 리눅스 (전통적 epoll) |
| :--- | :--- | :--- |
| **모델 종류** | **Proactor** (완료 기반) | **Reactor** (준비 기반) |
| **핵심 철학** | "네 버퍼 주소를 주면 내가 다 채우고 알려줄게." | "데이터가 왔으니 네가 직접 가져가." |
| **알림 시점** | 요청한 I/O 작업이 **완료된 후** 통지 | I/O를 수행할 **준비가 되었을 때** 통지 |

---

## 2. 데이터 흐름 및 메모리 관점 (1,500바이트 수신 예시)

데이터가 여러 번에 걸쳐 나누어 도착할 때, 두 모델은 메모리 복사 횟수와 문맥 교환에서 큰 차이를 보입니다.

### 🔴 리눅스 (전통적 epoll 방식)
- **메모리 경로:** `NIC` → `커널 버퍼` → `유저 버퍼` (**2단계 복사**)
- **동작:** 데이터가 도착할 때마다 커널이 알림 → 유저가 `read()` 호출 → 유저 영역으로 데이터 복사.
- **비효율:** 데이터 조각마다 유저/커널 모드 전환(Context Switch) 발생.

### 🔵 윈도우 (중첩 입출력 방식)
- **메모리 경로:** `NIC` → `유저 버퍼` (**직통 복사**)
- **동작:** 유저 버퍼 주소를 커널에 미리 대여 → 커널이 해당 주소에 직접 데이터를 꽂음 → 완료 시 1회 통지.
- **효율:** 중간 복사 단계가 생략(Zero-copy 지향)되며, 모드 전환이 최소화됨.



---

## 3. WSARecv 호출 및 처리 과정

`WSARecv`는 비동기 수신을 예약하는 핵심 함수입니다.

1. **예약 단계:** `WSABUF`에 버퍼 주소를 담아 커널에 전달.
2. **메모리 고정(Pinning):** 커널은 전달받은 유저 버퍼를 물리적 메모리에 고정하여 이동/해제를 방지.
3. **직접 전송:** 커널이 유저 버퍼에 데이터를 직접 기록.
4. **완료 통보:** 데이터가 모두 차면 `IOCP`나 `Event`를 통해 애플리케이션에 완료 신호 송신.



---

## 4. 실제 사용법 (C++ 코드 예시)

이벤트 기반의 중첩 입출력 구현 예시입니다.

```cpp
// 1. 구조체 준비 및 초기화
WSAOVERLAPPED overlapped;
WSABUF dataBuffer;
char buffer[1024]; // 데이터가 직접 저장될 공간
DWORD recvBytes = 0, flags = 0;

ZeroMemory(&overlapped, sizeof(WSAOVERLAPPED));
overlapped.hEvent = WSACreateEvent(); // 완료 통지용 이벤트

dataBuffer.len = 1024;
dataBuffer.buf = buffer;

// 2. 비동기 수신 예약 (커널에 버퍼 주소 전달)
int result = WSARecv(socket, &dataBuffer, 1, &recvBytes, &flags, &overlapped, NULL);

// 3. 비차단 특성 활용
if (result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING) {
    printf("I/O 작업이 배경에서 진행 중입니다. 다른 연산을 수행합니다...\n");
}

// 4. 완료 확인
WSAWaitForMultipleEvents(1, &overlapped.hEvent, TRUE, WSA_INFINITE, FALSE);
WSAGetOverlappedResult(socket, &overlapped, &recvBytes, FALSE, &flags);

printf("수신 완료: %d 바이트\n", recvBytes);