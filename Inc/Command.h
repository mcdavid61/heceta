/*-----------------------------------------------------------------------------
 *  @file		Command.h
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
#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <stdbool.h>
#include "stm32l4xx_hal.h"

void Command_Process(void);
bool Command_Has_Comm_Timed_Out(void);
void Command_IncomingData(UART_HandleTypeDef *huart);
void Command_ClearInputBuffer(void);
bool Command_CollectRS232Input(char * const pBuff, uint32_t nBufferLen, uint32_t * pBufferPos);

#define SERIAL_INPUT_BUFFER_SIZE 128
#define SERIAL_OUTPUT_BUFFER_SIZE 256

#endif /* _COMMAND_H_ */

/*************************** END OF FILE **************************************/
