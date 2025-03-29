#include "hd44780.h"

/**
 * @brief private macro
 * @details USE5VOLTS = 1: wait for more than 15ms after Vcc rises to 4.5V
 *          USE5VOLTS = 0: wait for more than 40ms after Vcc rises to 2.7V
 */
#define HD44780_USE5VOLTS 1u

// static LCD_HAL_I2C_Status_t Hd44780_PrepareCmd(Hd44780_Handle_t *hd44780_handle, const uint8_t cmd, const uint8_t regSel);

LCD_HAL_I2C_Status_t Hd44780_Init(Hd44780_Handle_t *hd44780_handle)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;
    uint8_t i2c_data = 0u;

    if (hd44780_handle == NULL)
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
        // Initialization by instruction
        // wait for more than 15ms after power on
#if (HD44780_USE5VOLTS)
        LCD_HAL_I2C_delay_ms(20);
#else
        LCD_HAL_I2C_delay_ms(50);
#endif
        // backlight on
        hd44780_handle->_status._ledState = HD44780_BACKLIGHT_OFF;
        // function set for 8bit interface
        i2c_data = HD44780_FUNCTION_SET(HD44780_FS_IFDL_8BIT, 0u, 0u);
        status = Hd44780_PrepareCmd(hd44780_handle, i2c_data, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(1);

        // wait for more than 4.1ms
        LCD_HAL_I2C_delay_ms(5);
        // function set for 8bit interface
        i2c_data = HD44780_FUNCTION_SET(HD44780_FS_IFDL_8BIT, 0u, 0u);
        status = Hd44780_PrepareCmd(hd44780_handle, i2c_data, HD44780_RS_INSTRUCTION_REG);

        // wait for more than 100us
        LCD_HAL_I2C_delay_ms(1);
        // function set for 8bit interface
        i2c_data = HD44780_FUNCTION_SET(HD44780_FS_IFDL_8BIT, 0u, 0u);
        status = Hd44780_PrepareCmd(hd44780_handle, i2c_data, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(10);

        // function set for 4bit interface (still in 8bit)
        i2c_data = HD44780_FUNCTION_SET(HD44780_FS_IFDL_4BIT, 0u, 0u);
        status = Hd44780_PrepareCmd(hd44780_handle, i2c_data, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(10);

        // wait for 37us or check BF

        // function set for N, L (now in 4bit interface)
        // the number of display lines and char font cannot be changed after this point
        i2c_data = HD44780_FUNCTION_SET(HD44780_FS_IFDL_4BIT, hd44780_handle->init.num_dispLines,
                                        hd44780_handle->init.char_font);
        status = Hd44780_PrepareCmd(hd44780_handle, i2c_data, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(1);
        hd44780_handle->_status._funcSet = i2c_data;

        // display off
        i2c_data = HD44780_DISP_CTRL(HD44780_DISPCTRL_DISP_OFF, HD44780_DISPCTRL_CURS_OFF, HD44780_DISPCTRL_BLINK_OFF);
        status = Hd44780_PrepareCmd(hd44780_handle, i2c_data, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(1);

        // display clear
        i2c_data = HD44780_CLEAR_DISPLAY;
        status = Hd44780_PrepareCmd(hd44780_handle, i2c_data, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(2);

        // entry mode set
        i2c_data = HD44780_ENTRY_MODE_SET(HD44780_EMS_ID_INC, HD44780_EMS_SHIFT_DISABLE);
        status = Hd44780_PrepareCmd(hd44780_handle, i2c_data, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(1);
        hd44780_handle->_status._ems = i2c_data;

        // display init
        // return home
        i2c_data = HD44780_RETURN_HOME;
        Hd44780_PrepareCmd(hd44780_handle, i2c_data, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(2);

        // backlight on
        hd44780_handle->_status._ledState = HD44780_BACKLIGHT_ON;
        // display on, blinking cursor on
        i2c_data = HD44780_DISP_CTRL(HD44780_DISPCTRL_DISP_ON, HD44780_DISPCTRL_CURS_ON, HD44780_DISPCTRL_BLINK_ON);
        Hd44780_PrepareCmd(hd44780_handle, i2c_data, HD44780_RS_INSTRUCTION_REG);
        hd44780_handle->_status._dispCtrl = i2c_data;
        LCD_HAL_I2C_delay_ms(1);
    }

    return status;
}

LCD_HAL_I2C_Status_t Hd44780_Clear(Hd44780_Handle_t *hd44780_handle)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;
    uint8_t cmd = 0u;

    if (hd44780_handle == NULL)
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
        cmd = HD44780_CLEAR_DISPLAY;
        status = Hd44780_PrepareCmd(hd44780_handle, cmd, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(1);
    }

    return status;
}

LCD_HAL_I2C_Status_t Hd44780_Return(Hd44780_Handle_t *hd44780_handle)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;
    uint8_t cmd = 0u;

    if (hd44780_handle == NULL)
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
        cmd = HD44780_RETURN_HOME;
        status = Hd44780_PrepareCmd(hd44780_handle, cmd, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(2);
    }

    return status;
}

LCD_HAL_I2C_Status_t Hd44780_ScrollDisplay(Hd44780_Handle_t *hd44780_handle, const Hd44780_Direction_t direction)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;
    uint8_t cmd = 0u;

    if (hd44780_handle == NULL || direction >= HD44780_RTL)
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
        cmd = HD44780_CURSOR_OR_DISP_SHIFT(HD44780_CDS_DISPLAY_SHIFT, (uint8_t)direction);
        status = Hd44780_PrepareCmd(hd44780_handle, cmd, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(1);
    }

    return status;
}

LCD_HAL_I2C_Status_t Hd44780_ToggleBlink(Hd44780_Handle_t *hd44780_handle, const Hd44780_Toggle_t toggle)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;
    uint8_t cmd = 0u;
    uint8_t display = 0u;
    uint8_t cursor = 0u;

    if (hd44780_handle == NULL)
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
        display = (hd44780_handle->_status._dispCtrl & HD44780_DISPCTRL_D_MASK) >> HD44780_DISPCTRL_D_POS;
        cursor = (hd44780_handle->_status._dispCtrl & HD44780_DISPCTRL_C_MASK) >> HD44780_DISPCTRL_C_POS;
        cmd = HD44780_DISP_CTRL(display, cursor, (uint8_t)toggle);
        status = Hd44780_PrepareCmd(hd44780_handle, cmd, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(1);
        hd44780_handle->_status._dispCtrl = cmd;
    }

    return status;
}

LCD_HAL_I2C_Status_t Hd44780_ToggleDisplay(Hd44780_Handle_t *hd44780_handle, const Hd44780_Toggle_t toggle)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;
    uint8_t cmd = 0u;
    uint8_t cursor = 0u;
    uint8_t blink = 0u;

    if (hd44780_handle == NULL)
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
        cursor = (hd44780_handle->_status._dispCtrl & HD44780_DISPCTRL_C_MASK) >> HD44780_DISPCTRL_C_POS;
        blink = (hd44780_handle->_status._dispCtrl & HD44780_DISPCTRL_B_MASK) >> HD44780_DISPCTRL_B_POS;
        cmd = HD44780_DISP_CTRL((uint8_t)toggle, cursor, blink);
        status = Hd44780_PrepareCmd(hd44780_handle, cmd, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(1);
        hd44780_handle->_status._dispCtrl = cmd;
    }

    return status;
}

LCD_HAL_I2C_Status_t Hd44780_ToggleCursor(Hd44780_Handle_t *hd44780_handle, const Hd44780_Toggle_t toggle)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;
    uint8_t cmd = 0u;
    uint8_t display = 0u;
    uint8_t blink = 0u;

    if (hd44780_handle == NULL)
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
        display = (uint8_t)(hd44780_handle->_status._dispCtrl & HD44780_DISPCTRL_D_MASK) >> HD44780_DISPCTRL_D_POS;
        blink = (hd44780_handle->_status._dispCtrl & HD44780_DISPCTRL_B_MASK) >> HD44780_DISPCTRL_B_POS;
        cmd = HD44780_DISP_CTRL(display, (uint8_t)toggle, blink);
        status = Hd44780_PrepareCmd(hd44780_handle, cmd, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(1);
        hd44780_handle->_status._dispCtrl = cmd;
    }

    return status;
}

