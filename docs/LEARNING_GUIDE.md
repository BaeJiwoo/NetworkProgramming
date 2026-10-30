# 교재 학습과 프로젝트 안내

[저장소 처음으로](../README.md) · [IOCP 단계별 안내](IOCP_GUIDE.md) · [mogameserver 개발 계획](MOGAMESERVER_ROADMAP.md)

열혈 TCP/IP 소켓 프로그래밍에서 배운 개념을 현재 실습 코드와 연결하는 목차입니다. 교재 장 번호와 코드의 대응이 기록되어 있지 않아, 우선 학습 주제로 찾아볼 수 있도록 구성했습니다. 아래 순서는 교재의 장 번호를 뜻하지 않습니다.

## 기초 소켓과 I/O 모델

| 학습 주제 | 코드 또는 프로젝트 | 열어야 할 솔루션 |
| --- | --- | --- |
| 바이트 순서 | [엔디언 확인](../study/tcpip/ETC/Edian/Edian/check_endian.cpp) | [Edian.sln](../study/tcpip/ETC/Edian/Edian.sln) |
| TCP 연결과 에코 | [TCPServer](../study/tcpip/EchoServer/TCPServer) · [TCPClient](../study/tcpip/EchoServer/TCPClient) | [EchoServer.sln](../study/tcpip/EchoServer/EchoServer.sln) |
| 수신 길이를 누적하는 클라이언트 | [Client_Stable](../study/tcpip/EchoServer/Client_Stable) | [EchoServer.sln](../study/tcpip/EchoServer/EchoServer.sln) |
| TCP 계산기 | [서버](../study/tcpip/NetworkCalculator/TCPCalculatorServer) · [클라이언트](../study/tcpip/NetworkCalculator/TCPCalculatorClient) | [NetworkCalculator.sln](../study/tcpip/NetworkCalculator/NetworkCalculator.sln) |
| UDP 계산기 | [서버](../study/tcpip/NetworkCalculator/UDPCalculatorServer) · [클라이언트](../study/tcpip/NetworkCalculator/UDPCalculatorClient) | [NetworkCalculator.sln](../study/tcpip/NetworkCalculator/NetworkCalculator.sln) |
| 긴급 데이터와 OOB | [TCPServer_OOB](../study/tcpip/EchoServer/TCPServer_OOB) · [TCPClient_OOB](../study/tcpip/EchoServer/TCPClient_OOB) | [EchoServer.sln](../study/tcpip/EchoServer/EchoServer.sln) |
| select 기반 다중화 | [IOMultiplexingServer](../study/tcpip/IOMultiplexing/IOMultiplexingServer) | [IOMultiplexing.sln](../study/tcpip/IOMultiplexing/IOMultiplexing.sln) |
| 멀티스레드 송수신 | [Server](../study/tcpip/MultithreadEchoServer/Server) · [Client](../study/tcpip/MultithreadEchoServer/Client) | [MultithreadEchoServer.sln](../study/tcpip/MultithreadEchoServer/MultithreadEchoServer.sln) |
| 논블로킹 소켓 | [Server_IOModel_SyncNonBlocking](../study/tcpip/EchoServer/Server_IOModel_SyncNonBlocking) | [EchoServer.sln](../study/tcpip/EchoServer/EchoServer.sln) |
| 윈도우 메시지 기반 I/O | [Server_WSAAsyncSelect](../study/tcpip/EchoServer/Server_WSAAsyncSelect) | [EchoServer.sln](../study/tcpip/EchoServer/EchoServer.sln) |
| 이벤트 기반 I/O | [Server_WSAEvent](../study/tcpip/EchoServer/Server_WSAEvent) | [EchoServer.sln](../study/tcpip/EchoServer/EchoServer.sln) |
| Overlapped I/O와 완료 루틴 | [이벤트 방식](../study/tcpip/EchoServer/Server_OverlappedIO) · [완료 루틴 방식](../study/tcpip/EchoServer/Server_OverlappedIOCmpRoutine) | [EchoServer.sln](../study/tcpip/EchoServer/EchoServer.sln) |
| IOCP 기본 예제 | [Server_IOCP](../study/tcpip/EchoServer/Server_IOCP) | [EchoServer.sln](../study/tcpip/EchoServer/EchoServer.sln) |
| IOCP 서버 확장 | [단계별 코드와 변경점](IOCP_GUIDE.md) | 단계별 솔루션 사용 |

각 행은 해당 개념의 코드를 찾기 위한 안내입니다. 구현 상태와 실행 성공 여부는 구분하며, 위 실습의 빌드 및 실행 상태는 현재 확인 전입니다.

## 첫 TCP 에코 실습

1. [EchoServer.sln](../study/tcpip/EchoServer/EchoServer.sln)을 Visual Studio에서 엽니다.
2. `Debug / x64` 구성을 선택하고 `TCPServer`, `TCPClient` 프로젝트를 각각 빌드합니다.
3. `TCPServer`를 먼저 실행합니다. 서버 코드의 포트는 `9000`입니다.
4. 서버를 실행해 둔 상태에서 `TCPClient`를 별도로 실행합니다. 클라이언트는 기본값 `127.0.0.1:9000`을 사용하며 실행 인수는 필요하지 않습니다.
5. 짧은 문자열을 입력해 송신과 수신을 확인합니다. 클라이언트에서 빈 줄을 입력하면 반복문을 종료합니다.

