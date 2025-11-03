#ifndef __SIGNAL_HANDLER_H__
#define __SIGNAL_HANDLER_H__

#include <signal.h>
#include <stdio.h>
#include <stdint.h>

#include "net.h"
#include "logger.h"
#include "adc.h"
#include "controller.h"

extern volatile sig_atomic_t g_stop;
extern double Voltage[10];

void init_exit_handler();
void init_signal_handlers();

#endif // SIGNAL_HANDLER_H__