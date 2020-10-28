/*
 * OptionByte.c
 *
 *  Created on: Oct 27, 2020
 *      Author: dmcmasters
 */

#include "OptionByte.h"
#include "main.h"
#include "stm32l4xx_hal_flash.h"
/*
 * Local variables
 */

void OptionByte_Check(void)
{
  FLASH_OBProgramInitTypeDef    pOBInit; // FLASH Option Byte structure

  // read the current configuration of the option bytes
  HAL_FLASHEx_OBGetConfig(&pOBInit);

  if (pOBInit.USERConfig & FLASH_OPTR_nSWBOOT0) // See if the bootload configuration is correct
  {
    pOBInit.OptionType = OPTIONBYTE_USER;       // prepare to write the option byte
    pOBInit.USERType   = OB_USER_nSWBOOT0;
    pOBInit.USERConfig = OB_BOOT0_FROM_OB;

    if (HAL_FLASH_Unlock() == HAL_OK)
    {
      if (HAL_FLASH_OB_Unlock() == HAL_OK)
      {
        HAL_FLASHEx_OBProgram(&pOBInit);
        HAL_FLASH_OB_Launch(); // reload the option bytes
        HAL_FLASH_OB_Lock();
      }
      HAL_FLASH_Lock();
    }
  }
}
