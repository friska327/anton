#include "stm32f10x.h"
#include "my_lib.h"
#include "ds18b20.h"
#include <math.h >

void alarm_search()
{
	char i = 0;
	char first_B = 0;
	char second_B = 0;
	
		initial_ds18b20 ();	
		send_Byte(ALARM_SEARCH);
		
	
		first_B  = recive_Bit();
		second_B = recive_Bit();
	
	
	if(first_B == 1 && second_B == 1 )
	{
		TIM4 -> CR1 &= ~TIM_CR1_CEN;
		GPIOA ->BSRR |= GPIO_BSRR_BS5;
	}
	else
	{
		TIM4 -> CR1 |= TIM_CR1_CEN;
	}
	
}


char READ_ROM_DS(struct ID_ds18b20 *ID_X)
{
	
	char i = 0;
	ID_X[0].disput_bit = 0;
	ID_X[1].disput_bit = 1;
	
	while(i < 2)
	{
	back:
	do{
		ID_X[i].num = 0;
		initial_ds18b20 ();	
		send_Byte(SEARCH_ROM);
		}while(Tree_search(&ID_X[i]) );
		
		if(Compute_CRC8(Tree_search(ID_X[i].ID), 7))
		{
			goto back;
		}
		
		i++;
	}
}

char Tree_search(struct ID_ds18b20 *pt)
{

		char first_B = 0;
		char second_B = 0;
		
		while(pt->num < 8)
	{
	for	(int count = 0; count < 8; count++)
		{
	first_B  = recive_Bit();
	second_B = recive_Bit();
	pt->ID[pt->num] = pt->ID[pt->num] >> 1;
			
	if(first_B == 0 && second_B == 1)
	{
		pt->ID[pt->num] |= 0;
		send_Bit(0);
	}
	if(first_B == 1 && second_B == 0)
	{
		pt->ID[pt->num] |= 128;
		send_Bit(1);
	}
	
	if(first_B == 0 && second_B == 0)
	{
		
		if(pt->disput_bit == 1)
		{
			pt->ID[pt->num] |= 128;
		}
		else
		{
			pt->ID[pt->num] |= 0;
		}
		send_Bit(pt->disput_bit);
	}
	
	if(first_B == 1 && second_B == 1 )
	{
		pt->disput_bit ^= 1;
		return 1;
	}
	}
	pt->num++;	
	}
	
	return 0;
}


/*
char READ_ROM_DS_1(struct ID_ds18b20 *ID1)
{	
	initial_ds18b20 ();
	send_Byte(READ_ROM);
	for(int i = 0; i < 8; i++)
	{
		ID1->ID[i] = read_Byte(); 
	}	
}
*/


void MATCH_R(char *addr)
{
	
	send_Byte(MATCH_ROM);
	
	for(int i = 0; i < 8; i++)
	{
		send_Byte(addr[i]); 
	}	
}


void read_scr(struct ID_ds18b20 *ID_X)
{
initial_ds18b20 ();
MATCH_R(ID_X->ID);
	
send_Byte(Read_Scratchpad);

		ID_X->MSB = read_Byte(); 
		ID_X->LSB = read_Byte();
		ID_X->TH = read_Byte();
		ID_X->TL = read_Byte();
		ID_X->ctrl = read_Byte();
}


void write_scr(struct ID_ds18b20 *ID_X)
{
initial_ds18b20 ();
MATCH_R(ID_X->ID);	

send_Byte(Write_Scratchpad);
send_Byte(ID_X->TH);	
send_Byte(ID_X->TL);	
send_Byte(ID_X->ctrl);
}


void copy_scr(struct ID_ds18b20 *ID_X)
{
initial_ds18b20 ();	
	
MATCH_R(ID_X->ID);
send_Byte(Copy_Scratchpad);
DELAY_us(100);
	
}




