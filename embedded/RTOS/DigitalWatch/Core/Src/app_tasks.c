#include "app_rtc.h"
#include "app_lcd.h"
#include "buttons.h"
#include "buzzer.h"
#include "usart.h"
#include "timers.h"
#include "queue.h"


/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

#define APP_CLOCK_TIMER_ID                                      ((uint32_t)0u)
#define APP_STOPWATCH_TIMER_ID                                  ((uint32_t)1u)
#define APP_ALARM_TIMER_ID                                      ((uint32_t)2u)
#define APP_IDLEBREAK_TIMER_ID                                  ((uint32_t)0u)
#define APP_ALARMTIMEOUT_TIMER_ID                               ((uint32_t)1u)

#define APP_TASKDELAY_TICKS                                     10
#define APP_DEBOUNCE_TICKS                                      pdMS_TO_TICKS(5)        // 5ms debounce interval
#define APP_HOLD_TICKS                                          pdMS_TO_TICKS(1000)      // 500ms hold duration
#define APP_HOLD_TIMEOUT_TICKS                                  pdMS_TO_TICKS(1000)     // 1 second timeout

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* initialize app state */
App_State_t appState =                      { .mode = APP_MODE_IDLE,
                                              .alarmMode.mode = APP_MODE_SETTING,
                                              .alarmMode.set = APP_SETMODE_MIN,
                                              .alarmMode.isAlarmExpired = pdFALSE,
                                            };
App_TmrStatus_t appTimerStatus;

App_ButtonState_t btnAPressed;
App_ButtonState_t btnBPressed;
App_ButtonState_t btnCPressed;
App_ButtonState_t btnDPressed;

TaskHandle_t handle_idlemode_task;
TaskHandle_t handle_alarmmode_task;
TaskHandle_t handle_runmode_task;
TaskHandle_t handle_setmode_task;
TaskHandle_t handle_curr_task;
TaskHandle_t uartprint_task;
TaskHandle_t lcdprint_task;

TimerHandle_t handle_clock_tmr;
TimerHandle_t handle_stopwatch_tmr;
TimerHandle_t idlebreak_tmr;
TimerHandle_t alarmtimeout_tmr;

QueueHandle_t uartprint_q;
QueueHandle_t lcdprint_q;

/* Private function prototypes -----------------------------------------------*/

static void PrepareLcdPrint(uint32_t timerId);

static void SetHourAlarm(void);
static void ExitAlarmSetMode(char *mssg);
static void AlarmMode_Set(void);

static void DismissAlarm(void);
static void SnoozeAlarm(BaseType_t isHourlySnooze);
static void ExitAlarmMode(char *mssg);
static void AlarmMode_Alarm(void);

void AppTasks_IdleMode(void *params);
void AppTasks_AlarmMode(void *params);
void AppTasks_RunMode(void *params);
void AppTasks_SetMode(void *params);

void AppTasks_UartPrint(void *params);
void AppTasks_LcdPrint(void *params);

void vTimerCallback(TimerHandle_t xTimer);
void vGenericTimerCallback(TimerHandle_t xTimer);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief initialize tasks, timers, queues, etc
 * 
 */
