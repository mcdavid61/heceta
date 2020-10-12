/*
 * ModbusSlave.h
 *
 *  Created on: Aug 18, 2020
 *      Author: BFS
 */

#ifndef MODBUSSLAVE_H_
#define MODBUSSLAVE_H_

#include "ByteFIFO.h"
#include "stm32l4xx_hal.h"

#define MODBUS_SLAVE_TIMER htim2
#define MODBUS_SLAVE_COMMUNICATION_TIMEOUT_FAULT_MS (5 * 60000)

typedef enum
{
	MODBUS_EXCEPTION_OK 						= 0x00,
	MODBUS_EXCEPTION_ILLEGAL_FUNCTION 			= 0x01,
	MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS 		= 0x02,
	MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE 		= 0x03,
	MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE 		= 0x04,
	MODBUS_EXCEPTION_ACKNOWLEDGE 				= 0x05,
	MODBUS_EXCEPTION_SLAVE_DEVICE_BUSY			= 0x06,
	MODBUS_EXCEPTION_MEMORY_PARITY_ERROR		= 0x08,
	MODBUS_EXCEPTION_GATEWAY_PATH_UNAVAILABLE 	= 0x0A,
	MODBUS_EXCEPTION_GATEWAY_TARGET_FAILED_RESP	= 0x0B,
	MODBUS_EXCEPTION_UNKNOWN					= 0xFF,
}	ModbusException_T;

typedef struct
{
	uint8_t nByte;
	bool bContiguousDataTimeout;
	bool bIncomingMsgTimeout;
} ModbusByte_T;

void ModbusSlave_Init(void);
const FIFOControl_T * ModbusSlave_GetFIFO(void);
void ModbusSlave_Debug_StartTimer(void);
void ModbusSlave_Process(void);
void ModbusSlave_SetupTimerValues(TIM_HandleTypeDef * htim);
bool ModbusSlave_CheckCRC(const uint8_t * pBuffer, uint32_t nBufferLen);

#endif /* MODBUSSLAVE_H_ */
