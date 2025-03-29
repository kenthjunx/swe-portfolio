#ifndef APP_BUTTON_H
#define APP_BUTTON_H

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"

typedef struct
{
    volatile TickType_t currTick;
    volatile TickType_t prevTick;
    volatile TickType_t holdStartTick;
    BaseType_t timerStarted;
    GPIO_PinState currBtnRead;
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
    uint16_t _state;
    uint8_t pressCount;
}App_ButtonState_t;

BaseType_t DebounceBtnRelease(GPIO_PinState currBtnRead, uint16_t *btnState);
BaseType_t DebounceBtnPress(GPIO_PinState currBtnRead, uint16_t *btnState);
BaseType_t PollForBtnPress(App_ButtonState_t *btn, BaseType_t pressDetect, TickType_t debounceTicks);
BaseType_t CountShortBtnPress(App_ButtonState_t *btn, TickType_t debounceTicks, TickType_t delayTicks);
BaseType_t PollForBtnHold(App_ButtonState_t *btn, TickType_t debounceTicks, TickType_t holdTicks, uint32_t timeoutTicks);
TickType_t CountElapsedBtnPress(App_ButtonState_t *btn, TickType_t debounceTicks, TickType_t timeoutTicks);


#endif /* APP_BUTTON_H */