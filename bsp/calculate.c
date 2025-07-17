#include "calculate.h"

/**
  * @brief  计算 ADC 采样结果的平均值（用于获取直流偏移量）
  * @param  AdcResult      指向 ADC 采样数据数组的指针
  * @param  num_of_samples 数组中采样点的数量
  * @retval 计算得到的平均值，类型为 float
  */
float calculate_avg(uint16_t* AdcResult, int num_of_samples) 
{
    uint32_t sum = 0;
    for (int i = 0; i < num_of_samples; i++) {
        sum += AdcResult[i];
    }
    return (float)sum / num_of_samples;
}

/**
  * @brief  计算去均值（去 DC 分量）后的 RMS 值，用于计算电压或电流的有效值
  * @param  AdcResult      指向 ADC 采样数据数组的指针
  * @param  avg            该组数据的平均值（直流偏移）
  * @param  num_of_samples 数组中采样点的数量
  * @retval 计算得到的 RMS 值，类型为 float
  */
float calculate_rms_dc_removed(uint16_t* AdcResult, float avg, int num_of_samples) 
{
    double sum_of_squares = 0.0;
    for (int i = 0; i < num_of_samples; i++) {
        float value = AdcResult[i] - avg;
        sum_of_squares += value * value;
    }
    return sqrt(sum_of_squares / num_of_samples);
}

/**
  * @brief  计算功率因数（Power Factor），即有功功率与视在功率的比值
  * @param  u_AdcResult    电压通道的 ADC 采样数据数组指针
  * @param  i_AdcResult    电流通道的 ADC 采样数据数组指针
  * @param  num_of_samples 数组中采样点的数量
  * @retval 功率因数，范围 [0, 1]，类型为 float
  */
float calculate_power_factor(uint16_t* u_AdcResult, uint16_t* i_AdcResult, int num_of_samples) 
{
    // 计算电压均值 u_avg，计算电流均值 i_avg，对应点相乘
    float u_avg = calculate_avg(u_AdcResult, num_of_samples);
    float i_avg = calculate_avg(i_AdcResult, num_of_samples);

    // 对应点相乘，积分求有功功率 P
    double sum_ui = 0.0;
    for (int i = 0; i < num_of_samples; i++) {
        float u_value = u_AdcResult[i] - u_avg;
        float i_value = i_AdcResult[i] - i_avg;
        sum_ui += u_value * i_value;
    }
    P = sum_ui / num_of_samples;

    // 计算电压、电流 RMS，赋值 res_rms_U, res_rms_I（用于后面主循环显示）
    float U_rms = calculate_rms_dc_removed(u_AdcResult, u_avg, num_of_samples);
    // res_rms_U = (kU * U_rms + bU);
    float I_rms = calculate_rms_dc_removed(i_AdcResult, i_avg, num_of_samples);
    // res_rms_I = (kI * I_rms + bI);

    // 视在功率 S = 电压有效值与电流有效值的乘积
    float S = U_rms * I_rms;

    // 返回功率因数（P / S）
    return P / S;
}