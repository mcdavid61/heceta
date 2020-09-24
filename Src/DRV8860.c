/*
 * DRV8860.c
 *
 *  Created on: Feb 5, 2020
 *      Author: dmcmasters
 */

#include "DRV8860.h"
#include "main.h"
#include "delay.h"

/*
	Function:	DRV8860_SpecialCommandPulse()
	Description:
		Does the special command pulse.
		At the conclusion of its run, leaves the latch in the high position
		and the clock in the low position.
*/
void DRV8860_SpecialCommandPulse(uint8_t nPulsePart2, uint8_t nPulsePart3, uint8_t nPulsePart4)
{
	//	For this particular operation, we'll need to send a pattern of
	//	latch and clock commands. This will cause the relay chips to go
	//	into a different read mode.

	uint8_t nCnt;
	//	Clock down
	DRV8860_PIN_CLK(0);
	delay_us(3);

	//	Latch down
	DRV8860_PIN_LAT(0);
	delay_us(1);

	//	Clock pulse
	DRV8860_PIN_CLK(1);
	delay_us(3);
	DRV8860_PIN_CLK(0);
	delay_us(3);

	//	Latch up
	DRV8860_PIN_LAT(1);
	delay_us(1);
	//	Latch down
	DRV8860_PIN_LAT(0);
	delay_us(1);

	//	Clock pulse x nPulsePart2
	for (nCnt = 0; nCnt < nPulsePart2; nCnt++)
	{
		//	Clock pulse
		DRV8860_PIN_CLK(1);
		delay_us(3);
		DRV8860_PIN_CLK(0);
		delay_us(3);
	}

	//	Latch up
	DRV8860_PIN_LAT(1);
	delay_us(1);
	//	Latch down
	DRV8860_PIN_LAT(0);
	delay_us(1);

	//	Clock pulse x nPulsePart3
	for (nCnt = 0; nCnt < nPulsePart3; nCnt++)
	{
		//	Clock pulse
		DRV8860_PIN_CLK(1);
		delay_us(3);
		DRV8860_PIN_CLK(0);
		delay_us(3);
	}

	//	Latch up
	DRV8860_PIN_LAT(1);
	delay_us(1);
	//	Latch down
	DRV8860_PIN_LAT(0);
	delay_us(1);

	//	Clock pulse x nPulsePart4
	for (nCnt = 0; nCnt < nPulsePart4; nCnt++)
	{
		//	Clock pulse
		DRV8860_PIN_CLK(1);
		delay_us(3);
		DRV8860_PIN_CLK(0);
		delay_us(3);
	}

	//	Latch up
	DRV8860_PIN_LAT(1);
	delay_us(1);
}

/*
	Function:	DRV8860_DataRegisterWrite
	Description:
		Performs a Control Register Write operation.
*/
void DRV8860_DataRegisterWrite(DRV8860_DataRegister_T * aWrite, uint8_t nDevCount)
{
	//	Latch down
	DRV8860_PIN_LAT(0);
	delay_us(1);

	DRV8860_PIN_CLK(0);
	delay_us(2);

	uint8_t nDevCntr = nDevCount;
	while (nDevCntr > 0)
	{
		//	Begin to clock out data.
		//	Remember--data clocked out on RISING EDGE of the clock.
		uint8_t nBitCount = (sizeof(DRV8860_DataRegister_T) * 8);
		while (nBitCount > 0)
		{
			//	Set the output pin
			bool bValueOut = ((aWrite[nDevCntr-1] >> (nBitCount-1)) & 1);
			DRV8860_PIN_DOUT(bValueOut);

			//	Make up for the 1us missing in the clock down
			//	from the previous iteration.
			delay_us(1);

			//	Clock Up
			DRV8860_PIN_CLK(1);
			delay_us(3);

			//	Clock down
			//	Note that the delay here is 2us, we'll
			//	make up for the 1us in the loop back.
			DRV8860_PIN_CLK(0);
			delay_us(2);

			//	Decrease the bit count
			nBitCount--;
		}

		//	Decrease the dev count.
		nDevCntr--;
	}
	//	Make up for the 1us missing in the clock down
	//	from the previous iteration.
	delay_us(1);

	//	Latch up
	DRV8860_PIN_LAT(1);
	delay_us(1);
}

