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

## ⚙️ 시작하기 (Getting Started)

### 요구 사항 (Prerequisites)
- Visual Studio (2019 또는 2022 권장)
- Windows 10/11

### 빌드 및 실행 (Build & Run)
1. 저장소를 로컬로 클론합니다.
   ```bash
   git clone [https://github.com/BaeJiwoo/NetworkProgramming.git](https://github.com/BaeJiwoo/NetworkProgramming.git)