LCD_HAL_I2C_Status_t Hd44780_MoveCursor(Hd44780_Handle_t *hd44780_handle, const uint8_t row, const uint8_t col)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;
    uint8_t cmd = 0u;

    if ((hd44780_handle == NULL) || !((row >= 1u) && (row <= hd44780_handle->init.rows)) ||
        !((col >= 1u) && (col <= 40u)))
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
        uint8_t ddr_addr = (uint8_t)((col - 1) + ((row - 1) << 6u));
        cmd = HD44780_SET_DDRAM_ADDR(ddr_addr);
        status = Hd44780_PrepareCmd(hd44780_handle, cmd, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(1);
    }

    return status;
}

LCD_HAL_I2C_Status_t Hd44780_ToggleBacklight(Hd44780_Handle_t *hd44780_handle, const Hd44780_Toggle_t toggle)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;
    uint8_t i2c_data = 0u;

    if (hd44780_handle == NULL)
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
        i2c_data = LCD_HAL_I2C_BACKLIGHT_PIN((uint8_t)toggle);
        status = LCD_HAL_I2C_SendCmd(hd44780_handle->i2cConfig, &i2c_data, 1u);
        hd44780_handle->_status._ledState = (uint8_t)toggle;
    }

    return status;
}

LCD_HAL_I2C_Status_t Hd44780_TextDirection(Hd44780_Handle_t *hd44780_handle, const Hd44780_Direction_t direction)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;
    uint8_t cmd = 0u;

    if (hd44780_handle == NULL || direction < HD44780_RTL)
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
        uint8_t shift = (hd44780_handle->_status._ems & HD44780_EMS_S_MASK) >> HD44780_EMS_S_POS;
        uint8_t id = 0u;

        if (direction == HD44780_RTL)
            id = HD44780_EMS_ID_DEC;
        else if (direction == HD44780_LTR)
            id = HD44780_EMS_ID_INC;
        else
            id = (hd44780_handle->_status._ems & HD44780_EMS_ID_MASK) >> HD44780_EMS_ID_POS;

        cmd = HD44780_ENTRY_MODE_SET(id, shift);
        status = Hd44780_PrepareCmd(hd44780_handle, cmd, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(1);
        hd44780_handle->_status._ems = cmd;
    }

    return status;
}

