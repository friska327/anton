
#include "stm32f10x.h"

float U;
float T_ADC;

void TIM2_Init () {																 // TIM4 using for start conversion
	  RCC  -> APB1ENR |= RCC_APB1ENR_TIM2EN; 				 // TIM4 clock enable
		TIM2 -> CR1 = TIM_CR1_CEN;										 // TIM4 counter enable
	
		TIM2->PSC = (SystemCoreClock / 10000 ) - 1;	
		TIM2->ARR = 2000;
		TIM2->CR2 |= TIM_CR2_MMS_1;
	
}

/*
void TIM3_Init () {																 // TIM4 using for start conversion
	  RCC  -> APB1ENR |= RCC_APB1ENR_TIM3EN; 				 // TIM4 clock enable
		TIM3 -> CR1 = TIM_CR1_CEN;										 // TIM4 counter enable
	
		TIM3->PSC = (SystemCoreClock / 10000 ) - 1;	
		TIM3->ARR = 2000;
				
		TIM3->DIER |= TIM_DIER_UIE;										 // Enable TIM4 interrupt
		NVIC_EnableIRQ (TIM3_IRQn);										 // Enable TIM4 interrupt	in interrupt controller
}
*/

void ADC_Init()
{

   RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
   
   GPIOA->CRL &= ~GPIO_CRL_CNF1;					// nalog mode input
   
   RCC->CFGR |= RCC_CFGR_ADCPRE_DIV2;  //28/2
  
   RCC->APB2ENR |= RCC_APB2ENR_ADC1EN ; 
   
   ADC1->CR2 |= ADC_CR2_CAL;
	 while (!(ADC1->CR2 & ADC_CR2_CAL)){;} //calibration
   
  
	 NVIC_EnableIRQ(ADC1_2_IRQn); // glabal interrupt enable
   
		ADC1->SMPR2 = ADC_SMPR2_SMP1_0; //55.5 cycle sampling
		ADC1->SMPR2 = ADC_SMPR2_SMP1_2;
   
   ADC1->CR1 = ADC_CR1_EOCIE;				// interrupt end of convertion
   
   ADC1->CR2 &= ~ADC_CR2_JEXTSEL;
	 ADC1->CR2 |= ADC_CR2_JEXTSEL_1;
	 ADC1->CR2	|= ADC_CR2_ADON;
	 ADC1->CR2	|= ADC_CR2_JEXTTRIG; 

   ADC1->JSQR &= ~ADC_JSQR_JL;
	 ADC1->JSQR &= ~ADC_JSQR_JSQ4;
	 ADC1->JSQR |= ADC_JSQR_JSQ4_0;
	 	
}
	

void ADC1_2_IRQHandler(void)
{
   if(ADC1->SR & ADC_SR_EOC)
   {
        U = (ADC1->JDR1 & 0x00000FFF) * 0.000805664062; //U volts
				T_ADC = U * 100 - 50;															//T gradus
	  }
   ADC1->SR=0;
}	
	




	
