#ifndef __ADC_H__
#define __ADC_H__


#include "ADS1263.h"

#include <stdint.h> // int32_t
#include <stdlib.h> // exit(0)
#include <stdio.h>  // printf

#define REF             2.555

int32_t init_adc(void);
int32_t read_adc(double* Voltage);
int32_t exit_adc(void);

#endif // __ADC_H__