void convert() //broadcast mess
{
	initial_ds18b20 ();
	send_Byte(SKIP_ROM);		
	send_Byte(CONVERT_T);
}

float read_T(char *addr)
{
		
	initial_ds18b20 ();
	MATCH_R(addr);
	//send_Byte(SKIP_ROM);
	send_Byte(Read_Scratchpad);
	
	
	float T = 0;
	char lsb = 0, msb = 0;
	char tmp = 0;
	int n = -5;
	
		
	lsb = read_Byte();
	msb = read_Byte();
	
	
	while(tmp++ < 8)
	{	
		n++;
		if(lsb & 0x01)
		{
			
				if(n < 0)
				{
						T += 1/pow(2, abs(n));
				}
				else
				{
					T += pow(2, abs(n));
				}
		}
	lsb = lsb >> 1;
	}
	
	tmp = 0;
	while(tmp++ < 3)
	{
		n++;
		if(msb & 0x01)
		{
				T += pow(2, n);
		}
		
	msb = msb >> 1;
	}
		
	return T;
}


char initial_ds18b20 ()
{	
	GPIOA ->BSRR |= GPIO_BSRR_BR1;
	DELAY_us(650);
	GPIOA ->BSRR |= GPIO_BSRR_BS1;
	DELAY_us(100);
	if (GPIOA->IDR & GPIO_IDR_IDR1)
	{	
		return 0;//not okey
	}
	else
	{	
		DELAY_us(2000);
		return 1;//okey
	}
}


void send_Byte (char tmp)
{
	char num = 0;
		while (num++ < 8)
	{
		GPIOA ->BSRR |= GPIO_BSRR_BR1;
		DELAY_us(5);
		
		if (tmp & 0x01)
		{
		GPIOA ->BSRR |= GPIO_BSRR_BS1;
		DELAY_us(70);
		}
		else
		{
		DELAY_us(70);
		GPIOA ->BSRR |= GPIO_BSRR_BS1;
		}
		DELAY_us(9);
		tmp = tmp >> 1;
	}

}
char read_Byte ()
{
	char tmp = 0;
	char num = 0;
		while (num++ < 8)
	{
		
		tmp = tmp >> 1;
		GPIOA ->BSRR |= GPIO_BSRR_BR1;
		DELAY_us(2);
		GPIOA ->BSRR |= GPIO_BSRR_BS1;
		DELAY_us(1);
		
		if(GPIOA ->IDR & GPIO_IDR_IDR1)
		{
		tmp += 128;
		}	
		DELAY_us(70);
	}
return tmp;
}

void send_Bit(char tmp)
{
		GPIOA ->BSRR |= GPIO_BSRR_BR1;
		DELAY_us(5);
		
		if (tmp == 1)
		{
		GPIOA ->BSRR |= GPIO_BSRR_BS1;
		DELAY_us(70);
		}
		else
		{
		DELAY_us(70);
		GPIOA ->BSRR |= GPIO_BSRR_BS1;
		}
		
		DELAY_us(9);
}

char recive_Bit()
{
		char tmp = 0;
		GPIOA ->BSRR |= GPIO_BSRR_BR1;
		DELAY_us(2);
		GPIOA ->BSRR |= GPIO_BSRR_BS1;
		DELAY_us(2);
		
		if(GPIOA ->IDR & GPIO_IDR_IDR1)
		{
		tmp = 1;
		}	
		DELAY_us(70);
		
		return tmp;
}



char Compute_CRC8 (uint8_t* data, uint8_t length) {
		uint8_t polynomial = 0x8C, crc = 0x0, i = 0, j = 0, lsb = 0, inbyte = 0;
		while (length--) {
				inbyte = data[j];
        for (i = 0; i < 8; i++) {
            lsb = (crc ^ inbyte) & 128;
            crc >>= 1;
            if (lsb) 
							crc ^= polynomial;
            inbyte <<= 1;
        }
				j++;
		}
		return crc; 
}



