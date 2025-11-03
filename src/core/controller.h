#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <stdint.h>
#include <stdio.h>
#include "net.h"
#include "logger.h"
#include "gpio_handler.h"
#include "servo_handler.h"

#define GPIO_SOLENOID 517
#define GPIO_SIREN 538
#define GPIO_IGNITION 528

int32_t init_controller(void);
int32_t exit_controller(void);
int32_t check_auto_shutdown(double* val);
int32_t run_controller(void);


#endif // __CONTROLLER_H__