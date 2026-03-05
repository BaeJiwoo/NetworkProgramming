#ifndef ERROR_UTILS_H
#define ERROR_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

// 에러 메시지 창을 띄우고 프로그램을 종료하는 함수
static inline void exit_with_error(const char* msg) {
    // 1. 콘솔에 에러 출력
    fprintf(stderr, "[FATA ERROR] %s\n", msg);

    // 2. Windows 팝업창 띄우기 (MB_ICONERROR: 빨간 X 아이콘, MB_OK: 확인 버튼)
    MessageBoxA(NULL, msg, "Critical Error", MB_OK | MB_ICONERROR);

    // 3. 윈소켓 정리 및 종료
    WSACleanup();
    exit(1);
}

#endif