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
#include "Command.h"

#define LED_TICK_INCREMENT	500

void LED_Process(void)
{
	static uint32_t ledTick=0;

	if (uwTick > ledTick)
	{
		ledTick = uwTick + LED_TICK_INCREMENT;
		HAL_GPIO_TogglePin(LED_AMBER_GPIO_Port, LED_AMBER_Pin);
	}

	if (Command_Has_Comm_Timed_Out())
	{
		HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
	}
	return;
}

/*************************** END OF FILE **************************************/
