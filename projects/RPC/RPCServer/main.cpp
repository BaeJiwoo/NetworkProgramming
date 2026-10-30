#include <iostream>
#include "MyRPCProject.h"

#pragma comment(lib, "rpcrt4.lib")

// [IDL 정의 함수 구현] 클라이언트가 호출하면 여기서 실행됩니다.
void HelloProc(const unsigned char* pszString) {
    std::cout << "[서버 수신]: " << pszString << std::endl;
}

// [IDL 정의 함수 구현] 서버 종료용
void Shutdown(void) {
    RPC_STATUS status;
    status = RpcMgmtStopServerListening(NULL);
    status = RpcServerUnregisterIf(NULL, NULL, FALSE);
}

int main() {
    RPC_STATUS status;

    // 1. 통신 방식 설정 (ncalrpc = 로컬 프로세스 간 통신)
    // "my_rpc_endpoint"는 클라이언트와 맞출 이름입니다.
    status = RpcServerUseProtseqEp(
        (RPC_WSTR)L"ncalrpc",
        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
        (RPC_WSTR)L"my_rpc_endpoint",
        NULL
    );

    // 2. 인터페이스 등록
    status = RpcServerRegisterIf(
        MyRPCProject_v1_0_s_ifspec, // IDL 파일명에 따라 이름이 자동 생성됨
        NULL,
        NULL
    );

    std::cout << "RPC 서버가 가동되었습니다. 클라이언트를 기다리는 중..." << std::endl;

    // 3. 서버 리스닝 시작 (여기서 프로그램이 멈춰서 대기합니다)
    status = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, FALSE);

    return 0;
}

// [필수] RPC 메모리 관리 함수 (직접 호출할 일은 없지만 선언은 필수)
void __RPC_FAR* __RPC_USER midl_user_allocate(size_t len) { return(malloc(len)); }
void __RPC_USER midl_user_free(void __RPC_FAR* ptr) { free(ptr); }