/*-----------------------------------------------------------------------------
 *  @file		Relay.c
 *
 *  @copyright	2020, Bacharach Inc. as an unpublished work
 *              All Rights Reserved.
 *
 * 	The information contained herein is confidential property of Bacharach Inc.
 *  The use, copying, transfer or disclosure of such information is prohibited
 *  except by express written agreement with Bacharach Inc.
 *
 *  @date		Feb 13, 2020
 *  @author 	dmcmasters
 *
 *  @brief
 *-------------------------------------------------------------------------------*/

#include "Relay.h"
#include "main.h"
#include <string.h>
#include "DRV8860.h"
#include "Fault.h"
#include "EEPROM.h"
#include "Configuration.h"

//

#define RELAY_TICK_INCREMENT 500

uint16_t relayPattern = 0;
_Bool	toggleFlag = FALSE;
_Bool commRelay = FALSE;

typedef enum
{
	RELAY_INIT,
	RELAY_CR_WRITE,
	RELAY_CR_VERIFY,
	RELAY_DR_WRITE,
	RELAY_DR_VERIFY,
}	RelayState_T;

static RelayState_T m_eRelayState = RELAY_INIT;

//	Requested relays from the user/MZ.
static uint16_t m_nRelayRequestMap;

//	Storage for the DR and CR of the DRV8860 to write out.
static DRV8860_DataRegister_T m_aDR[DRV8860_CNT] = {0};
static DRV8860_ControlRegister_T m_aCR[DRV8860_CNT] = {0};

//	Verify registers.
//	This is where we'll restore the actual state of things.
DRV8860_ControlRegister_T m_aCRVerify[DRV8860_CNT] = {0};
DRV8860_DataRegister_T m_aDRVerify[DRV8860_CNT] = {0};

static uint8_t m_nFaultCounter = 0;


/*
	Function:	Relay_Request()
	Description:
		End-user exposed function that allows the user to
		specifically request which relays should be activated.
		This is stored in a holding variable, since it is possible
		for the end result to be different due to fault relays being
		activated and what not.
		If the Heceta Relay Module is disabled, this will result in
		an ILLEGAL_DATA_ADDRESS exception.
*/
ModbusException_T Relay_Request(uint16_t nPattern)
{
	if (!Configuration_GetModuleDisable())
	{
		m_nRelayRequestMap = nPattern;
		return MODBUS_EXCEPTION_OK;
	}
	else
	{
		return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
	}

}




/*
	Function:	Relay_Set()
	Description:
		Sets the relay values, given a 16-bit pattern.
		Note that this occurs at the driver level, meaning the pattern
		requested here is the pattern that will be written.
*/
static bool Relay_Set(uint16_t nPattern)
{
	//	This code will take the a 16-bit pattern, and place it in the correct
	//	order within the DR registers.

	//	"A" registers
	m_aDR[DRV8860_A] = (nPattern >> 0) & 0xFF;

	//	"B" registers
	m_aDR[DRV8860_B] = (nPattern >> 8) & 0xFF;

	return true;
}

/*
	Function:	Relay_Process()
	Description:
		Runs through the state machine, ensuring that the
		values stored within the DRV8860s are expected.
*/
void Relay_Process(void)
{
	//	Component #0:	Heceta Module Disabled?
	//		If the Heceta Module is disabled, clear the relay request map.
	if (Configuration_GetModuleDisable())
	{
		m_nRelayRequestMap = 0;
	}

	//	Component #1:	DR Contents Building
	//		Handles the generation of the DR contents.
	//		The DR is generated based on the fault state of the system--
	//		If the system is under fault, the fault relays are unconditionally
	//		programmed to activate:
	//			DR = nRelaysRequested | nFaultRelays;
	//		Otherwise, the DR simply consists of the relays requested.
	//			DR = nRelaysRequested;

	//	Are in a fault state AND is the module not disabled?
	//	If we are, pull the Fault Register map from the EEPROM.
	bool bFaulted = !Fault_OK();
	uint16_t nRelayFaultMap = bFaulted ? EEPROM_GetFaultRegisterMap() : 0;

	//	Build the result.
	uint16_t nResult = m_nRelayRequestMap | nRelayFaultMap;

	//	Set the relays.
	//	Note that if the module is disabled, the relays will be forcibly set to zero.
	Relay_Set(Configuration_GetModuleDisable() ? 0 : nResult);

	//	Component #2:	State Machine Processing
	//		Handles the actual verification/write process
	//		of the state machines.
	switch(m_eRelayState)
	{
		case RELAY_INIT:
			//	Initialization code for the relay driver.
			//	None needed at this point.
			m_eRelayState = RELAY_CR_WRITE;
			break;
		case RELAY_CR_WRITE:
			//	As defined in the nCR, write out
			//	the control register configuration.
			DRV8860_ControlRegisterWrite(m_aCR, DRV8860_CNT);
			m_eRelayState = RELAY_CR_VERIFY;
			break;
		case RELAY_CR_VERIFY:
			//	Read in the present control register,
			//	and verify that it matches what we expect.
			DRV8860_ControlRegisterRead(m_aCRVerify, DRV8860_CNT);

			if (!memcmp(m_aCRVerify, m_aCR, sizeof(DRV8860_ControlRegister_T) * DRV8860_CNT))
			{
				//	Clear.
				m_nFaultCounter = 0;
				m_eRelayState = RELAY_DR_WRITE;
			}
			else
			{
				m_nFaultCounter++;
				m_eRelayState = RELAY_CR_WRITE;
			}
			break;
		case RELAY_DR_WRITE:
			//	As defined in the nCR, write out
			//	the control register configuration.
			DRV8860_DataRegisterWrite(m_aDR, DRV8860_CNT);
			m_eRelayState = RELAY_DR_VERIFY;
			break;
		case RELAY_DR_VERIFY:
			DRV8860_DataRegisterRead(m_aDRVerify, DRV8860_CNT);
			if (!memcmp(m_aDRVerify, m_aDR, sizeof(DRV8860_DataRegister_T) * DRV8860_CNT))
			{
				//	Clear.
				m_nFaultCounter = 0;
				m_eRelayState = RELAY_CR_VERIFY;
			}
			else
			{
				m_nFaultCounter++;
				m_eRelayState = RELAY_DR_WRITE;
			}
			break;
		default:
			m_eRelayState = RELAY_INIT;
			break;
	}
}

void Relay_Run_Demo()
{
	relayPattern = 1;
	toggleFlag = TRUE;
}

uint16_t Relay_Get(void)
{
	return (uint16_t) ((m_aDRVerify[DRV8860_B] << 8) | m_aDRVerify[DRV8860_A]);
}

uint16_t Relay_GetFaulted(void)
{
	return ((uint16_t) ((m_aDRVerify[DRV8860_B] << 8) | m_aDRVerify[DRV8860_A]) ^ (uint16_t) ((m_aDR[DRV8860_B] << 8) | m_aDR[DRV8860_A]));
}

void Relay_Set_CommRelay(_Bool state)
{
	commRelay = state;
	return;
}
/*************************** END OF FILE **************************************/
