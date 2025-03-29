#include "L298Nx2.h"

#define DCMOTOR_TIM TIM2

L298N_TypeDef MotorA, MotorB;

void MotorA_Init(GPIO_Pin *pinEN, GPIO_Pin *pinIN1, GPIO_Pin *pinIN2)
{
    L298N_Init(&MotorA, pinEN, pinIN1, pinIN2);
    // forwardA();
    stopA();
}

void forwardA(void)
{
    GPIO_WriteBit(MotorA.pinIN1.GPIO_Portx, MotorA.pinIN1.GPIO_Pinx, Bit_SET);
    GPIO_WriteBit(MotorA.pinIN1.GPIO_Portx, MotorA.pinIN2.GPIO_Pinx, Bit_RESET);

    MotorA.pwmVal = (MotorA.pwmVal <= 499) ? 749 : MotorA.pwmVal;
    TIM_SetCompare4(TIM2, MotorA.pwmVal);

    MotorA.direction = FORWARD;
    MotorA.isMoving = true;
}

void backwardA(void)
{
    GPIO_WriteBit(MotorA.pinIN1.GPIO_Portx, MotorA.pinIN1.GPIO_Pinx, Bit_RESET);
    GPIO_WriteBit(MotorA.pinIN2.GPIO_Portx, MotorA.pinIN2.GPIO_Pinx, Bit_SET);

    MotorA.pwmVal = (MotorA.pwmVal <= 499) ? 749 : MotorA.pwmVal;
    TIM_SetCompare4(TIM2, MotorA.pwmVal);

    MotorA.direction = BACKWARD;
    MotorA.isMoving = true;
}

void stopA(void)
{
    GPIO_WriteBit(MotorA.pinIN1.GPIO_Portx, MotorA.pinIN1.GPIO_Pinx, Bit_RESET);
    GPIO_WriteBit(MotorA.pinIN2.GPIO_Portx, MotorA.pinIN2.GPIO_Pinx, Bit_RESET);

    TIM_SetCompare4(TIM2, 499);

    MotorA.direction = STOP;
    MotorA.isMoving = false;
}

void setSpeedA(uint16_t pwmVAl)
{
    MotorA.pwmVal = pwmVAl;
}

bool isMovingA(void)
{
    return MotorA.isMoving;
}

Direction getDirectionA(void)
{
    return MotorA.direction;
}

void MotorB_Init(GPIO_Pin *pinEN, GPIO_Pin *pinIN1, GPIO_Pin *pinIN2)
{
    L298N_Init(&MotorB, pinEN, pinIN1, pinIN2);
    // forwardB();
    stopB();
}

void forwardB(void)
{
    GPIO_WriteBit(MotorB.pinIN1.GPIO_Portx, MotorB.pinIN1.GPIO_Pinx, Bit_SET);
    GPIO_WriteBit(MotorB.pinIN1.GPIO_Portx, MotorB.pinIN2.GPIO_Pinx, Bit_RESET);

    MotorB.pwmVal = (MotorB.pwmVal <= 499) ? 749 : MotorB.pwmVal;
    TIM_SetCompare3(TIM2, MotorB.pwmVal);

    MotorB.direction = FORWARD;
    MotorB.isMoving = true;
}

void backwardB(void)
{
    GPIO_WriteBit(MotorB.pinIN1.GPIO_Portx, MotorB.pinIN1.GPIO_Pinx, Bit_RESET);
    GPIO_WriteBit(MotorB.pinIN2.GPIO_Portx, MotorB.pinIN2.GPIO_Pinx, Bit_SET);

    MotorB.pwmVal = (MotorB.pwmVal <= 499) ? 749 : MotorB.pwmVal;
    TIM_SetCompare3(TIM2, MotorB.pwmVal);

    MotorB.direction = BACKWARD;
    MotorB.isMoving = true;
}

void stopB(void)
{
    GPIO_WriteBit(MotorB.pinIN1.GPIO_Portx, MotorB.pinIN1.GPIO_Pinx, Bit_RESET);
    GPIO_WriteBit(MotorB.pinIN2.GPIO_Portx, MotorB.pinIN2.GPIO_Pinx, Bit_RESET);

    TIM_SetCompare3(TIM2, 499);

    MotorB.direction = STOP;
    MotorB.isMoving = false;
}

void setSpeedB(uint16_t pwmVAl)
{
    MotorB.pwmVal = pwmVAl;
}

bool isMovingB(void)
{
    return MotorB.isMoving;
}

Direction getDirectionB(void)
{
    return MotorB.direction;
}
