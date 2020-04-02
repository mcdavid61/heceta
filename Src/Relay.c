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

#define RELAY_TICK_INCREMENT 250

uint16_t relayPattern = 1;
_Bool	toggleFlag = TRUE;

void Relay_Process(void)
{
	static	uint32_t	relayTick=0;

	if (TRUE == toggleFlag)
	{
		if(uwTick > relayTick)
		{
			relayTick = uwTick + RELAY_TICK_INCREMENT;

			DRV8860_Update_Driver_Output(relayPattern);
			relayPattern <<= 1;
			if (relayPattern == 0)
			{
				relayPattern = 1;
			}
		}
	}
	DRV8860_Update_Driver_Output(relayPattern);

	return;
}

/*************************** END OF FILE **************************************/
