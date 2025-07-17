#include "ti_msp_dl_config.h"
#include "bsp/oled.h"
#include "bsp/calculate.h"
#include "bsp/fft.h"
#include "bsp/Hamming02048.h"
#include "stdio.h"

extern float P; // 有功功率
volatile bool ADC_conv_cplt_flag; // ADC转换结束标志位

#define RESULT_SIZE (2048)
volatile uint16_t AdcResult_U[RESULT_SIZE];
volatile uint16_t AdcResult_I[RESULT_SIZE];

int main(void)
{
    SYSCFG_DL_init();
    NVIC_EnableIRQ(ADC0_INST_INT_IRQN);

    ADC_conv_cplt_flag  = false;
    uint16_t i = 0;

    OLED_Init();
    OLED_CLS();

    OLED_ShowString(1, 3,"1P Power Analyzer", 1); // 显示标题
    DL_ADC12_startConversion(ADC0_INST); // 已配置成多通道连续转换模式，只需要start一次

    while (1) 
    {
        // ADC未采样完成时就一直等待
        while (ADC_conv_cplt_flag == false) 
        { 
            __WFE();
        }

        // ADC采样完成时将结果写入数组
        AdcResult_U[i] = DL_ADC12_getMemResult(ADC0_INST, DL_ADC12_MEM_IDX_0);
        AdcResult_I[i] = DL_ADC12_getMemResult(ADC0_INST, DL_ADC12_MEM_IDX_1);
        i++; // 采样点计数变量自增
        ADC_conv_cplt_flag = false; // 将采样完成标志置false

        if (i >= RESULT_SIZE) 
        {
            // __BKPT(0);
            i = 0;

            // 所有数据采集完毕后开始计算

            // 计算平均值（直流值）
            float u_avg = calculate_avg((uint16_t*)AdcResult_U, RESULT_SIZE);
            float i_avg = calculate_avg((uint16_t*)AdcResult_I, RESULT_SIZE);

            // 计算有效值
            float U_rms = calculate_rms_dc_removed((uint16_t*)AdcResult_U, u_avg, RESULT_SIZE);
            float I_rms = calculate_rms_dc_removed((uint16_t*)AdcResult_I, i_avg, RESULT_SIZE);

            // 计算功率因数
            float power_factor = calculate_power_factor((uint16_t*)AdcResult_U, (uint16_t*)AdcResult_I, RESULT_SIZE);

            // 显示在OLED上
            char buffer[32];

            OLED_ShowString(3, 1, "U_rms:", 1);
            sprintf(buffer, "%.2f V", U_rms*3.3/4096);
            OLED_ShowString(3, 8, buffer, 1);

            OLED_ShowString(4, 1, "I_rms:", 1);
            sprintf(buffer, "%.2f A", I_rms*3.3/4096);
            OLED_ShowString(4, 8, buffer, 1);

            OLED_ShowString(5, 1, "P:", 1);
            sprintf(buffer, "%.2f W", P*3.3/4096*3.3/4096);
            OLED_ShowString(5, 4, buffer, 1);

            OLED_ShowString(6, 1, "PowerFactor:", 1);
            sprintf(buffer, "%.2f", power_factor);
            OLED_ShowString(6, 14, buffer, 1);
        }
    }
}

void ADC0_INST_IRQHandler(void)
{
    switch (DL_ADC12_getPendingInterrupt(ADC0_INST)) {
        case DL_ADC12_IIDX_MEM1_RESULT_LOADED: // mem1中断，需要与syscfg中的配置相配合
            ADC_conv_cplt_flag = true; // 在中断中将采样完成标志置true
            break;
        default:
            break;
    }
}
