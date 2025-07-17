#include "ti_msp_dl_config.h"
#include "\oled\oled.h"

volatile bool ADC_complete_flag;

#define RESULT_SIZE (2048)
volatile uint16_t AdcResult0[RESULT_SIZE];
volatile uint16_t AdcResult1[RESULT_SIZE];

int main(void)
{
    /* Initialize peripherals and enable interrupts */
    SYSCFG_DL_init();
    NVIC_EnableIRQ(ADC0_INST_INT_IRQN);

    ADC_complete_flag  = false;
    uint16_t i = 0;

    OLED_Init();
    OLED_CLS();
    // 最后一个参数TextSize设为1时字占一行，设为2时字号为原先2倍，占两行
    // OLED_ShowChar(1,1,'A',2);//字符
    // OLED_ShowNum(1,2,156,3,2);//无符号数
    // OLED_ShowString(3, 1,"LP-MSPM0G3507", 2);//字符串
    // OLED_ShowSignedNum(5,1,-99,2,1);//有符号整数
    // OLED_ShowFNum(5,4,-3.95,3,2,1);//带符号浮点数
    // OLED_ShowNum(6,4,15683842,8,2);//无符号整数
    // OLED_ShowHexNum(8,1,33,4,1);//16进制
    // OLED_ShowBinNum(8,6,15,4,1);//2进制

    OLED_ShowString(1, 3,"1P Power Analyzer", 1); // 显示标题
    DL_ADC12_startConversion(ADC0_INST); // 已配置成多通道连续转换模式，只需要start一次

    while (1) 
    {
        // ADC未采样完成时就一直等待
        while (ADC_complete_flag == false) 
        { 
            __WFE();
        }

        // ADC采样完成时将结果写入数组
        AdcResult0[i] = DL_ADC12_getMemResult(ADC0_INST, DL_ADC12_MEM_IDX_0);
        AdcResult1[i] = DL_ADC12_getMemResult(ADC0_INST, DL_ADC12_MEM_IDX_1);
        i++; // 采样点计数变量自增
        ADC_complete_flag = false; // 将采样完成标志置false

        if (i >= RESULT_SIZE) 
        {
            // __BKPT(0);
            i = 0;
        }
        // DL_ADC12_enableConversions(ADC0_INST);
    }
}

void ADC0_INST_IRQHandler(void)
{
    switch (DL_ADC12_getPendingInterrupt(ADC0_INST)) {
        case DL_ADC12_IIDX_MEM1_RESULT_LOADED: // mem1中断，需要与syscfg相配合
            ADC_complete_flag = true; // 在中断中将采样完成标志置true
            break;
        default:
            break;
    }
}