/*
	Function:	DRV8860_ControlRegisterWrite
	Description:
		Performs a Control Register Write operation.
*/
void DRV8860_ControlRegisterWrite(DRV8860_ControlRegister_T * aWrite, uint8_t nDevCount)
{
	//	For this particular operation, we'll need to send a pattern of
	//	latch and clock commands. This will cause the relay chips to go
	//	into a special write mode.
	DRV8860_SpecialCommandPulse(2,2,3);

	//	Set the latch down.
	//	This is a write command.
	DRV8860_PIN_LAT(0);
	delay_us(1);

	//	Begin to clock out data-- depending on the number of devices we're writing out to.
	uint8_t nDevCntr = nDevCount;

	while (nDevCntr > 0)
	{
		//	Begin to clock out data.
		//	Remember--data clocked out on RISING EDGE of the clock.
		uint8_t nBitCount = (sizeof(DRV8860_ControlRegister_T) * 8);
		while (nBitCount > 0)
		{
			//	Set the output pin
			bool bValueOut = ((aWrite[nDevCntr-1] >> (nBitCount-1)) & 1);
			DRV8860_PIN_DOUT(bValueOut);

			//	Make up for the 1us missing in the clock down
			//	from the previous iteration.
			delay_us(1);

			//	Clock Up
			DRV8860_PIN_CLK(1);
			delay_us(3);

			//	Clock down
			//	Note that the delay here is 2us, we'll
			//	make up for the 1us in the loop back.
			DRV8860_PIN_CLK(0);
			delay_us(2);


			//	Decrease the bit count
			nBitCount--;
		}

		//	Decrease the dev count.
		nDevCntr--;
	}

	//	Small delay to make up for the 1us missing
	delay_us(1);


	//	Latch up
	DRV8860_PIN_LAT(1);
	delay_us(1);
}

/*
	Function:	DRV8860_ControlRegisterRead
	Description:
		Performs a Control Register Read operation.
*/
void DRV8860_ControlRegisterRead(DRV8860_ControlRegister_T * aRead, uint8_t nDevCount)
{
	//	For this particular operation, we'll need to send a pattern of
	//	latch and clock commands. This will cause the relay chips to go
	//	into a different read mode.
	DRV8860_SpecialCommandPulse(4,2,3);


	//	Begin to clock out data-- depending on the number of devices we're writing out to.
	uint8_t nDevCntr = nDevCount;

	while (nDevCntr > 0)
	{
		//	Begin to clock out data.
		//	Remember--data clocked out on RISING EDGE of the clock.
		uint8_t nBitCount = (sizeof(DRV8860_ControlRegister_T) * 8);
		while (nBitCount > 0)
		{
			//	Upon the falling edge of the clock, we'll be able to read in the fault data.
			//	Wait one usec for that-- and then attempt to read the data coming out.
			//	This is written in such a way that it also passes the output through on read.
			bool bIncomingBit = DRV8860_PIN_DIN();
			aRead[nDevCntr-1] = (aRead[nDevCntr-1] << 1) | bIncomingBit;

			//	Clock Up
			DRV8860_PIN_CLK(1);
			delay_us(3);

			//	Clock down
			DRV8860_PIN_CLK(0);
			delay_us(3);

			//	Decrease the bit count
			nBitCount--;
		}

		//	Decrease the dev count.
		nDevCntr--;
	}
}

/*
	Function:	DRV8860_DataRegisterRead
	Description:
		Performs a Data Register Read operation.
*/
void DRV8860_DataRegisterRead(DRV8860_DataRegister_T * aRead, uint8_t nDevCount)
{
	//	For this particular operation, we'll need to send a pattern of
	//	latch and clock commands. This will cause the relay chips to go
	//	into a different read mode.
	DRV8860_SpecialCommandPulse(4,4,3);

	//	Begin to clock out data-- depending on the number of devices we're writing out to.
	uint8_t nDevCntr = nDevCount;

	while (nDevCntr > 0)
	{
		//	Begin to clock out data.
		//	Remember--data clocked out on RISING EDGE of the clock.
		uint8_t nBitCount = (sizeof(DRV8860_DataRegister_T) * 8);
		while (nBitCount > 0)
		{
			//	Upon the falling edge of the clock, we'll be able to read in the fault data.
			//	Wait one usec for that-- and then attempt to read the data coming out.
			//	This is written in such a way that it also passes the output through on read.
			bool bIncomingBit = DRV8860_PIN_DIN();
			aRead[nDevCntr-1] = (aRead[nDevCntr-1] << 1) | bIncomingBit;

			//	Clock Up
			DRV8860_PIN_CLK(1);
			delay_us(3);

			//	Clock down
			DRV8860_PIN_CLK(0);
			delay_us(3);

			//	Decrease the bit count
			nBitCount--;
		}

		//	Decrease the dev count.
		nDevCntr--;
	}
}




