#ifndef __CALCULATE_H__
#define __CALCULATE_H__

#include"ti_msp_dl_config.h"


float P; // 有功功率
float S; // 视在功率

float calculate_avg(uint16_t* ADCresult, int num_of_samples);
float calculate_rms_dc_removed(uint16_t* ADCresult, float avg, int num_of_samples);
float calculate_power_factor(volatile uint16_t* u_ADCresult, volatile  uint16_t* i_ADCresult, int num_of_samples);
float max115(float a0,float a1,float a2,float a3,float a4,float a5,float a6,float a7,float a8,float a9,float a10);

#endif /*__CALCULATE_H__*/