//
// Created by cheolsoon on 25. 11. 04.
//
#include "pwm_handler.h"
#include "sysfs_handler.h"

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define PWM_CHIP_PATH "/sys/class/pwm/pwmchip0"

// 드라이버/커널 반영 대기 (너무 짧으면 race가 생길 수 있음)
#define SYSFS_SETTLE_US  20000   // 20ms
#define SYSFS_RETRY_CNT  10

static int path_exists(const char *path) {
    return access(path, F_OK) == 0;
}

// sysfs write는 드물게 EBUSY/타이밍 문제로 실패할 수 있어서 재시도
static int write_sysfs_retry(const char *path, const char *value) {
    for (int i = 0; i < SYSFS_RETRY_CNT; ++i) {
        if (write_sysfs(path, value)) {
            return 1;
        }
        usleep(SYSFS_SETTLE_US);
    }
    return 0;
}

// PWM 채널 export
int32_t pwm_export(int32_t pwm_channel) {
    char pwm_dir[128];
    snprintf(pwm_dir, sizeof(pwm_dir), "%s/pwm%d", PWM_CHIP_PATH, pwm_channel);

    // 이미 export 되어 있으면 성공으로 처리
    if (path_exists(pwm_dir)) {
        return 1;
    }

    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%d", pwm_channel);

    char path[128];
    snprintf(path, sizeof(path), "%s/export", PWM_CHIP_PATH);

    // export 요청
    if (!write_sysfs_retry(path, buffer)) {
        // write 실패해도 커널이 이미 export 처리한 케이스가 있어 재확인
        usleep(SYSFS_SETTLE_US);
        if (path_exists(pwm_dir)) return 1;

        fprintf(stderr, "[WARN] ERROR while exporting PWM %d\n", pwm_channel);
        return 0;
    }

    // sysfs 생성될 때까지 대기
    for (int i = 0; i < SYSFS_RETRY_CNT; ++i) {
        if (path_exists(pwm_dir)) return 1;
        usleep(SYSFS_SETTLE_US);
    }

    fprintf(stderr, "[WARN] PWM %d export requested but pwm dir not appeared\n", pwm_channel);
    return 0;
}

// PWM 채널 unexport
int32_t pwm_unexport(int32_t pwm_channel) {
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%d", pwm_channel);

    char path[128];
    snprintf(path, sizeof(path), "%s/unexport", PWM_CHIP_PATH);

    if (!write_sysfs_retry(path, buffer)) {
        fprintf(stderr, "[WARN] ERROR while unexporting PWM %d\n", pwm_channel);
        return 0;
    }

    return 1;
}

// PWM Enable (1: ON, 0: OFF)
int32_t pwm_enable(int32_t pwm_channel, int32_t enable) {
    char path[128];
    snprintf(path, sizeof(path), "%s/pwm%d/enable", PWM_CHIP_PATH, pwm_channel);

    if (enable) {
        if (!write_sysfs_retry(path, "1")) {
            fprintf(stderr, "[ERROR] Failed to enable PWM %d\n", pwm_channel);
            return 0;
        }
    } else {
        if (!write_sysfs_retry(path, "0")) {
            fprintf(stderr, "[ERROR] Failed to disable PWM %d\n", pwm_channel);
            return 0;
        }
    }

    // enable/disable 직후 settle
    usleep(SYSFS_SETTLE_US);
    return 1;
}

// PWM 주기 설정 (단위: ns)
// ⚠️ period 변경은 글리치 원인이 되기 쉬워서: "disable 상태에서만" 하도록 강제
int32_t pwm_set_period(int32_t pwm_channel, int64_t period_ns) {
    if (period_ns <= 0) {
        fprintf(stderr, "[ERROR] Invalid period_ns=%lld\n", (long long)period_ns);
        return 0;
    }

    // 안전: period 바꾸기 전에는 disable 권장 (여기서 강제)
    if (!pwm_enable(pwm_channel, 0)) return 0;

    char path[128];
    char buffer[32];

    snprintf(path, sizeof(path), "%s/pwm%d/period", PWM_CHIP_PATH, pwm_channel);
    snprintf(buffer, sizeof(buffer), "%lld", (long long)period_ns);

    if (!write_sysfs_retry(path, buffer)) {
        fprintf(stderr, "[ERROR] Failed to set period for PWM %d\n", pwm_channel);
        return 0;
    }

    usleep(SYSFS_SETTLE_US);
    return 1;
}

// PWM 듀티사이클 설정 (단위: ns)
// ✅ 운용 중엔 period는 고정하고 duty만 업데이트하는 걸 권장
int32_t pwm_set_dutycycle(int32_t pwm_channel, int64_t duty_ns) {
    if (duty_ns < 0) {
        fprintf(stderr, "[ERROR] Invalid duty_ns=%lld\n", (long long)duty_ns);
        return 0;
    }

    char path[128];
    char buffer[32];

    snprintf(path, sizeof(path), "%s/pwm%d/duty_cycle", PWM_CHIP_PATH, pwm_channel);
    snprintf(buffer, sizeof(buffer), "%lld", (long long)duty_ns);

    if (!write_sysfs_retry(path, buffer)) {
        fprintf(stderr, "[ERROR] Failed to set duty_cycle for PWM %d\n", pwm_channel);
        return 0;
    }

    return 1;
}

// ✅ "서보용" 안정 설정 함수: disable -> duty=0 -> period -> duty -> enable
int32_t pwm_configure(int32_t pwm_channel, int64_t period_ns, int64_t duty_ns, int32_t enable) {
    if (period_ns <= 0 || duty_ns < 0 || duty_ns > period_ns) {
        fprintf(stderr, "[ERROR] Invalid period/duty. period=%lld duty=%lld\n",
                (long long)period_ns, (long long)duty_ns);
        return 0;
    }

    // (1) 반드시 disable
    if (!pwm_enable(pwm_channel, 0)) return 0;

    // (2) duty=0 먼저 (period 변경 시 글리치/에러 방지)
    if (!pwm_set_dutycycle(pwm_channel, 0)) return 0;

    // (3) period 설정
    {
        char path[128];
        char buffer[32];
        snprintf(path, sizeof(path), "%s/pwm%d/period", PWM_CHIP_PATH, pwm_channel);
        snprintf(buffer, sizeof(buffer), "%lld", (long long)period_ns);

        if (!write_sysfs_retry(path, buffer)) {
            fprintf(stderr, "[ERROR] Failed to set period for PWM %d\n", pwm_channel);
            return 0;
        }
        usleep(SYSFS_SETTLE_US);
    }

    // (4) duty 설정
    if (!pwm_set_dutycycle(pwm_channel, duty_ns)) return 0;

    // (5) enable
    if (enable) {
        if (!pwm_enable(pwm_channel, 1)) return 0;
    }

    return 1;
}
