#include "app_lcd.h"
#include "i2c.h"
#include "hd44780.h"

#include "timers.h"

#define APPLCD_BELLCHAR_ID                          ((uint8_t)0u)

extern TimerHandle_t idlebreak_tmr;

static I2cConfig_t config_lcdi2c;
static Hd44780_Handle_t handle_lcd;
static const uint8_t bell_char[8] = {   0b00000,
                                        0b00100,
                                        0b01110,
                                        0b01110,
                                        0b01110,
                                        0b11111,
                                        0b00100,
                                        0b00000
                                    };

void App_Lcd_Init(void)
{
    config_lcdi2c.hi2c = &hi2c1;
    config_lcdi2c.lcd_addr = LCD_HAL_I2C_ADDRESS_DEFAULT;

    handle_lcd.i2cConfig = &config_lcdi2c;
    handle_lcd.init.cols = 16u;
    handle_lcd.init.rows = 2u;
    handle_lcd.init.num_dispLines = HD44780_FS_NDL_2_LINE;
    handle_lcd.init.char_font = HD44780_FS_FONT_5X8_DOTS;

    Hd44780_Init(&handle_lcd);
    Hd44780_ToggleBlink(&handle_lcd, HD44780_OFF);
    Hd44780_ToggleCursor(&handle_lcd, HD44780_OFF);

    // generate the bell character
    Hd44780_GenerateChar(&handle_lcd, APPLCD_BELLCHAR_ID, bell_char);

}

void AppLcd_ToggleCursorBlink(BaseType_t state)
{
    if (state)
    {
        Hd44780_ToggleBlink(&handle_lcd, HD44780_ON);
    }
    else
    {
        Hd44780_ToggleBlink(&handle_lcd, HD44780_OFF);
    }
}

void AppLcd_EnableBacklight(BaseType_t state)
{
    Hd44780_ToggleBacklight(&handle_lcd, (Hd44780_Toggle_t)state);
    if (state == pdTRUE)
    {
        xTimerReset(idlebreak_tmr, 0);
    }
    
}

void AppLcd_ToggleBacklight(void)
{
    if (handle_lcd._status._ledState != HD44780_ON)
    {
        /* if btnA pressed once, turn on backlight */
        Hd44780_ToggleBacklight(&handle_lcd, HD44780_ON);
        xTimerReset(idlebreak_tmr, portMAX_DELAY);
    }
    else
    {
        /* if btnA pressed again, turn off backlight */
        Hd44780_ToggleBacklight(&handle_lcd, HD44780_OFF);
    }
}

/* moves the cursor to the specified clock/date parameter */
void AppLcd_Select(App_SetModeState_t param)
{
    switch (param)
    {
    case APP_SETMODE_HOUR:
        Hd44780_MoveCursor(&handle_lcd, 1, 1);
        break;
    case APP_SETMODE_MIN:
        Hd44780_MoveCursor(&handle_lcd, 1, 4);
        break;
    case APP_SETMODE_SEC:
        Hd44780_MoveCursor(&handle_lcd, 1, 7);
        break;
    case APP_SETMODE_AMPM:
        Hd44780_MoveCursor(&handle_lcd, 1, 10);
        break;
    case APP_SETMODE_DATE:
        Hd44780_MoveCursor(&handle_lcd, 2, 1);
        break;
    case APP_SETMODE_MONTH:
        Hd44780_MoveCursor(&handle_lcd, 2, 4);
        break;
    case APP_SETMODE_YEAR:
        Hd44780_MoveCursor(&handle_lcd, 2, 7);
        break;
    case APP_SETMODE_WEEKDAY:
        Hd44780_MoveCursor(&handle_lcd, 2, 12);
        break;
    default:
        break;
    }
}

void AppLcd_Print(AppLcd_Display_t *lines)
{
    if (strlen(lines->line1) != 0)
    {
        Hd44780_MoveCursor(&handle_lcd, 1, 1);
        Hd44780_Print(&handle_lcd, lines->line1);
        Hd44780_MoveCursor(&handle_lcd, 1, (uint8_t)strlen(lines->line1));
    }
    
    if (strlen(lines->line2) != 0)
    {
        Hd44780_MoveCursor(&handle_lcd, 2, 1);
        Hd44780_Print(&handle_lcd, lines->line2);
        Hd44780_MoveCursor(&handle_lcd, 2, (uint8_t)strlen(lines->line2));
    }

}

/**********************************************************************
 * LCD Calendar functions 
**********************************************************************/

void AppLcd_ResetClock(void)
{
    AppLcd_Display_t lines;

    memset(&lines, 0, sizeof(lines));

    sprintf(lines.line1, APPLCD_TIME_STRING_RESET);
    sprintf(lines.line2, APPLCD_DATE_STRING_RESET);

    Hd44780_Clear(&handle_lcd);
    HAL_Delay(2);
    AppLcd_Print(&lines);
}

void AppLcd_Update(App_SetModeState_t param, uint16_t value)
{
    char newTime[17];
    
    if (param == APP_SETMODE_AMPM)
    {
        sprintf(newTime, (value == RTC_HOURFORMAT12_AM) ? "AM" :
                        ((value == RTC_HOURFORMAT12_PM) ? "PM" : "  "));
    }
    else if (param == APP_SETMODE_WEEKDAY)
    {
        sprintf(newTime, APPCOMM_RTC_WEEKDAY(value));
    }
    else if (param == APP_SETMODE_YEAR)
    {
        sprintf(newTime, "%04d", value);
    }
    else
    {
        sprintf(newTime, "%02d", value);
    }
    
    Hd44780_Print(&handle_lcd, newTime);
    AppLcd_Select(param);
}

/**********************************************************************
* LCD Alarm functions
**********************************************************************/

void AppLcd_ResetAlarm(void)
{
    AppLcd_Display_t lines;

    memset(&lines, 0, sizeof(lines));

    sprintf(lines.line1, APPLCD_TIME_ALARM_STRING_RESET);
    sprintf(lines.line2, APPLCD_DATE_ALARM_STRING_RESET);

    Hd44780_Clear(&handle_lcd);
    HAL_Delay(2);
    AppLcd_Print(&lines);
}

void AppLcd_ActivateAlarm(BaseType_t status)
{
    Hd44780_MoveCursor(&handle_lcd, 1, 16);
    if (status == pdTRUE)
    {
        Hd44780_PrintChar(&handle_lcd, APPLCD_BELLCHAR_ID);
    }
    else
    {
        Hd44780_PrintChar(&handle_lcd, ' ');
    }
    
}

/**********************************************************************
 * LCD Stopwatch functions
**********************************************************************/

void AppLcd_ResetTimer(void)
{
    AppLcd_Display_t lines;

    memset(&lines, 0, sizeof(lines));

    sprintf(lines.line1, APPLCD_TIMER_STRING_RESET);

    Hd44780_Clear(&handle_lcd);
    HAL_Delay(2);
    AppLcd_Print(&lines);
}

