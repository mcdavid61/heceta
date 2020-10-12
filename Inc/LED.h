/*-----------------------------------------------------------------------------
 *  @file		LED.h
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
#ifndef _LED_H_
#define _LED_H_

#include <stdbool.h>

/******************************************************************************
 * Prototypes
 */
void LED_Process(void);
bool LED_Startup_Test(void);
void LED_CommunicationUpdate(void);

#endif /* _LED_H_ */

/*************************** END OF FILE **************************************/
