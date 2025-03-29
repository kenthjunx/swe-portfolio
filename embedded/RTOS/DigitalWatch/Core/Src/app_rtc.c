#include "app_rtc.h"
#include "rtc.h"

#include "app_lcd.h"

#define APPRTC_SEC_DIV                      (configTICK_RATE_HZ / APPRTC_TIMER_TICK_RATE)   /* 1 sec = 100 ticks */
#define APPRTC_MIN_DIV                      (APPRTC_SEC_DIV * 60)                           /* 1 min = 6,000 ticks */
#define APPRTC_HR_DIV                       (APPRTC_MIN_DIV * 60)                           /* 1 hr = 360,000 ticks */

RTC_TimeTypeDef currTime;
RTC_DateTypeDef currDate;
AppRtc_Timer_t stopwatch;
RTC_AlarmTypeDef alarmA;
RTC_AlarmTypeDef alarmB;

static BaseType_t AppRtc_IsLeapYear(uint8_t year)
{
    BaseType_t isLeapYear = pdFALSE;
    
    if (year % 400 == 0)
        isLeapYear = pdTRUE;
    else if (year % 100 == 0)
        isLeapYear = pdFALSE;
    else if (year % 4 == 0)
        isLeapYear = pdTRUE;
    else
        isLeapYear = pdFALSE;
    
    return isLeapYear;
}
static uint8_t AppRtc_EvaluateMonth(uint8_t month, uint8_t year)
{
    uint8_t numdays = 0u;

    switch (RTC_ByteToBcd2(month))
    {
    case RTC_MONTH_FEBRUARY:
        numdays = AppRtc_IsLeapYear(year) ? 29u : 28u;
        break;
    case RTC_MONTH_JANUARY:
    case RTC_MONTH_MARCH:
    case RTC_MONTH_MAY:
    case RTC_MONTH_JULY:
    case RTC_MONTH_AUGUST:
    case RTC_MONTH_OCTOBER:
    case RTC_MONTH_DECEMBER:
        numdays = 31u;
        break;
    case RTC_MONTH_APRIL:
    case RTC_MONTH_JUNE:
    case RTC_MONTH_SEPTEMBER:
    case RTC_MONTH_NOVEMBER:
        numdays = 30u;
        break;
    default:
        break;
    }

    return numdays;
}
static uint8_t AppRtc_EvaluateDate(uint8_t date, uint8_t month, uint8_t year)
{
    uint8_t numdays = AppRtc_EvaluateMonth(month, year);

    return ((date >= 1u) && (date <= numdays)) ? date : (date % numdays);
}

/**
 * @brief converts 12hr clock to 24hr clock
 * 
 * @param hour12 clock in 12hr format. unsigned int (positive num) as AM, signed int (negative num) as PM
 * @return uint8_t clock in 24hr format. unsigned int (0..23)
 */
uint8_t AppRtc_12HrTo24Hr(int8_t hour12)
{
    uint8_t hour24 = 0u;

    // if hour12 == 12AM,    hour24 = 0
    // if hour12 == 1 to 11, hour24 = hour12
    // if hour12 == 12PM,    hour24 = 12
    // if hour12 == 1 to 11, hour24 = hour12 + 12
    
    if (hour12 == 12)
    {
        hour24 = 0u;
    }
    else if (hour12 == -12)
    {
        hour24 = 12;
    }
    else if (hour12 < 0)
    {
        hour24 = (unsigned)(~hour12 + 13u);
    }
    else
    {
        hour24 = hour12;
    }

    return hour24;
}

/**
 * @brief converts 24hr clock to 12hr clock
 * 
 * @param hour24 clock in 24hr format. unsigned int (0..23)
 * @return int8_t clock in 12hr format. unsigned int (positive num) as AM, signed int (negative num) as PM
 */
int8_t AppRtc_24HrTo12Hr(uint8_t hour24)
{
    int8_t hour12 = 0u;

    // if hour24 == 0, hour12 = 12, AM
    // if hour24 1 to 11, hour12 = hour24, AM
    // if hour24 == 12, hour12 = 12, PM
    // if hour24 13 to 23, hour12 = hour24 - 12, PM

    if (hour24 == 0u)
    {
        hour12 = (signed)(12);
    }
    else if (hour24 == 12u)
    {
        hour12 = (signed)(-12);
    }
    else if (hour24 > 12u)
    {
        hour12 = (signed)(~hour24 + 13u);
    }
    else
    {
        hour12 = (signed)hour24;
    }
    
    return hour12;
}

