//
// Created by cheolsoon on 25. 11. 04.
//
#include "pwm_handler.h"
#include "sysfs_handler.h"
#include <unistd.h>

#define PWM_CHIP_PATH "/sys/class/pwm/pwmchip0"

// PWM 채널 export
int32_t pwm_export(int32_t pwm_channel) {
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%d", pwm_channel);

    char path[128];
    snprintf(path, sizeof(path), "%s/export", PWM_CHIP_PATH);

    if (!write_sysfs(path, buffer)) {
        fprintf(stderr, "[WARN] ERROR while exporting PWM %d\n", pwm_channel);
        return 0;
    }

    usleep(100000); // 커널 반영 대기
    return 1;
}

// PWM 채널 unexport
int32_t pwm_unexport(int32_t pwm_channel) {
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%d", pwm_channel);

    char path[128];
    snprintf(path, sizeof(path), "%s/unexport", PWM_CHIP_PATH);

    if (!write_sysfs(path, buffer)) {
        fprintf(stderr, "[WARN] ERROR while unexporting PWM %d\n", pwm_channel);
        return 0;
    }

    return 1;
}

// PWM 주기 설정 (단위: ns)
int32_t pwm_set_period(int32_t pwm_channel, int64_t period_ns) {
    char path[128];
    char buffer[32];

    snprintf(path, sizeof(path), "%s/pwm%d/period", PWM_CHIP_PATH, pwm_channel);
    snprintf(buffer, sizeof(buffer), "%lld", (long long)period_ns);

    if (!write_sysfs(path, buffer)) {
        fprintf(stderr, "[ERROR] Failed to set period for PWM %d\n", pwm_channel);
        return 0;
    }

    return 1;
}

// PWM 듀티사이클 설정 (단위: ns)
int32_t pwm_set_dutycycle(int32_t pwm_channel, int64_t duty_ns) {
    char path[128];
    char buffer[32];

    snprintf(path, sizeof(path), "%s/pwm%d/duty_cycle", PWM_CHIP_PATH, pwm_channel);
    snprintf(buffer, sizeof(buffer), "%lld", (long long)duty_ns);

    if (!write_sysfs(path, buffer)) {
        fprintf(stderr, "[ERROR] Failed to set duty_cycle for PWM %d\n", pwm_channel);
        return 0;
    }

    return 1;
}

// PWM Enable (1: ON, 0: OFF)
int32_t pwm_enable(int32_t pwm_channel, int32_t enable) {
    char path[128];
    snprintf(path, sizeof(path), "%s/pwm%d/enable", PWM_CHIP_PATH, pwm_channel);

    if (enable) {
        if (!write_sysfs(path, "1")) {
            fprintf(stderr, "[ERROR] Failed to enable PWM %d\n", pwm_channel);
            return 0;
        }
    } else {
        if (!write_sysfs(path, "0")) {
            fprintf(stderr, "[ERROR] Failed to disable PWM %d\n", pwm_channel);
            return 0;
        }
    }

    return 1;
}
