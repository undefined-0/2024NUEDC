# 单相功率分析仪
本仓库用于存放2024年电赛B题（单相功率分析仪）的软件程序。



## MSPM0G3507接线方法

* 四线0.96寸OLED屏幕

  * PA0 - I2C0 SDA

  * PA1 - I2C0 SCL

  * 3V3、GND两根电源线


* ADC采样IO

  * PA27 - ADC0 Channel 0 Pin（AdcResult_U）
  * PA26 - ADC0 Channel 1 Pin（AdcResult_I）


* 外部按键输入

  *   PA2 - 按下则匝数N自增
      *   N的范围：1~10