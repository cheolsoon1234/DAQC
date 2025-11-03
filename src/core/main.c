#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>

#include "basedef.h"
#include "net.h"
#include "adc.h"
#include "signal_handler.h"
#include "logger.h"
#include "voltage_mapper.h"
#include "controller.h"
#include "login.h"
#include "servo_handler.h"

extern volatile sig_atomic_t g_stop;
extern int32_t controls[8];

double Voltage[10];
double val[10];


static int32_t init(void) {
    init_exit_handler();
    init_signal_handlers();

    LOG_FILE_OPEN();
    LOG_WRITE_WITH_TIME("logger started");

    if (geteuid() != 0) {
        fprintf(stderr, "[WARN] run as root for ADC access\n");
        LOG_WRITE_WITH_TIME("[WARN], run as root for ADC access");
        return -1;
    }

    if (!init_adc()) {
        fprintf(stderr, "[ERROR] init_adc failed\n");
        LOG_WRITE_WITH_TIME("[ERROR], init_adc failed");
        return -1;
    }

    if (init_ws() != 0) {
        fprintf(stderr, "[ERROR] init_ws failed\n");
        LOG_WRITE_WITH_TIME("[ERROR], init_ws failed");
        return -1;
    }

    if (!init_servo(SERVO_0, INITIAL_ANGLE)) {
        fprintf(stderr, "[ERROR] init_servo failed\n");
        LOG_WRITE_WITH_TIME("[ERROR], init_servo failed");
        return -1;
    }

    if (!init_controller()) {
        fprintf(stderr, "[ERROR] init_controller failed\n");
        LOG_WRITE_WITH_TIME("[ERROR], init_controller failed");
        return -1;
    }

    return 0;
}

int32_t main(void) {
    if (init() != 0) {
        fprintf(stderr, "[ERROR] initialization failed\n");
	LOG_WRITE_WITH_TIME("[ERROR], initialization failed");
        exit(1);
        return -1;
    }

    printf("Starting main loop. Press Ctrl+C to exit.\n"); fflush(stdout);
    LOG_WRITE_WITH_TIME("[INFO], Starting main loop.");

    LOG_WRITE_WITH_TIME("v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, Pressure1, Pressure2, Thrust, Temp");


    while (!g_stop) { // main loop
                // adc handling
        read_adc(Voltage);

        // voltage to physical value mapping
        map_voltage_to_value(Voltage, val);

        // network event handling
        
        if (controls[0] == 1) send_json(val[0], val[1], val[2], val[3]);
        // if (controls[0] == 1) send_json(Voltage[0], Voltage[1], Voltage[3], Voltage[2]);
        
        net_poll(1); // 1ms 대기 및 이벤트 처리

        // controller handling
        if (run_controller() != 0) {
            fprintf(stderr, "[ERROR], run_controller error\n");
            LOG_WRITE_WITH_TIME("[ERROR], run_controller error");
            exit(1);
            break;
        }


        // logging values
        LOG_WRITE_WITH_TIME("%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%d,%d,%d,%d,%d,%d,%d,%d", 
            Voltage[0], Voltage[1], Voltage[2], Voltage[3], Voltage[4], Voltage[5], Voltage[6], Voltage[7], Voltage[8], Voltage[9], val[0], val[1], val[2], val[3],
            controls[0], controls[1], controls[2], controls[3], controls[4], controls[5], controls[6], &controls[7]);

        if (Check_Login() == 0) {
            fprintf(stderr, "[WARN] No user logged in. Exiting...\n");
            LOG_WRITE_WITH_TIME("[WARN], No user logged in. Exiting...");
            exit(1);
            break;
        }

    }
    
    return 0;
}
