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
	HOLDING_REGISTER(1100, "Module Disable",	 				NULL,	NULL) \
	HOLDING_REGISTER(1101, "Relay States Requested", 			Relay_Get_Relay,	Relay_Set_Relay) \
	HOLDING_REGISTER(1102, "Relay States Actual", 				Relay_Get_Relay,	NULL) \
	HOLDING_REGISTER(1103, "Fault Code HI",	 					NULL,	NULL) \
	HOLDING_REGISTER(1104, "Fault Code LO",	 					NULL,	NULL) \
	HOLDING_REGISTER(1105, "Relay Fault", 						NULL,	NULL) \
	HOLDING_REGISTER(1106, "Fault Flag", 						NULL,	NULL) \
	HOLDING_REGISTER(1107, "Supply Voltage", 					ADC_Get_Supply_Voltage,		NULL) \
	HOLDING_REGISTER(1108, "3.3V Reference Voltage", 			ADC_Get_3V3_Voltage,		NULL) \
	HOLDING_REGISTER(1109, "Vref Internal", 					ADC_Get_VrefInt_Voltage,	NULL) \
	HOLDING_REGISTER(1110, "Temperature (C)", 					ADC_Get_Temperature,		NULL) \
	HOLDING_REGISTER(2100, "Parameter Unlock", 					Configuration_GetParameterUnlockCode, 			Configuration_SetParameterUnlockCode) \
	HOLDING_REGISTER(2101,	"RS-485 Node Address",				Configuration_GetModbusAddress,					NULL) \
	HOLDING_REGISTER(2102,	"Baud Rate",						Configuration_IsBaudRate19200,					NULL) \
	HOLDING_REGISTER(2103,	"Stop Bits",						Configuration_GetStopBits,						NULL) \
	HOLDING_REGISTER(2104,	"Parity",							Configuration_GetParity,						NULL) \
	HOLDING_REGISTER(2105,	"Fault Relay Map",					Configuration_GetFaultRelayMap,					Configuration_SetFaultRelayMap) \
	HOLDING_REGISTER(2800,	"Manual Override Enable",			Configuration_GetManualOverrideEnabled,			Configuration_SetManualOverrideEnabled) \
	HOLDING_REGISTER(2801,	"Green LED State",					Configuration_GetGreenLED,						Configuration_SetGreenLED) \
	HOLDING_REGISTER(2802,	"Red LED State",					Configuration_GetRedLED,						Configuration_SetRedLED) \
	HOLDING_REGISTER(2803,	"Amber LED State",					Configuration_GetAmberLED,						Configuration_SetAmberLED) \

	//	To be continued.



#define INPUT_REGISTER(addr, str, read, write) \
	case addr: \
		pReadFunction = read; \
		pWriteFunction = write; \
		break;
#define FOREACH_INPUT_REGISTER(INPUT_REGISTER) \
	INPUT_REGISTER(1200, "Module Controller UID Char 1,2", 	Configuration_GetControllerUID_1_2, NULL) \
	INPUT_REGISTER(1201, "Module Controller UID Char 3,4", 	Configuration_GetControllerUID_3_4, NULL) \
	INPUT_REGISTER(1202, "Module Controller UID Char 5,6", 	Configuration_GetControllerUID_5_6, NULL) \
	INPUT_REGISTER(1203, "Module Controller UID Char 7,8", 	Configuration_GetControllerUID_7_8, NULL) \
	INPUT_REGISTER(1204, "Software Version Major", 	Configuration_GetMajorVersion, NULL) \
	INPUT_REGISTER(1205, "Software Version Minor", 	Configuration_GetMinorVersion, NULL) \
	INPUT_REGISTER(1206, "Software Version Build", 	Configuration_GetBuildVersion, NULL) \
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
