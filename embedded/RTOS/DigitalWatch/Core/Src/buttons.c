#include "buttons.h"
#include "task.h"

/* debouncing falling edge */
BaseType_t DebounceBtnRelease(GPIO_PinState currBtnRead, uint16_t *btnState)
{
    *btnState = (*btnState << 1u) | (currBtnRead != GPIO_PIN_RESET);
    return (*btnState == 0xFFC0u);
}

/* debouncing rising edge */
BaseType_t DebounceBtnPress(GPIO_PinState currBtnRead, uint16_t *btnState)
{
    *btnState = (*btnState << 1u) | (currBtnRead != GPIO_PIN_RESET);
    return (*btnState == 0x3FFFu);
}

BaseType_t PollForBtnPress(App_ButtonState_t *btn, BaseType_t pressDetect, TickType_t debounceTicks)
{
    BaseType_t state = pdFALSE;

    btn->currTick = xTaskGetTickCount();
    if ((btn->currTick - btn->prevTick) >= debounceTicks)
    {
        btn->currBtnRead = HAL_GPIO_ReadPin(btn->GPIOx, btn->GPIO_Pin);
        // poll for bounces every 5ms. valid press is 50ms of HIGH (5ms * 10 consecutive HIGHs)
        state = ((pressDetect) ? DebounceBtnPress : DebounceBtnRelease)(btn->currBtnRead, &btn->_state);

        btn->prevTick = btn->currTick;
    }
    return state;
}

/* count how many times this button is pressed */
BaseType_t CountShortBtnPress(App_ButtonState_t *btn, TickType_t debounceTicks, TickType_t delayTicks)
{
    TickType_t currTick = 0u, prevTick = 0u;

    if (PollForBtnPress(btn, pdFALSE, debounceTicks))
    {
        if (btn->pressCount == 0)
        {
            btn->timerStarted = pdTRUE;
            prevTick = currTick = xTaskGetTickCount();
        }
        btn->pressCount++;
    }

    /* hold system for btn press count */
    while (btn->timerStarted && ((currTick - prevTick) < delayTicks))
    {
        if (PollForBtnPress(btn, pdFALSE, debounceTicks))
        {
            btn->pressCount++;
        }
        currTick = xTaskGetTickCount();
    }
    btn->timerStarted = pdFALSE;

    return (btn->pressCount > 0);
}

BaseType_t PollForBtnHold(App_ButtonState_t *btn, TickType_t debounceTicks, TickType_t holdTicks, uint32_t timeoutTicks)
{
    BaseType_t longPressed = pdFALSE;

    if (PollForBtnPress(btn, pdTRUE, debounceTicks))
    {
        if (btn->timerStarted == pdFALSE)
        {
            /* start timer */
            btn->holdStartTick = xTaskGetTickCount();
            btn->timerStarted = pdTRUE;
        }
    }
    
    if (btn->timerStarted)
    {
        TickType_t elapsed = xTaskGetTickCount() - btn->holdStartTick;
        if ((HAL_GPIO_ReadPin(btn->GPIOx, btn->GPIO_Pin) == GPIO_PIN_SET) &&
            (elapsed >= holdTicks))
        {
            longPressed = pdTRUE;
            btn->timerStarted = pdFALSE;
        }
        
        if (elapsed >= timeoutTicks)
        {
            /* timer expired. stop timer */
            longPressed = pdTRUE;
            btn->timerStarted = pdFALSE;
        }
    }

    return longPressed;
}

/* count how long this button is pressed */
TickType_t CountElapsedBtnPress(App_ButtonState_t *btn, TickType_t debounceTicks, TickType_t timeoutTicks)
{
    TickType_t elapsedTime = 0u;
    TickType_t tempElapsed = 0u;

    /* detect btn press */
    if (PollForBtnPress(btn, pdTRUE, debounceTicks))
    {
        if (btn->timerStarted == pdFALSE)
        {
            /* start timer */
            btn->timerStarted = pdTRUE;
            btn->holdStartTick = xTaskGetTickCount();
        }
    }
    
    if (btn->timerStarted)
    {
        /* detect btn release */
        tempElapsed = xTaskGetTickCount() - btn->holdStartTick;
        if (tempElapsed >= timeoutTicks)
        {
            btn->timerStarted = pdFALSE;
            btn->holdStartTick = 0u;
            elapsedTime = tempElapsed;
        }
        else if (HAL_GPIO_ReadPin(btn->GPIOx, btn->GPIO_Pin) == GPIO_PIN_RESET)
        {
            btn->timerStarted = pdFALSE;
            btn->holdStartTick = 0u;
            elapsedTime = tempElapsed;
        }
    }

    return elapsedTime;
}
