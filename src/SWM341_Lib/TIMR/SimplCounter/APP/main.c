#include "SWM341.h"


int main(void)
{	
	uint32_t i;
	
	SystemInit();
	
	GPIO_Init(GPIOA, PIN0, 1, 0, 0, 0);				//�Ӽ��������룬����GPIOA.0������
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);
	
	PORT_Init(PORTB, PIN9, PORTB_PIN9_TIMR2_IN, 1);
	
	TIMR_Init(TIMR2, TIMR_MODE_COUNTER, 1, 3, 1);	//ÿ��3�������ؽ����ж�
	
	TIMR_Start(TIMR2);
	
	while(1==1)
	{
		GPIO_InvBit(GPIOA, PIN0);
		for(i = 0; i < SystemCoreClock/25; i++) __NOP();
	}
}

void TIMR2_Handler(void)
{
	TIMR_INTClr(TIMR2);
	
	GPIO_InvBit(GPIOA, PIN5);
}
