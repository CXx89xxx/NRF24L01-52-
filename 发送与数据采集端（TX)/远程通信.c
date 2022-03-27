/**************************************************
来源：参考网上
改写：NRF24L01+涡轮流量计
时间：
单位：
/**************************************************/

#include <reg51.h>		 //	51单片机通用寄存器地址库，52也部分通用
#include <stdio.h>
#include <string.h>		 //	字符串功能函数库
#include <api.h>		 //	NRF24L01的SPI寄存器地址库
#include <LQ12864.h>	 //	OLED屏幕功能函数库

#define uchar unsigned char	  //  uchar宏定义
#define uint unsigned int	  //  uint宏定义
sbit CE  = P0^0;
sbit CSN = P0^1;
sbit SCK = P0^2;
sbit MOSI= P0^3;
sbit MISO= P0^4;
sbit IRQ = P0^5;			   //射频接口配置

/***************************************************/
#define TX_ADR_WIDTH   32  // 32字节宽度的发送/接收地址
#define TX_PLOAD_WIDTH 26 // 数据通道有效数据宽度


uchar code TX_ADDRESS[TX_ADR_WIDTH] = {0x34,0x43,0x10,0x10,0x01}; // 定义一个静态发送地址
uchar RX_BUF[TX_PLOAD_WIDTH];		 //定义接收窗口数组
uchar TX_BUF[TX_PLOAD_WIDTH];		 //定义发送窗口数组
uchar box[TX_PLOAD_WIDTH];			  //定义过渡数据包数组
const uchar Chin[] = "Working ...";		//定义工作中标志数组
uchar flag;
uchar str[TX_PLOAD_WIDTH];			 //定义字符串数组
uchar bdata sta;
float Q,F,num;
sbit  RX_DR	 = sta^6;				 
sbit  TX_DS	 = sta^5;
sbit  MAX_RT = sta^4;				 //配置NRF射频模块中断接口


/**************************************************
函数: init_io()

描述:
    初始化IO
/**************************************************/
void init_io(void)
{
	CE  = 0;        // 待机
	CSN = 1;        // SPI禁止
	SCK = 0;        // SPI时钟置低
	IRQ = 1;        // 中断复位   	             
	memset(&str,0,sizeof(str));			  // 清空字符串数组
	memset(&TX_BUF,0,sizeof(TX_BUF));	  // 清空发送窗口数组
	memset(&box,0,sizeof(box));			  // 清空过渡数据包数组
	TMOD = 0X15;//定时器计数器工作方式配置
    TH0=0;
    TL0=0;
    TH1=(65536-45872)/256;
    TL1=(65536-45872)%256;
    EA = 1;     //总中断打开
    ET0 = 1;	//中断运行
    ET1 = 1;
    TR0 = 1;    //运行控制位
    TR1 = 1; 
}
/**************************************************/

/**************************************************
函数：delay_ms()

描述：
    延迟x毫秒
/**************************************************/

/**************************************************/

/**************************************************
函数：SPI_RW()

描述：
    根据SPI协议，写一字节数据到nRF24L01，同时从nRF24L01
	读出一字节
/**************************************************/
uchar SPI_RW(uchar byte)
{
	uchar i;
   	for(i=0; i<8; i++)          // 循环8次
   	{
   		MOSI = (byte & 0x80);   // byte最高位输出到MOSI
     	byte <<= 1;             // 低一位移位到最高位
   		SCK = 1;                // 拉高SCK，nRF24L01从MOSI读入1位数据，同时从MISO输出1位数据
   		byte |= MISO;       	// 读MISO到byte最低位
   		SCK = 0;            	// SCK置低
   	}
    return(byte);           	// 返回读出的一字节
}
/**************************************************/

/**************************************************
函数：SPI_RW_Reg()

描述：
    写数据value到reg寄存器
/**************************************************/
uchar SPI_RW_Reg(uchar reg, uchar value)
{
	uchar status;
  	CSN = 0;                   // CSN置低，开始传输数据
  	status = SPI_RW(reg);      // 选择寄存器，同时返回状态字
  	SPI_RW(value);             // 然后写数据到该寄存器
  	CSN = 1;                   // CSN拉高，结束数据传输
  	return(status);            // 返回状态寄存器
}
/**************************************************/

