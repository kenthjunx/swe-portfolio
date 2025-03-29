#ifndef APP_BUZZER_H
#define APP_BUZZER_H

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"

void ActivateBuzzer(BaseType_t buzzState);
void SetDutyCycle(uint32_t pulse);

#endif /* APP_BUZZER_H */