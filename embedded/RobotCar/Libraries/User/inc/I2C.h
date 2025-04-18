#include "stm32f10x.h"

void init_I2C1(void);
void I2C_StartTransmission(I2C_TypeDef *I2Cx, uint8_t transmissionDirection, uint8_t slaveAddress);
void I2C_WriteData(I2C_TypeDef *I2Cx, uint8_t data);
uint8_t I2C_ReadData(I2C_TypeDef *I2Cx);