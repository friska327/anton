#include "stm32f10x.h"
#include "my_lib.h"
#include "interrupt.h"
#include "SPI.h"
#include "ds18b20.h"

extern char BUFF_RX[];
extern char BUFF_TX[];
extern float 	T_I2C;
extern double T_ADC;
extern float T_ds18b20;


int main (void) {
	SystemCoreClockConfigure();                             
  SystemCoreClockUpdate();
	
	
	//I2C          // PB6 - I2C1_SCL; PB7 - I2C1_SDA
	I2C1_Init();
	DMA_I2C_Init();
	LM75A_Init();
	LM75A_SP_Write();
	// T_I2C
			
	//ADC					// PA1 - voltage
	ADC_Init();
	TIM2_Init ();
	// out T_ADC
	
	//ds18b20					// PA0 - voltage
	ds18b20 ();
	// out T_ds18b20
	
	//request
	TIM4_Init ();
	
	
	//SPI1 				//PA5 - SCK ,PA6 - MISO ,PA7 - MOSI, PB5 - CS
	SPI1_Init ();
	//DMA_SPI();
	//
	
	//USART     // PA9 - TX, PA10 - RX
	USART_init();
	DMI_init();
	//
	
	while (1)
  {
		
  } 
}