여러 실습이 같은 포트를 사용하므로 한 실습을 마친 뒤 해당 서버를 종료하고 다음 서버를 실행합니다. 실행 파일의 위치는 선택한 프로젝트의 빌드 출력 경로를 확인합니다.

추가 에코 클라이언트인 [Client_Stable](../study/tcpip/EchoServer/Client_Stable/StableEchoClnt_win.c)은 IP와 포트를 실행 인수로 받습니다. 예를 들어 해당 실행 파일의 폴더에서 다음과 같이 실행합니다.

```powershell
.\Client_Stable.exe 127.0.0.1 9000
```

## 응용 프로젝트와 수업 과제

프로젝트 이름과 현재 코드의 범위를 함께 표시합니다. 아래 상태 역시 빌드 성공이나 기능 완성을 의미하지 않습니다.

| 프로젝트 | 열어야 할 솔루션 | 현재 코드 범위 |
| --- | --- | --- |
| ChatServer | [ChatServer.sln](../projects/ChatServer/ChatServer.sln) | 서버 프로젝트가 있으며 `main.cpp`는 비어 있음. IOCP Lv8 및 Lv9의 채팅 관련 작업과 별도 위치 |
| FTP | [FTP.sln](../projects/FTP/FTP.sln) | IOCP 서버와 파일 관리 관련 코드가 있음. 클라이언트 프로젝트에는 컴파일할 소스가 등록되어 있지 않음 |
| RPC | [RPC.sln](../projects/RPC/RPC.sln) | IDL과 로컬 RPC 서버 코드가 있음. 솔루션에는 서버 프로젝트만 포함 |
| Serialization_Networking | [Serialization_Networking.sln](../projects/Serialization_Networking/Serialization_Networking.sln) | 현재 서버는 포트 `8080`에서 받은 데이터를 돌려주는 `SimpleTcpServer` 구조 |
| 2DMultiplayer | [2DMultiplayer.sln](../projects/2DMultiplayer/2DMultiplayer.sln) | SFML을 사용하는 클라이언트 코드와 [기획 메모](../projects/2DMultiplayer/Readme.md)가 있음. 서버 프로젝트에는 컴파일할 소스가 등록되어 있지 않음 |
| DCU_TCPIP | [DCU_TCPIP.sln](../assignments/DCU_TCPIP/DCU_TCPIP.sln) | 과제용 클라이언트와 서버가 포함됨. 별도 `Assignment1` 프로젝트는 소스 미등록 상태이며 솔루션에 포함되지 않음 |

## 프로젝트별 준비 조건

| 대상 | 확인할 내용 |
| --- | --- |
| 공통 | Windows, Visual Studio의 C++ 데스크톱 개발 도구, v143 도구 집합, Windows SDK. 현재 프로젝트의 Windows SDK 지정 값은 `10.0` |
| IOCP Lv9 | `hiredis` 파일이 포함되어 있고 `redis++` 헤더도 참조함. 프로젝트에는 개인 PC의 라이브러리 절대 경로가 있으며 Redis 접속 주소도 코드에 지정되어 있음. [Lv9 실행 조건](IOCP_GUIDE.md) 참고 |
| 2DMultiplayer 클라이언트 | 프로젝트에서 SFML 3.0.2 폴더의 헤더와 라이브러리를 참조함. 경로가 `C:\Libraries\SFML-3.0.2`로 지정되어 있어 로컬 설치 상태 확인 필요 |
| 빈 프로젝트 및 진입 파일 | 위 표의 준비 상태를 먼저 확인하고 실행할 프로젝트를 선택 |

`EchoServer` 아래의 `IOC{` 프로젝트도 소스 미등록 상태이며 솔루션에 포함되지 않습니다. 실제 IOCP 기본 실습의 진입점은 위 목차에 연결된 `Server_IOCP`입니다.

## 챕터를 공부한 뒤 남길 기록

한 챕터를 마치면 다음 항목을 기록합니다. 교재의 장 제목을 그대로 사용하면 다음 복습 때 코드와 연결하기 쉽습니다.

- **교재 장과 제목:** 사용 중인 교재의 장 번호와 제목
- **실습 코드:** 관련 소스 파일과 솔루션 링크
- **배운 개념:** 이번 장에서 이해한 통신 흐름이나 API의 역할
- **실행 결과:** 사용한 구성, 서버와 클라이언트 실행 방법, 관찰한 결과
- **남은 의문:** 오류, 예상과 달랐던 동작, 추가로 확인할 점
- **mogameserver 연결점:** 공통화할 수 있는 부분과 응용별로 달라지는 부분

기존 개념 노트는 [README의 학습 노트 목록](../README.md)에서 이어서 볼 수 있습니다.
