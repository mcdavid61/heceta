/*
 * Fault.h
 *
 *  Created on: Oct 6, 2020
 *      Author: BFS
 */

#ifndef FAULT_H_
#define FAULT_H_

#include <stdint.h>
#include <stdbool.h>

//	Configuration parameters

//	TODO:	This should eventually be pulled from the linker script.
//			I'm not as familiar with editing that, so for now, we'll hardcode the value.
#define FAULT_CRC_FLASH_SIZE_BYTES (1024 * 128)
#define FAULT_CRC_FLASH_SIZE_WORD (FAULT_CRC_FLASH_SIZE_BYTES / 4)
#define FAULT_CRC_DIVISOR (16)
#define FAULT_CRC_CALCULATE_RATE_MS (60000)


/*
	Enum:	Fault_T
	Description:
		The following enumerated data types reflect the bit
		that represents this type of fault.
*/
typedef enum
{
	FAULT_SYSTEM_FIRMWARE,
	FAULT_VOLTAGE_3_3V_OUT_OF_SPEC,
	FAULT_VOLTAGE_VIN_OUT_OF_SPEC,
	FAULT_RELAY,
	FAULT_MODBUS,
	FAULT_TEMPERATURE,
}	Fault_T;

void Fault_Set(Fault_T eFault, bool bActive);
void Fault_Activate(Fault_T eFault);
void Fault_Clear(Fault_T eFault);
bool Fault_Get(Fault_T eFault);
uint16_t Fault_GetAll(void);
bool Fault_OK(void);

//	CRC
void Fault_CRC_Process(void);

#endif /* FAULT_H_ */
