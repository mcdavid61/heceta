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
    pReadFunction  = read; \
    pWriteFunction = write; \
    break;

#define FOREACH_HOLDING_REGISTER(HOLDING_REGISTER) \
  HOLDING_REGISTER(1101,  "Relay States Requested", Relay_Get,                              Relay_Request) \
  HOLDING_REGISTER(1102,  "Relay States Actual",    Relay_Get,                              NULL) \
  HOLDING_REGISTER(1103,  "Relay Fault",            Relay_GetFaulted,                       NULL) \
  HOLDING_REGISTER(1104,  "Fault Flag",             Fault_NotOK,                            NULL) \
  HOLDING_REGISTER(1105,  "Fault Code",             Fault_GetAll,                           NULL) \
  HOLDING_REGISTER(1106,  "Supply Voltage",         ADC_Get_Supply_Voltage,                 NULL) \
  HOLDING_REGISTER(1107,  "3.3V Reference Voltage", ADC_Get_3V3_Voltage,                    NULL) \
  HOLDING_REGISTER(1108,  "Temperature (C)",        ADC_Get_Temperature,                    NULL) \
  HOLDING_REGISTER(2100,  "Parameter Unlock",       Configuration_GetParameterUnlockCode,   Configuration_SetParameterUnlockCode) \
  HOLDING_REGISTER(2101,  "RS-485 Node Address",    Configuration_GetModbusAddress,         NULL) \
  HOLDING_REGISTER(2102,  "Baud Rate",              Configuration_IsBaudRate19200,          NULL) \
  HOLDING_REGISTER(2103,  "Stop Bits",              Configuration_GetStopBits,              NULL) \
  HOLDING_REGISTER(2104,  "Parity",                 Configuration_GetParity,                NULL) \
  HOLDING_REGISTER(2105,  "Fault Relay Map",        Configuration_GetFaultRelayMap,         Configuration_SetFaultRelayMap) \
  HOLDING_REGISTER(2106,  "Failsafe Relay Enable",  Configuration_GetFailsafeRelayEnable,   Configuration_SetFailsafeRelayEnable) \
  HOLDING_REGISTER(2800,  "Manual Override Enable", Configuration_GetManualOverrideEnabled, Configuration_SetManualOverrideEnabled) \
  HOLDING_REGISTER(2801,  "Green LED State",        Configuration_GetGreenLED,              Configuration_SetGreenLED) \
  HOLDING_REGISTER(2802,  "Red LED State",          Configuration_GetRedLED,                Configuration_SetRedLED) \
  HOLDING_REGISTER(2803,  "Amber LED State",        Configuration_GetAmberLED,              Configuration_SetAmberLED) \
  HOLDING_REGISTER(2804,  "Switches",               Configuration_GetSwitches,              NULL) \
  HOLDING_REGISTER(4100,  "Restart",                Configuration_GetRestart,               Configuration_SetRestart) \
  HOLDING_REGISTER(4101,  "Factory Reset",          Configuration_GetFactoryReset,          Configuration_SetFactoryReset) \

// To be continued.

#define INPUT_REGISTER(addr, str, read, write) \
  case addr: \
    pReadFunction  = read; \
    pWriteFunction = write; \
    break;
#define FOREACH_INPUT_REGISTER(INPUT_REGISTER) \
  INPUT_REGISTER(1200, "Software Version Major",  Configuration_GetMajorVersion, NULL) \
  INPUT_REGISTER(1201, "Software Version Minor",  Configuration_GetMinorVersion, NULL) \
  INPUT_REGISTER(1202, "Software Version Build",  Configuration_GetBuildVersion, NULL) \
  // To be continued.

#define COIL(addr, str, read, write) \
  case addr: \
    pReadFunction  = read; \
    pWriteFunction = write; \
    break;
#define FOREACH_COIL(COIL) \
  COIL(04100, "Restart",        NULL, NULL) \
  COIL(04101, "Factory Reset",    NULL, NULL) \
  COIL(04102, "Clear Last Faults",  NULL, NULL) \
  // To be continued.

#define OBJECT_ID(id, str, ascii, write) \
  case id: \
    pASCIIStr = (uint8_t*) ascii; \
    break;
#define FOREACH_OBJECT_ID(OBJECT_ID) \
  /*      ID    Identifier String     String Ptr,       Func Ptr*/ \
  OBJECT_ID(  0x00, "VendorName",         VENDOR_NAME,      NULL) \
  OBJECT_ID(  0x01, "ProductCode",        PRODUCT_CODE,     NULL) \
  OBJECT_ID(  0x02, "MajorMinorRevision",     MAJOR_MINOR_REV,    NULL) \

// To be continued.

bool              ModbusDataModel_ReadCoil(uint16_t nAddress, bool* bReturn);
ModbusException_T ModbusDataModel_ReadHoldingRegister(uint16_t nAddress, uint16_t* nReturn);
ModbusException_T ModbusDataModel_ReadInputRegister(uint16_t nAddress, uint16_t* nReturn);
ModbusException_T ModbusDataModel_ReadObjectID( uint16_t nObjectID, uint8_t* pBuffer,
                                                int nBufferLen,
                                                uint8_t* nBufferUsed);
ModbusException_T ModbusDataModel_WriteHoldingRegister(uint16_t nAddress, uint16_t* nValue);

#endif/* MODBUSINTERFACE_H_ */
