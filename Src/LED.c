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
#include "Configuration.h"



#define LED_TICK_INCREMENT	500

void LED_Process(void)
{
	//	Note that the LED_Process() function can be disabled or effectively
	//	stubbed out due to, perhaps, a debug function requiring use of the
	//	pins of which the LEDs are connected to.
	#ifndef DEBUG_DISABLE_LEDS

	//	There are two configurations for the LEDs
	//	-	Regular Mode
	//			Aka, this is the mode that the system normally runs in.
	//	-	Manual Mode
	//			Override from the MODBUS communication / configuration struct.
	//			This occurs whenever the bEnable flag within the ManualOutputConfiguration_T struct
	//			is enabled.
	if (Configuration_GetManualOverrideEnabled())
	{
		HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, Configuration_GetGreenLED() ? GPIO_PIN_SET : GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, Configuration_GetRedLED() ? GPIO_PIN_SET : GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_AMBER_GPIO_Port, LED_AMBER_Pin, Configuration_GetAmberLED() ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
	else
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
	}
	#endif

}

/*************************** END OF FILE **************************************/
