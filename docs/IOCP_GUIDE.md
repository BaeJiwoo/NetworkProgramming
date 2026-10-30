# IOCP 단계별 안내

[저장소 처음으로](../README.md) · [교재 학습 안내](LEARNING_GUIDE.md) · [mogameserver 개발 계획](MOGAMESERVER_ROADMAP.md)

IOCP 실습은 두 위치에 있습니다. [EchoServer의 Server_IOCP](../study/tcpip/EchoServer/Server_IOCP)는 기본 예제이고, [study/iocp](../study/iocp)는 C++ 서버 구조를 단계별로 확장한 코드입니다. 아래 단계는 교재 챕터 번호와 별개입니다.

## 단계별 코드와 솔루션

기본 코드에서 Lv9까지의 읽을거리와 진입점을 정리했습니다. 빌드 및 실행은 확인 전이므로, 단계가 높다는 이유만으로 이전 단계보다 안정적이라고 판단하지 않습니다.

| 단계 | 코드에서 살펴볼 내용 | 열어야 할 파일 |
| --- | --- | --- |
| 기본 코드 | [IOCompletionPort.h](../study/iocp/IOCP_EchoServer/IOCompletionPort.h)의 IOCP 서버 구조. `main.cpp`는 비어 있으며 솔루션에 포함되지 않음 | [프로젝트 파일](../study/iocp/IOCP_EchoServer/IOCP_EchoServer.vcxproj) |
| Lv2 | `IOCompletionPort`를 생성하고 실행하는 진입점, 워커 스레드와 비동기 송수신 | [IOCP.sln](../study/iocp/IOCP.sln) |
| Lv3 | `IOCPServer`와 이를 상속하는 `EchoServer`, 접속 및 수신 콜백 | [Lv3 솔루션](../study/iocp/IOCP_EchoServer_Lv3/IOCP_EchoServer_Lv3.sln) |
| Lv4 | `ClientInfo`로 연결 관련 동작을 나누고, 에코 처리에 큐와 별도 스레드를 사용 | [Lv4 솔루션](../study/iocp/IOCP_EchoServer_Lv4/IOCP_EchoServer_Lv4.sln) |
| Lv5 | 송신 데이터를 저장하는 버퍼와 실제 `WSASend`를 수행하는 `SendIO` 분리 | [Lv5 솔루션](../study/iocp/IOCP_EchoServer_Lv5/IOCP_EchoServer_Lv5.sln) |
| Lv6 | 송신 요청을 큐에 보관하고, 완료 처리에서 다음 송신을 이어가는 구조 | [Lv6 솔루션](../study/iocp/IOCP_EchoServer_Lv6/IOCP_EchoServer_Lv6.sln) |
| Lv7 | `AcceptEx`를 이용한 접속 수락과 완료 처리, 연결 객체 재사용 관련 흐름 | [Lv7 솔루션](../study/iocp/Project1IOCP_EchoServer_Lv7/Project1IOCP_EchoServer_Lv7.sln) |
| Lv8 | `PacketManager`, `User`, `UserManager` 도입. 패킷 헤더와 로그인 처리 코드 | [Lv8 솔루션](../study/iocp/IOCP_EchoServer_Lv8/IOCP_EchoServer_Lv8.sln) |
| Lv9 | `RedisConnector`, `RedisManager`, `RedisTask`와 로그인 요청의 Redis 연동 작업 | [Lv9 솔루션](../study/iocp/IOCP_EchoServer_Lv9/IOCP_EchoServer_Lv9.sln) |

`IOCP.sln`은 Lv2만 포함합니다. Lv3부터는 위 표의 개별 솔루션을 엽니다. Lv7의 실제 폴더명은 `Project1IOCP_EchoServer_Lv7`입니다.

## 비교하며 읽는 순서

