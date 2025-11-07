#include "controller.h"

volatile int32_t arm_flag;
extern int32_t controls[8];
volatile int32_t run_controls[8] = {0};

int32_t init_controller(void) {
    // 초기화 코드 작성 (필요 시)
    // GPIO 5: Solenoid
    // GPIO 26: Siren
    // GPIO 16: Ignition

    GPIO_Export(GPIO_SOLENOID);
    GPIO_Set_Direction(GPIO_SOLENOID, "out");
    GPIO_Write(GPIO_SOLENOID, 0); // 초기 상태 Low

    GPIO_Export(GPIO_IGNITION);
    GPIO_Set_Direction(GPIO_IGNITION, "out");
    GPIO_Write(GPIO_IGNITION, 0); // 초기 상태 Low

    GPIO_Export(GPIO_SIREN);
    GPIO_Set_Direction(GPIO_SIREN, "out");
    GPIO_Write(GPIO_SIREN, 0); // 초기 상태 Low

    arm_flag = 0; // 초기화 시 arm_flag를 0으로 설정

    return 1;
}

int32_t exit_controller(void) {
    // 종료 처리 코드 작성 (필요 시)

    LOG_WRITE_WITH_TIME("[INFO] Controller exiting, resetting GPIOs");
    printf("[INFO] Controller exiting, resetting GPIOs\n");

    GPIO_Write(GPIO_SOLENOID, 0);
    GPIO_Write(GPIO_IGNITION, 0);
    GPIO_Write(GPIO_SIREN, 0);

    deinit_servo(SERVO_0);

    return 1;
}


int32_t check_auto_shutdown(double* val) {
    if (val[0] >= 70.0) {
        LOG_WRITE_WITH_TIME("[WARN], Auto Terminiated by Pressure Sensor 1 (Feed Pressure)");
        fprintf(stderr, "[WARN] Auto Terminiated by Pressure Sensor 1 (Feed Pressure)");
        exit(1);
    } 
    if (val[1] >= 90.0) {
        LOG_WRITE_WITH_TIME("[WARN], Auto Terminiated by Pressure Sensor 2 (Chamber Pressure)");
        fprintf(stderr, "[WARN] Auto Terminiated by Pressure Sensor 2 (Chamber Pressure)");
        exit(1);
    } 
    if (val[3] >= 600.0) {
        LOG_WRITE_WITH_TIME("[WARN], Auto Terminiated by Tempature Sensor");
        fprintf(stderr, "[WARN] Auto Terminiated by Tempature Sensor");
        exit(1);
    }
}

