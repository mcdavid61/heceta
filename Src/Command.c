/*-----------------------------------------------------------------------------
 *  @file		Command.c
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

#include "Command.h"
#include "main.h"
#include "Relay.h"

void Command_Process(void)
{
	uint8_t cmd;
	UART_HandleTypeDef* uart = Main_Get_UART_Handle();

	if(HAL_TIMEOUT != HAL_UART_Receive(uart, &cmd, 1, 1))
	{
		printf("%c", cmd);

		switch(cmd)
		{
		case 'r':
		case 'R':
			Relay_Run_Demo();
			break;

		case '[':
			Relay_Set_Relay(0);
			break;

		case ']':
			Relay_Set_Relay(0xFFFF);
			break;

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			Relay_Set_Relay(1 << (cmd - '1'));
			break;

		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
			Relay_Set_Relay(1 << (cmd - 'a' + 9));
			break;
		case '?':
			printf("\n\rNo help at this time!\n\r");
			break;

		default:
			break;
		}

		printf("\n\r> ");

	}

}

/*************************** END OF FILE **************************************/