/**************************************************
函数：SPI_Read()

描述：
    从reg寄存器读一字节
/**************************************************/
uchar SPI_Read(uchar reg)
{
	uchar reg_val;
  	CSN = 0;                    // CSN置低，开始传输数据
  	SPI_RW(reg);                // 选择寄存器
  	reg_val = SPI_RW(0);        // 然后从该寄存器读数据
  	CSN = 1;                    // CSN拉高，结束数据传输
  	return(reg_val);            // 返回寄存器数据
}
/**************************************************/

/**************************************************
函数：SPI_Read_Buf()

描述：
    从reg寄存器读出bytes个字节，通常用来读取接收通道
	数据或接收/发送地址
/**************************************************/
uchar SPI_Read_Buf(uchar reg, uchar *pBuf, uchar bytes)
{
	uchar status, i;
  	CSN = 0;                    // CSN置低，开始传输数据
  	status = SPI_RW(reg);       // 选择寄存器，同时返回状态字
  	for(i=0; i<bytes; i++)
	{
	 pBuf[i] = SPI_RW(0);		// 逐个字节从nRF24L01读出
	}	
  	CSN = 1;                     // CSN拉高，结束数据传输

  	return(status);             // 返回状态寄存器
}
/**************************************************/

/**************************************************
函数：SPI_Write_Buf()

描述：
    把pBuf缓存中的数据写入到nRF24L01，通常用来写入发
	射通道数据或接收/发送地址
/**************************************************/
uchar SPI_Write_Buf(uchar reg, uchar * pBuf, uchar bytes)
{
	uchar status, i;
  	CSN = 0;                    // CSN置低，开始传输数据
  	status = SPI_RW(reg);       // 选择寄存器，同时返回状态字
  	for(i=0; i<bytes; i++)
    	SPI_RW(*pBuf++);        // 逐个字节写入nRF24L01
  	CSN = 1;                    // CSN拉高，结束数据传输
  	return(status);             // 返回状态寄存器
}
/**************************************************/



/**************************************************
函数：RX_Mode()

描述：
    这个函数设置nRF24L01为接收模式，等待接收发送设备的数据包
/**************************************************/
void RX_Mode(void)
{
   	CE = 0;
  	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);  // 接收设备接收通道0使用和发送设备相同的发送地址
  	SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);               // 使能接收通道0自动应答
  	SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);           // 使能接收通道0
  	SPI_RW_Reg(WRITE_REG + RF_CH, 40);                 // 选择射频通道
  	SPI_RW_Reg(WRITE_REG + RX_PW_P0, TX_PLOAD_WIDTH);  // 接收通道0选择和发送通道相同有效数据宽度
  	SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x0f);            // 数据传输率2Mbps，发射功率0dBm，低噪声放大器增益
  	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);              // CRC使能，16位CRC校验，上电，接收模式
  	CE = 1;                                            // 拉高CE启动接收设备
}
/**************************************************/

/**************************************************
函数：TX_Mode()

描述：
    这个函数设置nRF24L01为发送模式，（CE=1持续至少10us），
	130us后启动发射，数据发送结束后，发送模块自动转入接收
	模式等待应答信号。
/**************************************************/
void TX_Mode()
{

	CE = 0;
  	SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);     // 写入发送地址
  	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);  // 为了应答接收设备，接收通道0地址和发送地址相同
  	SPI_Write_Buf(WR_TX_PLOAD, TX_BUF, TX_PLOAD_WIDTH);                  // 写数据包到TX FIFO
  	SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);       // 使能接收通道0自动应答
  	SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);   // 使能接收通道0
  	SPI_RW_Reg(WRITE_REG + SETUP_RETR, 0x0a);  // 自动重发延时等待250us+86us，自动重发10次
  	SPI_RW_Reg(WRITE_REG + RF_CH, 20);         // 选择射频通道40
  	SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x0f);    // 数据传输率2Mbps，发射功率0dBm，低噪声放大器增益
  	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e);      // CRC使能，16位CRC校验，上电
	CE = 1;
}

