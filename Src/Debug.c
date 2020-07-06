/*
 * Debug.c
 *
 *  Created on: Jun 29, 2020
 *      Author: dmcmasters
 */

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

uint16_t Debug_Write(char *ptr, uint16_t len)
{
	uint16_t i=0;
	for(i=0;i<len;i++)
	{
		ITM_SendChar((*ptr++));
	}
	return len;
}
