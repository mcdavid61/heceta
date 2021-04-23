/*
 * UUID.c
 *
 *  Created on: Jul 17, 2020
 *      Author: dmcmasters
 */

#include "UUID.h"
#include "main.h"

/*
 * Function to return 32-bit unique ID based on the 96-bit unique ID of the STM32L431 part.
 * Run a CRC32 on the 3 long words of the ID to generate a 32-bit value.
 *
 */

uint32_t UUID_Get_ID(void)
{
	CRC_HandleTypeDef* crc = Main_Get_CRC_Handle();

	return( HAL_CRC_Calculate(crc, STM32_UUID, 3));
}
