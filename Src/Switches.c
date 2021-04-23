/*
 * Switches.c
 *
 *  Created on: Apr 13, 2020
 *      Author: dmcmasters
 */
#include "main.h"
#include "Switches.h"
#include <stdint.h>

uint8_t	switches = 0;


uint8_t Switches_Read(void)
{

		switches = HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin);
		switches |= HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) << 1;
		switches |= HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin) << 2;
		switches |= HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) << 3;
		switches |= HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin) << 4;
		switches |= HAL_GPIO_ReadPin(SW6_GPIO_Port, SW6_Pin) << 5;
		switches |= HAL_GPIO_ReadPin(SW7_GPIO_Port, SW7_Pin) << 6;
		switches |= HAL_GPIO_ReadPin(SW8_GPIO_Port, SW8_Pin) << 7;

		return switches;
}