LCD_HAL_I2C_Status_t Hd44780_AutoScroll(Hd44780_Handle_t *hd44780_handle, const Hd44780_Toggle_t toggle)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;
    uint8_t cmd = 0u;

    if (hd44780_handle == NULL)
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
        uint8_t id = (hd44780_handle->_status._ems & HD44780_EMS_ID_MASK) >> HD44780_EMS_ID_POS;

        cmd = HD44780_ENTRY_MODE_SET(id, (uint8_t)toggle);
        status = Hd44780_PrepareCmd(hd44780_handle, cmd, HD44780_RS_INSTRUCTION_REG);
        LCD_HAL_I2C_delay_ms(1);
        hd44780_handle->_status._ems = cmd;
    }

    return status;
}

LCD_HAL_I2C_Status_t Hd44780_GenerateChar(Hd44780_Handle_t *hd44780_handle, const uint8_t addr, const uint8_t *charmap)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;
    uint8_t cmd = 0u;

    if (hd44780_handle == NULL || addr >= 8u)
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
        cmd = HD44780_SET_CGRAM_ADDR(addr << 3u);
        status = Hd44780_PrepareCmd(hd44780_handle, cmd, HD44780_RS_INSTRUCTION_REG);

        uint8_t len = (hd44780_handle->init.char_font == HD44780_FS_FONT_5X8_DOTS) ?
                       8u : 10u;
        uint8_t i = 0u;
        do
        {
            status = Hd44780_PrepareCmd(hd44780_handle, (charmap[i] & 0x1Fu), HD44780_RS_DATA_REG);
            LCD_HAL_I2C_delay_ms(1);
        } while ((++i < len) && (status == LCD_HAL_I2C_OK));
        
    }

    return status;
}

