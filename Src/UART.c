/*
 * UART.c
 *
 *  Created on: Aug 7, 2020
 *      Author: Constantino Flouras
 */

#include "stm32l4xx_hal.h"
#include "main.h"
#include "Command.h"

/*
	Function:	HAL_UART_RxCpltCallback()
	Description:
		Whenever data is received on the serial USART, go ahead and
		attempt to insert it into the appropriate FIFO.
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	//	Command UART
	if (Main_Get_Command_UART_Handle() == huart)
	{
		//	Accept the incoming data, if possible.
		//	Command_UART_RxCpltCallback();
	}

	//	RS485 UART
	if (Main_Get_Modbus_UART_Handle() == huart)
	{
		//	Modbus incoming data.
		//	TODO:	Figure out if this is ever used-- it shouldn't be.
		//	ModbusSlave_UART_RxCpltCallback();
	}
}

/*
	Function:	HAL_UART_ErrorCallback()
	Description:
		Overwrites a weak function.
		Allows us to have some sense of control over failures
*/
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	//	Command UART
	if (Main_Get_Command_UART_Handle() == huart)
	{
		//	Call the appropriate error callback.
		Command_UART_ErrorCallback();
	}
}
