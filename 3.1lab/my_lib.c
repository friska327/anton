#include "stm32f10x.h"






void SystemCoreClockConfigure(void) {

  RCC->CR |= ((uint32_t)RCC_CR_HSION);                     // Enable HSI
  while ((RCC->CR & RCC_CR_HSIRDY) == 0);                  // Wait for HSI Ready

  RCC->CFGR = RCC_CFGR_SW_HSI;                             // HSI is system clock
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);  // Wait for HSI used as system clock
	
  RCC->CR &= ~RCC_CR_PLLON;                                // Disable PLL

   RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL);
  RCC->CFGR |=  (RCC_CFGR_PLLSRC_HSI_Div2 | RCC_CFGR_PLLMULL7);

  RCC->CR |= RCC_CR_PLLON;                                 // Enable PLL
  while((RCC->CR & RCC_CR_PLLRDY) == 0) __NOP();           // Wait till PLL is ready

  RCC->CFGR &= ~RCC_CFGR_SW;                               // Select PLL as system clock source
  RCC->CFGR |=  RCC_CFGR_SW_PLL;
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  // Wait till PLL is system clock src
}


void GPIO_Init (void) 
{
	//I2C1 - first

  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; /* Enable GPIOB clock            */
	
	GPIOB->CRL	|= GPIO_CRL_CNF6;				//Alternative fan open drain 
	GPIOB->CRL	|= GPIO_CRL_MODE6;			//50 Mhz
	
	GPIOB->CRL	|= GPIO_CRL_CNF7;				//Alternative fan open drain 
	GPIOB->CRL	|= GPIO_CRL_MODE7;			//50 Mhz
	
}

