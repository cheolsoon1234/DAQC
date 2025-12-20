#include "servo_handler.h"
#include "pwm_handler.h"
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

// Servo 설정
#define SERVO_PERIOD_NS 20000000  // 20ms

// 각도-듀티 매핑
#define SERVO_ANGLE_MIN 0
#define SERVO_ANGLE_MAX 270
#define SERVO_DUTY_MIN  500000    // 0.5ms
#define SERVO_DUTY_MID  1500000   // 135도
#define SERVO_DUTY_MAX  2500000   // 2.5ms

#define MAX_PWM_CH 32
static int32_t s_inited[MAX_PWM_CH];
static int32_t s_last_duty[MAX_PWM_CH];

// 선형 보간
static int32_t servo_angle_to_duty(int32_t angle) {
    if (angle <= 0) return SERVO_DUTY_MIN;
    if (angle >= 270) return SERVO_DUTY_MAX;

    if (angle <= 135) {
        // 0~135도 → 0.5~1.5ms
        return SERVO_DUTY_MIN + (SERVO_DUTY_MID - SERVO_DUTY_MIN) * angle / 135;
    } else {
        // 135~270도 → 1.5~2.5ms
        return SERVO_DUTY_MID + (SERVO_DUTY_MAX - SERVO_DUTY_MID) * (angle - 135) / 135;
    }
}

int32_t init_servo(int32_t pwm_channel, int32_t initial_angle) {
    if (pwm_channel < 0 || pwm_channel >= MAX_PWM_CH) {
        fprintf(stderr, "[ERROR] pwm_channel out of range: %d\n", pwm_channel);
        return 0;
    }

    if (!pwm_export(pwm_channel)) {
        fprintf(stderr, "[ERROR] Failed to export PWM channel %d\n", pwm_channel);
        return 0;
    }

    // disable -> duty=0 -> period -> duty(initial) -> enable
    if (!pwm_enable(pwm_channel, 0)) {
        fprintf(stderr, "[ERROR] Failed to disable PWM %d\n", pwm_channel);
        return 0;
    }

    // period 바꾸기 전에 duty 0 한번 넣어 글리치/에러 가능성 낮춤
    if (!pwm_set_dutycycle(pwm_channel, 0)) {
        fprintf(stderr, "[ERROR] Failed to set duty=0 for PWM %d\n", pwm_channel);
        return 0;
    }

    if (!pwm_set_period(pwm_channel, SERVO_PERIOD_NS)) {
        fprintf(stderr, "[ERROR] Failed to set PWM period\n");
        return 0;
    }

    // 초기 각도 duty 설정
    if (initial_angle < SERVO_ANGLE_MIN) initial_angle = SERVO_ANGLE_MIN;
    if (initial_angle > SERVO_ANGLE_MAX) initial_angle = SERVO_ANGLE_MAX;

    int32_t duty_ns = servo_angle_to_duty(initial_angle);
    if (!pwm_set_dutycycle(pwm_channel, duty_ns)) {
        fprintf(stderr, "[ERROR] Failed to set initial duty cycle\n");
        return 0;
    }

    if (!pwm_enable(pwm_channel, 1)) {
        fprintf(stderr, "[ERROR] Failed to enable PWM for servo\n");
        return 0;
    }

    s_inited[pwm_channel] = 1;
    s_last_duty[pwm_channel] = duty_ns;
    return 1;
}

int32_t servo_setAngle(int32_t pwm_channel, int32_t angle) {
    if (pwm_channel < 0 || pwm_channel >= MAX_PWM_CH || !s_inited[pwm_channel]) {
        fprintf(stderr, "[ERROR] servo not initialized (pwm_channel=%d)\n", pwm_channel);
        return 0;
    }

    if (angle < SERVO_ANGLE_MIN) angle = SERVO_ANGLE_MIN;
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;

    int32_t duty_ns = servo_angle_to_duty(angle);

    if (duty_ns == s_last_duty[pwm_channel]) {
        return 1;
    }

    if (!pwm_set_dutycycle(pwm_channel, duty_ns)) {
        fprintf(stderr, "[ERROR] Failed to set duty cycle for servo\n");
        return 0;
    }

    s_last_duty[pwm_channel] = duty_ns;
    return 1;
}

int32_t deinit_servo(int32_t pwm_channel) {
    if (pwm_channel < 0 || pwm_channel >= MAX_PWM_CH) return 0;

    // PWM OFF
    if (!pwm_enable(pwm_channel, 0)) {
        fprintf(stderr, "[WARN] Failed to disable PWM channel %d\n", pwm_channel);
    }

    s_inited[pwm_channel] = 0;
    s_last_duty[pwm_channel] = 0;

    // 필요 시 unexport
    // pwm_unexport(pwm_channel);

    return 1;
}
