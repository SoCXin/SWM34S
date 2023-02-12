#include <string.h>
#include "SWM341.h"

#define TX_LEN	24
#define RX_LEN	24*3
uint16_t RX_Buffer[RX_LEN] = {0};
uint16_t TX_Buffer[TX_LEN] = {0x0001, 0x0102, 0x0203, 0x0304, 0x0405, 0x0506, 0x0607, 0x0708, 0x0809, 0x090A, 0x0A0B, 0x0B0C,
							  0x0C0D, 0x0D0E, 0x0E0F, 0x0F10, 0x1011, 0x1112, 0x1213, 0x1314, 0x1415, 0x1516, 0x1617, 0x1718};

volatile uint32_t RX_Complete = 0;

void SerialInit(void);
void I2S_Slave_Init(void);
void I2S_Master_Init(void);

int main(void)
{	
	uint32_t i;
	
	SystemInit();
	
	SerialInit();
	
	I2S_Slave_Init();
	
	I2S_Master_Init();
	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);		// 调试指示信号
		
	while(1==1)
	{
		if(RX_Complete == 1)
		{
			RX_Complete = 0;
			
			for(i = 0; i < RX_LEN; i++)
			{
				printf("%04X, ", RX_Buffer[i]);
			}
			printf("\r\n\r\n");
			
			memset(RX_Buffer, 0x00, sizeof(RX_Buffer[0]) * RX_LEN);
			
			for(i = 0; i < SystemCoreClock/8; i++) __NOP();
			
//			I2S_Open(SPI0);
			DMA_CH_Open(DMA_CH1);	// 重新开始，再次搬运
		}
	}
}


void I2S_Master_Init(void)
{
	I2S_InitStructure I2S_initStruct;
	DMA_InitStructure DMA_initStruct;
	
	PORT_Init(PORTM, PIN3, PORTM_PIN3_SPI0_SSEL, 0);	// I2S_WS
	PORT_Init(PORTM, PIN2, PORTM_PIN2_SPI0_SCLK, 0);	// I2S_CK
	PORT_Init(PORTM, PIN5, PORTM_PIN5_SPI0_MOSI, 0);	// I2S_DO
	PORT_Init(PORTM, PIN4, PORTM_PIN4_SPI0_MISO, 1);	// I2S_DI
	
	I2S_initStruct.Mode = I2S_MASTER_RX;
	I2S_initStruct.FrameFormat = I2S_I2S_PHILIPS;
	I2S_initStruct.DataLen = I2S_DATALEN_16;
	I2S_initStruct.ClkFreq = 44100 * 2 * 16;	// 44.1K
	I2S_initStruct.RXThresholdIEn = 0;
	I2S_initStruct.TXThresholdIEn = 0;
	I2S_initStruct.TXCompleteIEn  = 0;
	I2S_Init(SPI0, &I2S_initStruct);
	
	I2S_Open(SPI0);
	
	
	// SPI0 RX DMA
	SPI0->CTRL |= (1 << SPI_CTRL_DMARXEN_Pos);
	
	DMA_initStruct.Mode = DMA_MODE_SINGLE;
	DMA_initStruct.Unit = DMA_UNIT_HALFWORD;
	DMA_initStruct.Count = RX_LEN;
	DMA_initStruct.SrcAddr = (uint32_t)&SPI0->DATA;
	DMA_initStruct.SrcAddrInc = 0;
	DMA_initStruct.DstAddr = (uint32_t)RX_Buffer;
	DMA_initStruct.DstAddrInc = 1;
	DMA_initStruct.Handshake = DMA_CH1_SPI0RX;
	DMA_initStruct.Priority = DMA_PRI_LOW;
	DMA_initStruct.DoneIE = 1;
	DMA_CH_Init(DMA_CH1, &DMA_initStruct);
	
	DMA_CH_Open(DMA_CH1);
}


