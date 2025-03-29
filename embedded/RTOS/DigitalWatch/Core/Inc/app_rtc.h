#ifndef APP_RTC_H
#define APP_RTC_H

#include "main.h"

#define APPRTC_TIMER_TICK_RATE                                  100        /* 1\/10 chronograph */
#define APPRTC_RTC_YEAR_RESET                                   2000

#define APPRTC_RTC_12HOUR_ABS_MASK(HOUR12)                      ((HOUR12) >> 8)
#define APPRTC_RTC_12HOUR_ABS(HOUR12)                           (((HOUR12) ^ APPRTC_RTC_12HOUR_ABS_MASK(HOUR12)) - APPRTC_RTC_12HOUR_ABS_MASK(HOUR12))

typedef struct
{
    uint32_t counter;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t subsec;
} AppRtc_Timer_t;

uint8_t AppRtc_12HrTo24Hr(int8_t hour12);
int8_t AppRtc_24HrTo12Hr(uint8_t hour24);
uint32_t ResyncTimers(void);

/**********************************************************************
 * RTC Calendar functions 
**********************************************************************/

void AppRtc_CalendarInit(void);
void AppRtc_ActivateRtc(BaseType_t status);
void AppRtc_IncTick(void);
void AppRtc_SetHourFormat(AppRtc_HourFormat_t format);
void AppRtc_SetCalendar(void);
void AppRtc_GetCalendar(char *timestr, char *datestr);
uint8_t AppRtc_GetRtc(App_SetModeState_t param);
void AppRtc_SetRtc(App_SetModeState_t param, uint8_t value);
void AppRtc_Update(App_SetModeState_t param, uint8_t *value);
void AppRtc_ToggleTimeFormat(AppRtc_HourFormat_t *timeFormat);
/* interrupt-context callback */
void vTimeSecCallback(void);

/**********************************************************************
* RTC Alarm functions
**********************************************************************/

void AppRtc_AlarmInit(void);
void AppRtc_ActivateAlarm(BaseType_t status);
void AppRtc_GetAlarm(char *timestr, char *datestr);
uint8_t AppRtc_GetRtcAlarm(App_SetModeState_t param);
void AppRtc_SetRtcAlarm(App_SetModeState_t param, uint8_t value);
void AppRtc_UpdateAlarm(App_SetModeState_t param, uint8_t *value);
/* interrupt-context callback */
void vAlarmCallback(void);

/**********************************************************************
 * Stopwatch timer functions
**********************************************************************/

void AppRtc_TimerInit(void);
void AppRtc_ResetTimer(void);
void AppRtc_IncTickTimer(void);
void AppRtc_GetTime(char *timestr);


#endif /* APP_RTC_H */