void App_Tasks_Init(void)
{
    BaseType_t status = pdFAIL;

    status = xTaskCreate(AppTasks_IdleMode, "idlemode_task", 200, NULL, 1, &handle_idlemode_task);
    configASSERT(status == pdPASS);

    status = xTaskCreate(AppTasks_AlarmMode, "alarmmode_task", 200, NULL, 2, &handle_alarmmode_task);
    configASSERT(status == pdPASS);

    status = xTaskCreate(AppTasks_RunMode, "runmode_task", 200, NULL, 2, &handle_runmode_task);
    configASSERT(status == pdPASS);

    status = xTaskCreate(AppTasks_SetMode, "setmode_task", 200, NULL, 2, &handle_setmode_task);
    configASSERT(status == pdPASS);

    status = xTaskCreate(AppTasks_UartPrint, "uart_print", 200, NULL, 3, &uartprint_task);
    configASSERT(status == pdPASS);

    status = xTaskCreate(AppTasks_LcdPrint, "lcd_print", 200, NULL, 3, &lcdprint_task);
    configASSERT(status == pdPASS);

    handle_clock_tmr = xTimerCreate("clock_timer", pdMS_TO_TICKS(1000), pdTRUE, (void *)APP_CLOCK_TIMER_ID, vTimerCallback);
    configASSERT(handle_clock_tmr != NULL);

    handle_stopwatch_tmr = xTimerCreate("stopwatch_timer", pdMS_TO_TICKS(APPRTC_TIMER_TICK_RATE), pdTRUE, (void *)APP_STOPWATCH_TIMER_ID, vTimerCallback);
    configASSERT(handle_stopwatch_tmr != NULL);
    
    idlebreak_tmr = xTimerCreate("idle_break_tmr", pdMS_TO_TICKS(60000), pdFALSE, (void *)APP_IDLEBREAK_TIMER_ID, vGenericTimerCallback);
    configASSERT(idlebreak_tmr != NULL);

    alarmtimeout_tmr = xTimerCreate("alarmtimeout_tmr", pdMS_TO_TICKS(20000), pdFALSE, (void *)APP_ALARMTIMEOUT_TIMER_ID, vGenericTimerCallback);
    configASSERT(alarmtimeout_tmr != NULL);

    uartprint_q = xQueueCreate(5, sizeof(char *));
    configASSERT(uartprint_q != NULL);

    lcdprint_q = xQueueCreate(2, sizeof(AppLcd_Display_t *));
    configASSERT(lcdprint_q != NULL);

    btnAPressed.GPIOx = BTN_A_GPIO_Port;
    btnAPressed.GPIO_Pin = BTN_A_Pin;

    btnBPressed.GPIOx = BTN_B_GPIO_Port;
    btnBPressed.GPIO_Pin = BTN_B_Pin;

    btnCPressed.GPIOx = BTN_C_GPIO_Port;
    btnCPressed.GPIO_Pin = BTN_C_Pin;

    btnDPressed.GPIOx = BTN_D_GPIO_Port;
    btnDPressed.GPIO_Pin = BTN_D_Pin;

    char *mssg;
    mssg = "idle mode\n";
    xQueueSend(uartprint_q, &mssg, portMAX_DELAY);

    // start idle task immediately
    xTaskNotify(handle_idlemode_task, 0, eNoAction);
    // display date and time
    // start timer as soon as possible
    xTimerStart(handle_clock_tmr, portMAX_DELAY);
    xTimerStart(idlebreak_tmr, portMAX_DELAY);
}

/* task-context getTime */
static void PrepareLcdPrint(uint32_t timerId)
{
    AppLcd_Display_t *timedatestr = pvPortMalloc(sizeof(AppLcd_Display_t));

    if (timedatestr == NULL) return;    // memory full: do not print

    switch (timerId)
    {
    case APP_CLOCK_TIMER_ID:                /* clock timer. update display every second */
    {
        // notify display task to print time and date
        AppRtc_GetCalendar(timedatestr->line1, timedatestr->line2);
        break;
    }
    case APP_STOPWATCH_TIMER_ID:            /* stopwatch timer. update display every 100ms */
    {
        AppRtc_IncTickTimer();
        AppRtc_GetTime(timedatestr->line1);
        memset(timedatestr->line2, 0, sizeof(timedatestr->line2));        // no need to print the date
        break;
    }
    case APP_ALARM_TIMER_ID:                /* alarm print */
    {
        AppRtc_GetAlarm(timedatestr->line1, timedatestr->line2);
        memset(timedatestr->line2, 0, sizeof(timedatestr->line2));        // no need to print the date
        break;
    }
    default:
    {
        /* nohing to do here */
        vPortFree(timedatestr);
        return;
    }
    }

    if (xQueueSend(lcdprint_q, &timedatestr, 0) != pdPASS)
    {
        vPortFree(timedatestr);
    }
}

