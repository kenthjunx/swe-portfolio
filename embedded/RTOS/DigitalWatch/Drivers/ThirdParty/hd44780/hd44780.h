#ifndef HD44780_H
#define HD44780_H

#ifdef USE_LCD_HAL_I2C
#include "lcd_hal_i2c.h"
#elif defined USE_LCD_HAL_GPIO
#include "lcd_hal_gpio.h"
#else
#error "You need to have either the GPIO or I2C HAL libraries to use this library."
#endif

typedef enum __attribute__((packed))
{
    HD44780_OFF = 0x00u,
    HD44780_ON = 0x01u
} Hd44780_Toggle_t;

typedef enum __attribute__((packed))
{
    HD44780_LEFT = 0x00u,
    HD44780_RIGHT,
    HD44780_RTL,
    HD44780_LTR,
} Hd44780_Direction_t;

typedef struct
{
    uint8_t num_dispLines;
    uint8_t char_font;
    uint8_t rows;
    uint8_t cols;
} Hd44780_Init_t;

typedef struct
{
    uint8_t _ledState;
    uint8_t _funcSet;
    uint8_t _dispCtrl;
    uint8_t _ems;
} Hd44780_IntlStats_t;

typedef struct
{
#ifdef USE_LCD_HAL_I2C
    void *i2cConfig;
#endif
    Hd44780_Init_t init;
    Hd44780_IntlStats_t _status;                        /* private members! should not be modified.
                                                        please allocate memory, either dynamically or statically */
} Hd44780_Handle_t;

#define HD44780_CLEAR_DISPLAY                           (uint8_t)0x01u

#define HD44780_RETURN_HOME                             (uint8_t)0x02u

#define HD44780_EMS_SHIFT_ENABLE                        0x01u
#define HD44780_EMS_SHIFT_DISABLE                       0x00u
#define HD44780_EMS_S_MASK                              0x1u
#define HD44780_EMS_S_POS                               0u
#define HD44780_EMS_ID_INC                              0x01u
#define HD44780_EMS_ID_DEC                              0x00u
#define HD44780_EMS_ID_MASK                             0x2u
#define HD44780_EMS_ID_POS                              1u
#define HD44780_ENTRY_MODE_SET(ID, S)                   (uint8_t)((uint8_t)0x04u | ((uint8_t)(ID) << HD44780_EMS_ID_POS) | \
                                                                 ((uint8_t)(S) << HD44780_EMS_S_POS))

#define HD44780_DISPCTRL_DISP_ON                        0x01u
#define HD44780_DISPCTRL_DISP_OFF                       0x00u
#define HD44780_DISPCTRL_D_MASK                         0x04u
#define HD44780_DISPCTRL_D_POS                          2u
#define HD44780_DISPCTRL_CURS_ON                        0x01u
#define HD44780_DISPCTRL_CURS_OFF                       0x00u
#define HD44780_DISPCTRL_C_MASK                         0x02u
#define HD44780_DISPCTRL_C_POS                          1u
#define HD44780_DISPCTRL_BLINK_ON                       0x01u
#define HD44780_DISPCTRL_BLINK_OFF                      0x00u
#define HD44780_DISPCTRL_B_MASK                         0x01u
#define HD44780_DISPCTRL_B_POS                          0u
#define HD44780_DISP_CTRL(D, C, B)                      (uint8_t)((uint8_t)0x08u | ((uint8_t)(D) << HD44780_DISPCTRL_D_POS) | \
                                                                 ((uint8_t)(C) << HD44780_DISPCTRL_C_POS) | \
                                                                 ((uint8_t)(B) << HD44780_DISPCTRL_B_POS))

#define HD44780_CDS_DISPLAY_SHIFT                       0x01u
#define HD44780_CDS_CURSOR_MOVE                         0x00u
#define HD44780_CDS_SC_POS                              3u
#define HD44780_CDS_SHIFT_RIGHT                         0x01u
#define HD44780_CDS_SHIFT_LEFT                          0x00u
#define HD44780_CDS_RL_POS                              2u
#define HD44780_CURSOR_OR_DISP_SHIFT(SC, RL)            (uint8_t)((uint8_t)0x10u | ((uint8_t)(SC) << HD44780_CDS_SC_POS) | \
                                                                 ((uint8_t)(RL) << HD44780_CDS_RL_POS))

