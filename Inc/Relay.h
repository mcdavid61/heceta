/*-----------------------------------------------------------------------------
 *  @file		Relay.h
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
#ifndef _RELAY_H_
#define _RELAY_H_

#include <stdint.h>
#include <stdbool.h>

#define DRV8860_CNT (2)

bool Relay_Set(uint16_t nPattern);
void Relay_Process(void);
uint16_t Relay_Get(void);
void Relay_Run_Demo();
void Relay_Set_CommRelay(_Bool state);



#endif /* _RELAY_H_ */

/*************************** END OF FILE **************************************/
