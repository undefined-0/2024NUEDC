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
float calculate_power_factor(volatile uint16_t* u_AdcResult, volatile  uint16_t* i_AdcResult, int num_of_samples) 
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
    S = U_rms * I_rms;

    // 返回功率因数（P / S）
    return P / S;
}

/**
  * @brief  对输入的 11 个浮点数值进行排序并返回前 5 个最大值的和（最大值平滑算法）
  * @param  a0~a10   输入的 11 个浮点数值
  * @retval 前 5 个最大值的累加和
  */
float max115(float a0,float a1,float a2,float a3,float a4,float a5,float a6,float a7,float a8,float a9,float a10)
{
    int i,j;
    float tmp;
    float a[11];
    a[0]=a0;
    a[1]=a1;
    a[2]=a2;
    a[3]=a3;
    a[4]=a4;
    a[5]=a5;
    a[6]=a6;
    a[7]=a7;
    a[8]=a8;
    a[9]=a9;
    a[10]=0;//a10;
    for(i=0;i<11;i++)
    {
        for(j=i+1;j<11;j++)
        {
            if(a[i]<a[j])
            {
                tmp=a[i];
                a[i]=a[j];
                a[j]=tmp;
            }
        }
    }
    return (a[0]+a[1]+a[2]+a[3]+a[4]);
}