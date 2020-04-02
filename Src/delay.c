/*
 * delay.c
 *
 *  Created on: Feb 7, 2020
 *      Author: dmcmasters
 */

#include "delay.h"


#define USEC_LIMIT 1
#define MSEC_LIMIT 727
#define SEC_LIMIT 726745



void delay_us(uint32_t delay)
{
	for(uint32_t j = 0; j < delay; j++)
	{
		for(uint32_t i=0; i<USEC_LIMIT; i++);
	}
}

void delay_ms(uint32_t delay)
{
	for(uint32_t j = 0; j < delay; j++)
	{
		for(uint32_t i=0; i<MSEC_LIMIT; i++);
	}
}

void delay_sec(uint32_t delay)
{
	for(uint32_t j = 0; j < delay; j++)
	{
		for(uint32_t i=0; i<SEC_LIMIT; i++);
	}
}
