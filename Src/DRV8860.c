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
	//	Clock down
	DRV8860_PIN_CLK(0);

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