1. **Lv2에서 Lv3:** 소켓 입출력과 응용 동작을 어떤 클래스와 콜백으로 나누었는지 확인합니다.
2. **Lv3에서 Lv4:** 연결 정보를 관리하는 코드와 수신 데이터를 처리하는 스레드가 어디로 이동했는지 봅니다.
3. **Lv4에서 Lv6:** 송신 버퍼 방식과 송신 큐 방식을 비교하고, 요청부터 완료까지 데이터가 보관되는 위치를 따라갑니다.
4. **Lv6에서 Lv7:** 접속 수락 요청과 완료 통지가 기존 송수신 처리와 어떻게 합쳐지는지 봅니다.
5. **Lv7에서 Lv8:** 수신 바이트를 패킷으로 해석하는 부분과 사용자별 상태 관리의 경계를 확인합니다.
6. **Lv8에서 Lv9:** 로그인 요청이 Redis 작업으로 전달되고 응답이 돌아오는 경로를 따라갑니다.

Lv8과 Lv9에는 채팅방 관련 패킷 정의도 있습니다. 패킷 정의가 존재하는 것과 해당 기능의 처리 흐름이 완성된 것은 별개이므로, 실제 등록된 처리 함수는 `PacketManager.cpp`에서 확인합니다.

## 서버와 클라이언트 실행 조건

Lv2부터 Lv9까지의 `main.cpp`에는 서버 포트 `11021`, 최대 클라이언트 수 `100`이 지정되어 있습니다. 실행할 단계의 서버를 먼저 시작하고, 같은 포트를 사용하는 다른 단계의 서버는 종료해 둡니다.

[텍스트 에코 클라이언트 솔루션](../study/iocp/client/client.sln)은 서버 IP와 포트를 인수로 받습니다. 빌드 출력 폴더에서 다음과 같이 실행합니다.

```powershell
.\client.exe 127.0.0.1 11021
```

이 클라이언트는 입력한 문자열을 그대로 전송합니다. Lv8과 Lv9는 `PACKET_HEADER`와 패킷 ID를 해석하므로, 이 클라이언트만으로 로그인이나 채팅 패킷을 검증할 수 없습니다. 해당 단계는 각 단계의 `Packet.h` 형식에 맞는 클라이언트가 필요합니다.

Lv2의 서버는 입력을 기다린 뒤 종료 처리를 호출하고, Lv3부터 Lv9의 서버는 콘솔에서 `quit`를 입력하면 종료 처리를 호출하는 구조입니다. 실제 종료 동작은 실행 확인 대상입니다.

## Lv9 실행 조건

Lv9는 Redis 연동 작업이 진행 중인 단계입니다. 실행 전 다음 파일에 지정된 조건을 확인합니다.

| 파일 | 현재 지정된 조건 |
| --- | --- |
| [프로젝트 설정](../study/iocp/IOCP_EchoServer_Lv9/IOCP_EchoServer_Lv9.vcxproj) | 일부 구성에서 개인 PC의 절대 경로로 `Libs/include`와 `Libs/lib`를 참조하고 `hiredis.lib`를 링크 |
| [Libs](../study/iocp/IOCP_EchoServer_Lv9/Libs) | hiredis 헤더와 라이브러리 파일 포함 |
| [RedisConnector.h](../study/iocp/IOCP_EchoServer_Lv9/RedisConnector.h) | `sw/redis++/redis++.h` 참조. 포함된 hiredis 파일과 별도로 헤더 및 링크 의존성 확인 필요 |
| [PacketManager.cpp](../study/iocp/IOCP_EchoServer_Lv9/PacketManager.cpp) | Redis 접속 대상이 `172.18.175.153:6379`로 코드에 지정되어 있음 |

## 다음 학습 기록

단계를 비교할 때는 다음 질문에 대한 답을 남깁니다.

- 이전 단계에서 바꾸려던 문제는 무엇인가?
- 접속, 수신, 송신, 종료 시 어떤 클래스와 스레드가 동작하는가?
- 비동기 작업이 완료될 때까지 버퍼를 누가 보관하는가?
- 연결이 끊기거나 요청이 실패했을 때 자원을 누가 정리하는가?
- 이 중 `mogameserver`에서 공통으로 제공할 부분은 무엇인가?

관련 개념은 [IOCP 구조체 노트](notes/IOCP관련구조체.md)와 [Overlapped I/O 노트](notes/OverlappedIO.md)를 참고합니다.
