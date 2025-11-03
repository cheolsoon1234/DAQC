#include "adc.h"
//#include "util/logger.h"
//#include "net/net.h"

int32_t init_adc(void) {
    DEV_Module_Init();

    // 0 is singleChannel, 1 is diffChannel
    ADS1263_SetMode(1);

    DEV_Delay_ms(50);
    // ADC1 38400 SPS 초기화
    if(ADS1263_init_ADC1(ADS1263_1200SPS) == 1) {
        printf("\r\n END \r\n");
        DEV_Module_Exit();
        return 0;
    }
    return 1;
}

int32_t read_adc(double* Voltage) {

    UBYTE ChannelList[10] = {0,1,2,3,4,5,6,7,8,9};
    UDOUBLE Value[10];

    // ADC1에서 10채널 읽기
    ADS1263_GetAll(ChannelList, Value, 10);

    for(int32_t i = 0; i < 10; i++) {  // 부호 처리
        if((Value[i]>>31) == 1) *(Voltage) = REF*2 - Value[i]/2147483648.0 * REF;
        else                    *(Voltage) = Value[i]/2147483647.0 * REF;
        Voltage++;
    }

    return 1;
}

int32_t exit_adc(void) {
    DEV_Module_Exit();
    return 1;
}