uint32_t ResyncTimers(void)
{
    // Get the current RTC time
    HAL_RTC_GetTime(&hrtc, &currTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &currDate, RTC_FORMAT_BIN);

    // Calculate milliseconds remaining until the next second
    return (1000u - currTime.SubSeconds);
}

/**********************************************************************
 * RTC Calendar functions 
**********************************************************************/

void AppRtc_SetHourFormat(AppRtc_HourFormat_t format)
{
    HAL_StatusTypeDef status;
    hrtc.Init.HourFormat = (format == APPRTC_HOURFORMAT_24) ? RTC_HOURFORMAT_24 : RTC_HOURFORMAT_12;

    status = HAL_RTC_Init(&hrtc);
    configASSERT(status == HAL_OK);
}

void AppRtc_SetCalendar(void)
{
    HAL_RTC_SetTime(&hrtc, &currTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &currDate, RTC_FORMAT_BIN);
}

void AppRtc_GetCalendar(char *timestr, char *datestr)
{
    // GetTime and GetDate must be called together
    HAL_RTC_GetTime(&hrtc, &currTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &currDate, RTC_FORMAT_BIN);
    
    char *format;
    format = (hrtc.Init.HourFormat == RTC_HOURFORMAT_24) ? "   " :
            ((currTime.TimeFormat == RTC_HOURFORMAT12_PM) ? "PM" : "AM");

    // HH:MM:SS
    sprintf(timestr, "%02d:%02d:%02d %s", currTime.Hours, currTime.Minutes, currTime.Seconds, format);
    // DD/MM/YYYY
    sprintf(datestr, "%02d/%02d/%04d %s", currDate.Date,
                                        currDate.Month,
                                        (currDate.Year + APPRTC_RTC_YEAR_RESET),
                                        APPCOMM_RTC_WEEKDAY(currDate.WeekDay));
}

uint8_t AppRtc_GetRtc(App_SetModeState_t param)
{
    uint8_t value = 0u;

    switch (param)
    {
    case APP_SETMODE_SEC:
        value = currTime.Seconds;
        break;
    case APP_SETMODE_MIN:
        value = currTime.Minutes;
        break;
    case APP_SETMODE_HOUR:
        value = currTime.Hours;
        break;
    case APP_SETMODE_AMPM:
        value = currTime.TimeFormat;
        break;
    case APP_SETMODE_WEEKDAY:
        value =currDate.WeekDay;
        break;
    case APP_SETMODE_DATE:
        value =currDate.Date;
        break;
    case APP_SETMODE_MONTH:
        value =currDate.Month;
        break;
    case APP_SETMODE_YEAR:
        value = currDate.Year;
        break;
    default:
        break;
    }

    return value;
}

void AppRtc_SetRtc(App_SetModeState_t param, uint8_t value)
{
    switch (param)
    {
    case APP_SETMODE_SEC:
        currTime.Seconds = value;
        break;
    case APP_SETMODE_MIN:
        currTime.Minutes = value;
        break;
    case APP_SETMODE_HOUR:
        currTime.Hours = value;
        break;
    case APP_SETMODE_AMPM:
        currTime.TimeFormat = value;
        break;
    case APP_SETMODE_DATE:
        currDate.Date = value;
        break;
    case APP_SETMODE_MONTH:
        currDate.Month = value;
        break;
    case APP_SETMODE_YEAR:
        currDate.Year = value;
        break;
    case APP_SETMODE_WEEKDAY:
        currDate.WeekDay = value;
        break;
    default:
        break;
    }
}

