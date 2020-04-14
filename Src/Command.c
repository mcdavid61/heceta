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
#include "Switches.h"

extern uint16_t ADC_24V_Mon;
extern uint16_t ADC_3V3_Mon;
extern uint16_t ADC_Temperature;

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

		case 'v':
		case 'V':
			printf("\n\r24V = %d mV, 3.3V = %d mV, Temp. = %d °C\n\r", ADC_24V_Mon, ADC_3V3_Mon, ADC_Temperature);
			break;

		case 's':
		case 'S':
			printf("\n\rSwitches = %2X", Switches_Read());
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
		default:
			printf("\n\rR - Run demo\n\r");
			printf("S - Read switches\n\r");
			printf("[ - All off.\n\r");
			printf("] - All on.\n\r");
			printf("1-9, a-g - Individual Relay");
			printf("? - Help\n\r");
			break;
		}

		printf("\n\r> ");

	}

}

/*************************** END OF FILE **************************************/
