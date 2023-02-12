#include "SWM341.h"

#define RX_LEN	32
char RX_Buffer[RX_LEN] = {0};

void SerialInit(void);

int main(void)
{
	DMA_InitStructure DMA_initStruct;
	
	SystemInit();
	
	SerialInit();
	
	DMA_initStruct.Mode = DMA_MODE_SINGLE;
	DMA_initStruct.Unit = DMA_UNIT_BYTE;
	DMA_initStruct.Count = RX_LEN;
	DMA_initStruct.SrcAddr = (uint32_t)&UART0->DATA;
	DMA_initStruct.SrcAddrInc = 0;
	DMA_initStruct.DstAddr = (uint32_t)RX_Buffer;
	DMA_initStruct.DstAddrInc = 1;
	DMA_initStruct.Handshake = DMA_CH1_UART0RX;
	DMA_initStruct.Priority = DMA_PRI_LOW;
	DMA_initStruct.DoneIE = 1;
	DMA_CH_Init(DMA_CH1, &DMA_initStruct);
	DMA_CH_Open(DMA_CH1);
   	
	while(1==1)
	{
	}
}


void DMA_Handler(void)
{
	uint32_t i;
	
	if(DMA_CH_INTStat(DMA_CH1))
	{
		DMA_CH_INTClr(DMA_CH1);
		
		/* 注意：打印会占用很长时间，导致接收丢失数据，
		         实际应用中可将 RX_Buffer 中数据拷贝到另一个 Buffer 中，从而快速重启 DMA_CH1 通道 */
		for(i = 0; i < RX_LEN; i++) printf("%c", RX_Buffer[i]);
		
		DMA_CH_Open(DMA_CH1);	// 重新开始，再次搬运
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
	UART_initStruct.RXThreshold = 3;
	UART_initStruct.RXThresholdIEn = 0;
	UART_initStruct.TXThreshold = 3;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutTime = 10;
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
