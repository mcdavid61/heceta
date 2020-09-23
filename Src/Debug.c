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

/*
	Function:	DEBUG_GPIO_INIT()
	Description:
		Depending on the macros defined, overwrites some of the
		GPIO configuration in such a way for different functionality.
*/
void DEBUG_GPIO_INIT(void)
{
#ifdef DEBUG_USE_J19_HEADER_AS_RELAY_OUTPUT
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, LED_GREEN_Pin|LED_AMBER_Pin|LED_RED_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : LED_GREEN_Pin LED_AMBER_Pin LED_RED_Pin */
	GPIO_InitStruct.Pin = LED_GREEN_Pin|LED_AMBER_Pin|LED_RED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pin : SPI3_CS_Pin */
	GPIO_InitStruct.Pin = SPI3_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(SPI3_CS_GPIO_Port, &GPIO_InitStruct);

	#endif
}
