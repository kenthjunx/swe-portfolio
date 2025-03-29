#include "lcd_hal_i2c.h"
#include "FreeRTOS.h"
#include "task.h"

#define LCD_HAL_I2C_ADDRESS_SHIFT(ADDR, RW) (((ADDR) << 1u) | (RW))

LCD_HAL_I2C_Status_t LCD_HAL_I2C_SendCmd(I2cConfig_t *i2c_handle, uint8_t *pTxBuffer, const uint16_t count)
{
    HAL_StatusTypeDef returnStatus = HAL_ERROR;
    uint16_t lcd_addr = 0u;

    if (i2c_handle == NULL)
    {
        returnStatus = HAL_ERROR;
    }
    else
    {
        /* prepare i2c for write operation */
        lcd_addr = LCD_HAL_I2C_ADDRESS_SHIFT(i2c_handle->lcd_addr, 0u);

        /* send to i2c */
        returnStatus = HAL_I2C_Master_Transmit(i2c_handle->hi2c, lcd_addr, pTxBuffer, count, 5u);
    }

    return (returnStatus != HAL_OK) ? LCD_HAL_I2C_ERROR : LCD_HAL_I2C_OK;
}

LCD_HAL_I2C_Status_t LCD_HAL_I2C_ReadData(I2cConfig_t *i2c_handle, uint8_t *pRxBuffer, const uint16_t count)
{
    HAL_StatusTypeDef returnStatus = HAL_ERROR;
    uint16_t lcd_addr = 0u;

    if (i2c_handle == NULL)
    {
        returnStatus = HAL_ERROR;
    }
    else
    {
        /* prepare i2c for read operation */
        lcd_addr = LCD_HAL_I2C_ADDRESS_SHIFT(i2c_handle->lcd_addr, 1u);

        /* perform read operation */
        returnStatus = HAL_I2C_Master_Receive(i2c_handle->hi2c, lcd_addr, pRxBuffer, count, 5u);
    }

    return (returnStatus != HAL_OK) ? LCD_HAL_I2C_ERROR : LCD_HAL_I2C_OK;
}

void LCD_HAL_I2C_delay_ms(uint32_t delay)
{
    HAL_Delay(delay);
}
