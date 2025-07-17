//OLED IIC 4针脚
//              GND    电源地
//              VCC  接3.3v电源
//              SCL   PA31
//              SDA   PA28
//TextSize=2为两倍大小，占两行
#include "ti_msp_dl_config.h"
#include "\oled\oled.h"

volatile bool gCheckADC;

#define RESULT_SIZE (64)
volatile uint16_t gAdcResult0[RESULT_SIZE];
volatile uint16_t gAdcResult1[RESULT_SIZE];
// volatile uint16_t gAdcResult2[RESULT_SIZE];
// volatile uint16_t gAdcResult3[RESULT_SIZE];

int main(void)
{
    /* Initialize peripherals and enable interrupts */
    SYSCFG_DL_init();
    NVIC_EnableIRQ(ADC12_0_INST_INT_IRQN);

    gCheckADC  = false;
    uint16_t i = 0;

    OLED_Init();
    OLED_CLS();
    // OLED_ShowChar(1,1,'A',2);//字符
    // OLED_ShowNum(1,2,156,3,2);//无符号数
    // OLED_ShowString(3, 1,"LP-MSPM0G3507", 2);//字符串
    // OLED_ShowSignedNum(5,1,-99,2,1);//有符号整数
    // OLED_ShowFNum(5,4,-3.95,3,2,1);//带符号浮点数
    // OLED_ShowNum(6,4,15683842,8,2);//无符号整数
    // OLED_ShowHexNum(8,1,33,4,1);//16进制
    // OLED_ShowBinNum(8,6,15,4,1);//2进制
    OLED_ShowString(1, 1,"1P Power Analyzer", 1);//字符串
    DL_ADC12_startConversion(ADC12_0_INST); // 已配置成多通道连续转换模式，只需要start一次

    while (1) {
        

        /* Wait until all data channels have been loaded. */
        while (gCheckADC == false) {
            __WFE();
        }

        /* Store ADC Results into their respective buffer */
        gAdcResult0[i] =
            DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_0);
        gAdcResult1[i] =
            DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_1);
        // gAdcResult2[i] =
        //     DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_2);
        // gAdcResult3[i] =
        //     DL_ADC12_getMemResult(ADC12_0_INST, DL_ADC12_MEM_IDX_3);

        i++;
        gCheckADC = false;
        /* Reset index of buffers, set breakpoint to check buffers. */
        if (i >= RESULT_SIZE) {
            // __BKPT(0);
            i = 0;
        }
        else{
            ;/*No action required*/
        }
        // DL_ADC12_enableConversions(ADC12_0_INST);
    }
}

/* Check for the last result to be loaded then change boolean */
void ADC12_0_INST_IRQHandler(void)
{
    switch (DL_ADC12_getPendingInterrupt(ADC12_0_INST)) {
        case DL_ADC12_IIDX_MEM1_RESULT_LOADED:
            gCheckADC = true;
            break;
        default:
            break;
    }
}