#define HD44780_FS_IFDL_8BIT                            0x01u
#define HD44780_FS_IFDL_4BIT                            0x00u
#define HD44780_FS_DL_POS                               4u
#define HD44780_FS_NDL_2_LINE                           0x01u
#define HD44780_FS_NDL_1_LINE                           0x00u
#define HD44780_FS_N_POS                                3u
#define HD44780_FS_FONT_5X10_DOTS                       0x01u
#define HD44780_FS_FONT_5X8_DOTS                        0x00u
#define HD44780_FS_F_POS                                2u
#define HD44780_FUNCTION_SET(DL, N, F)                  (uint8_t)((uint8_t)0x20u | ((uint8_t)(DL) << HD44780_FS_DL_POS) | \
                                                                 ((uint8_t)(N) << HD44780_FS_N_POS) | \
                                                                 ((uint8_t)(F) << HD44780_FS_F_POS))

#define HD44780_SET_ACG_MASK                            0x3Fu
#define HD44780_SET_ACG_POS                             0u
#define HD44780_SET_CGRAM_ADDR(ACG)                     (uint8_t)((uint8_t)0x40u | \
                                                                (((uint8_t)(ACG)) & HD44780_SET_ACG_MASK) << HD44780_SET_ACG_POS)

#define HD44780_SET_ADD_MASK                            0x7Fu
#define HD44780_SET_ADD_POS                             0u
#define HD44780_SET_DDRAM_ADDR(ADD)                     (uint8_t)((uint8_t)0x80u | \
                                                                (((uint8_t)(ADD) & HD44780_SET_ADD_MASK) << HD44780_SET_ADD_POS))

#define HD44780_BUSY_FLAG_MASK                          0x80u
#define HD44780_BUSY_FLAG_POS                           7u
#define HD44780_ADDR_CNTR_MASK                          0x7Fu
#define HD44780_ADDR_CNTR_POS                           0u
#define HD44780_READ_BUSY_FLAG(DATA)                    (((uint8_t)(DATA) & HD44780_BUSY_FLAG_MASK) >> HD44780_BUSY_FLAG_POS)
#define HD44780_READ_ADDR_CNTR(DATA)                    (((uint8_t)(DATA) & HD44780_ADDR_CNTR_MASK) >> HD44780_ADDR_CNTR_POS)


#define HD44780_RS_DATA_REG                             0x01u
#define HD44780_RS_INSTRUCTION_REG                      0x00u

#define HD44780_RW_READ                                 0x01u
#define HD44780_RW_WRITE                                0x00u

#define HD44780_BACKLIGHT_ON                            0x01u
#define HD44780_BACKLIGHT_OFF                           0x00u

#define HD44780_ENABLE                                  0x01u
#define HD44780_DISABLE                                 0x00u

LCD_HAL_I2C_Status_t Hd44780_Init(Hd44780_Handle_t *hd44780_handle);
LCD_HAL_I2C_Status_t Hd44780_Clear(Hd44780_Handle_t *hd44780_handle);
LCD_HAL_I2C_Status_t Hd44780_Return(Hd44780_Handle_t *hd44780_handle);
LCD_HAL_I2C_Status_t Hd44780_ScrollDisplay(Hd44780_Handle_t *hd44780_handle, const Hd44780_Direction_t direction);
LCD_HAL_I2C_Status_t Hd44780_ToggleBlink(Hd44780_Handle_t *hd44780_handle, const Hd44780_Toggle_t toggle);
LCD_HAL_I2C_Status_t Hd44780_ToggleDisplay(Hd44780_Handle_t *hd44780_handle, const Hd44780_Toggle_t toggle);
LCD_HAL_I2C_Status_t Hd44780_ToggleCursor(Hd44780_Handle_t *hd44780_handle, const Hd44780_Toggle_t toggle);
LCD_HAL_I2C_Status_t Hd44780_MoveCursor(Hd44780_Handle_t *hd44780_handle, const uint8_t row, const uint8_t col);
LCD_HAL_I2C_Status_t Hd44780_ToggleBacklight(Hd44780_Handle_t *hd44780_handle, const Hd44780_Toggle_t toggle);
LCD_HAL_I2C_Status_t Hd44780_TextDirection(Hd44780_Handle_t *hd44780_handle, const Hd44780_Direction_t direction);
LCD_HAL_I2C_Status_t Hd44780_AutoScroll(Hd44780_Handle_t *hd44780_handle, const Hd44780_Toggle_t toggle);
LCD_HAL_I2C_Status_t Hd44780_Print(Hd44780_Handle_t *hd44780_handle, const char *str);
LCD_HAL_I2C_Status_t Hd44780_PrintChar(Hd44780_Handle_t *hd44780_handle, const char charVal);
LCD_HAL_I2C_Status_t Hd44780_GenerateChar(Hd44780_Handle_t *hd44780_handle, const uint8_t addr, const uint8_t *charmap);

LCD_HAL_I2C_Status_t Hd44780_PrepareCmd(Hd44780_Handle_t *hd44780_handle, const uint8_t cmd, const uint8_t regSel);

#endif /* HD44780_H */