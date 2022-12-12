
#include "stm32f10x.h"
#include "my_lib.h"
	extern		float BUFFER_TX[];
	extern		float BUFFER_RX[];
	
#define CS_SET() SET_BIT(GPIOB->ODR,GPIO_ODR_ODR5)			// SET 1 to CS PIN
#define CS_RESET() CLEAR_BIT(GPIOB->ODR,GPIO_ODR_ODR5)  // SET 0 to CS PIN

#define BUFFER_SIZE 					 		16							// max chunk for writing supported by 25LC640 EEPROM	(in bytes)						
#define TIMEOUT 					 		200000							// timeout for SPI in mcs


// commands for 25LC640 EEPROM
#define READ									 0x03								// Read data from memory
#define WRITE									 0x02								// Write data to memory
#define WREN									 0x06								// Set the write enable latch
#define WRDI									 0x04								// Reset the write enable latch
#define RDSR									 0x05								// Read Status register
#define WRSR									 0x01								// Write Status register

#define READ_TEST									0								// for EEPROM read test after reset


volatile uint32_t msTicks;                                 // counts 1ms timeTicks

/*----------------------------------------------------------------------------
 * SysTick_Handler:
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void) {
  msTicks++;
}

/*----------------------------------------------------------------------------
 * Delay: delays a number of Systicks
 *----------------------------------------------------------------------------*/
void DelayMicro (uint32_t dlyTicks) {
  SysTick->LOAD = dlyTicks * (SystemCoreClock/1000000);
		SysTick->VAL = 10;
		SysTick->CTRL |= (SysTick_CTRL_ENABLE |SysTick_CTRL_TICKINT| SysTick_CTRL_CLKSOURCE);

		while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG));
		SysTick->CTRL &= ~(SysTick_CTRL_COUNTFLAG | SysTick_CTRL_ENABLE);
}

uint32_t GetTicks () {
	return msTicks;
}

void DelayMicros (int us)
{		
		SysTick->LOAD = us * (SystemCoreClock/1000000);
		SysTick->VAL = 10;
		SysTick->CTRL |= (SysTick_CTRL_ENABLE |SysTick_CTRL_TICKINT| SysTick_CTRL_CLKSOURCE);

		while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG));
		SysTick->CTRL &= ~(SysTick_CTRL_COUNTFLAG | SysTick_CTRL_ENABLE);
}


void SPI1_Init () {
	
			uint32_t tmpreg;
			RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;  				 								// APB2 clock enabling for alternative function
			
			RCC->APB2ENR |=  RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN; 	// Configure Chip Select PIN (PB6 - CS) 
		
			// SPI1 pins PB6 - CS
			GPIOB->CRL &= ~(GPIO_CRL_CNF5 | GPIO_CRL_MODE5); 					 // Clear CNF and MODE bits for PIN 6
			GPIOB->CRL |= GPIO_CRL_MODE5_0;														 // Output mode, max freq = 10 MHz, GP push-pull
		  
			//SPI1 pins configure (PA5 - SCK ,PA6 - MISO ,PA7 - MOSI)
			GPIOA->CRL &= ~(GPIO_CRL_CNF5 | GPIO_CRL_CNF6 | GPIO_CRL_CNF7);  							// Clear CNF and MODE bits for PIN 6
			// PIN 5(SCK) and PIN7(MOSI) - output mode, 50 MHZ alternative function pus-pull
			// PIN 6(MISO) - input mode, floating input
			GPIOA->CRL |= GPIO_CRL_CNF5_1 | GPIO_CRL_CNF6_0 | GPIO_CRL_CNF7_1 | GPIO_CRL_MODE5 | GPIO_CRL_MODE7;
			
			RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; 											// APB2 clock enabling for SPI1
			
			 // clear bits in CR1
			SPI1->CR1 &= 	~(SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE | SPI_CR1_CRCEN | SPI_CR1_CRCNEXT | \ 
                    SPI_CR1_DFF | SPI_CR1_RXONLY | SPI_CR1_LSBFIRST | \
                    SPI_CR1_BR | SPI_CR1_CPOL | SPI_CR1_CPHA);																	
			// SPI enable, master mode, prescaler = 32 (100), F(SPI) = Fpclk / 32 = 32/32 MHz = 1 MHz
			SPI1->CR1 |= SPI_CR1_SPE | SPI_CR1_BR_2 | SPI_CR1_MSTR;
			
			SPI1->CR2 &= ~0x00FF; 																			// clear CR reg
}

