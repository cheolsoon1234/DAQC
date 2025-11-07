#include "adc.h"
//#include "util/logger.h"
//#include "net/net.h"

int32_t init_adc(void) {
    DEV_Module_Init();
    DEV_GPIO_Mode(DEV_RST_PIN, 1);   // OUTPUT으로 강제 설정
    DEV_Digital_Write(DEV_RST_PIN, 1);
    DEV_Delay_ms(300);
    DEV_Digital_Write(DEV_RST_PIN, 0);
    DEV_Delay_ms(300);
    DEV_Digital_Write(DEV_RST_PIN, 1);
    DEV_Delay_ms(300);


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

    for(int32_t i = 0; i < 10; i++) {
        int32_t signedValue = (int32_t)Value[i]; // 부호 있는 값으로 변환
        *(Voltage) = (double)signedValue / 2147483648.0 * REF;
        Voltage++;
    }


    return 1;
}

int32_t exit_adc(void) {
    DEV_Module_Exit();
    return 1;
}