static void SetHourAlarm(void)
{
    uint8_t ampm = 0u;
    int8_t hour12 = 0;

    configASSERT((appState.mode == APP_MODE_ALARM) && (appState.alarmMode.mode == APP_MODE_SETTING));

    /* for alarm mode, hour format is always 12hr */
    /* if 12hr format, convert currTime to 24hr for easy counting */
    ampm = AppRtc_GetRtcAlarm(APP_SETMODE_AMPM);
    hour12 = (signed)((ampm == RTC_HOURFORMAT12_PM) ? (-btnCPressed.pressCount) : btnCPressed.pressCount);
    btnCPressed.pressCount = AppRtc_12HrTo24Hr(hour12);

    /* update the param value */
    AppRtc_UpdateAlarm(APP_SETMODE_HOUR, &(btnCPressed.pressCount));
    ampm = (((btnCPressed.pressCount) >= 0u) &&
            ((btnCPressed.pressCount) < 12u)) ? RTC_HOURFORMAT12_AM : RTC_HOURFORMAT12_PM;

    /* after updating the cntr val, convert back to 12hr format for calendar setting and display */
    hour12 = AppRtc_24HrTo12Hr(btnCPressed.pressCount);
    btnCPressed.pressCount = APPRTC_RTC_12HOUR_ABS(hour12);

    /* update the screen. update first the am/pm */
    AppLcd_Select(APP_SETMODE_AMPM);
    AppLcd_Update(APP_SETMODE_AMPM, ampm);
    AppRtc_SetRtcAlarm(APP_SETMODE_AMPM, ampm);

    /* update next the hr */
    AppLcd_Select(APP_SETMODE_HOUR);

    /* update the param value */
    AppRtc_SetRtcAlarm(APP_SETMODE_HOUR, btnCPressed.pressCount);
    /* update the screen */
    AppLcd_Update(APP_SETMODE_HOUR, btnCPressed.pressCount);
}

static void ExitAlarmSetMode(char *mssg)
{
    configASSERT((appState.mode == APP_MODE_ALARM) && (appState.alarmMode.mode == APP_MODE_SETTING));

    appState.mode = APP_MODE_IDLE;
    appState.alarmMode.set = APP_SETMODE_MIN;
    /* reset btn C/D counter */
    btnCPressed.pressCount = 0u;
    btnDPressed.pressCount = 0u;

    /* turn off blinking cursor */
    AppLcd_ToggleCursorBlink(pdFALSE);
    /* set the alarm values */
    AppRtc_ActivateAlarm(pdTRUE);
    AppLcd_ActivateAlarm(pdTRUE);

    /* start rtc clock printing Fin */
    xTimerStart(handle_clock_tmr, portMAX_DELAY);

    xTaskAbortDelay(handle_idlemode_task);
    xQueueSend(uartprint_q, &mssg, portMAX_DELAY);
}

static void AlarmMode_Set(void)
{
    char *mssg;
    TickType_t elapsedTime = 0u;

    /* turn on blinking cursor */
    AppLcd_ToggleCursorBlink(pdTRUE);
    appState.alarmMode.set = APP_SETMODE_MIN;
    AppLcd_Select(appState.alarmMode.set);
    mssg = "adjusting alarm minutes\n";
    xQueueSend(uartprint_q, &mssg, portMAX_DELAY);

    while ((appState.mode == APP_MODE_ALARM) && (appState.alarmMode.mode == APP_MODE_SETTING))
    {

        if (PollForBtnPress(&btnAPressed, pdFALSE, APP_DEBOUNCE_TICKS))
        {
            AppLcd_ToggleBacklight();
        }

        if (PollForBtnPress(&btnBPressed, pdFALSE, APP_DEBOUNCE_TICKS))
        {
            AppLcd_EnableBacklight(pdTRUE);

            ExitAlarmSetMode("idle mode\n");
        }

        elapsedTime = CountElapsedBtnPress(&btnCPressed, APP_DEBOUNCE_TICKS, APP_HOLD_TICKS);
        if (elapsedTime >= APP_HOLD_TICKS)                          /* long press c key: hourly snooze */
        {
            AppLcd_EnableBacklight(pdTRUE);

            mssg = "alarm every hour\n";
            xQueueSend(uartprint_q, &mssg, portMAX_DELAY);

            /* display alarm bell */
            AppLcd_ActivateAlarm(pdTRUE);

            /* set alarm for every hr */
            appState.alarmMode.isHourlySnooze = pdTRUE;
            appState.alarmMode.set = APP_SETMODE_HOUR;

            /* get the current value of param selected */
            AppLcd_Select(appState.alarmMode.set);
            btnCPressed.pressCount = AppRtc_GetRtcAlarm(appState.alarmMode.set);

            /* set hours alarm */
            SetHourAlarm();

            /* set mins alarm to zero (mins alarm is ignored) */
            /* update the param value */
            AppRtc_SetRtcAlarm(APP_SETMODE_MIN, 0u);
            /* update the screen */
            AppLcd_Select(APP_SETMODE_MIN);
            AppLcd_Update(APP_SETMODE_MIN, 0u);

            ExitAlarmSetMode("idle mode\n");
            /* reset btnC state */
            btnCPressed._state = 0u;
            /* stay here until user releases the button */
            while (HAL_GPIO_ReadPin(btnCPressed.GPIOx, btnCPressed.GPIO_Pin));
        }
        else if (elapsedTime >= APP_DEBOUNCE_TICKS)                 /* short press c key: set alarm params */
        {
            AppLcd_EnableBacklight(pdTRUE);
            appState.alarmMode.isHourlySnooze = pdFALSE;

            /* if setMode is SETMODE_HOUR, update the 12 hour format (am/pm) */
            if (appState.alarmMode.set == APP_SETMODE_HOUR)
            {
                /* set hours alarm */
                SetHourAlarm();
            }
            else    /* (appState.alarmMode.set == APP_SETMODE_MIN) */
            {
                /* increment values (hr/min) */
                AppRtc_UpdateAlarm(appState.alarmMode.set, &(btnCPressed.pressCount));
                /* update the param value */
                AppRtc_SetRtcAlarm(appState.alarmMode.set, btnCPressed.pressCount);
                /* update the screen */
                AppLcd_Update(appState.alarmMode.set, btnCPressed.pressCount);
            }

        }
        else
        {
            /* nothing to do here */
        }
        
        if (PollForBtnPress(&btnDPressed, pdFALSE, APP_DEBOUNCE_TICKS))
        {
            AppLcd_EnableBacklight(pdTRUE);

            btnDPressed.pressCount = (btnDPressed.pressCount + 1u) % 2u;
            switch (btnDPressed.pressCount)
            {
            case 1:         /* flash hr */
                mssg = "adjusting alarm hour\n";
                appState.alarmMode.set = APP_SETMODE_HOUR;
                break;
            default:        /* flash min */
                mssg = "adjusting alarm minute\n";
                appState.alarmMode.set = APP_SETMODE_MIN;
                break;
            }

            xQueueSend(uartprint_q, &mssg, portMAX_DELAY);
            AppLcd_Select(appState.alarmMode.set);
            /* get the current value of param selected */
            btnCPressed.pressCount = AppRtc_GetRtcAlarm(appState.alarmMode.set);
        }

        /* allow other tasks to run */
        vTaskDelay(APP_TASKDELAY_TICKS);
    }

}

