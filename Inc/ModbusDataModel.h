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
	HOLDING_REGISTER(00000,	"Polling",				ModbusDataModel_ReturnResetState_uint16_t,	NULL) \
	HOLDING_REGISTER(00010,	"Polling",				ModbusDataModel_ReturnResetState_uint16_t,	NULL) \
	HOLDING_REGISTER(40000,	"Polling",				ModbusDataModel_ReturnResetState_uint16_t,	NULL) \
	HOLDING_REGISTER(42100, "Parameter Unlock", 	NULL, NULL) \
	//	To be continued.



#define INPUT_REGISTER(addr, str, read, write) \
	case addr: \
		pReadFunction = read; \
		pWriteFunction = write; \
		break;
#define FOREACH_INPUT_REGISTER(INPUT_REGISTER) \
	INPUT_REGISTER(31100, "Offline Mode Status", 	NULL, NULL), \
	INPUT_REGISTER(31101, "Temperature (C)", 		NULL, NULL), \
	INPUT_REGISTER(31102, "Fault Code", 			NULL, NULL),
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

#endif /* MODBUSINTERFACE_H_ */
