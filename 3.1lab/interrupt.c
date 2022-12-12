#include "stm32f10x.h"
#include "my_lib.h"
#include "ds18b20.h"
#include "SPI.h"

			float BUFFER_TX[300];
			float BUFFER_RX[300];
			int		ptr = 0;
extern char RX_BUFF[];
extern float 	T_I2C;
extern float 	T_ADC;
extern float  T_ds18b20;



void TIM4_IRQHandler () {													 // LM75A
		TIM4->SR &= ~TIM_SR_UIF;	
		LM75A_T_READ();
		convert();
		read_T();
		
		if(ptr > 297)
		{
			ptr = 0;
		}
		
		BUFFER_TX[ptr++] = T_I2C;
		BUFFER_TX[ptr++ + 100] = T_ADC;
		BUFFER_TX[ptr++ + 200] = T_ds18b20;
}


void I2C1_ER_IRQHandler()
{
	if(I2C1->SR1 && I2C_SR1_AF)
	{
	
	//TIM4->SR &= ~TIM_SR_UIF;
	I2C1->CR1 |= I2C_CR1_STOP;
		
	DMA1_Channel6->CCR &= ~DMA_CCR6_EN;
	DMA1_Channel7->CCR &= ~DMA_CCR7_EN;
		
	}
}

void USART1_IRQHandler() // IDLE
{

		if(RX_BUFF[0] == '1')
		{
			ReadData(0, BUFFER_RX, 1200);
		}
		else
		{
			for(int i = 0; i < 300; i++)
			{
				WriteData(i * 4, &(BUFFER_TX[i])  , 4);
			  //WriteData(0, BUFFER_TX , 20);
				DelayMicros(5000);
			}
		}
			
		//IDLE interrupt
		USART1->SR; //IDLE bit clear
		USART1->DR; //
	
		DMI_channel5_go(RX_BUFF, BUFF_SIZE);//RX mess
}

