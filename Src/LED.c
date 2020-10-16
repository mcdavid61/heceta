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
#include "Fault.h"


#define LED_COMM_MZ_TIMEOUT	5000
#define LED_TICK_INCREMENT	500
#define LED_COMM_TIME 250
#define LED_COMM_FLASH 75

//	Timestamp for when the last Modbus data was sent out.
//	This replaces direct control of the LED from the byte
//	output handler and keeps it within this module.
static uint32_t m_nCommunicationTimestamp = (uint32_t) (0-LED_COMM_TIME);

//	Boolean, indicates startup tasks complete
static bool m_bStartupTasksComplete;


/*
	Function: LED_CommunicationTimestampUpdate()
	Description:
		Indicate that there is a communication update, which should
		be reflected in the LEDs.
 */
void LED_CommunicationUpdate(void)
{
	m_nCommunicationTimestamp = uwTick;
}


void LED_Process(void)
{
	//	Note that the LED_Process() function can be disabled or effectively
	//	stubbed out due to, perhaps, a debug function requiring use of the
	//	pins of which the LEDs are connected to.
	#ifndef DEBUG_DISABLE_LEDS

	//	Determine how the LEDs are currently being controlled.
	if (Configuration_GetManualOverrideEnabled())
	{
		//	If the Manual Override flag is enabled, allow the Modbus
		//	interface to control the LEDs individually.
		HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, Configuration_GetGreenLED() ? GPIO_PIN_SET : GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, Configuration_GetRedLED() ? GPIO_PIN_SET : GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_AMBER_GPIO_Port, LED_AMBER_Pin, Configuration_GetAmberLED() ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
	else
	{
		//	We're simply running the system as normal, and the LEDs reflect
		//	the statees of which they should normally reflect:
		//		Green LED solid --> power
		//		Green LED flashing --> power and communications
		//		Amber LED flashing --> heartbeat
		//		Red LED solid --> loss of communications after 5 seconds

		//
		static uint32_t nAmberHeartbeatTimestamp;
		static uint32_t nGreenToggleTimestamp;

		//	Amber LED
		//	Toggles every LED_TICK_INCREMENT milliseconds to indicate
		//	that the system code is still running.
		if (uwTick - nAmberHeartbeatTimestamp > LED_TICK_INCREMENT)
		{
			nAmberHeartbeatTimestamp = uwTick;
			HAL_GPIO_TogglePin(LED_AMBER_GPIO_Port, LED_AMBER_Pin);
		}

		//	Green LED
		//	If communication has happened within the last second, flashes
		//	at a toggle rate of LED_COMM_FLASH.
		if (uwTick - m_nCommunicationTimestamp < LED_COMM_TIME)
		{
			//	There has been communication in the last 1000 milliseconds.
			//	Flash the LED at a constant rate.
			if (uwTick - nGreenToggleTimestamp > LED_COMM_FLASH)
			{
				nGreenToggleTimestamp = uwTick;
				HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
			}
		}
		else
		{
			//	No communication.
			//	If there's a pending toggle of the LED to the "ON" state, let that complete
			//  so that we're naturally left "ON".
			if (uwTick - nGreenToggleTimestamp > LED_COMM_FLASH)
			{
				HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
			}

		}

		//	Configure the red LED, based on how long it has been since we've heard
		//	from the MZ module.
		if (uwTick - m_nCommunicationTimestamp > LED_COMM_MZ_TIMEOUT)
		{
			//	We've lost communication with the MZ. Go ahead and activate the LED.
			HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);

			//	Additionally, update the m_nCommunicationTimestamp to reflect this "past state"
			//	Set the communication clock to the past-- this helps avoid overflow if for whatever
			//	reason we lose communication for a long period of time.
			m_nCommunicationTimestamp = (uwTick - LED_COMM_MZ_TIMEOUT - 1);
		}
		else
		{
			//	Communication has been restored. Go ahead and clear this.
			HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
		}

		/*
		if (Command_Has_Comm_Timed_Out())
		{
			HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
		}
		else
		{
			HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
		}
		*/
	}
	#endif

}


typedef enum
{
	LED_STARTUP_TEST_INIT,
	LED_STARTUP_TEST_GREEN,
	LED_STARTUP_TEST_AMBER,
	LED_STARTUP_TEST_RED,
	LED_STARTUP_TEST_COMPLETE,
}	LED_Startup_Test_State_T;

#define LED_STARTUP_TEST_GAPTIME (500)

static LED_Startup_Test_State_T m_eStartupTestState = LED_STARTUP_TEST_INIT;

void LED_Startup_Process(void)
{
	static uint32_t nTimer = 0;

	bool bGreen = GPIO_PIN_RESET;
	bool bAmber = GPIO_PIN_RESET;
	bool bRed = GPIO_PIN_RESET;

	switch(m_eStartupTestState)
	{
		case LED_STARTUP_TEST_INIT:
			nTimer = uwTick;
			m_eStartupTestState = LED_STARTUP_TEST_GREEN;
			break;
		case LED_STARTUP_TEST_COMPLETE:
		case LED_STARTUP_TEST_GREEN:
			bGreen = GPIO_PIN_SET;
			if ((uwTick - nTimer) > LED_STARTUP_TEST_GAPTIME)
			{
				nTimer = uwTick;
				m_eStartupTestState = LED_STARTUP_TEST_AMBER;
			}
			break;
		case LED_STARTUP_TEST_AMBER:
			bAmber = GPIO_PIN_SET;
			if ((uwTick - nTimer) > LED_STARTUP_TEST_GAPTIME)
			{
				nTimer = uwTick;
				m_eStartupTestState = LED_STARTUP_TEST_RED;
			}
			break;
		case LED_STARTUP_TEST_RED:
			bRed = GPIO_PIN_SET;
			if ((uwTick - nTimer) > LED_STARTUP_TEST_GAPTIME)
			{
				nTimer = uwTick;
				m_eStartupTestState = LED_STARTUP_TEST_COMPLETE;
				m_bStartupTasksComplete = true;
			}
			break;
		default:
			break;
	}

	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 	bGreen);
	HAL_GPIO_WritePin(LED_AMBER_GPIO_Port, LED_AMBER_Pin, 	bAmber);
	HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, 		bRed);
}

bool LED_StartupTasksComplete()
{
	return m_bStartupTasksComplete;
}

/*************************** END OF FILE **************************************/
