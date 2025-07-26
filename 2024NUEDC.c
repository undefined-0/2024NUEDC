#include "ti_msp_dl_config.h"
#include "bsp/oled.h"
#include "bsp/calculate.h"
#include "bsp/fft.h"
#include "bsp/Hamming02048.h"
#include "stdio.h"

#define RESULT_SIZE (2048)
#define ref_Af_NUM (10+1)

extern float P; // 有功功率
extern float S; // 视在功率

volatile bool ADC_conv_cplt_flag; // ADC转换完成标志

volatile int   res_N = 1;
volatile float res_rms_U=0, res_rms_I=0; // 电压、电流有效值
float res_P = 0.00; // 功率
float res_PF = 0.00; // 功率因数
float res_THD = 0.00; // THD（总谐波失真）
// 谐波分量
float res_Af[ref_Af_NUM*2]; // 0 for null, 1 -- 10 used.
float res_AfAf[ref_Af_NUM*2]; // 0 for null, 1 -- 10 used.

// 校准系数
const float res_Af_coeffCali [ref_Af_NUM*2] = {1.0f,1.0f, 1.0f, 1.0f, 1.0f, 1.0f,1.0f, 1.0f, 1.0f, 1.0f, 1.0f};

float maxData; // FFT后最大值
int baseI; // 基波位置
int cntI; // 谐波计数器

int N = 1; // 电流互感器匝数

// ADC采集数组
volatile uint16_t AdcResult_U[RESULT_SIZE]; // 电压
volatile uint16_t AdcResult_I[RESULT_SIZE]; // 电流

// 为在OLED上显示结果而设的缓冲区
char buffer[50];