static void DismissAlarm(void)
{
    configASSERT(appState.mode == APP_MODE_ALARM);

    /* stop the alarm and buzzer regardless whether timeout expired or not */
    xTimerStop(alarmtimeout_tmr, portMAX_DELAY);
    AppRtc_ActivateAlarm(pdFALSE);
    AppLcd_ActivateAlarm(pdFALSE);

    appState.alarmMode.mode = APP_MODE_SETTING;
    appState.alarmMode.isAlarmExpired = pdFALSE;
    appState.alarmMode.isHourlySnooze = pdFALSE;
}

static void SnoozeAlarm(BaseType_t isHourlySnooze)
{
    uint8_t snooze = 0u;

    configASSERT((appState.mode == APP_MODE_ALARM) && (appState.alarmMode.mode == APP_MODE_ALARM));

    DismissAlarm();
    PrepareLcdPrint(APP_CLOCK_TIMER_ID);

    /* start snooze function */
    if (isHourlySnooze)
    {
        appState.alarmMode.isHourlySnooze = pdTRUE;
        /* snooze every hour */
        snooze = AppRtc_GetRtcAlarm(APP_SETMODE_HOUR);
        AppRtc_UpdateAlarm(APP_SETMODE_HOUR, &snooze);
        /* set the alarm values */
        AppRtc_SetRtcAlarm(APP_SETMODE_HOUR, snooze);
        AppRtc_SetRtcAlarm(APP_SETMODE_MIN, 0u);        /* ignore minutes alarm */
    }
    else
    {
        /* alarm again after 5 mins */
        snooze = AppRtc_GetRtcAlarm(APP_SETMODE_MIN) + 4u;
        AppRtc_UpdateAlarm(APP_SETMODE_MIN, &snooze);
        /* set the alarm values */
        AppRtc_SetRtcAlarm(APP_SETMODE_MIN, snooze);
    }

    AppRtc_ActivateAlarm(pdTRUE);
    AppLcd_ActivateAlarm(pdTRUE);
}

