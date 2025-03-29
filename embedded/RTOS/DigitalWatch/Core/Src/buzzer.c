#include "buzzer.h"

extern TIM_HandleTypeDef htim2;

void ActivateBuzzer(BaseType_t buzzState)
{
    if (buzzState == pdTRUE)
    {
        HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    }
    else
    {
        HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
    }
}

void SetDutyCycle(uint32_t pulse)
{
    TIM_CCxChannelCmd(htim2.Instance, TIM_CHANNEL_1, TIM_CCx_DISABLE);
    /* Set the Capture Compare Register value */
    htim2.Instance->CCR1 = pulse;
    TIM_CCxChannelCmd(htim2.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);
}