void I2S_Slave_Init(void)
{
	I2S_InitStructure I2S_initStruct;
	DMA_InitStructure DMA_initStruct;
	
	PORT_Init(PORTB, PIN5, PORTB_PIN5_SPI1_SSEL, 1);	// I2S_WS
	PORT_Init(PORTB, PIN2, PORTB_PIN2_SPI1_SCLK, 1);	// I2S_CK
	PORT_Init(PORTB, PIN3, PORTB_PIN3_SPI1_MISO, 0);	// I2S_DO
	PORT_Init(PORTB, PIN4, PORTB_PIN4_SPI1_MOSI, 1);	// I2S_DI
	
	I2S_initStruct.Mode = I2S_SLAVE_TX;
	I2S_initStruct.FrameFormat = I2S_I2S_PHILIPS;
	I2S_initStruct.DataLen = I2S_DATALEN_16;
	I2S_initStruct.ClkFreq = 0;
	I2S_initStruct.RXThresholdIEn = 0;
	I2S_initStruct.TXThresholdIEn = 0;
	I2S_initStruct.TXCompleteIEn  = 0;
	I2S_Init(SPI1, &I2S_initStruct);
	
	I2S_Open(SPI1);
	
	
	// SPI0 TX DMA
	SPI1->CTRL |= (1 << SPI_CTRL_DMATXEN_Pos);
	
	DMA_initStruct.Mode = DMA_MODE_SINGLE;
	DMA_initStruct.Unit = DMA_UNIT_HALFWORD;
	DMA_initStruct.Count = TX_LEN;
	DMA_initStruct.SrcAddr = (uint32_t)TX_Buffer;
	DMA_initStruct.SrcAddrInc = 1;
	DMA_initStruct.DstAddr = (uint32_t)&SPI1->DATA;
	DMA_initStruct.DstAddrInc = 0;
	DMA_initStruct.Handshake = DMA_CH2_SPI1TX;
	DMA_initStruct.Priority = DMA_PRI_LOW;
	DMA_initStruct.DoneIE = 1;
	DMA_CH_Init(DMA_CH2, &DMA_initStruct);
	
	DMA_CH_Open(DMA_CH2);
}


void DMA_Handler(void)
{
	if(DMA_CH_INTStat(DMA_CH1))
	{
		DMA_CH_INTClr(DMA_CH1);
		
		GPIO_InvBit(GPIOA, PIN5);
		
		/* 如果不停止，I2S主机一直发送时钟；
		   如果停止，时钟可能停在I2S传输数据的中间，导致传输错位 */
//		I2S_Close(SPI0);
		
		RX_Complete = 1;
	}
	
	if(DMA_CH_INTStat(DMA_CH2))
	{
		DMA_CH_INTClr(DMA_CH2);
		
		DMA_CH_Open(DMA_CH2);
	}
}


void SerialInit(void)
{
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTM, PIN0, PORTM_PIN0_UART0_RX, 1);	//GPIOM.0配置为UART0输入引脚
	PORT_Init(PORTM, PIN1, PORTM_PIN1_UART0_TX, 0);	//GPIOM.1配置为UART0输出引脚
 	
 	UART_initStruct.Baudrate = 57600;
	UART_initStruct.DataBits = UART_DATA_8BIT;
	UART_initStruct.Parity = UART_PARITY_NONE;
	UART_initStruct.StopBits = UART_STOP_1BIT;
	UART_initStruct.RXThresholdIEn = 0;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutIEn = 0;
 	UART_Init(UART0, &UART_initStruct);
	UART_Open(UART0);
}

/****************************************************************************************************************************************** 
* 函数名称: fputc()
* 功能说明: printf()使用此函数完成实际的串口打印动作
* 输    入: int ch		要打印的字符
*			FILE *f		文件句柄
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
int fputc(int ch, FILE *f)
{
	UART_WriteByte(UART0, ch);
	
	while(UART_IsTXBusy(UART0));
 	
	return ch;
}