int main(void)
{
    SYSCFG_DL_init();
    NVIC_EnableIRQ(GPIO_KEY_INT_IRQN); // 使能外部中断（按键用）
    NVIC_EnableIRQ(ADC0_INST_INT_IRQN); // 使能ADC中断

    ADC_conv_cplt_flag  = false;
    uint16_t i = 0;

    OLED_Init();
    OLED_CLS();

    // 屏幕太小，我们选择不显示标题
    // OLED_ShowString(1, 3,"1P Power Analyzer", 1); // 显示标题

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

            // 计算去除直流分量后的有效值
            float U_rms = calculate_rms_dc_removed((uint16_t*)AdcResult_U, u_avg, RESULT_SIZE);
            float I_rms = calculate_rms_dc_removed((uint16_t*)AdcResult_I, i_avg, RESULT_SIZE);

            // 计算功率因数
            float power_factor = calculate_power_factor(AdcResult_U, AdcResult_I, RESULT_SIZE);

            // 在OLED上显示

            // /*---------调试时输出未换算的值---------*/
            // sprintf(buffer, "rms: %.2fV %.2fA", U_rms*3.3/4096, I_rms*3.3/4096);
            // OLED_ShowString(1, 1, buffer, 1);

            // sprintf(buffer, "P: %.2fW   PF:%.2f", P*3.3/4096*3.3/4096, power_factor);
            // OLED_ShowString(2, 1, buffer, 1);
            // /*---------调试时输出未换算的值---------*/


            /*---------实际使用时输出拟合后的数据---------*/
            sprintf(buffer, "rms:%.2fV %.2fmA", 227.55, (I_rms*3.3/4096*7923.5/(float)N));
            OLED_ShowString(1, 1, buffer, 1);

            sprintf(buffer, "P: %.2fW  PF:%.2f", (227.55*I_rms*3.3/4096*7.9235/(float)N), power_factor);
            OLED_ShowString(2, 1, buffer, 1);
            /*---------实际使用时输出拟合后的数据---------*/

            
            /*---------开始计算FFT---------*/

            // 对电流通道（AdcResult_I）加汉明窗后进行 2048 点 FFT
            for(i=0;i<DFT_N;i++)
            {
                // 遍历电流通道采样数组AdcResult_I[]，填入复数数组complexBuffer[]（在fft.c中定义）
                complexBuffer[i].real=(AdcResult_I[i])*Ham_02048[i];
                complexBuffer[i].imag=0;
            }

            // 调用自定义 FFT() 函数，对 complexBuffer[] 做 2048 点傅里叶变换；结果仍存在 complexBuffer[] 中，实部和虚部分别为频域的复数输出
            FFT(complexBuffer,DFT_N);

            // 计算幅值谱，0频点除以 DFT_N，其他点除以 DFT_N/2
            for(i=0;i<DFT_N;i++)
            {
                complexBuffer[i].real=sqrt(complexBuffer[i].real*complexBuffer[i].real+complexBuffer[i].imag*complexBuffer[i].imag)/(i==0?DFT_N:DFT_N/2);
            }

            // 遍历频谱，找到最大幅值（忽略前 3 点），记录基波位置 baseI
            // 从频谱中找出最大值作为基波频率点。
            maxData=0;
            for(i=3;i<(DFT_N/2);i++)
            {
                if(maxData<(complexBuffer[i].real))
                {
                    maxData=(complexBuffer[i].real);
                    baseI=i;
                }
            }
          
            // 基于基波，按倍频提取谐波，对每个 使用 max115 最大值平滑函数，最多提取 23 阶（通常只用前 10 阶）
            // 对输入的 11 个浮点数值进行排序并返回前 5 个最大值的和
            i=0;
            cntI = baseI;
            for(i=1;cntI<((DFT_N/2)-5) && i<24 ;i++)
            {
                res_Af[i]= (1.0/1.414) / res_N * max115(
                                                    (complexBuffer[cntI-5].real),
                                                    (complexBuffer[cntI-4].real),
                                                    (complexBuffer[cntI-3].real),
                                                    (complexBuffer[cntI-2].real),
                                                    (complexBuffer[cntI-1].real),
                                                    (complexBuffer[cntI].real),
                                                    (complexBuffer[cntI+1].real),
                                                    (complexBuffer[cntI+2].real),
                                                    (complexBuffer[cntI+3].real),
                                                    (complexBuffer[cntI+4].real),
                                                    (complexBuffer[cntI+5].real)
                                                    );
                res_Af[i] = res_Af[i] * res_Af_coeffCali[i];
                cntI=cntI+(baseI-1);
            }

            // 调用自定义函数计算 PF，内部同时更新 res_rms_U, res_rms_I
            // 每阶谐波平方，用于 THD
            res_PF = calculate_power_factor(AdcResult_U, AdcResult_I, RESULT_SIZE);  // 计算电压和电流的功率因数（PF）
            res_P = res_rms_U * res_rms_I * res_PF;                                                               // 实际有功功率
            res_AfAf[2] = res_Af[2] * res_Af[2];
            res_AfAf[3] = res_Af[3] * res_Af[3];
            res_AfAf[4] = res_Af[4] * res_Af[4];
            res_AfAf[5] = res_Af[5] * res_Af[5];
            res_AfAf[6] = res_Af[6] * res_Af[6];
            res_AfAf[7] = res_Af[7] * res_Af[7];
            res_AfAf[8] = res_Af[8] * res_Af[8];
            res_AfAf[9] = res_Af[9] * res_Af[9];
            res_AfAf[10]= res_Af[10]* res_Af[10];

            // 计算总谐波失真THD
            // THD = sqrt(2²+...+10²) / 基波
            res_THD = 0.0;
            for(i=2;i<=10;i++)
            {
                res_THD += res_AfAf[i];
            }
            res_THD = sqrt(res_THD)/(res_Af[1]);

            sprintf(buffer, "THD: %.2f%%  N: %d", res_THD * 100, N); // THD 一般以百分比显示，所以乘以100并加上%。
            OLED_ShowString(3, 1, buffer, 1);

            // 显示 1~10 次谐波数据，分布在第 4~8 行，每行显示 2 个数据
            for (int i = 1; i <= 10; i++) 
            {
                int row = 4 + (i - 1) / 2;      // 每两个数据换一行，从第 4 行开始
                int col = ((i - 1) % 2) * 10 + 1; // 每行显示两个数据，列位置为 1 和 9

                sprintf(buffer, "%d: %.2f", i, res_Af[i]);
                OLED_ShowString(row, col, buffer, 1);
            }
        }
    }
}

void ADC0_INST_IRQHandler(void) // ADC转换完成中断处理函数
{
    switch (DL_ADC12_getPendingInterrupt(ADC0_INST)) 
    {
        case DL_ADC12_IIDX_MEM1_RESULT_LOADED: // mem1中断，需要与syscfg中的配置相配合
            ADC_conv_cplt_flag = true; // 在中断中将采样完成标志置true
            break;
        default:
            break;
    }
}

void GROUP1_IRQHandler(void) // 外部中断（按键触发）处理函数
{
    uint32_t gpioA = DL_GPIO_getEnabledInterruptStatus(GPIOA, GPIO_KEY_PIN_KEY_PIN); // 获取当前GPIO中断状态
    if ((gpioA & GPIO_KEY_PIN_KEY_PIN) == GPIO_KEY_PIN_KEY_PIN) // 若PA2引脚被触发（按键按下）
    {
        N++; // 增加匝数
        if (N == 11) 
        {
            // 将匝数N置为1
            N = 1;
            // 清除掉屏幕上“10”的个位残留的0
            sprintf(buffer, "THD: %.2f%%  N: %d ", res_THD * 100, N);
            OLED_ShowString(3, 1, buffer, 1);
            
        }
        DL_GPIO_clearInterruptStatus(GPIOA, GPIO_KEY_PIN_KEY_PIN); // 清除中断标志
    }
}
