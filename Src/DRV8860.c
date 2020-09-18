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
	Function:	DRV8860_DataWrite
	Description:
		Performs a Data Write operation.
*/
void DRV8860_DataWrite(uint16_t nPattern)
{
	//	Latch down
	DRV8860_PIN_LAT(0);

	//	Begin to clock in data.
	uint8_t nBitCount = 0;
	while (nBitCount < 16)
	{
		DRV8860_PIN_DOUT(!!(nPattern & 0x8000));
		DRV8860_PIN_CLK(0);
		delay_us(3);
		DRV8860_PIN_CLK(1);
		delay_us(3);

		nPattern = (nPattern << 1);
		nBitCount++;
	}

	//	Data clocked in.
	delay_us(3);

	//	Clock up
	DRV8860_PIN_CLK(1);
	//	Latch up
	DRV8860_PIN_LAT(1);
}

/*
	Function:	DRV8860_DataRegisterRead
	Description:
		Performs a Data Register Read operation.
*/
uint16_t DRV8860_DataRegisterRead()
{
	//	Counter
	int cnt;

	//	Value to return
	uint16_t nDataRead = 0;

	//	For this particular operation, we'll need to send a pattern of
	//	latch and clock commands. This will cause the relay chips to go
	//	into a different read mode.

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

	//	Clock pulse x 4
	for (cnt = 0; cnt < 4; cnt++)
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

	//	Clock pulse x 4
	for (cnt = 0; cnt < 4; cnt++)
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

	//	Clock pulse x 3
	for (cnt = 0; cnt < 3; cnt++)
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


	//	Begin to clock in data.
	uint8_t nBitCount = 0;
	while (nBitCount < 16)
	{

		//	Upon the falling edge of the clock, we'll be able to read in the fault data.
		//	Wait one usec for that-- and then attempt to read the data coming out.
		//	This is written in such a way that it also passes the output through on read.
		bool bIncomingBit = DRV8860_PIN_DIN();
		nDataRead = (nDataRead << 1) | bIncomingBit;

		//	Clock Up
		DRV8860_PIN_CLK(1);
		delay_us(3);

		//	Clock down
		DRV8860_PIN_CLK(0);
		delay_us(3);

		//	Increase the bit count
		nBitCount++;
	}

	return nDataRead;
}












/*
	Function:	DRV8860_FaultRead
	Description:
		Performs a Fault Read operation.
*/
uint32_t DRV8860_FaultRead()
{
	//	Value to return
	uint32_t nDataRead;

	//	For this particular operation, we'll need to send a pattern of
	//	latch and clock commands. This will cause the relay chips to go
	//	into a different read mode.

	//	Latch down
	DRV8860_PIN_LAT(0);
	delay_us(1);

	//	Clock down
	DRV8860_PIN_CLK(0);
	delay_us(1);

	//	Latch up
	DRV8860_PIN_LAT(1);
	delay_us(2);

	//	Begin to clock in data.
	uint8_t nBitCount = 0;
	while (nBitCount < 32)
	{
		//	Clock Up
		DRV8860_PIN_CLK(1);
		delay_us(3);

		//	Clock down
		DRV8860_PIN_CLK(0);
		delay_us(1);

		//	Upon the falling edge of the clock, we'll be able to read in the fault data.
		//	Wait one usec for that-- and then attempt to read the data coming out.
		//	This is written in such a way that it also passes the output through on read.
		bool bIncomingBit = DRV8860_PIN_DIN();
		nDataRead = (nDataRead << 1) | bIncomingBit;

		//	Rest of the clock cycle gap
		delay_us(2);

		//	Increase the bit count
		nBitCount++;
	}

	//	Latch down
	DRV8860_PIN_LAT(0);
	delay_us(3);

	//	Latch up
	DRV8860_PIN_LAT(1);
	delay_us(1);

	//	Clock Up
	DRV8860_PIN_CLK(1);
	delay_us(3);

	return nDataRead;
}

void DRV8860_Update_Driver_Output(uint16_t pattern)
{
/*
  uint8_t bit_count;
  HAL_GPIO_WritePin(R_EN_GPIO_Port, R_EN_Pin, 1);
  DRV8860_PIN_CLK(0);
  DRV8860_PIN_LAT(0);

  for (bit_count=0; bit_count<16; bit_count++)
  {
	  if (pattern & 0x8000)
	  {
		  DRV8860_PIN_DOUT(1);
	  }
	  else
	  {
		  DRV8860_PIN_DOUT(0);
	  }

	  DRV8860_PIN_CLK(1);
	  delay_us(1);
	  DRV8860_PIN_CLK(0);

	  pattern = pattern << 1;

  }
  HAL_GPIO_WritePin(R_DOUT_GPIO_Port, R_DOUT_Pin, 1);
  DRV8860_PIN_LAT(1);
  DRV8860_PIN_CLK(1);

  return;
*/
  DRV8860_DataWrite(pattern);
}


