#include <math.h>
#include "fft.h"

Complex complexBuffer[DFT_N] = {0}; // 定义复数数组，后将传给FFT函数做2048点傅里叶变换，结果仍存在其中

/**
  * @brief  减去直流偏置的实现
  * @param  complexBuffer
  * @param  N
  * @retval 无
  */
void RemoveDCBias(Complex *complexBuffer, int N) {
    float sumReal = 0.0;
    float sumImag = 0.0;
    for (int i = 0; i < N; i++) {
        sumReal += complexBuffer[i].real;
        sumImag += complexBuffer[i].imag;
    }
    float avgReal = sumReal / N;
    float avgImag = sumImag / N;
    for (int i = 0; i < N; i++) {
        complexBuffer[i].real -= avgReal;
        complexBuffer[i].imag -= avgImag;
    }
}


// FFT函数实现
void FFT(Complex *complexBuffer, int N) {
    Complex temp;
    int i, j, k, istep;
    float theta, wr, wi, wpr, wpi;

    RemoveDCBias(complexBuffer, N); // 移除直流偏置

    // 位反转置换 (Bit-reversal Permutation)
    j = 0;
    for (i = 0; i < N; ++i) {
        if (j > i) {
            temp = complexBuffer[i];
            complexBuffer[i] = complexBuffer[j];
            complexBuffer[j] = temp;
        }
        int m = N >> 1;
        while (m >= 1 && j >= m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }

    // 迭代计算 FFT
    int m = 1;
    while (m < N) {
        istep = m << 1;
        theta = -2 * M_PI / istep;
        wpr = cos(theta);
        wpi = sin(theta);
        wr = 1.0;
        wi = 0.0;
        for (k = 0; k < m; k++) {
            for (i = k; i < N; i += istep) {
                j = i + m;
                temp.real = wr * complexBuffer[j].real - wi * complexBuffer[j].imag;
                temp.imag = wr * complexBuffer[j].imag + wi * complexBuffer[j].real;

                complexBuffer[j].real = complexBuffer[i].real - temp.real;
                complexBuffer[j].imag = complexBuffer[i].imag - temp.imag;

                complexBuffer[i].real += temp.real;
                complexBuffer[i].imag += temp.imag;
            }
            double tmp = wr;
            wr = tmp * wpr - wi * wpi;
            wi = wi * wpr + tmp * wpi;
        }
        m = istep;
    }
}
