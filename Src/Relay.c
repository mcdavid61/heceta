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

//	TODO:	Do we want to directly adjust the m_aDR variable whenever we want to make
//			an adjustment?
//			Do we want to constantly bitbang the DR register output for each cycle?
//			Is it enough to verify that the DR remains consistent during the verification step.
static DRV8860_DataRegister_T m_aDR[DRV8860_CNT] = {0xF0, 0x0F};
static DRV8860_ControlRegister_T m_aCR[DRV8860_CNT] = {0xA, 0xB};

//	Verify registers.
//	This is where we'll restore the actual state of things.
DRV8860_ControlRegister_T m_aCRVerify[DRV8860_CNT];
DRV8860_DataRegister_T m_aDRVerify[DRV8860_CNT];

static uint8_t m_nFaultCounter = 0;


bool Relay_Set(uint16_t nPattern)
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

	return;
}

void Relay_Run_Demo()
{
	relayPattern = 1;
	toggleFlag = TRUE;
}

bool Relay_Set_Relay(uint16_t relay)
{
	//	Test commit
	relayPattern = relay;
	toggleFlag = FALSE;
	return true;
}

uint16_t Relay_Get(void)
{
	return (uint16_t) ((m_aDRVerify[DRV8860_B] << 8) | m_aDRVerify[DRV8860_A]);
}

void Relay_Set_CommRelay(_Bool state)
{
	commRelay = state;
	return;
}
/*************************** END OF FILE **************************************/