static void ExitAlarmMode(char *mssg)
{
    configASSERT(appState.mode == APP_MODE_ALARM);

    xQueueSend(uartprint_q, &mssg, portMAX_DELAY);

    ActivateBuzzer(pdFALSE);
    /**
     * @todo change appState based on curr_task
     * 
     */
    appState.mode = APP_MODE_IDLE;
    xTimerStart(handle_clock_tmr, portMAX_DELAY);            // start rtc clock printing
    xTaskNotify(handle_idlemode_task, 0, eNoAction);
}

static void AlarmMode_Alarm(void)
{
    PrepareLcdPrint(APP_ALARM_TIMER_ID);
    xTimerStop(handle_clock_tmr, portMAX_DELAY);
    /* lcd always on while alarm */
    AppLcd_EnableBacklight(pdTRUE);
    /* turn on alarm buzzer */
    ActivateBuzzer(pdTRUE);
    /* start ticking for alarm timeout. 20sec/tick */
    xTimerStart(alarmtimeout_tmr, portMAX_DELAY);

    /**
     * @todo from alarm ISR:
     * while alarm is ringing, only the buzzer is active, the current mode (idle/run/set) is still active.
     * can only stop the alarm once you exit the current mode
     */
    while ((appState.mode == APP_MODE_ALARM) && (appState.alarmMode.mode == APP_MODE_ALARM))
    {
        if (PollForBtnPress(&btnBPressed, pdFALSE, APP_DEBOUNCE_TICKS))             /* b key to snooze the alarm */
        {
            SnoozeAlarm(appState.alarmMode.isHourlySnooze);

            ExitAlarmMode("alarm snoozed\n");
        }

        if (appState.alarmMode.isHourlySnooze == pdTRUE)
        {
            if (CountElapsedBtnPress(&btnCPressed, APP_DEBOUNCE_TICKS, APP_HOLD_TICKS))
            {
                DismissAlarm();

                ExitAlarmMode("alarm dismissed\n");
                /* reset btnC state */
                btnCPressed._state = 0u;
                /* stay here until user releases the button */
                while (HAL_GPIO_ReadPin(btnCPressed.GPIOx, btnCPressed.GPIO_Pin));
            }
            
        }
        else /* (appState.alarmMode.isHourlySnooze == pdFALSE) */
        {
            if (PollForBtnPress(&btnCPressed, pdFALSE, APP_DEBOUNCE_TICKS) ||        /* c key to stop the alarm */
                PollForBtnPress(&btnDPressed, pdFALSE, APP_DEBOUNCE_TICKS))          /* d key to stop the alarm */
            {
                DismissAlarm();

                ExitAlarmMode("alarm dismissed\n");
            }
        }

        if (appState.alarmMode.isAlarmExpired == pdTRUE)
        {
            SnoozeAlarm(appState.alarmMode.isHourlySnooze);

            ExitAlarmMode("alarm snoozed\n");
        }

        vTaskDelay(APP_TASKDELAY_TICKS);
    }
}

/*****************************************************************************
 * Task Handlers
 *****************************************************************************/

/******************************** print tasks ********************************/

void AppTasks_UartPrint(void *params)
{
    char *txdata;
    while (1)
    {
        xQueueReceive(uartprint_q, &txdata, portMAX_DELAY);
        HAL_UART_Transmit(&huart1, (uint8_t *)txdata, (uint16_t)strlen(txdata), HAL_MAX_DELAY);
    }
}

void AppTasks_LcdPrint(void *params)
{
    // char *timestr, *datestr;
    AppLcd_Display_t *lines;

    // memset(&lines, 0, sizeof(lines));

    while (1)
    {
        // xQueueReceive(lcdprint_q, &timestr, portMAX_DELAY);
        // xQueueReceive(lcdprint_q, &datestr, portMAX_DELAY);

        // strncpy(lines.line1, timestr, sizeof(lines.line1));
        // strncpy(lines.line2, datestr, sizeof(lines.line2));

        xQueueReceive(lcdprint_q, &lines, portMAX_DELAY);

        AppLcd_Print(lines);

        vPortFree(lines);
    }

}

/***************************** App Modes tasks *******************************/

