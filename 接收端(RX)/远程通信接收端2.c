/**************************************************
��Դ���ο�����
��д��NRF24L01+����������
ʱ�䣺
��λ��
ע�ͣ�Ϊ�˷��㣬������ͬע�Ͳο�����˴���
/**************************************************/

#include <reg51.h>
#include <stdio.h>
#include <string.h>
#include <api.h>
#include <LQ12864.h>

#define uchar unsigned char
#define uint unsigned int
sbit CE  = P0^0;
sbit CSN = P0^1;
sbit SCK = P0^2;
sbit MOSI= P0^3;
sbit MISO= P0^4;
sbit IRQ = P0^5;


/***************************************************/
#define TX_ADR_WIDTH   32  // 32�ֽڿ�ȵķ���/���յ�ַ
#define TX_PLOAD_WIDTH 26 // ����ͨ����Ч���ݿ��


uchar code TX_ADDRESS[TX_ADR_WIDTH] = {0x34,0x43,0x10,0x10,0x01}; // ����һ����̬���͵�ַ
uchar RX_BUF[TX_PLOAD_WIDTH];
uchar TX_BUF[TX_PLOAD_WIDTH];
const uchar Chin[] = "Working ...";
uchar flag;
uchar DATA[TX_PLOAD_WIDTH];
uchar str[TX_PLOAD_WIDTH];
uchar bdata sta;
sbit  RX_DR	 = sta^6;
sbit  TX_DS	 = sta^5;
sbit  MAX_RT = sta^4;


/**************************************************
����: init_io()

����:
    ��ʼ��IO
/**************************************************/
void init_io(void)
{
	CE  = 0;        // ����
	CSN = 1;        // SPI��ֹ
	SCK = 0;        // SPIʱ���õ�
	IRQ = 1;        // �жϸ�λ
	memset(&str,0,sizeof(str));
}
/**************************************************/

/**************************************************
������delay_ms()

������
    �ӳ�x����
/**************************************************/
void delay_ms(uchar x)
{
    uchar i, j;
    i = 0;
    for(i=0; i<x; i++)
    {
       j = 250;
       while(--j);
	   j = 250;
       while(--j);
    }
}
/**************************************************/

/**************************************************
������SPI_RW()

������
    ����SPIЭ�飬дһ�ֽ����ݵ�nRF24L01��ͬʱ��nRF24L01
	����һ�ֽ�
/**************************************************/
uchar SPI_RW(uchar byte)
{
	uchar i;
   	for(i=0; i<8; i++)          // ѭ��8��
   	{
   		MOSI = (byte & 0x80);   // byte���λ�����MOSI
     	byte <<= 1;             // ��һλ��λ�����λ
   		SCK = 1;                // ����SCK��nRF24L01��MOSI����1λ���ݣ�ͬʱ��MISO���1λ����
   		byte |= MISO;       	// ��MISO��byte���λ
   		SCK = 0;            	// SCK�õ�
   	}
    return(byte);           	// ���ض�����һ�ֽ�
}
/**************************************************/

/**************************************************
������SPI_RW_Reg()

������
    д����value��reg�Ĵ���
/**************************************************/
uchar SPI_RW_Reg(uchar reg, uchar value)
{
	uchar status;
  	CSN = 0;                   // CSN�õͣ���ʼ��������
  	status = SPI_RW(reg);      // ѡ��Ĵ�����ͬʱ����״̬��
  	SPI_RW(value);             // Ȼ��д���ݵ��üĴ���
  	CSN = 1;                   // CSN���ߣ��������ݴ���
  	return(status);            // ����״̬�Ĵ���
}
/**************************************************/

/**************************************************
������SPI_Read()

������
    ��reg�Ĵ�����һ�ֽ�
/**************************************************/
uchar SPI_Read(uchar reg)
{
	uchar reg_val;
  	CSN = 0;                    // CSN�õͣ���ʼ��������
  	SPI_RW(reg);                // ѡ��Ĵ���
  	reg_val = SPI_RW(0);        // Ȼ��ӸüĴ���������
  	CSN = 1;                    // CSN���ߣ��������ݴ���
  	return(reg_val);            // ���ؼĴ�������
}
/**************************************************/

/**************************************************
������SPI_Read_Buf()

������
    ��reg�Ĵ�������bytes���ֽڣ�ͨ��������ȡ����ͨ��
	���ݻ����/���͵�ַ
/**************************************************/
uchar SPI_Read_Buf(uchar reg, uchar *pBuf, uchar bytes)
{
	uchar status, i;
  	CSN = 0;                    // CSN�õͣ���ʼ��������
  	status = SPI_RW(reg);       // ѡ��Ĵ�����ͬʱ����״̬��
  	for(i=0; i<bytes; i++)
	{
	 pBuf[i] = SPI_RW(0);		// ����ֽڴ�nRF24L01����
	}
	//strcpy(DATA,pBuf);		
  	CSN = 1;                     // CSN���ߣ��������ݴ���

  	return(status);             // ����״̬�Ĵ���
}
/**************************************************/

