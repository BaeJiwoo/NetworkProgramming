# 🌐 Network Programming

이 저장소는 C/C++ 및 WinSock2 API를 활용하여 기초적인 소켓 통신부터 고성능 서버 아키텍처까지 단계별로 구현하고 학습하는 프로젝트입니다.

## 🚀 소개 (Introduction)
<!-- 프로젝트의 진짜 목적을 간략히 적어주세요. -->
본 프로젝트는 Windows 환경에서 네트워크 애플리케이션을 개발하기 위한 핵심 기술을 다룹니다. 기본적인 TCP/UDP 통신 모델을 시작으로, 다수의 클라이언트를 효율적으로 처리하기 위한 **IOCP(I/O Completion Port)** 기반의 비동기 서버 구현을 목표로 하고 있습니다.

## 🛠 기술 스택 (Tech Stack)
- **Language:** C, C++
- **OS:** Windows
- **Network API:** WinSock2
- **IDE:** Visual Studio

## 💡 주요 구현 내용 (Features)
<!-- 실제 구현하신 핵심 기능들로 수정해 주세요. -->
- **Basic Socket Programming:** TCP 및 UDP 기반의 에코/채팅 서버 기초 구현
- **Multi-threading:** 스레드 풀(Thread Pool)을 활용한 다중 클라이언트 접속 처리
- **Asynchronous I/O:** `select`, `WSAAsyncSelect`, `WSAEventSelect` 모델 등 다양한 I/O 모델 비교 학습
- **IOCP (I/O Completion Port):** 대규모 트래픽 처리를 위한 고성능 논블로킹(Non-blocking) 서버 아키텍처 구현 (현재 진행 중)

## 📈 단계별 구현 및 발전 과정 (Step-by-Step Progress)

본 프로젝트는 단순한 API 사용법 암기가 아닌, 서버 아키텍처의 병목 현상을 해결하고 성능을 끌어올리는 과정에 초점을 맞춰 단계별로 구현되었습니다.

### 🔹 Phase 1: 소켓 프로그래밍 기초 (Basic Sockets)
- **TCP/UDP 기반 Echo Server & Client 구현**
- 블로킹(Blocking) 소켓의 한계점과 데이터 경계(Boundary)에 대한 이해
- 구조체 직렬화/역직렬화를 통한 간단한 애플리케이션 계층 프로토콜(패킷) 설계

### 🔹 Phase 2: 멀티스레딩과 동기화 (Multi-threading)
- 다중 클라이언트 접속 처리를 위한 스레드(Thread) 생성 및 관리
- 뮤텍스(Mutex), 크리티컬 섹션(Critical Section)을 활용한 스레드 동기화 및 데드락(Deadlock) 방지
- 스레드 풀(Thread Pool) 개념 도입

### 🔹 Phase 3: 다양한 I/O 모델 적용 (Asynchronous I/O)
- **Select 모델:** 다중 I/O 처리를 위한 기초적인 멀티플렉싱 구현
- **WSAEventSelect 모델:** 이벤트 기반의 논블로킹(Non-blocking) 소켓 통신 구현
- **Overlapped I/O:** 비동기 입출력을 통한 CPU와 I/O 연산의 중첩 처리 학습

### 🔹 Phase 4: 고성능 서버 아키텍처, IOCP (현재 진행 중)
가장 효율적인 윈도우 서버 모델인 **IOCP(I/O Completion Port)** 를 레벨별로 고도화하며 구현 중입니다.
- **Level 1 ~ 기초:** Worker Thread와 Completion Port 객체 연결 및 기본 비동기 에코 서버 구현
- **Level 중급:** 세션(Session) 관리, 멀티스레드 환경에서의 안전한 패킷 큐잉 및 브로드캐스팅
- **Level 9 (최근 작업):** 메모리 단편화 방지를 위한 메모리 풀(Memory Pool) 및 링 버퍼(Ring Buffer) 도입, 더미 클라이언트를 활용한 부하 테스트(Stress Test) 
*(※ Level 9의 상세 구현 내용에 맞게 이 부분을 수정해 주세요.)*

## ⚙️ 시작하기 (Getting Started)

### 요구 사항 (Prerequisites)
- Visual Studio (2019 또는 2022 권장)
- Windows 10/11

### 빌드 및 실행 (Build & Run)
1. 저장소를 로컬로 클론합니다.
   ```bash
   git clone [https://github.com/BaeJiwoo/NetworkProgramming.git](https://github.com/BaeJiwoo/NetworkProgramming.git)
