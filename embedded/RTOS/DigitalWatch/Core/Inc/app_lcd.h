#ifndef APP_LCD_H
#define APP_LCD_H

#include "main.h"

#define APPLCD_LCD_LINE_LEN                                 16
#define APPLCD_TIME_STRING_RESET                            "12:00:00 AM"
#define APPLCD_DATE_STRING_RESET                            "16/11/2024 SAT"
#define APPLCD_TIME_ALARM_STRING_RESET                      "6:00 AM"
#define APPLCD_DATE_ALARM_STRING_RESET                      "16/11/2024 SAT"
#define APPLCD_TIMER_STRING_RESET                           "00:00.0"

typedef struct
{
    char line1[APPLCD_LCD_LINE_LEN];
    char line2[APPLCD_LCD_LINE_LEN];
} AppLcd_Display_t;

void App_Lcd_Init(void);
void AppLcd_ToggleCursorBlink(BaseType_t state);
void AppLcd_EnableBacklight(BaseType_t state);
void AppLcd_ToggleBacklight(void);
void AppLcd_Select(App_SetModeState_t param);
void AppLcd_Print(AppLcd_Display_t *lines);

/**********************************************************************
 * LCD Calendar functions 
**********************************************************************/

void AppLcd_ResetClock(void);
void AppLcd_Update(App_SetModeState_t param, uint16_t value);

/**********************************************************************
* LCD Alarm functions
**********************************************************************/

void AppLcd_ResetAlarm(void);
void AppLcd_ActivateAlarm(BaseType_t status);

/**********************************************************************
 * LCD Stopwatch functions
**********************************************************************/

void AppLcd_ResetTimer(void);


#endif /* APP_LCD_H */