# NRF24L01-52-
基于STC90C52RC的涡轮流量计对流体进行测量，并且用OLED屏幕显示数据，用NRF24L01进行远程通信。
一、项目简介
1、所需硬件
      三线的霍尔涡轮流量计，0.96寸OLED屏幕两块，两块ST90C52RC开发板，两块NRF24L01增强型模块。
2、工作大致原理
      流体带动涡轮旋转，霍尔传感器将数据转换成固定脉冲输入到单片机的P3.4外部中断引脚，引发中断，每隔0.5s采集一次数据，并将脉冲频率读出后，根据频率-流量公式将流量算出来发送到OLED屏幕进行显示，同时发送到NRF模块的TX窗口进行数据发送。接收端接收到数据后，将它显示在OLED屏幕上。
注：每次中断结束后都会重新将计算好的流量数据发送到TX窗口进一步激活发送，中断相当于起到一个NRF24L01的发送复位作用。
二、关于头文件：
用reg51.h还是reg52.h在这个项目里面都是一样的。
三、本人开发过程中遇到的问题
望能解决NRF使用的疑惑
1、为什么程序不报错，但是NRF通信不成功？
答：
环境原因：
       NRF通信频率是2.4~2.5G，与大部分的无线产品相同的通信频率，当你的模块，特别是非增强型的NRF24L01模块，会容易收到信号干扰导致丢包，譬如你板子旁边的无线鼠标，以及WIFI等。但是影响较为微弱。不用很担心。
硬件原因：
       我第一款打的板子因为布线不够优化，导致NRF的通信成功率不算很高，后来优化排线和空间布局，将复位的大电容远离NRF的8P接口后，通信成功率高了很多，盲猜是因为电容会起到干扰作用。还有一个原因是接线的杜邦线没有接牢也会通信不太成功。经典就是你需要按压排线口，屏幕（NRF模块）才显示（接收）到数据。
还有一个就是AMS1117-33降压电路设计的不合理导致电压不稳定也可能导致通信不超过。
2、有时候屏幕上会显示乱码，或者涡轮旋转但是数据不更新（不采集）？
乱码很可能是因为存放数据的数组内存溢出什么的。多重启几次开发板或者复位几下试一试。
数据不采集很可能是中断溢出了，也是建议重启板子，或者单独调试一下涡轮流量计这个模块，记得把排线接牢一点。或者你把num=10改成20试一试，等好了以后再改回10。就是改变采样间隔。
3、NRF通信始终不成功怎么办？
答：我对于NRF模块的调试也搞了很长时间，如果一次不成功，就多试一试，按照数据手册微调程序，比如发送与接收的端口改成别的（0~40都可以用），或者改一改重发次数，或者改一改check()函数，或者改变发射功率试一试。总之多改一改代码。
里面比较关键的功能函数是SPI_Read_Buf(uchar reg, uchar *pBuf, uchar bytes)，以及TX_MODE()和RX_MODE()，SPI_RW（）不要去改动，这是一个很重要的基础函数。