void AppTasks_IdleMode(void *params)
{
    char *mssg;

    App_Lcd_Init();
    AppLcd_ResetClock();

    while (xTaskNotifyWait(0, 0, NULL, portMAX_DELAY))
    {
    while (appState.mode == APP_MODE_IDLE)
    {
        /* poll for any button presses */
        if (PollForBtnPress(&btnAPressed, pdFALSE, APP_DEBOUNCE_TICKS) == pdTRUE)
        {
            AppLcd_ToggleBacklight();
        }

        if (CountShortBtnPress(&btnBPressed, APP_DEBOUNCE_TICKS, pdMS_TO_TICKS(600)))
        {
            AppLcd_EnableBacklight(pdTRUE);

            // notify corresponding task
            switch (btnBPressed.pressCount)
            {
            case 1:
            {
                appState.mode = APP_MODE_ALARM;
                appState.alarmMode.mode = APP_MODE_SETTING;
                appState.alarmMode.set = APP_SETMODE_MIN;
                mssg = "alarm settings mode\n";
                handle_curr_task = handle_alarmmode_task;
                PrepareLcdPrint(APP_ALARM_TIMER_ID);
                break;
            }
            case 2:
            {
                appState.mode = APP_MODE_RUNNING;
                mssg = "running mode\n";
                handle_curr_task = handle_runmode_task;
                break;
            }
            case 3:
            {
                appState.mode = APP_MODE_SETTING;
                mssg = "settings mode\n";
                handle_curr_task = handle_setmode_task;
                break;
            }
            default:
                break;
            }

            // ignore if more than 3 presses
            if (handle_curr_task != NULL)
            {
                // polling for response from menu expired. reset the count
                btnBPressed.pressCount = 0u;

                xQueueSend(uartprint_q, &mssg, portMAX_DELAY);

                // pause printing of time and date. rtc should still be running, just not printing
                xTimerStop(handle_clock_tmr, portMAX_DELAY);

                /*  notify the next task then block this task indefinitely */
                xTaskNotify(handle_curr_task, 0, eNoAction);
                vTaskDelay(portMAX_DELAY);
            }

        }

        if (PollForBtnPress(&btnCPressed, pdFALSE, APP_DEBOUNCE_TICKS))
        {
            AppLcd_EnableBacklight(pdTRUE);

            /* 12/24 hr format select */
            AppRtc_ToggleTimeFormat(&(appState.hourFormat));
            mssg = (appState.hourFormat == APPRTC_HOURFORMAT_24) ? "24hr format\n" : "12hr format\n";

            xQueueSend(uartprint_q, &mssg, portMAX_DELAY);

            PrepareLcdPrint(APP_CLOCK_TIMER_ID);
        }

        if (PollForBtnPress(&btnDPressed, pdFALSE, APP_DEBOUNCE_TICKS))
        {
            /* nothing to do here */
        }

        /**
         * @todo resync rtc and clock_tmr
         * 
         */
        /* allow other tasks to run */
        vTaskDelay(APP_TASKDELAY_TICKS);
    }
    }
}

void AppTasks_AlarmMode(void *params)
{

    while (xTaskNotifyWait(0, 0, NULL, portMAX_DELAY))
    {
        if ((appState.mode == APP_MODE_ALARM) && (appState.alarmMode.mode == APP_MODE_ALARM))
        {
            AlarmMode_Alarm();
        }
        else if ((appState.mode == APP_MODE_ALARM) && (appState.alarmMode.mode == APP_MODE_SETTING))
        {
            AlarmMode_Set();
        }
        else
        {
            /* nothing to do here */
        }
    }

}