int32_t run_controller(void) {

    int32_t checksum = 0;
    for (int i = 0; i < 7; i++) { // 체크섬 계산
        checksum += controls[i];
    }

    if (checksum != controls[6]) {
        fprintf(stderr, "[WARN] Checksum mismatch: calculated %d, received %d\n", checksum, controls[7]);
        LOG_WRITE_WITH_TIME("[WARN], Checksum mismatch: calculated %d, received %d", checksum, controls[7]);
        return -1; // 체크섬 불일치 시 동작하지 않음
    }

    // printf("[CTRL] Received controls: ARM=%d, Solenoid=%d, Siren=%d, Ignition=%d, Checksum=%d\n",
    //        controls[1], controls[2], controls[3], controls[4], controls[6]);
    // fflush(stdout);

    if (run_controls[0] != controls[0]) {
        
        printf("[CTRL] Data command changed: %d -> %d\n", run_controls[0], controls[0]);
        fflush(stdout);

        if (controls[0] == 1) {
            LOG_WRITE_WITH_TIME("[INFO], Data enabled");
        } else {
            LOG_WRITE_WITH_TIME("[INFO], Data disabled");
        }
        
        run_controls[0] = controls[0];
    }


    if (run_controls[1] != controls[1]) {
        
        printf("[CTRL] ARM command changed: %d -> %d\n", run_controls[1], controls[1]);
        fflush(stdout);

        if (controls[1] == 1) { // arm
            arm_flag = 1;
            LOG_WRITE_WITH_TIME("[INFO], System armed");
            printf("[INFO] System armed\n");

        } else {
            arm_flag = 0;
            LOG_WRITE_WITH_TIME("[INFO], System disarmed");
            printf("[INFO] System disarmed\n");
            
            // 비활성화 시 모든 출력 끄기
            GPIO_Write(GPIO_SOLENOID, 0);
            GPIO_Write(GPIO_SIREN, 0);
            GPIO_Write(GPIO_IGNITION, 0);
        }

        run_controls[1] = controls[1];
    }

    

    if (run_controls[2] != controls[2]) {

        printf("[CTRL] Solenoid command changed: %d -> %d\n", run_controls[2], controls[2]);
        fflush(stdout);

        if (controls[2] == 1) { // solenoid
            if (arm_flag == 0) {
                fprintf(stderr, "[WARN] Solenoid command received without arming\n");
                LOG_WRITE_WITH_TIME("[WARN], Solenoid command received without arming");
                return 1; // arm_flag가 설정되지 않은 경우 동작하지 않음
            }

            LOG_WRITE_WITH_TIME("[INFO], Solenoid activated");
            printf("[INFO] Solenoid activated\n");

            GPIO_Write(GPIO_SOLENOID, 1);
        } else {

            LOG_WRITE_WITH_TIME("[INFO], Solenoid deactivated");
            printf("[INFO] Solenoid deactivated\n");

            GPIO_Write(GPIO_SOLENOID, 0);
        }

        run_controls[2] = controls[2];
    }    

    if (run_controls[3] != controls[3]) {

        printf("[CTRL] Ignition command changed: %d -> %d\n", run_controls[3], controls[3]);
        fflush(stdout);

        if (controls[3] == 1) { // ignition
            if (arm_flag == 0) {
                fprintf(stderr, "[WARN] Ignition command received without arming\n");
                LOG_WRITE_WITH_TIME("[WARN], Ignition command received without arming");
                return 1; // arm_flag가 설정되지 않은 경우 동작하지 않음
            }

            LOG_WRITE_WITH_TIME("[INFO], Ignition activated");
            printf("[INFO] Ignition activated\n");

            GPIO_Write(GPIO_IGNITION, 1);

        } else {

            LOG_WRITE_WITH_TIME("[INFO], Ignition deactivated");
            printf("[INFO] Ignition deactivated\n");
            
            GPIO_Write(GPIO_IGNITION, 0);
        }

        run_controls[3] = controls[3];
    }

    
    if (run_controls[4] != controls[4]) {

        printf("[CTRL] Siren command changed: %d -> %d\n", run_controls[4], controls[4]);
        fflush(stdout);

        if (controls[4] == 1) { // siren

            LOG_WRITE_WITH_TIME("[INFO], Siren activated");
            printf("[INFO] Siren activated\n");

            GPIO_Write(GPIO_SIREN, 1);

        } else {

            LOG_WRITE_WITH_TIME("[INFO], Siren deactivated");
            printf("[INFO], Siren deactivated\n");

            GPIO_Write(GPIO_SIREN, 0);
        }

        run_controls[4] = controls[4];
    }

    if (run_controls[6] != controls[6]) {
        
        printf("[CTRL] Servo angle command changed: %d -> %d\n", run_controls[6], controls[6]);
        fflush(stdout);

        if (controls[6] >= 5 && controls[6] <= 85) {  // 서보 작동 범위 5~85도 제한

            if (servo_setAngle(SERVO_0, controls[6])) {
                LOG_WRITE_WITH_TIME("[INFO], Servo operated %d", run_controls[6]);
                printf("[INFO], Servo operated : %d\n", run_controls[6]);
            } else {
                LOG_WRITE_WITH_TIME("[INFO], Servo operation failure (Input : %d)", controls[6]);
                printf("[INFO] Servo operation failure (Input : %d)\n", controls[6]);
                printf("[INFO] Exiting Program\n");
                exit(1);
            }

            run_controls[6] = controls[6];

        } else {
            LOG_WRITE_WITH_TIME("[INFO], Servo operating range exceeded (Input : %d)", controls[6]);
            printf("[INFO] Servo operating range exceeded (Input : %d)\n", controls[6]);
        }

        
    }


    return 0;
}