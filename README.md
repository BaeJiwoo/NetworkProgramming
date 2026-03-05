# 🌐 Network Programming (WinSock2)

윤상배 저, **'뇌를 자극하는 TCP/IP 소켓 프로그래밍'**을 기반으로 학습하며 Windows 환경에서 구현한 네트워크 프로그램 저장소입니다.  
모든 소스 코드는 **Visual Studio**를 통해 빌드 및 테스트되었습니다.

---

## 🛠 개발 환경 (Environment)

- **OS**: Windows 10 / 11
- **IDE**: Visual Studio
- **Language**: C / C++
- **API**: WinSock2 (Windows Sockets)

---

## 🚀 주요 구현 프로젝트 (Main Projects)

### 1. EchoServer (TCP / UDP)

서버가 클라이언트로부터 받은 메시지를 그대로 다시 돌려주는 기초 통신 모델입니다.

- **TCP Echo**: 데이터 경계가 없는 스트림 방식의 신뢰성 있는 통신
- **UDP Echo**: 데이터 경계가 존재하는 데이터그램 방식의 빠른 통신

---

### 2. NetworkCalculator (TCP / UDP)

수식과 피연산자를 전송하여 서버에서 계산 결과를 반환하는 네트워크 계산기입니다.

- **Protocol Design**: 구조체 기반 데이터 패킷 설계 및 직렬화 학습
- **Multi-Mode**: 동일한 계산 로직을 TCP와 UDP 두 방식으로 각각 구현

---

## 📁 프로젝트 구조 (Project Structure)

```text
NetworkProgramming/
└── TCPIP_Socket_Programming/
    ├── docs/                   # 학습 정리 및 관련 이론 문서 (예정)
    └── src/                    # 주요 소스 코드
        ├── EchoServer/         # 에코 서버 & 클라이언트 (TCP/UDP)
        └── NetworkCalculator/  # 계산기 서버 & 클라이언트 (TCP/UDP)
```

---

## ⚙️ 설정 및 실행 방법 (Setup & How to Run)

### 1. Visual Studio 프로젝트 설정 (필수)

Windows 소켓 라이브러리 `ws2_32.lib` 를 사용하기 위해 다음 두 가지 방법 중 하나를 선택합니다.

#### 방법 A — 소스 코드에 직접 추가 (권장)

소스 파일(.cpp 또는 .h) 상단에 아래 코드를 추가합니다.

```cpp
#pragma comment(lib, "ws2_32.lib")
```

#### 방법 B — 프로젝트 속성에서 추가

1. 솔루션 탐색기에서 **프로젝트 우클릭 → 속성**
2. **구성 속성 → 링커 → 입력**
3. **추가 종속성** 항목에 다음 입력

```
ws2_32.lib;
```

---

### 2. 보안 경고(C4996) 해결 방법

`inet_addr()` 등 구형 함수 사용 시 발생하는 경고를 방지하려면 다음 중 하나를 적용합니다.

#### 방법 A — 프로젝트 설정

프로젝트 속성 → **C/C++ → 일반 → SDL 검사**  
값을 **아니요 (/sdl-)** 로 변경

#### 방법 B — 코드에서 해결

소스 코드 최상단에 다음 코드 추가

```cpp
#define _WINSOCK_DEPRECATED_NO_WARNINGS
```

---

### 3. 빌드 및 실행 순서

1. `TCPIP_Socket_Programming.sln` 실행
2. 상단 메뉴에서 **Build → Build Solution** 선택  
   또는 **Ctrl + Shift + B**
3. **Server 실행**  
   서버 프로젝트의 `.exe` 파일을 먼저 실행
4. **Client 실행**  
   클라이언트 `.exe` 실행 후 서버와 연결

---

## 📚 참고 서적 (Reference)

- 도서명: **뇌를 자극하는 TCP/IP 소켓 프로그래밍**
- 저자: **윤상배**
- 출판사: **한빛미디어**

---

## 👤 제작자 (Author)

**BaeJiwoo**

GitHub: `@BaeJiwoo`
