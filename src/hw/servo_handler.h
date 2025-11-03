#ifndef __SERVO_HANDLER_H__
#define __SERVO_HANDLER_H__

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "pwm_handler.h"

#define SERVO_0         0
#define INITIAL_ANGLE   5

int32_t init_servo(int32_t pwm_channel, int32_t initial_angle);
int32_t servo_setAngle(int32_t pwm_channel, int32_t angle);
int32_t deinit_servo(int32_t pwm_channel);

#endif // __SERVO_HANDLER_H__