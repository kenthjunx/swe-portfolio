/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/**
 * @defgroup AppRtc_HourFormat_Enum
 * @{
 */
typedef enum
{
  APPRTC_HOURFORMAT_12 = 0u,
  APPRTC_HOURFORMAT_24,
} AppRtc_HourFormat_t;
/**
 * @}
 */

typedef enum
{
  APP_MODE_IDLE = 0u,
  APP_MODE_ALARM,
  APP_MODE_RUNNING,
  APP_MODE_SETTING
} App_Mode_t;

typedef enum
{
  APP_SETMODE_SEC = 0u,
  APP_SETMODE_MIN,
  APP_SETMODE_HOUR,
  APP_SETMODE_AMPM,
  APP_SETMODE_WEEKDAY,
  APP_SETMODE_DATE,
  APP_SETMODE_MONTH,
  APP_SETMODE_YEAR,
} App_SetModeState_t;

typedef enum
{
  APP_RUNMODE_STOP = 0u,
  APP_RUNMODE_START,
  APP_RUNMODE_RESET,
} App_RunModeState_t;

typedef struct
{
  App_Mode_t mode;
  // App_Mode_t prevMode;
  App_SetModeState_t set;
  BaseType_t isAlarmExpired;
  BaseType_t isHourlySnooze;
} App_AlarmModeState_t;

typedef struct
{
  BaseType_t noTimeout;
  BaseType_t noBreak;
  BaseType_t buzzerOn;
} App_TmrStatus_t;

typedef struct
{
  App_Mode_t mode;
  AppRtc_HourFormat_t hourFormat;
  App_AlarmModeState_t alarmMode;
  App_SetModeState_t setMode;
  App_RunModeState_t runMode;
}App_State_t;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define APPCOMM_RTC_MONDAY                                      "MON"
#define APPCOMM_RTC_TUESDAY                                     "TUE"
#define APPCOMM_RTC_WEDNESDAY                                   "WED"
#define APPCOMM_RTC_THURSDAY                                    "THU"
#define APPCOMM_RTC_FRIDAY                                      "FRI"
#define APPCOMM_RTC_SATURDAY                                    "SAT"
#define APPCOMM_RTC_SUNDAY                                      "SUN"
#define APPCOMM_RTC_WEEKDAY(DAY)                                (((DAY) == RTC_WEEKDAY_MONDAY)    ? APPCOMM_RTC_MONDAY    : \
                                                                (((DAY) == RTC_WEEKDAY_TUESDAY)   ? APPCOMM_RTC_TUESDAY   : \
                                                                (((DAY) == RTC_WEEKDAY_WEDNESDAY) ? APPCOMM_RTC_WEDNESDAY : \
                                                                (((DAY) == RTC_WEEKDAY_THURSDAY)  ? APPCOMM_RTC_THURSDAY  : \
                                                                (((DAY) == RTC_WEEKDAY_FRIDAY)    ? APPCOMM_RTC_FRIDAY    : \
                                                                (((DAY) == RTC_WEEKDAY_SATURDAY)  ? APPCOMM_RTC_SATURDAY  : \
                                                                (((DAY) == RTC_WEEKDAY_SUNDAY)    ? APPCOMM_RTC_SUNDAY    : "   ")))))))

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void App_Tasks_Init(void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_BUILTIN_Pin GPIO_PIN_13
#define LED_BUILTIN_GPIO_Port GPIOC
#define BTN_BUILTIN_Pin GPIO_PIN_0
#define BTN_BUILTIN_GPIO_Port GPIOA
#define BTN_BUILTIN_EXTI_IRQn EXTI0_IRQn
#define BTN_A_Pin GPIO_PIN_12
#define BTN_A_GPIO_Port GPIOB
#define BTN_B_Pin GPIO_PIN_13
#define BTN_B_GPIO_Port GPIOB
#define BTN_C_Pin GPIO_PIN_14
#define BTN_C_GPIO_Port GPIOB
#define BTN_D_Pin GPIO_PIN_15
#define BTN_D_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
extern TaskHandle_t uartprint_task;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