LCD_HAL_I2C_Status_t Hd44780_Print(Hd44780_Handle_t *hd44780_handle, const char *str)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;

    if (hd44780_handle == NULL)
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
        for(uint8_t i = 0; (str[i] != '\0') && (i < hd44780_handle->init.cols); i++)
        {
            status = Hd44780_PrepareCmd(hd44780_handle, str[i], HD44780_RS_DATA_REG);
            
            if(status != LCD_HAL_I2C_OK)
                break;
        }
    }

    return status;
}

LCD_HAL_I2C_Status_t Hd44780_PrintChar(Hd44780_Handle_t *hd44780_handle, const char charVal)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;

    if (hd44780_handle == NULL)
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
        status = Hd44780_PrepareCmd(hd44780_handle, (uint8_t)charVal, HD44780_RS_DATA_REG);
        HAL_Delay(1);
    }

    return status;
}

LCD_HAL_I2C_Status_t Hd44780_PrepareCmd(Hd44780_Handle_t *hd44780_handle, const uint8_t cmd,
                                               const uint8_t regSel)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;
    uint8_t i2c_data[4] = {0};

    if (hd44780_handle == NULL)
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
#ifdef USE_LCD_HAL_I2C
        /* prepare the data */
        i2c_data[0] = LCD_HAL_I2C_MODE((cmd >> 4u), HD44780_RW_WRITE, regSel) |       // send MSB
                      LCD_HAL_I2C_BACKLIGHT_PIN(hd44780_handle->_status._ledState) |  // retain user backlight
                      LCD_HAL_I2C_ENABLE_PIN(HD44780_ENABLE);                         // enable pin

        i2c_data[1] = LCD_HAL_I2C_MODE(0u, HD44780_RW_WRITE, regSel) |                // send no data
                      LCD_HAL_I2C_BACKLIGHT_PIN(hd44780_handle->_status._ledState) |  // retain user backlight
                      LCD_HAL_I2C_ENABLE_PIN(HD44780_DISABLE);                        // disable EN pin

        i2c_data[2] = LCD_HAL_I2C_MODE(cmd, HD44780_RW_WRITE, regSel) |               // send LSB
                      LCD_HAL_I2C_BACKLIGHT_PIN(hd44780_handle->_status._ledState) |  // retain user backlight
                      LCD_HAL_I2C_ENABLE_PIN(HD44780_ENABLE);

        i2c_data[3] = LCD_HAL_I2C_MODE(0u, HD44780_RW_WRITE, regSel) |                // send no data
                      LCD_HAL_I2C_BACKLIGHT_PIN(hd44780_handle->_status._ledState) |  // retain user backlight
                      LCD_HAL_I2C_ENABLE_PIN(HD44780_DISABLE);                        // disable EN pin

        /* send the cmd to i2c */
        status = LCD_HAL_I2C_SendCmd(hd44780_handle->i2cConfig, i2c_data, 4u);
#elif defined USE_LCD_HAL_GPIO
        /**
         * @todo impement sending cmd to gpio
         *
         */
#endif
    }

    return status;
}

/**
 * @todo implement the following:
 *       Read data to CG or DDRAM
 * @note the following functions is not yet ready
 *
 */
