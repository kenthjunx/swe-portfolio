#ifndef LCD_HAL_I2C_H
#define LCD_HAL_I2C_H

#include <stdint.h>
#include <stddef.h>
#include "stm32f1xx_hal.h"

#define LCD_HAL_I2C_ADDRESS_0                            (uint8_t)0x20u
#define LCD_HAL_I2C_ADDRESS_1                            (uint8_t)0x21u
#define LCD_HAL_I2C_ADDRESS_2                            (uint8_t)0x22u
/**
 * @todo implement the other device i2c addresses
 */
#define LCD_HAL_I2C_ADDRESS_6                            (uint8_t)0x26u
#define LCD_HAL_I2C_ADDRESS_7                            (uint8_t)0x27u
#define LCD_HAL_I2C_ADDRESS_DEFAULT                      LCD_HAL_I2C_ADDRESS_7

typedef enum
{
    LCD_HAL_I2C_OK       = 0x00u,
    LCD_HAL_I2C_ERROR,
} LCD_HAL_I2C_Status_t;

typedef struct
{
    void *hi2c;
    uint16_t lcd_addr;
} I2cConfig_t;


/*******************************************************************************
 * PCF8574 Pin Configurations
********************************************************************************/
#define LCD_HAL_I2C_BACKLIGHT_POS                        3u
#define LCD_HAL_I2C_BACKLIGHT_PIN(x)                     ((uint8_t)(x) << LCD_HAL_I2C_BACKLIGHT_POS)

#define LCD_HAL_I2C_ENABLE_POS                           2u
#define LCD_HAL_I2C_ENABLE_PIN(x)                        ((uint8_t)(x) << LCD_HAL_I2C_ENABLE_POS)

#define LCD_HAL_I2C_MODE_DATA_MASK                       0xF0u
#define LCD_HAL_I2C_MODE_DATA_POS                        4u
#define LCD_HAL_I2C_MODE_RW_MASK                         0x02u
#define LCD_HAL_I2C_MODE_RW_POS                          1u
#define LCD_HAL_I2C_MODE_RS_MASK                         0x01u
#define LCD_HAL_I2C_MODE_RS_POS                          0u
#define LCD_HAL_I2C_MODE(DATA,RW,RS)                     (uint8_t)((uint8_t)(((DATA) << LCD_HAL_I2C_MODE_DATA_POS) & LCD_HAL_I2C_MODE_DATA_MASK)) | \
                                                              ((uint8_t)(((RW) << LCD_HAL_I2C_MODE_RW_POS) & LCD_HAL_I2C_MODE_RW_MASK))       | \
                                                              ((uint8_t)(((RS) << LCD_HAL_I2C_MODE_RS_POS) & LCD_HAL_I2C_MODE_RS_MASK))

LCD_HAL_I2C_Status_t LCD_HAL_I2C_SendCmd(I2cConfig_t *i2c_handle, uint8_t *pTxBuffer, const uint16_t count);
LCD_HAL_I2C_Status_t LCD_HAL_I2C_ReadData(I2cConfig_t *i2c_handle, uint8_t *pRxBuffer, const uint16_t count);

void LCD_HAL_I2C_delay_ms(uint32_t delay);

#endif /* LCD_HAL_I2C_H */