void AppRtc_Update(App_SetModeState_t param, uint8_t *value)
{
    switch (param)
    {
    case APP_SETMODE_SEC:
        *value = 0;
        break;
    case APP_SETMODE_MIN:
        *value = (*value + 1u) % 60;
        break;
    case APP_SETMODE_HOUR:
        *value = IS_RTC_HOUR24(*value + 1u) ? (*value + 1u) : 0u;
        break;
    case APP_SETMODE_AMPM:
        *value = IS_RTC_HOURFORMAT12(*value + 1u) ? (*value + 1u) : (*value % 2);
        break;
    case APP_SETMODE_WEEKDAY:
        *value = IS_RTC_WEEKDAY(*value + 1u) ? (*value + 1u) : ((*value % 7) + 1u);
        break;
    case APP_SETMODE_DATE:
        *value = AppRtc_EvaluateDate(*value + 1u, currDate.Month, currDate.Year + APPRTC_RTC_YEAR_RESET);
        break;
    case APP_SETMODE_MONTH:
        *value = IS_RTC_MONTH(*value + 1u) ? (*value + 1u) : ((*value % 12) + 1u);
        break;
    case APP_SETMODE_YEAR:
        *value = IS_RTC_YEAR(*value + 1u) ? (*value + 1u) : ((*value % 99) + 1u);
        break;
    default:
        break;
    }
}

void AppRtc_ToggleTimeFormat(AppRtc_HourFormat_t *timeFormat)
{
    int8_t hour12 = 0;
    uint8_t hour24 = 0u;

    if (*timeFormat == APPRTC_HOURFORMAT_12)
    {
        // convert from 12 hr to 24hr
        AppRtc_SetHourFormat(APPRTC_HOURFORMAT_24);
        *timeFormat = APPRTC_HOURFORMAT_24;
        
        hour12 = AppRtc_GetRtc(APP_SETMODE_HOUR);
        hour12 = ((AppRtc_GetRtc(APP_SETMODE_AMPM) == RTC_HOURFORMAT12_PM) ? (-hour12) : hour12);
        hour24 = AppRtc_12HrTo24Hr(hour12);

        AppRtc_SetRtc(APP_SETMODE_HOUR, hour24);
    }
    else /* (timeFormat == APPRTC_HOURFORMAT_24) */
    {
        // convert from 24hr to 12hr
        AppRtc_SetHourFormat(APPRTC_HOURFORMAT_12);
        *timeFormat = APPRTC_HOURFORMAT_12;

        hour24 = AppRtc_GetRtc(APP_SETMODE_HOUR);
        hour12 = AppRtc_24HrTo12Hr(hour24);

        AppRtc_SetRtc(APP_SETMODE_HOUR, APPRTC_RTC_12HOUR_ABS(hour12));
        AppRtc_SetRtc(APP_SETMODE_AMPM, (hour12 < 0) ? RTC_HOURFORMAT12_PM : RTC_HOURFORMAT12_AM);
    }
    
    // set the calendar values
    AppRtc_SetCalendar();
}

#ifdef ALARM_SEC
void AppRtc_ActivateRtc(BaseType_t status)
{
    if (status == pdTRUE)
    {
        /* update the current time and date */
        HAL_RTC_GetTime(&hrtc, &currTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &currDate, RTC_FORMAT_BIN);

        HAL_RTC_SetTime(&hrtc, &currTime, RTC_FORMAT_BIN);
        HAL_RTC_SetDate(&hrtc, &currDate, RTC_FORMAT_BIN);
        
        HAL_RTC_SetAlarm_IT(&hrtc, &alarmB, RTC_FORMAT_BIN);
    }
    else
    {
        HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_B);
    }
    
}

void AppRtc_IncTick(void)
{
}


