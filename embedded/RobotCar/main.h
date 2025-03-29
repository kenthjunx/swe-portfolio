/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/

#include "stm32f10x.h"
#include <math.h>
#include <stdio.h>
#include "I2C.h"
#include "LiquidCrystal_I2C.h"
#include "L298N.h"
#include "L298Nx2.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

void RCC_Config(void);
void GPIO_Config(void);
void TIM_Config(void);
void OC_Config(void);
void NVIC_Config(void);

void LCD_Init(void);
void button_Init(void);
void USS_Init(void);
void servo_Init(void);
void buzzer_Init(void);
void MotorDriver_Init(void);

void ITtick_Increment(void);
void Ptick_Decrement(void);
uint16_t HCSR04_GetDistance(void);
void toggleLED(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void delay_ms(uint32_t delayTime_ms);
void delay_us(uint32_t delayTime_us);

void I2C_Start(void);
void I2C_Address(uint8_t slave_address);
void I2c_Write(uint8_t data);
void I2C_Stop(void);
#endif /* __MAIN_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