/**
 * @todo to be fixed
 * @brief cannot be implemented. as the preparation for the command to read takes time,
 *        by the time the read command is sent, the busy flag has already cleared.
 */
LCD_HAL_I2C_Status_t Hd44780_ReadBusyFlag(Hd44780_Handle_t *hd44780_handle, uint8_t *pBF)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;
    uint8_t cmd = 0u;
    uint8_t rxData[2] = {0u};

    if (hd44780_handle == NULL)
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
        /* first write the instruction */
        cmd = 0x00u;                                                                      // no instructions, just read
        rxData[0] = LCD_HAL_I2C_MODE(cmd, HD44780_RW_READ, HD44780_RS_INSTRUCTION_REG) |  // send MSB
                    LCD_HAL_I2C_BACKLIGHT_PIN(hd44780_handle->_status._ledState) |        // retain user backlight
                    LCD_HAL_I2C_ENABLE_PIN(HD44780_ENABLE);                               // enable pin

        rxData[1] = LCD_HAL_I2C_MODE(cmd, HD44780_RW_READ, HD44780_RS_INSTRUCTION_REG) |  // send no data
                    LCD_HAL_I2C_BACKLIGHT_PIN(hd44780_handle->_status._ledState) |         // retain user backlight
                    LCD_HAL_I2C_ENABLE_PIN(HD44780_DISABLE);                               // disable EN pin

        status = LCD_HAL_I2C_SendCmd(hd44780_handle->i2cConfig, rxData, 2u);

        /* then read the pins */
        status = LCD_HAL_I2C_ReadData(hd44780_handle->i2cConfig, rxData, 1u);
        *pBF = HD44780_READ_BUSY_FLAG(rxData[0]);
    }
    return status;
}

LCD_HAL_I2C_Status_t Hd44780_ScanChar(Hd44780_Handle_t *hd44780_handle, char *charVal)
{
    LCD_HAL_I2C_Status_t status = LCD_HAL_I2C_ERROR;
    uint8_t i2c_data[4] = {0};
    uint8_t cmd = 0u;

    if (hd44780_handle == NULL)
    {
        status = LCD_HAL_I2C_ERROR;
    }
    else
    {
        i2c_data[0] = LCD_HAL_I2C_MODE((cmd << 4u), HD44780_RW_READ, HD44780_RS_DATA_REG)  |  // send MSB
                      LCD_HAL_I2C_BACKLIGHT_PIN(hd44780_handle->_status._ledState) |  // retain user backlight
                      LCD_HAL_I2C_ENABLE_PIN(HD44780_ENABLE);                         // enable pin

        i2c_data[1] = LCD_HAL_I2C_MODE(0u, HD44780_RW_READ, HD44780_RS_DATA_REG)   |  // send no data
                      LCD_HAL_I2C_BACKLIGHT_PIN(hd44780_handle->_status._ledState) |  // retain user backlight
                      LCD_HAL_I2C_ENABLE_PIN(HD44780_DISABLE);                        // disable EN pin

        i2c_data[0] = LCD_HAL_I2C_MODE(cmd, HD44780_RW_READ, HD44780_RS_DATA_REG)  |  // send MSB
                      LCD_HAL_I2C_BACKLIGHT_PIN(hd44780_handle->_status._ledState) |  // retain user backlight
                      LCD_HAL_I2C_ENABLE_PIN(HD44780_ENABLE);                         // enable pin

        i2c_data[1] = LCD_HAL_I2C_MODE(0u, HD44780_RW_READ, HD44780_RS_DATA_REG)   |  // send no data
                      LCD_HAL_I2C_BACKLIGHT_PIN(hd44780_handle->_status._ledState) |  // retain user backlight
                      LCD_HAL_I2C_ENABLE_PIN(HD44780_DISABLE);                        // disable EN pin
        // i2c_data[2] = LCD_HAL_I2C_MODE(cmd, )
        UNUSED(i2c_data);
    }
    
    return status;
}
