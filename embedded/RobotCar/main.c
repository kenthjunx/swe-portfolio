#include "main.h"

// uint8_t heart[8] = {0x0, 0xa, 0x1f, 0x1f, 0xe, 0x4, 0x0};
uint16_t dist_mm;
char data[16];
char *msg;

int main()
{
    /* System Peripherals Initialization */
    RCC_Config();
    GPIO_Config();
    TIM_Config();

    /* Device Drivers Initialization */
    LCD_Init();
    button_Init();
    USS_Init();
    MotorDriver_Init();
    // servo_Init();
    buzzer_Init();

    TIM_Cmd(TIM3, ENABLE);
    // TIM_ITConfig(TIM3, (TIM_IT_CC1 | TIM_IT_CC2), ENABLE);

    uint16_t pwmVal = 499;
    while (1)
    {
        /* US sensor */
        dist_mm = HCSR04_GetDistance();
        LCDI2C_setCursor(0, 0);
        sprintf(data, "dist = %d", dist_mm);
        LCDI2C_write_String(data);
        LCDI2C_setCursor(12, 0);
        LCDI2C_write_String("mm");

        /* motor */
        // forwardA();
        // forwardB();
        // delay_ms(1000);

        // stopA();
        // stopB();
        // delay_ms(1000);

        // backwardA();
        // backwardB();
        // delay_ms(1000);

        // stopA();
        // stopB();
        // delay_ms(1000);

        /* servo */
        /* off */
        // TIM_SetCompare1(TIM3, 0);
        // delay_ms(1000);

        // /* leftmost position */
        // TIM_SetCompare1(TIM3, 25);
        // delay_ms(1000);

        // /* 1ms pulse width - all the way to the left */
        // // TIM_SetCompare1(TIM3, 50);
        // // delay_ms(1000);

        // /* 1.5ms pulse width - middle */
        // TIM_SetCompare1(TIM3, 75);
        // delay_ms(1000);

        // /* 2ms pulse width - all the way to the right */
        // // TIM_SetCompare1(TIM3, 100);
        // // delay_ms(1000);

        // /* rightmost position */
        // TIM_SetCompare1(TIM3, 125);
        // delay_ms(1000);

        if (dist_mm <= 100 && getDirectionA() == FORWARD)
        {
            stopA();
            stopB();
        }

        switch (getDirectionA())
        {
        case FORWARD:
            msg = "Forward";
            break;
        case BACKWARD:
            msg = "Backward";
            break;
        case STOP:
            msg = "Stop";
            break;
        }

        LCDI2C_setCursor(0, 1);
        LCDI2C_write_String(msg);

        pwmVal = (getDirectionA() == BACKWARD && getDirectionB() == BACKWARD)
                     ? 499
                     : 0;
        TIM_SetCompare2(TIM3, pwmVal);

        /* delay for LCD printing */
        delay_ms(500);
        LCDI2C_clear();
    }
}