void AppTasks_RunMode(void *params)
{
    while(xTaskNotifyWait(0, 0, NULL, portMAX_DELAY))
    {
    char *mssg;
    BaseType_t status;
    BaseType_t btnC_pressed = pdFALSE;

    AppLcd_ResetTimer();

    while (appState.mode == APP_MODE_RUNNING)
    {

        if (PollForBtnPress(&btnAPressed, pdFALSE, APP_DEBOUNCE_TICKS) == pdTRUE)
        {
            AppLcd_ToggleBacklight();
        }

        if (PollForBtnPress(&btnBPressed, pdFALSE, APP_DEBOUNCE_TICKS))
        {
            AppLcd_EnableBacklight(pdTRUE);

            appState.mode = APP_MODE_IDLE;
            appState.runMode = APP_RUNMODE_STOP;
            mssg = "idle mode\n";

            /**
             * @todo stop stopwatch timer printing and resume clock printing
             * @todo stopwatch timer is still running, just not printing
             * 
             */
            // stop stopwatch timer and resume clock printing
            xTimerStop(handle_stopwatch_tmr, portMAX_DELAY);
            AppRtc_ResetTimer();
            xQueueSend(uartprint_q, &mssg, portMAX_DELAY);

            xTimerStart(handle_clock_tmr, portMAX_DELAY);

            xTaskAbortDelay(handle_idlemode_task);
        }

        if (PollForBtnPress(&btnCPressed, pdFALSE, APP_DEBOUNCE_TICKS))
        {
            AppLcd_EnableBacklight(pdTRUE);

            // start the rtc timer and print the time and date every second
            if (!btnC_pressed)
            {
                /* start running rtc timer */
                appState.runMode = APP_RUNMODE_START;
                btnC_pressed = pdTRUE;

                mssg = "timer started\n";
                xQueueSend(uartprint_q, &mssg, portMAX_DELAY);

                status = xTimerStart(handle_stopwatch_tmr, 0);
                configASSERT(status == pdPASS);
            }
            else
            {
                /* stop running rtc timer */
                appState.runMode = APP_RUNMODE_STOP;
                btnC_pressed = pdFALSE;

                mssg = "timer stopped\n";
                xQueueSend(uartprint_q, &mssg, portMAX_DELAY);

                status = xTimerStop(handle_stopwatch_tmr, 0);
                configASSERT(status == pdPASS);
            }

        }

        if (PollForBtnPress(&btnDPressed, pdFALSE, APP_DEBOUNCE_TICKS))
        {
            AppLcd_EnableBacklight(pdTRUE);

            appState.runMode = APP_RUNMODE_RESET;

            if (xTimerIsTimerActive(handle_clock_tmr))
            {
                status = xTimerReset(handle_clock_tmr, portMAX_DELAY);
                configASSERT(status == pdPASS);
            }

            AppRtc_ResetTimer();
            AppLcd_ResetTimer();

            mssg = "timer reset\n";
            xQueueSend(uartprint_q, &mssg, portMAX_DELAY);

        }

        /* allow other tasks to run */
        vTaskDelay(APP_TASKDELAY_TICKS);
    }
    }
}

