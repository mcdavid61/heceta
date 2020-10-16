/*
 * EEPROM.c
 *
 *  Created on: Oct 2, 2020
 *      Author: BFS
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "EEPROM.h"
#include "SPIFlash.h"
#include "CRC.h"

static EEPROM_Configuration_T m_sEEPROMConfiguration;
static EEPROM_Configuration_T m_sEEPROMConfigurationVerify;
static unsigned int m_nEEPROMConfigurationFaultCntr = 0;
static bool m_bEEPROMConfigurationDirty = false;

typedef enum
{
	//	The following states are "init" states
	EEPROM_STATE_STARTUP_READ,
	EEPROM_STATE_STARTUP_VERIFY,

	//	The following states are "ready" states
	EEPROM_STATE_IDLE,
	EEPROM_STATE_WRITE,
	EEPROM_STATE_READ,
	EEPROM_STATE_VERIFY,
} EEPROM_State_T;

static EEPROM_State_T m_eEEPROMState = EEPROM_STATE_STARTUP_READ;

/*
	Function:	EEPROM_Process()
	Description:
		Handles all of the processing of the EEPROM--
		for example, when data changes and needs to be updated
		in flash.
*/
void EEPROM_Process(void)
{
	switch(m_eEEPROMState)
	{
		case EEPROM_STATE_STARTUP_READ:
			if (!SPIFlash_Read((uint8_t *) &m_sEEPROMConfiguration, 0, 0, sizeof(m_sEEPROMConfiguration)))
			{
				//	TODO:	Fatal error.
			}
			m_eEEPROMState = EEPROM_STATE_STARTUP_VERIFY;
			break;
		case EEPROM_STATE_STARTUP_VERIFY:
			if (SPIFlash_IsFree())
			{
				//	Operation has completed.
				//	Verify that the CRC is correct.
				uint16_t nCalculatedCRC = CRC16((uint8_t *) &m_sEEPROMConfiguration, sizeof(m_sEEPROMConfiguration) - sizeof(uint16_t));

				//	Compare the calculated CRC against the actual CRC.
				if (nCalculatedCRC != m_sEEPROMConfiguration.nCRC)
				{
					//	This is not a good configuration.
					//	Wipe the configuration to zero, and prepare for it to be written out.
					memset(&m_sEEPROMConfiguration, 0, sizeof(m_sEEPROMConfiguration));
					m_bEEPROMConfigurationDirty = true;
				}
				m_eEEPROMState = m_bEEPROMConfigurationDirty ? EEPROM_STATE_WRITE : EEPROM_STATE_IDLE;
			}
			break;
		case EEPROM_STATE_IDLE:
			if (m_bEEPROMConfigurationDirty)
			{
				m_eEEPROMState = EEPROM_STATE_WRITE;
			}
			break;
		case EEPROM_STATE_WRITE:
			if (SPIFlash_IsFree())
			{
				m_sEEPROMConfiguration.nCRC = CRC16((uint8_t *) &m_sEEPROMConfiguration, sizeof(m_sEEPROMConfiguration) - sizeof(uint16_t));
				if (!SPIFlash_Write((uint8_t *) &m_sEEPROMConfiguration, 0, 0, sizeof(m_sEEPROMConfiguration)))
				{
					//	TODO:	Fatal error.
				}
				m_eEEPROMState = EEPROM_STATE_READ;
			}
			break;
		case EEPROM_STATE_READ:
			if (SPIFlash_IsFree())
			{
				if (!SPIFlash_Read((uint8_t *) &m_sEEPROMConfigurationVerify, 0, 0, sizeof(m_sEEPROMConfiguration)))
				{
					//	TODO:	Fatal error.
				}
				m_eEEPROMState = EEPROM_STATE_VERIFY;
			}
			break;
		case EEPROM_STATE_VERIFY:
			if (SPIFlash_IsFree())
			{
				//	Do a memory compare against the expected configuration and the actual
				//	condiguration. If there's a discrepancy, attempt to rewrite

				int nCompare = memcmp(&m_sEEPROMConfigurationVerify, &m_sEEPROMConfiguration, sizeof(m_sEEPROMConfiguration));
				if (!nCompare)
				{
					/// These are equal. No longer dirty.
					m_bEEPROMConfigurationDirty = false;

					//	Reset the fault counter, we were able to successfully write.
					m_nEEPROMConfigurationFaultCntr = 0;
					m_eEEPROMState = EEPROM_STATE_IDLE;
				}
				else
				{
					//	This didn't work--the configuration wasn't saved properly.
					m_nEEPROMConfigurationFaultCntr += 1;

					//	Restore to the WRITE state and try again.
					m_eEEPROMState = EEPROM_STATE_IDLE;
				}
			}
			break;

		default:
			break;

	}
}

/*
	Function:	EEPROM_MarkConfigurationAsDirty()
	Description:
		Marks the present configuration as dirty, so it
		can be written to the flash.
*/
void EEPROM_MarkConfigurationAsDirty(void)
{
	m_bEEPROMConfigurationDirty = true;
}

/*
	Function:	EEPROM_Ready()
	Description:
		Returns true if the EEPROM is in an initialized state.
*/
bool EEPROM_Ready(void)
{
	return (m_eEEPROMState >= EEPROM_STATE_IDLE);
}
/*
	Function:	EEPROM_GetFaultRegisterMap()
	Description:
		Returns the present fault map, as we believe
		it is stored in the EEPROM.
*/
uint16_t EEPROM_GetFaultRegisterMap(void)
{
	return EEPROM_Ready() ? m_sEEPROMConfiguration.nFaultRegisterMap : 0;
}

/*
	Function:	EEPROM_SetFaultRegisterMap()
	Description:
		Sets the fault map, and the dirty bit.
*/
void EEPROM_SetFaultRegisterMap(uint16_t nFaultRegisterMap)
{
	if (EEPROM_Ready())
	{
		m_sEEPROMConfiguration.nFaultRegisterMap = nFaultRegisterMap;
		EEPROM_MarkConfigurationAsDirty();
	}
}


