#ifndef __FFT_H__
#define __FFT_H__

#define DFT_N 2048 // DFT点数
#define M_PI 3.14159265358979323846

// 定义复数结构体
typedef struct {
    float real;
    float imag;
} Complex;

extern Complex s[DFT_N];

void FFT(Complex *complexBuffer, int N);
void RemoveDCBias(Complex *complexBuffer, int N);

#endif /*__FFT_H__*/
