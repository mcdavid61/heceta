/*
 * Fault.c
 *
 *  Created on: Oct 6, 2020
 *      Author: BFS
 *
 *  Description:
 *  	Fault manager for the Heceta Relay Module.
 *  	Also responsible for the CRC module checking.
 */

#include <stdint.h>
#include <stdbool.h>
#include "Main.h"
#include "Fault.h"
#include "stm32l4xx_hal.h"

static uint16_t m_nFault = 0;

/*
	Function:	Fault_Set
	Description:
		Activates or clears a fault as specified, based on a boolean.
*/
void Fault_Set(Fault_T eFault, bool bActive)
{
	if (bActive)
	{
		Fault_Activate(eFault);
	}
	else
	{
		Fault_Clear(eFault);
	}
}

/*
	Function:	Fault_Activate
	Description:
		Activates a fault as specified.
*/
void Fault_Activate(Fault_T eFault)
{
	m_nFault |= (1 << eFault);
}
/*
	Function:	Fault_Clear
	Description:
		Clears a fault as specified.
*/
void Fault_Clear(Fault_T eFault)
{
	m_nFault &= ~(1 << eFault);
}

bool Fault_Get(Fault_T eFault)
{
	return !!(m_nFault & (1 << eFault));
}
uint16_t Fault_GetAll()
{
	return m_nFault;
}
/*
	Function:	Fault_OK
	Description:
		Returns the overall fault state of the system.
		If there are any faults that are not okay,
*/
bool Fault_OK(void)
{
	return !m_nFault;
}

//	CRC Module

typedef enum
{
	FAULTCRCSTATE_CALCULATE,
	FAULTCRCSTATE_PROCESS,
	FAULTCRCSTATE_IDLE,
}	FaultCRCState_T;

static FaultCRCState_T m_eFaultCRCState = FAULTCRCSTATE_IDLE;
static bool m_bFaultCRCStartupPass = false;
static uint32_t m_nLastSuccessfulCRCPassTimestamp = 0;

//	Indicates which section of the flash we're currently reading.

static uint16_t m_nFaultCRCSection = 0;
static uint32_t m_nCalculatedCRC = 0;
static uint32_t m_nStoredCRC = 0;


/*
	Function:	Fault_CRC_Process
	Description:
		Calculates the CRC of the system code, and verifies that it is correct.
*/
void Fault_CRC_Process(void)
{
	uint32_t (*pCRCFunc)(CRC_HandleTypeDef * hcrc, uint32_t pBuffer[], uint32_t BufferLength) = NULL;
	uint32_t nAddress;
	uint32_t nChunkSizeWord;

	switch(m_eFaultCRCState)
	{
		case FAULTCRCSTATE_CALCULATE:
			//	Iteratively run through the CRC calculation.
			//	Note that this typically is done in increments of, we'll say, 1/16th
			//	of the flash at a time.
			pCRCFunc = (m_nFaultCRCSection == 0) ?
					HAL_CRC_Calculate : HAL_CRC_Accumulate;

			//	Recall that this is an integer.
			//	It'll explicitly be cast to an address when the time is right.
			nAddress = 0x08000000 + (m_nFaultCRCSection * (FAULT_CRC_FLASH_SIZE_BYTES / FAULT_CRC_DIVISOR));

			//	Calculate the chunk size, in words.
			//	If it's the last chunk size, remove a word for the CRC.
			nChunkSizeWord = (FAULT_CRC_FLASH_SIZE_WORD / FAULT_CRC_DIVISOR);
			nChunkSizeWord -= (m_nFaultCRCSection == (FAULT_CRC_DIVISOR-1)) ? 1 : 0;

			m_nCalculatedCRC = pCRCFunc(Main_Get_CRC_Handle(), (uint32_t *) nAddress, nChunkSizeWord);

			//	Determine the next phase.
			m_nFaultCRCSection += 1;

			//	If this was the last step of the process, go ahead and transition to
			//	the next phase.
			m_eFaultCRCState = (m_nFaultCRCSection == FAULT_CRC_DIVISOR) ?
					FAULTCRCSTATE_PROCESS : m_eFaultCRCState;
			break;
		case FAULTCRCSTATE_PROCESS:
			//	Compare the calculated CRC with the actual CRC.
			//	If they match, we've passed the test.
			m_nStoredCRC = *(uint32_t*)0x0801FFFC;

			//	Does the CRC match what it should be?
			if (m_nStoredCRC == m_nCalculatedCRC)
			{
				//	Matches.
				//	Go ahead and indicate that we've seen something successful.
				m_bFaultCRCStartupPass = true;

				//	Record the last successful timestamp.
				m_nLastSuccessfulCRCPassTimestamp = uwTick;

				//	Go to the IDLE state.
				m_eFaultCRCState = FAULTCRCSTATE_IDLE;

				//	DEBUG:	Print out success.
				printf("[CRC] Passed.\r\n");
			}
			else
			{
				//	Failure handler.
				printf("[CRC] Failed.\r\n");
				//	TODO:	Implement.
				while(1);
			}
			break;
		case FAULTCRCSTATE_IDLE:
			if (((uwTick - m_nLastSuccessfulCRCPassTimestamp) > 60000) || !m_bFaultCRCStartupPass)
			{
				//	Run a calculation.
				m_eFaultCRCState = FAULTCRCSTATE_CALCULATE;
				m_nFaultCRCSection = 0;
			}
			break;
	}
}

/*
	Function:	Fault_CRC_Ready()
	Description:
		Returns true or false when the CRC has been verified to be correct at least once.
		This is used on startup to ensure we're running a valid configuration.
*/
bool Fault_CRC_StartupTasksComplete(void)
{
	return m_bFaultCRCStartupPass;
}



