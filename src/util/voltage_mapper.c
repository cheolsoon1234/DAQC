#include "voltage_mapper.h"

int32_t map_voltage_to_value(double* voltage, double* value) {
    // 예시 매핑: 0V -> 0, 2.5V -> 100 
    value[0] = ((voltage[0]) / 2.500) * 100.000 * 1.8 -2;
    value[1] = ((voltage[1]) / 2.500) * 100.000 * 1.8 -2;
    value[2] = ((voltage[3]) / 2.500) * 500 * 9.81 * 3.3;
    value[3] = ((voltage[2]) / 2.500) * 1220.000 * 2 -20;
    value[4] = voltage[4];
    return 1;
}