void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *hrtc)
{
    // increment the alarm tick
    HAL_RTC_GetAlarm(hrtc, &alarmB, RTC_ALARM_B, RTC_FORMAT_BIN);
    alarmB.AlarmTime.Seconds = IS_RTC_SECONDS(alarmB.AlarmTime.Seconds + 1u) ? (alarmB.AlarmTime.Seconds + 1u) : 0u;
    // active the alarm
    if (HAL_RTC_SetAlarm_IT(hrtc, &alarmB, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
    vTimeSecCallback();
}

__weak void vTimeSecCallback(void)
{
    /* NOTE: This function should not be modified, when the callback is needed,
            the vTimeSecCallback could be implemented in the user file
    */
}
#endif

/**********************************************************************
* RTC Alarm functions
**********************************************************************/

/* make sure the alarm is deactivated first before activating it */
void AppRtc_ActivateAlarm(BaseType_t status)
{
    if (status == pdTRUE)
    {
        /* activate the alarm interrupt */
        HAL_RTC_SetAlarm_IT(&hrtc, &alarmA, RTC_FORMAT_BIN);
    }
    else
    {
        HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
    }
    
}

void AppRtc_GetAlarm(char *timestr, char *datestr)
{
    HAL_RTC_GetAlarm(&hrtc, &alarmA, RTC_ALARM_A, RTC_FORMAT_BIN);
    
    char *format;
    format = (alarmA.AlarmTime.TimeFormat == RTC_HOURFORMAT12_PM) ? "PM" : "AM";

    // HH:MM:SS
    sprintf(timestr, "%02d:%02d    %s", alarmA.AlarmTime.Hours, alarmA.AlarmTime.Minutes, format);
    // DD/MM/YYYY
    // sprintf(datestr, "%02d/%02d/%04d %s", currDate.Date,
    //                                     currDate.Month,
    //                                     (currDate.Year + APPRTC_RTC_YEAR_RESET),
    //                                     APPCOMM_RTC_WEEKDAY(currDate.WeekDay));
}

uint8_t AppRtc_GetRtcAlarm(App_SetModeState_t param)
{
    uint8_t value = 0u;

    switch (param)
    {
    case APP_SETMODE_MIN:
        value = alarmA.AlarmTime.Minutes;
        break;
    case APP_SETMODE_HOUR:
        value = alarmA.AlarmTime.Hours;
        break;
    case APP_SETMODE_AMPM:
        value = alarmA.AlarmTime.TimeFormat;
        break;
    default:
        break;
    }

    return value;
}

void AppRtc_SetRtcAlarm(App_SetModeState_t param, uint8_t value)
{
    switch (param)
    {
    case APP_SETMODE_MIN:
        alarmA.AlarmTime.Minutes = value;
        break;
    case APP_SETMODE_HOUR:
        alarmA.AlarmTime.Hours = value;
        break;
    case APP_SETMODE_AMPM:
        alarmA.AlarmTime.TimeFormat = value;
        break;
    default:
        break;
    }
}

void AppRtc_UpdateAlarm(App_SetModeState_t param, uint8_t *value)
{
    switch (param)
    {
    case APP_SETMODE_MIN:
        *value = IS_RTC_MINUTES(*value + 1u) ? (*value + 1u) : 0u;
        break;
    case APP_SETMODE_HOUR:
        *value = IS_RTC_HOUR24(*value + 1u) ? (*value + 1u) : 0u;
        break;
    case APP_SETMODE_AMPM:
        *value = IS_RTC_HOURFORMAT12(*value + 1u) ? (*value + 1u) : (*value % 2);
        break;
    default:
        break;
    }
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
    vAlarmCallback();
}

__weak void vAlarmCallback(void)
{
    /* NOTE: This function should not be modified, when the callback is needed,
            the vAlarmCallback could be implemented in the user file
    */
}

/**********************************************************************
 * Stopwatch timer functions
**********************************************************************/

void AppRtc_ResetTimer(void)
{
    stopwatch.counter = 0u;
    stopwatch.subsec  = 0u;
    stopwatch.sec = 0u;
    stopwatch.min = 0u;
}

void AppRtc_IncTickTimer(void)
{
    ++(stopwatch.counter);
    stopwatch.subsec  = (stopwatch.counter) % APPRTC_SEC_DIV;
    stopwatch.sec     = (stopwatch.counter  / APPRTC_SEC_DIV) % 60;
    stopwatch.min     = (stopwatch.counter  / APPRTC_MIN_DIV) % 60;
    stopwatch.hour    = (stopwatch.counter  / APPRTC_HR_DIV);
}

void AppRtc_GetTime(char *timestr)
{
    // HH:MM:SS:BB
    if (stopwatch.hour == 0u)
    {
        sprintf(timestr, "%02d:%02d.%01d", stopwatch.min, stopwatch.sec, stopwatch.subsec);
    }
    else
    {
        sprintf(timestr, "%02d:%02d:%02d.%01d", stopwatch.hour, stopwatch.min, stopwatch.sec, stopwatch.subsec);
    }
    
}

