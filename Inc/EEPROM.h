/*
 * EEPROM.h
 *
 *  Created on: Oct 2, 2020
 *      Author: BFS
 */

#ifndef EEPROM_H_
#define EEPROM_H_

typedef enum
{
  NVVER_V0,
  NVVER_MAX = 0xFFFF,
} EEPROM_Version_T;

typedef struct
{
  // Configuration structure version
  EEPROM_Version_T    nVersion;

  // Parameters
  uint16_t    nFaultRegisterMap;
  uint16_t    nFailsafeRelayEnable;

  // CRC
  // The CRC shall always be the last value.
  uint16_t    nCRC;

} EEPROM_Configuration_T;

void     EEPROM_Process(void);
uint16_t EEPROM_GetFaultRegisterMap(void);
void     EEPROM_SetFaultRegisterMap(uint16_t nFaultRegisterMap);
uint16_t EEPROM_GetFailsafeRelayEnable(void);
void     EEPROM_SetFailsafeRelayEnable(uint16_t nFailsafeRelayEnable);
#endif/* EEPROM_H_ */