uint8_t SPI_Transmit (uint8_t *src, uint32_t size, uint32_t timeout) {
			uint32_t tickstart;			
			uint32_t TXCnt = size;													// bytes Tx counter

			tickstart = GetTicks () ;												// Get current Ticks count

			if ((SPI1->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE)   // Check if the SPI is already enabled 
			{
				SET_BIT(SPI1->CR1, SPI_CR1_SPE); 							// IF not enabled then Enable SPI peripheral 
			}
			
			while (TXCnt > 0U)															// while we have bytes to transmit 
			{
				if (SPI1->SR & SPI_SR_TXE) 										// Wait until TXE flag is set to send data (TX buffer is empty)
				{
					SPI1->DR = *src;														// send byte from src via SPI
					src++;																			// increment pointer to next byte
					TXCnt--;																		// decrement bytes counter
				}
				else
				{
					 if ((GetTicks ()  - tickstart) > timeout)	// if TX buffer not empty during timeout then return 1
						return 1;
				}
			}

			while ((SPI1->SR & SPI_SR_BSY));							 // check if BUSY flag is set and continue                                                  		
			// Clear the SPI overrun flag in 2 lines mode
			(void)SPI1->DR;  															// read DR to clear overrun flag        
}

uint8_t SPI_TransmitReceive(uint8_t *src, uint8_t *dst, uint32_t size, uint32_t timeout)
{
   uint32_t TXCnt = size;																				// bytes Tx counter
   uint32_t RXCnt = TXCnt;																			// bytes Rx counter
   uint32_t TXAllowed = 1;																			// allow Tx flag
	 uint32_t curTick = 0;
   
   uint32_t start = GetTicks () ;
		
   while (TXCnt || RXCnt) {																			//  while we have bytes to transmit and receive
      if (TXAllowed && (SPI1->SR & SPI_SR_TXE) && TXCnt) {			// chek conditions to transmit
         SPI1->DR = *src;																				// send byte from src via SPI
				 src++;																									// increment pointer to next byte
         TXCnt--;																								// decrement bytes counter
         TXAllowed = 0;																					// next data is a reception (Rx). Tx not allowed 
      }
      
      if  ((SPI1->SR & SPI_SR_RXNE) && RXCnt) {									// if Rx buffer not empty then read byte from DR
				 *dst = SPI1->DR;																				// read data from DR to our buffer
				 dst++;																									// increment data pointer
         RXCnt--;																								// decrement bytes counter
         TXAllowed = 1; 																				// allow Tx for next byte
      }
      
      curTick = GetTicks () ;
			if ((curTick - start) > timeout) 													// return 1 if condidition above did not set during timeout
         return 1;
   }
 
}

void WriteEnableOrDisable(unsigned char Enable){
		unsigned char SendOneByte=0;
		CS_RESET();																									// set CS to zero
		if(Enable==1)
		{
				SendOneByte=WREN; 																			// write enable command 
		}
		else
		{
				SendOneByte=WRDI; 																			// write disable command 
		}
		SPI_Transmit(&SendOneByte, 1, TIMEOUT) ;										// send command to eeprom
		CS_SET(); 																									// set CS to one
} 

void WriteData (uint16_t AddresOfData, void *WriteArrayOfEEProm,uint32_t SizeOfArray) {
			unsigned char SendOneByte;
			WriteEnableOrDisable(1);																	// send write enable command
			CS_RESET();																								// set CS to zero
			DelayMicro(1000);
			SendOneByte=WRITE;																				// write data command
			SPI_Transmit(&SendOneByte, 1, TIMEOUT); 									// send write data command
			 
			SendOneByte= AddresOfData>>8;															// get high byte of eeprom address
			SPI_Transmit(&SendOneByte, 1, TIMEOUT); 									// send high byte of eeprom address
			SendOneByte= AddresOfData & 0x00FF; 											// get low byte of eeprom address
			SPI_Transmit(&SendOneByte, 1, TIMEOUT); 									// send low byte of eeprom address
			
			SPI_Transmit(WriteArrayOfEEProm, SizeOfArray, TIMEOUT);		// send data to eeprom
			DelayMicro(4000);
			CS_SET(); 																								// set CS to one
}

void ReadData (uint16_t AddresOfData, void *DataArrayOfEEProm,uint32_t SizeOfArray) {
		unsigned char SendOneByte;
		CS_RESET();
		DelayMicro(1000);
		SendOneByte=READ; 																					// read data command
		SPI_Transmit(&SendOneByte, 1, TIMEOUT);											// send read data command

		SendOneByte= AddresOfData>>8; 															// get high byte of eeprom address
		SPI_Transmit(&SendOneByte, 1, TIMEOUT); 										// send high byte of eeprom address
		SendOneByte= AddresOfData & 0x00FF; 												// get low byte of eeprom address
		SPI_Transmit(&SendOneByte, 1, TIMEOUT); 										// send low byte of eeprom address

		SPI_TransmitReceive(DataArrayOfEEProm, DataArrayOfEEProm, SizeOfArray, TIMEOUT); // receive data from eeprom

		CS_SET(); 																									// set CS to one
		DelayMicro(1000);
}


