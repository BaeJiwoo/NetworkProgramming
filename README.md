# NetworkProgramming

윤성우의 **열혈 TCP/IP 소켓 프로그래밍**을 챕터별로 따라 공부하고, 배운 내용을 바탕으로 **mogameserver** 게임 서버 라이브러리를 만들어 가는 저장소입니다.

기초 소켓 실습, IOCP 서버를 단계별로 확장한 코드, 응용 프로젝트와 학습 노트를 함께 보관합니다. 현재 서버 코드는 개별 실행 프로젝트로 구성되어 있으며, `mogameserver`의 독립 라이브러리 프로젝트는 아직 없습니다.

## 바로 가기

| 하려는 일 | 안내 |
| --- | --- |
| 교재에서 배운 주제의 코드 찾기 | [교재 학습과 프로젝트 안내](docs/LEARNING_GUIDE.md) |
| IOCP 단계별 차이와 솔루션 찾기 | [IOCP 단계별 안내](docs/IOCP_GUIDE.md) |
| 학습 내용을 라이브러리 개발로 연결하기 | [mogameserver 개발 계획](docs/MOGAMESERVER_ROADMAP.md) |
| 기본 TCP 에코 코드 읽기 | [서버](study/tcpip/EchoServer/TCPServer/echo_server.cpp) · [클라이언트](study/tcpip/EchoServer/TCPClient/echo_client.cpp) |
| 최근 IOCP 작업 살펴보기 | [Lv9 Redis 연동 작업](study/iocp/IOCP_EchoServer_Lv9) |

## 학습 흐름

1. 소켓 생성, 주소 표현, 바이트 순서와 TCP 연결 과정을 익힙니다.
2. TCP와 UDP 계산기, 에코 서버로 송수신 방식을 비교합니다.
3. 멀티스레드와 여러 I/O 모델로 연결 처리 방식을 확장합니다.
4. IOCP 단계별 코드를 비교하며 연결 관리, 송신 큐, 패킷 처리를 살펴봅니다.
5. 각 단계에서 재사용할 부분을 기록하고 `mogameserver`의 책임과 인터페이스를 설계합니다.

위 순서는 저장소를 읽는 안내 순서입니다. 교재에서 배운 주제에 해당하는 코드는 [학습 안내](docs/LEARNING_GUIDE.md)에서 찾아볼 수 있습니다.

## 저장소 구성

```text
NetworkProgramming/
├─ README.md
├─ study/
│  ├─ tcpip/                     교재의 소켓 프로그래밍 실습
│  │  ├─ EchoServer/             TCP 에코와 여러 I/O 모델
│  │  ├─ NetworkCalculator/      TCP와 UDP 계산기
│  │  ├─ IOMultiplexing/         select 기반 실습
│  │  ├─ MultithreadEchoServer/  멀티스레드 에코 실습
│  │  ├─ ETC/                    바이트 순서 실습
│  │  └─ error.h                 실습의 공용 오류 처리
│  └─ iocp/                      IOCP 단계별 서버와 클라이언트
├─ projects/
│  ├─ ChatServer/                채팅 프로젝트 준비 파일
│  ├─ FTP/                       파일 전송 관련 작업
│  ├─ RPC/                       RPC 실습
│  ├─ Serialization_Networking/  TCP 서버 클래스 실습
│  └─ 2DMultiplayer/             2D 클라이언트와 서버 프로젝트
├─ assignments/
│  └─ DCU_TCPIP/                 수업 과제
└─ docs/
   ├─ LEARNING_GUIDE.md          학습 주제와 프로젝트 안내
   ├─ IOCP_GUIDE.md              IOCP 단계별 안내
   ├─ MOGAMESERVER_ROADMAP.md    라이브러리 개발 계획
   ├─ notes/                    개념 정리와 학습 회고
   └─ design/                   채팅 시스템 설계 자료
```

## 시작하기

소켓 실습은 Windows의 WinSock2를 사용하고, C++ 프로젝트는 Visual Studio v143 도구 집합으로 설정되어 있습니다. Visual Studio 2022의 C++ 데스크톱 개발 도구와 Windows SDK를 준비합니다.

첫 실습은 [EchoServer.sln](study/tcpip/EchoServer/EchoServer.sln)의 `TCPServer`와 `TCPClient`입니다. 같은 PC에서 서버를 먼저 실행하고 클라이언트를 실행하면, 코드에 지정된 `127.0.0.1:9000`으로 연결을 시도합니다. [실행 순서와 프로젝트별 준비 조건](docs/LEARNING_GUIDE.md)을 참고하세요.

문서의 구현 설명은 현재 코드 기준입니다. 빌드 및 실행 결과는 아직 확인 전이며, 프로젝트별 준비 상태는 각 안내에 표시합니다. 특히 `IOCP.sln`은 Lv2만 포함하고, Lv3부터는 각 단계의 솔루션을 사용합니다.

## 학습 노트

| 주제 | 문서 |
| --- | --- |
| 소켓 생성과 종료 | [소켓생성및삭제](docs/notes/소켓생성및삭제.md) |
| 입출력 다중화 | [입출력다중화](docs/notes/입출력다중화.md) |
| Overlapped I/O | [OverlappedIO](docs/notes/OverlappedIO.md) |
| IOCP 구조체 | [IOCP관련구조체](docs/notes/IOCP관련구조체.md) |
| 학습 회고 | [소켓프로그래밍 1회독 후기](docs/notes/소켓프로그래밍1회독후기.md) |
| 채팅 시스템 설계 자료 | [Damso](docs/design/Damso) |

라이선스는 [LICENSE](LICENSE)를 참고하세요.
