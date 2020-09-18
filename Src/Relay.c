/*-----------------------------------------------------------------------------
 *  @file		Relay.c
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
 *  @brief
 *-------------------------------------------------------------------------------*/

#include "Relay.h"
#include "main.h"
#include "DRV8860.h"

#define RELAY_TICK_INCREMENT 500

uint16_t relayPattern = 0;
_Bool	toggleFlag = FALSE;
_Bool commRelay = FALSE;

/*
	Function:	Relay_Process()
	Description:
		The following is a new
*/



void Relay_Process(void)
{
	static	uint32_t	relayTick=0;

	if (TRUE == toggleFlag)
	{
		if(uwTick > relayTick)
		{
			relayTick = uwTick + RELAY_TICK_INCREMENT;
			if (TRUE == commRelay)
			{
				relayPattern |= 0x8000;
			}
			else
			{
				relayPattern &= 0x7FFF;
			}

			DRV8860_Update_Driver_Output(~relayPattern);
			relayPattern <<= 1;
			if (relayPattern == 0x8000)
			{
				relayPattern = 1;
			}
		}
	}
	else
	{
		DRV8860_Update_Driver_Output(~relayPattern);
	}

	return;
}

void Relay_Run_Demo()
{
	relayPattern = 1;
	toggleFlag = TRUE;
}

bool Relay_Set_Relay(uint16_t relay)
{
	//	Test commit
	relayPattern = relay;
	toggleFlag = FALSE;
	return true;
}

uint16_t Relay_Get_Relay(void)
{
	return relayPattern;
}

void Relay_Set_CommRelay(_Bool state)
{
	commRelay = state;
	return;
}
/*************************** END OF FILE **************************************/
