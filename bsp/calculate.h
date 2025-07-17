#ifndef __CALCULATE_H__
#define __CALCULATE_H__

#include"ti_msp_dl_config.h"


float P; // 有功功率

float calculate_avg(uint16_t* ADCresult, int num_of_samples);
float calculate_rms_dc_removed(uint16_t* ADCresult, float avg, int num_of_samples);
float calculate_power_factor(uint16_t* u_ADCresult, uint16_t* i_ADCresult, int num_of_samples);

#endif /*__CALCULATE_H__*/