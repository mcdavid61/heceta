/*-----------------------------------------------------------------------------
Module Name: CRC.h

Copyright 2013 Bacharach Inc. as an unpublished work
All Rights Reserved.

The information cantained herein is confidential property of Bacharach Inc.
The use, copying, transfer or disclosure of such infromation is prohibited
except by express wirtten agreement with Company.

First Written on 12/30/13 by Chris Prozzo

Module Description: Prototypes for CRC.c


------------------------------------------------------------------------------*/
#ifndef CRC_H
#define CRC_H

#include <stdint.h>

//------------------------------------------------------------------------------
unsigned int CRC16(const unsigned char *puchMsg,unsigned int usDataLen);
uint16_t CRC_Fast_CRC16(uint16_t sum, uint32_t address, uint32_t len);

#endif // CRC_H
/*************************** END OF FILE **************************************/
