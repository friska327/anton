
#include "stm32f10x.h"


#define Freq 100000
#define Addr_Recive 		0x91
#define Addr_Transmite 	0x90
#define CFG 0

float T_I2C;


char BUFF_TX [10];
char BUFF_RX [10];
int	 BUFF_DATA [2];

void TIM4_Init () {																 // TIM4 using for start conversion
	
	  RCC  -> APB1ENR |= RCC_APB1ENR_TIM4EN; 				 // TIM4 clock enable
		TIM4 -> CR1 = TIM_CR1_CEN;										 // TIM4 counter enable
	
		TIM4->PSC = (SystemCoreClock / 10000 ) - 1;	
		TIM4->ARR = 3000;
				
		TIM4->DIER |= TIM_DIER_UIE;										 // Enable TIM4 interrupt
		NVIC_EnableIRQ (TIM4_IRQn);										 // Enable TIM4 interrupt	in interrupt controller
}

void I2C1_Init()
{
	
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	RCC->CFGR	|= RCC_CFGR_PPRE1_DIV2;
	
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; /* Enable GPIOB clock            */
	
	GPIOB->CRL	|= GPIO_CRL_CNF6;				//Alternative fan open drain 
	GPIOB->CRL	|= GPIO_CRL_MODE6;			//50 Mhz
	
	GPIOB->CRL	|= GPIO_CRL_CNF7;				//Alternative fan open drain 
	GPIOB->CRL	|= GPIO_CRL_MODE7;			//50 Mhz
	
	
		I2C1->CR2 |= I2C_CR2_LAST;//set next DMA EOT is last transfer
	
		I2C1->CR2 |= I2C_CR2_DMAEN; //enable DMA 
		
		I2C1->CCR |= 100;  //based on calculation
		I2C1->TRISE= 72; //output max rise 
	
		//I2C1->CR2 |= I2C_CR2_ITBUFEN;
		I2C1->CR2 |= I2C_CR2_ITERREN;
		//I2C1->CR2 |= I2C_CR2_ITEVTEN;
		NVIC_EnableIRQ (I2C1_ER_IRQn);
				
		I2C1->CR1 |=I2C_CR1_PE;

}
	
	
void DMA_I2C_Init()
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	
	//6 chennel //TX I2C
	DMA1_Channel6->CCR |= DMA_CCR6_MINC;
	DMA1_Channel6->CCR |= DMA_CCR6_DIR;
	DMA1_Channel6->CPAR = &I2C1->DR;
		
	DMA1_Channel6->CCR |= DMA_CCR6_TCIE;
	NVIC_EnableIRQ(DMA1_Channel6_IRQn);
		
	//7 chaneel //RX I2C
	
	DMA1_Channel7->CCR |= DMA_CCR7_MINC;
	DMA1_Channel7->CPAR = &I2C1->DR;
	
	DMA1_Channel7->CCR |= DMA_CCR7_TCIE;		//full transfer
	NVIC_EnableIRQ(DMA1_Channel7_IRQn);
	
}
	

void DMA_TX_GO(char *mass, char num) // 6 channel
{
	
	
	DMA1_Channel6->CMAR = mass;
	DMA1_Channel6->CNDTR = num;
	
	DMA1_Channel6->CCR |= DMA_CCR6_EN;

}

void DMA_RX_GO(char *mass, char num) // 7 channel
{
	
	DMA1_Channel7->CMAR = mass;
	DMA1_Channel7->CNDTR = num;
	
	DMA1_Channel7->CCR |= DMA_CCR7_EN;
}


void DMA1_Channel6_IRQHandler()
{
	DMA1_Channel6->CCR &= ~DMA_CCR6_EN;
	
	while(!(I2C_SR1_BTF && I2C1->SR1)){;}
		
	I2C1->CR1 |= I2C_CR1_STOP;	
	DMA1->IFCR |= DMA_IFCR_CTCIF6;
}	