/**************************************************/

/**************************************************




/**************************************************/

/**************************************************
函数：Check_ACK()

描述：
    检查接收设备有无接收到数据包，设定没有收到应答信
	号是否重发
/**************************************************/
uchar Check_ACK(bit clear)
{
	while(IRQ);
	sta = SPI_RW(NOP);                    // 返回状态寄存器
	if(MAX_RT)
		if(clear)                         // 是否清除TX FIFO，没有清除在复位MAX_RT中断标志后重发
			SPI_RW(FLUSH_TX);
    SPI_RW_Reg(WRITE_REG + STATUS, sta);  // 清除TX_DS或MAX_RT中断标志
	IRQ = 1;
	if(TX_DS)
		return(0x00);
	else
		return(0xff);
}

/**************************************************
函数：display()

描述：
    显示函数
/**************************************************/
void display(float i)
{
 sprintf(str,"%4.2fL/min",i);		 // 将计算得到的数据进行格式处理，打印成不超出屏幕宽度的十进制浮点数
 OLED_P8x16Str(32,4,str);		     // 将打印好的数据显示在屏幕上
 sprintf(box,"%4.2f",i);			 // 将计算得到的数据进行打包处理，打包成适合发送的
 strcpy(TX_BUF,box);				 // 将打包好的数据递送到发送窗口
}


/**************************************************
函数：KeyButtons()

描述：
    发射模式并且检测是否发送，发送一字节数据
/**************************************************/
void KeyButtons()
{   
   
	TX_Mode();		        // 把nRF24L01设置为发送模式并发送数据
	Check_ACK(1);               // 等待发送完毕，清除TX FIFO

}
/**************************************************/
/**************************************************
函数：read()

描述：
    水流量数据采集函数
/**************************************************/
uint read()
{
	uint t1,th1,th2;//读两次高位，两次高位一样说明没有低位进位，读数更加精确。
	uint val;
	while(1)
	{
	   th1 = TH0;
	   t1 = TL0;
	   th2 = TH0;
	   if(th1==th2)
	   break; 	
	} 
	val = th1*256+t1;	 //th1为高8位，t1为低8位
	return val;  //返回计数值，为频率值
}

/**************************************************
函数：main()

描述：
    主函数
/**************************************************/
void main(void)
{
        
	   init_io();	              // 数据内存以及NRF射频系统初始化
	   OLED_Init();	              // 屏幕函数初始化
	   OLED_P8x16Str(0,0,Chin);	  // 显示工作中标志
	   OLED_P8x16Str(0,2,"Collecting data");	// 屏幕显示数据采集中	            
	   while(1)
	   {
		display(Q);					// 进入数据采集并且显示到屏幕上
        KeyButtons();				// 对NRF系统模式进行设置，并且发送数据
		 if(MAX_RT)					// 数据发送完毕标志
	      {
		   SPI_RW_Reg(FLUSH_TX,0);	 // 发送窗口数据清零
       	  }
	
       }  
}



void T1_time()interrupt 3		//计数器/定时器0中断入口
{
   TH1=(65536-45872)/256;  //计时50ms
   TL1=(65536-45872)%256; 
   num++;
   if(num==10)    // 0.5s更新一次数据 
   {
   	num=0;
   	F=read();    //每间隔0.5s读一次计数器0，该值为频率。计算出Q后立即把计数器01清零重新计数
   	if(F>0)
	{
	 Q = (F+3)/8.1;  //流量计传感器频率-流量转换公式
	 TH0=0;		    
	 TL0=0;  		  //计数器清零
	} 
	else
	 {
		Q=0;       //如果不加，则Q！=0；
		TH0 = 0;
		TL0 = 0;   //计数器清零
	 } 
	    
	} 
}