void AppTasks_SetMode(void *params)
{
    while (xTaskNotifyWait(0, 0, NULL, portMAX_DELAY))
    {
    // initializations
    char *mssg;
    uint8_t ampm = 0u;
    int8_t hour12 = 0;

    // turn on blinking cursor
    AppLcd_ToggleCursorBlink(pdTRUE);
    AppLcd_Select(appState.setMode);
    mssg = "adjusting seconds\n";
    xQueueSend(uartprint_q, &mssg, portMAX_DELAY);

    while (appState.mode == APP_MODE_SETTING)
    {
        if (PollForBtnPress(&btnAPressed, pdFALSE, APP_DEBOUNCE_TICKS))
        {
            AppLcd_ToggleBacklight();
        }

        if (PollForBtnPress(&btnBPressed, pdFALSE, APP_DEBOUNCE_TICKS))
        {
            AppLcd_EnableBacklight(pdTRUE);

            /* set the settings. this returns to idle mode */
            // turn off blinking cursor
            AppLcd_ToggleCursorBlink(pdFALSE);
            // set the calendar values
            AppRtc_SetCalendar();
            // reset btnC counter
            btnCPressed.pressCount = 0u;
            btnDPressed.pressCount = 0u;

            appState.mode = APP_MODE_IDLE;
            appState.setMode = APP_SETMODE_SEC;

            mssg = "idle mode\n";
            xQueueSend(uartprint_q, &mssg, portMAX_DELAY);

            // resume clock printing
            xTimerStart(handle_clock_tmr, portMAX_DELAY);

            xTaskAbortDelay(handle_idlemode_task);
        }

        if (PollForBtnPress(&btnCPressed, pdFALSE, APP_DEBOUNCE_TICKS))
        {
            AppLcd_EnableBacklight(pdTRUE);

            /* if setMode is SETMODE_HOUR, update the 12 hour format (am/pm) */
            if (appState.setMode == APP_SETMODE_HOUR)
            {
                if (appState.hourFormat == APPRTC_HOURFORMAT_12)
                {
                    ampm = AppRtc_GetRtc(APP_SETMODE_AMPM);

                    /* if 12hr format, convert currTime to 24hr for easy counting */
                    hour12 = (signed)((ampm == RTC_HOURFORMAT12_PM) ? (-btnCPressed.pressCount) : btnCPressed.pressCount);
                    btnCPressed.pressCount = AppRtc_12HrTo24Hr(hour12);

                    /* update the param value */
                    AppRtc_Update(appState.setMode, &(btnCPressed.pressCount));
                    ampm = (((btnCPressed.pressCount) >= 0u) &&
                            ((btnCPressed.pressCount) < 12u)) ? RTC_HOURFORMAT12_AM : RTC_HOURFORMAT12_PM;

                    /* after updating the cntr val, convert back to 12hr format for calendar setting and display */
                    hour12 = AppRtc_24HrTo12Hr(btnCPressed.pressCount);
                    btnCPressed.pressCount = APPRTC_RTC_12HOUR_ABS(hour12);

                    /* update the screen. update first the am/pm */
                    AppLcd_Select(APP_SETMODE_AMPM);
                    AppLcd_Update(APP_SETMODE_AMPM, ampm);
                    AppRtc_SetRtc(APP_SETMODE_AMPM, ampm);

                    /* update next the hr */
                    AppLcd_Select(appState.setMode);
                }
                else    /* (appState.hourFormat == APPRTC_HOURFORMAT_24) */
                {
                    /* update the param value */
                    AppRtc_Update(appState.setMode, &(btnCPressed.pressCount));
                }
            }
            else
            {
                /* increment values (hr/min/mon/day/week/secs) */
                AppRtc_Update(appState.setMode, &(btnCPressed.pressCount));
            }

            /* update the param value */
            AppRtc_SetRtc(appState.setMode, btnCPressed.pressCount);

            /* update the screen */
            AppLcd_Update(appState.setMode, btnCPressed.pressCount);
        }

        if (PollForBtnPress(&btnDPressed, pdFALSE, APP_DEBOUNCE_TICKS))
        {
            AppLcd_EnableBacklight(pdTRUE);

            btnDPressed.pressCount = (btnDPressed.pressCount + 1u) % 6;

            /* poll for number of btnD presses */
            switch (btnDPressed.pressCount)
            {
            case 1:     /* select hr */
                mssg = "adjusting hour\n";
                appState.setMode = APP_SETMODE_HOUR;
                break;
            case 2:     /* select min */
                mssg = "adjusting minute\n";
                appState.setMode = APP_SETMODE_MIN;
                break;
            case 3:     /* select month */
                mssg = "adjusting month\n";
                appState.setMode = APP_SETMODE_MONTH;
                break;
            case 4:     /* select date */
                mssg = "adjusting date\n";
                appState.setMode = APP_SETMODE_DATE;
                break;
            case 5:     /* select weekday */
                mssg = "adjusting weekday\n";
                appState.setMode = APP_SETMODE_WEEKDAY;
                break;
            default:    /* select sec */
                mssg = "adjusting seconds\n";
                appState.setMode = APP_SETMODE_SEC;
                break;
            }

            xQueueSend(uartprint_q, &mssg, portMAX_DELAY);
            AppLcd_Select(appState.setMode);
            /* get the current value of param selected */
            btnCPressed.pressCount = AppRtc_GetRtc(appState.setMode);
        }

        /* allow other tasks to run */
        vTaskDelay(APP_TASKDELAY_TICKS);
    }

    }
}

/*****************************************************************************
 * Interrupt Handlers/Callbacks
 *****************************************************************************/

void vAlarmCallback(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* ring the buzzer */

    xTaskNotifyFromISR(handle_alarmmode_task, 0, eNoAction, &xHigherPriorityTaskWoken);

    appState.mode = APP_MODE_ALARM;
    appState.alarmMode.mode = APP_MODE_ALARM;

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void vTimerCallback(TimerHandle_t xTimer)
{
    uint32_t timerId = (uint32_t)pvTimerGetTimerID(xTimer);

    PrepareLcdPrint(timerId);
}

/* this timer callback is used for button timeouts, inactivity timeouts, alarm timeouts */
void vGenericTimerCallback(TimerHandle_t xTimer)
{
    uint32_t timerId = (uint32_t)pvTimerGetTimerID(xTimer);

    switch (timerId)
    {
    case APP_IDLEBREAK_TIMER_ID:            /* idle timeout: turn off lcd after 60 secs */
    {
        AppLcd_EnableBacklight(pdFALSE);
        break;
    }
    case APP_ALARMTIMEOUT_TIMER_ID:         /* alarm timeout: snooze the alarm after 20 secs */
    {
        appState.alarmMode.isAlarmExpired = pdTRUE;
        break;
    }
    default:
        break;
    }

}

/*****************************************************************************
 * Hook Functions
 *****************************************************************************/


