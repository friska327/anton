#include "stm32f10x.h"
#include "my_lib.h"

#define USART_BAUD 57600
//#define USART_BAUD 9600
extern char RX_BUFF[];

void DMI_channel4_go(char *mass, int num) //TX
{
	
	DMA1_Channel4->CCR &= ~DMA_CCR4_EN;
	
	DMA1_Channel4->CMAR = mass;	
	DMA1_Channel4->CNDTR = num;
	
	DMA1_Channel4->CCR |= DMA_CCR4_EN;	
}


void DMI_channel5_go(char *mass, int num)		//RX
{
	DMA1_Channel5->CCR &= ~DMA_CCR5_EN;	
	
	DMA1_Channel5->CMAR = mass;
	DMA1_Channel5->CNDTR = num;
	
	DMA1_Channel5->CCR |= DMA_CCR5_EN;	
}

void DMI_init()
{
	
RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	
DMA1_Channel4->CCR |= DMA_CCR4_MINC;
DMA1_Channel4->CCR |= DMA_CCR4_DIR;
//DMA1_Channel4->CCR |= DMA_CCR4_TCIE;
DMA1_Channel4->CPAR = &USART1->DR;
//NVIC_EnableIRQ (DMA1_Channel4_IRQn);	
	
DMA1_Channel5->CPAR = &USART1->DR;
DMA1_Channel5->CCR |= DMA_CCR5_MINC;
	
DMI_channel5_go(RX_BUFF, BUFF_SIZE);
	
}


void USART_init()
{
	
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN; 
	
	GPIOA->CRH &= (~GPIO_CRH_CNF9_0);										//out
	GPIOA->CRH |= (GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9);
	
	GPIOA->CRH &= ~GPIO_CRH_CNF10;											//in
	GPIOA->CRH |= GPIO_CRH_CNF10_1;
	GPIOA->CRH &= (~(GPIO_CRH_MODE10));
	GPIOA->BSRR |= GPIO_ODR_ODR10;
			
	USART1->BRR &= 0; 
	USART1->BRR |= (SystemCoreClock/(USART_BAUD));
	
	USART1->CR1 |= USART_CR1_UE;  ////
	USART1->CR1 |= USART_CR1_TE | USART_CR1_RE ; 
	USART1->CR2 = 0;
	USART1->CR3 = 0;
	
	USART1->CR3 |= USART_CR3_DMAT;
	USART1->CR3 |= USART_CR3_DMAR;
	
	USART1->CR1 |= USART_CR1_IDLEIE;  
	NVIC_EnableIRQ (USART1_IRQn);
	//USART1->CR1 |= USART_CR1_TCIE;
	

	
	
	
}