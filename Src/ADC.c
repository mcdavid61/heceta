/*
 * ADC.c
 *
 *  Created on: Apr 13, 2020
 *      Author: dmcmasters
 */

#include <stdbool.h>
#include "ADC.h"
#include "main.h"
#include "Fault.h"

#define ADC_TICK_INCREMENT    250

/* Variable to report ADC sequencer status */
uint8_t    ubSequenceCompleted = RESET; /* Set when all ranks of the sequence have been converted */

// Boolean value, representing whether or not at least one value was calculated.
// This is used for the startup test.
// When this value is 0, the ADC is ready to report back values.
static uint8_t    m_nADCValuesOK = ADC_ITERATIONS;

uint16_t    ADC_24V_Mon     = 0;
uint16_t    ADC_3V3_Mon     = 0;
uint16_t    ADC_Temperature = 0;
uint16_t    ADC_VrefInt     = 0;
uint16_t    ADC_Vref        = 0;

uint16_t    ADC_VrefInt_Counts = 0;

double    temporary;

extern __IO uint16_t    aADCxConvertedValues[ADC_NUM_CHANNELS];

void ADC_Process(void)
{
  ADC_HandleTypeDef*   adc = Main_Get_ADC_Handle();

  static uint32_t    adcTick = 0;

  if (uwTick > adcTick)
  {
    adcTick = uwTick + ADC_TICK_INCREMENT;

    /* Start ADC conversion */
    /* Since sequencer is enabled in discontinuous mode, this will perform    */
    /* the conversion of the next rank in sequencer.                          */
    /* Note: For this example, conversion is triggered by software start,     */
    /*       therefore "HAL_ADC_Start()" must be called for each conversion.  */
    /*       Since DMA transfer has been initiated previously by function     */
    /*       "HAL_ADC_Start_DMA()", this function will keep DMA transfer      */
    /*       active.                                                          */
    if (HAL_ADC_Start(adc) != HAL_OK)
    {
      Error_Handler();
    }

    /* After each intermediate conversion,
       - EOS remains reset (EOS is set only every third conversion)
       - EOC is set then immediately reset by DMA reading of DR register.
       Therefore, the only reliable flag to check the conversion end is EOSMP
       (end of sampling flag).
       Once EOSMP is set, the end of conversion will be reached when the successive
       approximations are over.
       RM indicates 12.5 ADC clock cycles for the successive approximations
       duration with a 12-bit resolution, or 185.ns at 80 MHz.
       Therefore, it is known that the conversion is over at
       HAL_ADC_PollForConversion() API return */
    if (HAL_ADC_PollForEvent(adc, ADC_EOSMP_EVENT, 10) != HAL_OK)
    {
      Error_Handler();
    }

    if (ubSequenceCompleted == SET)
    {
      /* Computation of ADC conversions raw data to physical values */
      /* Note: ADC results are transferred into array "aADCxConvertedValues"  */
      /*       in the order of their rank in ADC sequencer.                   */
      ADC_Vref = (double)((uint32_t)*VREFINT_CALDATA * ADC_VREF_CAL_VOLT) /  aADCxConvertedValues[3];

      ADC_24V_Mon = (double)aADCxConvertedValues[0] * ADC_Vref / ADC_MAX_COUNTS * 10590 / 590;
      ADC_3V3_Mon = (double)aADCxConvertedValues[1] * ADC_Vref / ADC_MAX_COUNTS * 25000 / 15000;

      temporary       = ((double)aADCxConvertedValues[2] * (uint32_t)*VREFINT_CALDATA / aADCxConvertedValues[3]) - (uint32_t)*ADC_TEMP1_COUNTS;
      temporary      *= (double)(ADC_TEMP2 - ADC_TEMP1);
      temporary      /= (double)(int32_t)((uint32_t)*ADC_TEMP2_COUNTS - (uint32_t)*ADC_TEMP1_COUNTS);
      ADC_Temperature = temporary + ADC_TEMP1;

      ADC_VrefInt = (double)aADCxConvertedValues[3] * ADC_Vref / ADC_MAX_COUNTS;

      ubSequenceCompleted = RESET;

      // Reflect our internal counter for when these values are ready.
      m_nADCValuesOK = (m_nADCValuesOK > 0) ? m_nADCValuesOK - 1 : 0;
    }
  }

  // Fault handling
  if (!m_nADCValuesOK)
  {
    bool    b24VFault    = (ADC_24V_Mon < ADC_VIN_TOLERANCE_LOW || ADC_24V_Mon > ADC_VIN_TOLERANCE_HIGH);
    bool    b3V3Fault    = (ADC_3V3_Mon < ADC_3V3_TOLERANCE_LOW || ADC_3V3_Mon > ADC_3V3_TOLERANCE_HIGH);
    bool    bTemperature = ((int16_t) ADC_Temperature) < ADC_TEMPERATURE_TOLERANCE_LOW ||
                           ((int16_t) ADC_Temperature) > ADC_TEMPERATURE_TOLERANCE_HIGH;
    Fault_Set(FAULT_VOLTAGE_VIN_OUT_OF_SPEC, b24VFault);
    Fault_Set(FAULT_VOLTAGE_3_3V_OUT_OF_SPEC, b3V3Fault);
    Fault_Set(FAULT_TEMPERATURE, bTemperature);
  }
}

uint16_t ADC_Get_Supply_Voltage(void)
{
  return (!m_nADCValuesOK) ? ADC_24V_Mon : 0;
}

uint16_t ADC_Get_3V3_Voltage(void)
{
  return (!m_nADCValuesOK) ? ADC_3V3_Mon : 0;
}

uint16_t ADC_Get_VrefInt_Voltage(void)
{
  return (!m_nADCValuesOK) ? ADC_VrefInt : 0;
}

uint16_t ADC_Get_Temperature(void)
{
  return (!m_nADCValuesOK) ? ADC_Temperature : 0;
}

/*
   Function:  ADC_StartupTasksComplete()
   Description:
    Returns a boolean, indicating whether the startup tasks are complete.
 */
bool ADC_StartupTasksComplete(void)
{
  return (!m_nADCValuesOK);
}

/**
 * @brief  Conversion complete callback in non blocking mode
 * @param  AdcHandle : ADC handle
 * @note   This example shows a simple way to report end of conversion
 *         and get conversion result. You can add your own implementation.
 * @retval None
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
  /* Report to main program that ADC sequencer has reached its end */
  ubSequenceCompleted = SET;
}
/**
 * @brief  Conversion DMA half-transfer callback in non blocking mode
 * @param  hadc: ADC handle
 * @retval None
 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{

}

/**
 * @brief  ADC error callback in non blocking mode
 *        (ADC conversion with interruption or transfer by DMA)
 * @param  hadc: ADC handle
 * @retval None
 */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc)
{
  /* In case of ADC error, call main error handler */
  Error_Handler();
}
