/*
 * ModbusDataModel.h
 *
 *  Created on: Aug 28, 2020
 *      Author: BFS
 */

#ifndef MODBUSINTERFACE_H_
#define MODBUSDATAMODEL_H_

#include "ModbusSlave.h"

#define HOLDING_REGISTER(addr, str, read, write) \
	case addr: \
		pReadFunction = read; \
		pWriteFunction = write; \
		break;
#define FOREACH_HOLDING_REGISTER(HOLDING_REGISTER) \
	HOLDING_REGISTER(42100, "Parameter Unlock", 				Configuration_GetParameterUnlockCode, 			Configuration_SetParameterUnlockCode) \
	HOLDING_REGISTER(42101,	"RS-485 Node Address",				Configuration_GetModbusAddress,					NULL) \
	HOLDING_REGISTER(42102,	"Baud Rate",						Configuration_IsBaudRate19200,					NULL) \
	HOLDING_REGISTER(42103,	"Stop Bits",						Configuration_GetStopBits,						NULL) \
	HOLDING_REGISTER(42104,	"Parity",							Configuration_GetParity,						NULL) \
	HOLDING_REGISTER(42105,	"Fault Relay Map",					Configuration_GetFaultRelayMap,					Configuration_SetFaultRelayMap) \
	HOLDING_REGISTER(42800,	"Manual Override Enable",			Configuration_GetManualOverrideEnabled,			Configuration_SetManualOverrideEnabled) \
	HOLDING_REGISTER(42801,	"Green LED State",					Configuration_GetGreenLED,						Configuration_SetGreenLED) \
	HOLDING_REGISTER(42802,	"Red LED State",					Configuration_GetRedLED,						Configuration_SetRedLED) \
	HOLDING_REGISTER(42803,	"Amber LED State",					Configuration_GetAmberLED,						Configuration_SetAmberLED) \

	//	To be continued.



#define INPUT_REGISTER(addr, str, read, write) \
	case addr: \
		pReadFunction = read; \
		pWriteFunction = write; \
		break;
#define FOREACH_INPUT_REGISTER(INPUT_REGISTER) \
	INPUT_REGISTER(31100, "Offline Mode Status", 	ModbusDataModel_ReturnResetState_uint16_t, NULL) \
	INPUT_REGISTER(31101, "Temperature (C)", 		NULL, NULL) \
	INPUT_REGISTER(31102, "Fault Code", 			NULL, NULL)
	//	To be continued.

#define COIL(addr, str, read, write) \
	case addr: \
		pReadFunction = read; \
		pWriteFunction = write; \
		break;
#define FOREACH_COIL(COIL) \
	COIL(04100, "Restart", 				NULL, NULL) \
	COIL(04101, "Factory Reset", 		NULL, NULL) \
	COIL(04102, "Clear Last Faults", 	NULL, NULL) \
	//	To be continued.

bool ModbusDataModel_ReadCoil(uint16_t nAddress, bool * bReturn);
ModbusException_T ModbusDataModel_ReadHoldingRegister(uint16_t nAddress, uint16_t * nReturn);
ModbusException_T ModbusDataModel_ReadInputRegister(uint16_t nAddress, uint16_t * nReturn);
ModbusException_T ModbusDataModel_WriteHoldingRegister(uint16_t nAddress, uint16_t * nValue);

#endif /* MODBUSINTERFACE_H_ */
