/*
 * DRV8860.c
 *
 *  Created on: Feb 5, 2020
 *      Author: dmcmasters
 */

#include "DRV8860.h"
#include "main.h"
#include "delay.h"


void DRV8860_Update_Driver_Output(uint16_t pattern)
{
  uint8_t bit_count;
  HAL_GPIO_WritePin(R_EN_GPIO_Port, R_EN_Pin, 1);
  HAL_GPIO_WritePin(R_CLK_GPIO_Port, R_CLK_Pin, 0);
  HAL_GPIO_WritePin(R_LAT_GPIO_Port, R_LAT_Pin, 0);

  for (bit_count=0; bit_count<16; bit_count++)
  {
	  if (pattern & 0x8000)
	  {
		  HAL_GPIO_WritePin(R_DOUT_GPIO_Port, R_DOUT_Pin, 1);
	  }
	  else
	  {
		  HAL_GPIO_WritePin(R_DOUT_GPIO_Port, R_DOUT_Pin, 0);
	  }

	  HAL_GPIO_WritePin(R_CLK_GPIO_Port, R_CLK_Pin, 1);
	  delay_us(1);
	  HAL_GPIO_WritePin(R_CLK_GPIO_Port, R_CLK_Pin, 0);

	  pattern = pattern << 1;

  }
  HAL_GPIO_WritePin(R_DOUT_GPIO_Port, R_DOUT_Pin, 1);
  HAL_GPIO_WritePin(R_LAT_GPIO_Port, R_LAT_Pin, 1);
  HAL_GPIO_WritePin(R_CLK_GPIO_Port, R_CLK_Pin, 1);

  return;
}
