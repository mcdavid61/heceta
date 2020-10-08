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
}	Fault_T;

void Fault_Set(Fault_T eFault, bool bActive);
void Fault_Activate(Fault_T eFault);
void Fault_Clear(Fault_T eFault);
bool Fault_Get(Fault_T eFault);
uint16_t Fault_GetAll(void);
bool Fault_OK(void);

#endif /* FAULT_H_ */
