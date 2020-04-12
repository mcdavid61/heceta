/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "defines.h"
#include <stdio.h>

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
UART_HandleTypeDef* Main_Get_UART_Handle(void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define _24V0_MON_Pin GPIO_PIN_0
#define _24V0_MON_GPIO_Port GPIOC
#define _3V3_MON_Pin GPIO_PIN_1
#define _3V3_MON_GPIO_Port GPIOC
#define EE_SCK_Pin GPIO_PIN_1
#define EE_SCK_GPIO_Port GPIOA
#define R_DOUT_Pin GPIO_PIN_2
#define R_DOUT_GPIO_Port GPIOA
#define USART3_CS_Pin GPIO_PIN_4
#define USART3_CS_GPIO_Port GPIOA
#define EE_CS_Pin GPIO_PIN_5
#define EE_CS_GPIO_Port GPIOA
#define EE_MISO_Pin GPIO_PIN_6
#define EE_MISO_GPIO_Port GPIOA
#define EE_MISOA7_Pin GPIO_PIN_7
#define EE_MISOA7_GPIO_Port GPIOA
#define SW1_Pin GPIO_PIN_0
#define SW1_GPIO_Port GPIOB
#define SW2_Pin GPIO_PIN_1
#define SW2_GPIO_Port GPIOB
#define SW3_Pin GPIO_PIN_2
#define SW3_GPIO_Port GPIOB
#define SW4_Pin GPIO_PIN_11
#define SW4_GPIO_Port GPIOB
#define SW5_Pin GPIO_PIN_12
#define SW5_GPIO_Port GPIOB
#define SW6_Pin GPIO_PIN_13
#define SW6_GPIO_Port GPIOB
#define SW7_Pin GPIO_PIN_14
#define SW7_GPIO_Port GPIOB
#define SW8_Pin GPIO_PIN_15
#define SW8_GPIO_Port GPIOB
#define RS485_DE_Pin GPIO_PIN_8
#define RS485_DE_GPIO_Port GPIOA
#define RS485_TX_Pin GPIO_PIN_9
#define RS485_TX_GPIO_Port GPIOA
#define RS485_RX_Pin GPIO_PIN_10
#define RS485_RX_GPIO_Port GPIOA
#define LED_GREEN_Pin GPIO_PIN_10
#define LED_GREEN_GPIO_Port GPIOC
#define LED_AMBER_Pin GPIO_PIN_11
#define LED_AMBER_GPIO_Port GPIOC
#define LED_RED_Pin GPIO_PIN_12
#define LED_RED_GPIO_Port GPIOC
#define SPI3_CS_Pin GPIO_PIN_2
#define SPI3_CS_GPIO_Port GPIOD
#define R_EN_Pin GPIO_PIN_5
#define R_EN_GPIO_Port GPIOB
#define R_FLT_Pin GPIO_PIN_6
#define R_FLT_GPIO_Port GPIOB
#define R_DIN_Pin GPIO_PIN_7
#define R_DIN_GPIO_Port GPIOB
#define R_LAT_Pin GPIO_PIN_8
#define R_LAT_GPIO_Port GPIOB
#define R_CLK_Pin GPIO_PIN_9
#define R_CLK_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
