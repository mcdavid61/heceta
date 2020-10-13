/*
 * ADC.c
 *
 *  Created on: Apr 13, 2020
 *      Author: dmcmasters
 */

#include "ADC.h"
#include "main.h"
#include "Fault.h"

#define ADC_TICK_INCREMENT    250

/* Variable to report ADC sequencer status */
uint8_t    ubSequenceCompleted = RESET; /* Set when all ranks of the sequence have been converted */

uint16_t    ADC_24V_Mon     = 0;
uint16_t    ADC_3V3_Mon     = 0;
uint16_t    ADC_Temperature = 0;
uint16_t    ADC_VrefInt     = 0;

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
	  ADC_24V_Mon         = (double)aADCxConvertedValues[0] * ADC_REF_VOLT / ADC_MAX_COUNTS * 10590 / 590;
	  ADC_3V3_Mon         = (double)aADCxConvertedValues[1] * ADC_REF_VOLT / ADC_MAX_COUNTS * 25000 / 15000;
	  temporary           = (((double)aADCxConvertedValues[2]) * 1.1) - (uint32_t)*TS30;
	  temporary          *= (double)(110 - 30);
	  temporary          /= (double)(int32_t)((uint32_t)*TS110 - (uint32_t)*TS30);
	  ADC_Temperature     = temporary + 30;
	  ADC_VrefInt         = (double)aADCxConvertedValues[3] * ADC_REF_VOLT / ADC_MAX_COUNTS;
	  ubSequenceCompleted = RESET;
	}
	}

	//	Fault handling
	bool b24VFault = (ADC_24V_Mon < ADC_VIN_TOLERANCE_LOW || ADC_24V_Mon > ADC_VIN_TOLERANCE_HIGH);
	bool b3V3Fault = (ADC_3V3_Mon < ADC_3V3_TOLERANCE_LOW || ADC_3V3_Mon > ADC_3V3_TOLERANCE_HIGH);
	bool bTemperature = ((int16_t) ADC_Temperature) < ADC_TEMPERATURE_TOLERANCE_LOW ||
						((int16_t) ADC_Temperature) > ADC_TEMPERATURE_TOLERANCE_HIGH;
	Fault_Set(FAULT_VOLTAGE_VIN_OUT_OF_SPEC, b24VFault);
	Fault_Set(FAULT_VOLTAGE_3_3V_OUT_OF_SPEC, b3V3Fault);
	Fault_Set(FAULT_TEMPERATURE, bTemperature);
}

uint16_t ADC_Get_Supply_Voltage(void)
{
	return ADC_24V_Mon;
}

uint16_t ADC_Get_3V3_Voltage(void)
{
	return ADC_3V3_Mon;
}

uint16_t ADC_Get_VrefInt_Voltage(void)
{
	return ADC_VrefInt;
}

uint16_t ADC_Get_Temperature(void)
{
	return ADC_Temperature;
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
