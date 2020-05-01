/*-----------------------------------------------------------------------------
 *  @file		LED.c
 *
 *  @copyright	2020, Bacharach Inc. as an unpublished work
 *              All Rights Reserved.
 *
 * 	The information contained herein is confidential property of Bacharach Inc.
 *  The use, copying, transfer or disclosure of such information is prohibited
 *  except by express written agreement with Bacharach Inc.
 *
 *  @date		Feb 13, 2020
 *  @author 	dmcmasters
 *
 *  @brief		Contains drivers and functions for handling the Nucleo board Green
 *              LED
 *-------------------------------------------------------------------------------*/

#include "LED.h"
#include "main.h"

#define LED_TICK_INCREMENT	100

void LED_Process(void)
{
	static uint32_t ledTick=0;
	static uint8_t led = 0;

	if (uwTick > ledTick)
	{
		ledTick = uwTick + LED_TICK_INCREMENT;
		switch (led)
		{
		case 0:
			HAL_GPIO_TogglePin(LED_AMBER_GPIO_Port, LED_AMBER_Pin);
			led++;
			break;

		case 1:
			HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
			led++;
			break;

		default:
			led = 0;
			break;
		}
	}
	return;
}

/*************************** END OF FILE **************************************/
