#ifndef __PWM_HANDLER_H__
#define __PWM_HANDLER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

int32_t pwm_export(int32_t pwm_channel);
int32_t pwm_unexport(int32_t pwm_channel);
int32_t pwm_set_period(int32_t pwm_channel, int64_t period_ns);
int32_t pwm_set_dutycycle(int32_t pwm_channel, int64_t duty_ns);
int32_t pwm_enable(int32_t pwm_channel, int32_t enable);
int32_t pwm_configure(int32_t pwm_channel, int64_t period_ns, int64_t duty_ns, int32_t enable);

#endif // __PWM_HANDLER_H__