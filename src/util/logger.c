//
// Created by admin on 2025-07-03.
//

#include "logger.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

FILE *LogFile = NULL;
int32_t LogFile_Open_Flag = 0;

void LOG_FILE_OPEN() {
    // 보장: logs 디렉토리 생성 (기존 존재 시 무시)
    const char *dir = "logs";
    struct stat st;
    if (stat(dir, &st) != 0) {
        if (mkdir(dir, 0755) != 0) {
            perror("mkdir logs");
        }
    }

    // 안전한 런타임 타임스탬프 파일명
    time_t now = time(NULL);
    struct tm tm_info;
    localtime_r(&now, &tm_info);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y%m%d_%H%M%S", &tm_info);

    char path[256];
    snprintf(path, sizeof(path), "%s/LogFile_%s.csv", dir, ts);

    LogFile = fopen(path, "w+");
    if (LogFile == NULL) {
        fprintf(stderr, "[ERROR] Failed to open log file: %s\n", path);
        perror("fopen");
        exit(1);
    }
    LogFile_Open_Flag = 1;
}
void LOG_FILE_CLOSE() {
    if (LogFile == NULL) {
        fprintf(stderr, "[WARN] Log file is not open\n");
        return;
    }
    fclose(LogFile);
    LogFile_Open_Flag = 0;
    LogFile = NULL;
    return;
}
void LOG_WRITE(char* data, ...) {
    if (LogFile == NULL) {
        fprintf(stderr, "[WARN] Log file is not open\n");
        return;
    }
    fprintf(LogFile, "%s\r\n", data);
    fflush(LogFile);
    return;
}
void LOG_WRITE_WITH_TIME(const char* format, ...) {
    if (LogFile == NULL) {
        fprintf(stderr, "[WARN] Log file is not open\n");
        return;
    }
    char message[512];
    va_list args;

    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);  // 가변 인자 -> 문자열로
    va_end(args);

    fprintf(LogFile, "%s, %s\r\n", Get_Current_Time(), message);
    fflush(LogFile);
    return;
}



