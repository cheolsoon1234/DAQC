#include "signal_handler.h"

volatile sig_atomic_t g_stop = 0;

static void exit_handler() {
    exit_controller();
    exit_adc();
    net_stop();
    LOG_WRITE_WITH_TIME("[INFO], Program Terminated.");
    LOG_FILE_CLOSE();
    printf("[INFO] Program Terminated.\n");
    return;
}

void init_exit_handler() {
    if (atexit(exit_handler) != 0) fprintf(stderr, "[ERROR] atexit registration failed\n");
    return;
}

static void signal_handler(int signo) { // 신호 핸들러 함수

    switch (signo) {
        case SIGINT:  // Ctrl + C
            printf("[INFO] SIGINT(Ctrl+C)\n");
            LOG_WRITE_WITH_TIME("[INFO], SIGINT(Ctrl+C)");
            g_stop = 1;
            exit(0);
            break;
        case SIGTERM: // kill 명령어 기본 시그널
            printf("[INFO] SIGTERM(kill)\n");
            LOG_WRITE_WITH_TIME("[INFO], SIGTERM(kill)");
            g_stop = 1;
            exit(0);
            break;
        case SIGTSTP: // Ctrl+Z
            printf("[INFO] SIGTSTP (Ctrl+Z)\n"); // 원래는 프로세스가 멈추지만, 여기선 종료하도록 변경
            LOG_WRITE_WITH_TIME("[INFO], SIGTSTP(Ctrl+Z)");
            g_stop = 1;
            exit(0);
            break;
        case SIGSEGV: // 잘못된 메모리 접근
            printf("[INFO] SIGSEGV(segmentation fault)\n");
            LOG_WRITE_WITH_TIME("[INFO], SIGSEGV(segmentation fault)");
            g_stop = 1;
            exit(0);
            break;
        case SIGABRT: // abort() 호출
            printf("[INFO] SIGABRT(abort called)\n");
            LOG_WRITE_WITH_TIME("[INFO], SIGABRT(abort called)");
            g_stop = 1;
            exit(0);
            break;
        case SIGUSR1: // 사용자 정의 신호 1
            printf("[INFO] SIGUSR1(user defined termination signal 1)\n");
            LOG_WRITE_WITH_TIME("[INFO], SIGUSR1(user defined termination signal 1)");
            g_stop = 1;
            exit(0);
            break;
        case SIGUSR2: // 사용자 정의 신호 2
            printf("[INFO] SIGUSR2(user defined termination signal 2)\n");
            LOG_WRITE_WITH_TIME("[INFO], SIGUSR2(user defined termination signal 2)");
            g_stop = 1;
            exit(0);
            break;
        default:
            printf("[INFO] unknown signal(%d)\n", signo);
            LOG_WRITE_WITH_TIME("[INFO], unknown signal");
            g_stop = 1;
            exit(0);
            break;
    }
    return;
}

void init_signal_handlers() {
    if (signal(SIGINT,  signal_handler) == SIG_ERR) fprintf(stderr, "[ERROR] SIGINT Handler  registration failed\n");  // Ctrl+C
    if (signal(SIGTERM, signal_handler) == SIG_ERR) fprintf(stderr, "[ERROR] SIGTERM Handler registration failed\n");  // kill
    if (signal(SIGTSTP, signal_handler) == SIG_ERR) fprintf(stderr, "[ERROR] SIGTSTP Handler registration failed\n");  // Ctrl+Z
    if (signal(SIGSEGV, signal_handler) == SIG_ERR) fprintf(stderr, "[ERROR] SIGSEGV Handler registration failed\n");  // 세그폴트
    if (signal(SIGABRT, signal_handler) == SIG_ERR) fprintf(stderr, "[ERROR] SIGABRT Handler registration failed\n");  // abort
    if (signal(SIGUSR1, signal_handler) == SIG_ERR) fprintf(stderr, "[ERROR] SIGUSR1 Handler registration failed\n");  // 사용자 정의 1
    if (signal(SIGUSR2, signal_handler) == SIG_ERR) fprintf(stderr, "[ERROR] SIGUSR2 Handler registration failed\n");  // 사용자 정의 2
    return;
}