void DMA1_Channel7_IRQHandler()  
{
	DMA1_Channel7->CCR &= ~DMA_CCR7_EN;
	
	I2C1->CR1 |= I2C_CR1_STOP;
	DMA1->IFCR |= DMA_IFCR_CTCIF7;
	
	int T_tmp = 0;
	
	T_tmp = BUFF_RX[0];
	
	T_tmp <<= 1;
	BUFF_RX[1] >>= 7;
	
	T_tmp |= BUFF_RX[1];
	
	T_I2C = ((float)T_tmp) * 0.5;
	

	}

void Send_Addr(char ADD)
{
		I2C1->CR1 |= I2C_CR1_START;
 
		while(!(I2C1->SR1&I2C_SR1_SB)){;}  
  
		(void)I2C1->SR1;	//nado
			
		I2C1->DR = ADD;	
			
		while(((I2C1->SR1)&I2C_SR1_ADDR)==0){;}
		(void)I2C1->SR1; //nado
		(void)I2C1->SR2;
}

void Send_Byte(char byte)
{
	while(!(I2C1->SR1 && I2C_SR1_TXE)){;}
		I2C1->DR = byte;	
		//while(!(I2C_SR1_BTF && I2C1->SR1)){;}
	
}

void LM75A_Tos_Write(char Tos)
{
		while(I2C1->SR2&I2C_SR2_BUSY){;}
		Send_Addr(Addr_Transmite);
		
		BUFF_TX[0] = 	3;
		BUFF_TX[1] = 	Tos;	
		//BUFF_TX[2] = 	128;
		BUFF_TX[2] = 	0;	
		DMA_TX_GO(BUFF_TX, 3);
}

void LM75A_TH_Write(char TH)
{
		while(I2C1->SR2&I2C_SR2_BUSY){;}
		Send_Addr(Addr_Transmite);
		
		BUFF_TX[0] = 	2;
		BUFF_TX[1] = 	TH;
		BUFF_TX[2] = 	128;		
		DMA_TX_GO(BUFF_TX, 3);
}

LM75A_CONF_Write()
{
		while(I2C1->SR2&I2C_SR2_BUSY){;}
		Send_Addr(Addr_Transmite);
			
		BUFF_TX[0] = 	1;
		BUFF_TX[1] = 	CFG;
			
		DMA_TX_GO(BUFF_TX, 2);	
}

void LM75A_Init()
{
		LM75A_TH_Write(25);
		LM75A_Tos_Write(26);
}


void LM75A_SP_Write()
{
		while(I2C1->SR2&I2C_SR2_BUSY){;}
		Send_Addr(Addr_Transmite);
		
		BUFF_TX[0] = 	0;
		DMA_TX_GO(BUFF_TX, 1);
}


void LM75A_TH_READ()
{
		while(I2C1->SR2&I2C_SR2_BUSY){;}
		Send_Addr(Addr_Transmite);
		BUFF_TX[0] = 	2;
		//BUFF_TX[0] = 	100;
		DMA_TX_GO(BUFF_TX, 1);	
		
		I2C1->CR1 |= I2C_CR1_ACK;
		Send_Addr(Addr_Recive); //////	proverit'
		DMA_RX_GO(BUFF_RX, 1);
}
	

void LM75A_CONF_READ()
{
		while(I2C1->SR2&I2C_SR2_BUSY){;}
		Send_Addr(Addr_Transmite);
		BUFF_TX[0] = 	1;
		DMA_TX_GO(BUFF_TX, 1);	
		
		I2C1->CR1 |= I2C_CR1_ACK;
		Send_Addr(Addr_Recive);	
		DMA_RX_GO(BUFF_RX, 1);
}


void LM75A_Tos_READ()
{
		while(I2C1->SR2&I2C_SR2_BUSY){;}
		Send_Addr(Addr_Transmite);
		BUFF_TX[0] = 	3;
		DMA_TX_GO(BUFF_TX, 1);	
		
		I2C1->CR1 |= I2C_CR1_ACK;
		Send_Addr(Addr_Recive);	////// proverit'
		DMA_RX_GO(BUFF_RX, 2);
}
	
void LM75A_T_READ()
{
		I2C1->CR1 |= I2C_CR1_ACK;
		Send_Addr(Addr_Recive);	////// proverit'
		DMA_RX_GO(BUFF_RX, 2);
}
	

	
	
	
	
	