/**************************************************
������SPI_Write_Buf()

������
    ��pBuf�����е�����д�뵽nRF24L01��ͨ������д�뷢
	��ͨ�����ݻ����/���͵�ַ
/**************************************************/
uchar SPI_Write_Buf(uchar reg, uchar * pBuf, uchar bytes)
{
	uchar status, i;
  	CSN = 0;                    // CSN�õͣ���ʼ��������
  	status = SPI_RW(reg);       // ѡ��Ĵ�����ͬʱ����״̬��
  	for(i=0; i<bytes; i++)
    	SPI_RW(*pBuf++);        // ����ֽ�д��nRF24L01
  	CSN = 1;                    // CSN���ߣ��������ݴ���
  	return(status);             // ����״̬�Ĵ���
}
/**************************************************/



/**************************************************
������RX_Mode()

������
    �����������nRF24L01Ϊ����ģʽ���ȴ����շ����豸�����ݰ�
/**************************************************/
void RX_Mode(void)
{
   	CE = 0;
  	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);  // �����豸����ͨ��0ʹ�úͷ����豸��ͬ�ķ��͵�ַ
  	SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);               // ʹ�ܽ���ͨ��0�Զ�Ӧ��
  	SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);           // ʹ�ܽ���ͨ��0
  	SPI_RW_Reg(WRITE_REG + RF_CH, 20);                 // ѡ����Ƶͨ��
  	SPI_RW_Reg(WRITE_REG + RX_PW_P0, TX_PLOAD_WIDTH);  // ����ͨ��0ѡ��ͷ���ͨ����ͬ��Ч���ݿ��
  	SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x0f);            // ���ݴ�����2Mbps�����书��0dBm���������Ŵ�������
  	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);              // CRCʹ�ܣ�16λCRCУ�飬�ϵ磬����ģʽ
  	CE = 1;                                            // ����CE���������豸
}
/**************************************************/

/**************************************************
������TX_Mode()

������
    �����������nRF24L01Ϊ����ģʽ����CE=1��������10us����
	130us���������䣬���ݷ��ͽ����󣬷���ģ���Զ�ת�����
	ģʽ�ȴ�Ӧ���źš�
/**************************************************/
void TX_Mode()
{

	CE = 0;
  	SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);     // д�뷢�͵�ַ
  	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);  // Ϊ��Ӧ������豸������ͨ��0��ַ�ͷ��͵�ַ��ͬ
  	SPI_Write_Buf(WR_TX_PLOAD, TX_BUF, TX_PLOAD_WIDTH);                  // д���ݰ���TX FIFO
  	SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);       // ʹ�ܽ���ͨ��0�Զ�Ӧ��
  	SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);   // ʹ�ܽ���ͨ��0
  	SPI_RW_Reg(WRITE_REG + SETUP_RETR, 0x0a);  // �Զ��ط���ʱ�ȴ�250us+86us���Զ��ط�10��
  	SPI_RW_Reg(WRITE_REG + RF_CH, 20);         // ѡ����Ƶͨ��40
  	SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x0f);    // ���ݴ�����2Mbps�����书��0dBm���������Ŵ�������
  	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e);      // CRCʹ�ܣ�16λCRCУ�飬�ϵ�
	CE = 1;
}

/**************************************************/

/**************************************************




/**************************************************/

/**************************************************
������Check_ACK()

������
    �������豸���޽��յ����ݰ����趨û���յ�Ӧ����
	���Ƿ��ط�
/**************************************************/
uchar Check_ACK(bit clear)
{
	while(IRQ);
	sta = SPI_RW(NOP);                    // ����״̬�Ĵ���
	if(MAX_RT)
		if(clear)                         // �Ƿ����TX FIFO��û������ڸ�λMAX_RT�жϱ�־���ط�
			SPI_RW(FLUSH_TX);
    SPI_RW_Reg(WRITE_REG + STATUS, sta);  // ���TX_DS��MAX_RT�жϱ�־
	IRQ = 1;
	if(TX_DS)
		return(0x00);
	else
		return(0xff);
}


/**************************************************
������main()

������
    ������
/**************************************************/
void main(void)
{
	   init_io();					  // ��Ƶ�����ݳ�ʼ��
	   OLED_Init();		              // ��Ļ���ó�ʼ��
	   OLED_P8x16Str(0,0,Chin);
	  	            
	   while(1)
	   {
	    RX_Mode();				  // ��NRF��Ƶģ�����óɽ���ģʽ
	 	sta = SPI_Read(STATUS);	  // ��״̬�Ĵ���
		SPI_RW_Reg(WRITE_REG + STATUS, 0xff);  // ���RX_DS�жϱ�־
		 SPI_RW_Reg(WRITE_REG+STATUS,sta);// ��� RX_DR �� TX_DS �Լ� MAX_RT �жϱ�־
	     if(RX_DR)
		 {	
		    SPI_Read_Buf(RD_RX_PLOAD, RX_BUF, TX_PLOAD_WIDTH);  // ��RX FIFO������
 		    OLED_P8x16Str(0,2,"Received Sign:");  // ���յ�����źű�־��ʾ
			//sprintf(str,"%4.2f",RX_BUF); 
			OLED_P8x16Str(32,4,RX_BUF);			  // �����յ��������͵���Ļ����ʾ
			OLED_P8x16Str(65,4,"L/min");		  // ��ʾˮ������λ									 
  		 }
		
       }
	   
}

