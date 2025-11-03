#include "servo_handler.h"

// Servo 설정
#define SERVO_PERIOD_NS 20000000  // 20ms

// 각도-듀티 매핑
#define SERVO_ANGLE_MIN 0
#define SERVO_ANGLE_MAX 270
#define SERVO_DUTY_MIN  500000    // 0.5ms
#define SERVO_DUTY_MID  1500000   // 135도
#define SERVO_DUTY_MAX  2500000   // 2.5ms

// 선형 보간을 위한 함수
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

    if (!pwm_export(pwm_channel)) {
        fprintf(stderr, "[ERROR] Failed to export PWM channel %d\n", pwm_channel);
        return 0;
    }

    usleep(100000); // export 반영 대기

    if (!PWM_Set_Period(pwm_channel, SERVO_PERIOD_NS)) {
        fprintf(stderr, "[ERROR] Failed to set PWM period\n");
        return 0;
    }

    // 초기 각도 적용
    if (!Servo_SetAngle(pwm_channel, initial_angle)) {
        fprintf(stderr, "[ERROR] Failed to set initial servo angle\n");
        return 0;
    }

    return 1;
}

int32_t servo_setAngle(int32_t pwm_channel, int32_t angle) {
    if (angle < SERVO_ANGLE_MIN) angle = SERVO_ANGLE_MIN;
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;

    int32_t duty_ns = servo_angle_to_duty(angle);

    if (!pwm_set_dutycycle(pwm_channel, duty_ns)) {
        fprintf(stderr, "[ERROR] Failed to set duty cycle for servo\n");
        return 0;
    }

    if (!pwm_enable(pwm_channel, 1)) {
        fprintf(stderr, "[ERROR] Failed to enable PWM for servo\n");
        return 0;
    }

    return 1;
}

int32_t deinit_servo(int32_t pwm_channel) {
    if (!PWM_Enable(pwm_channel, 0)) {
        fprintf(stderr, "[WARN] Failed to disable PWM channel %d\n", pwm_channel);
    }

    if (!PWM_Unexport(pwm_channel)) {
        fprintf(stderr, "[WARN] Failed to unexport PWM channel %d\n", pwm_channel);
    }

    return 1;
}