#include "L298N.h"

void L298N_Init(L298N_TypeDef *Motor, GPIO_Pin *pinEN, GPIO_Pin *pinIN1, GPIO_Pin *pinIN2)
{
    Motor->pinEN = *pinEN;
    Motor->pinIN1 = *pinIN1;
    Motor->pinIN2 = *pinIN2;
    Motor->pwmVal = 749;
    Motor->isMoving = false;
    Motor->canMove = true;
    Motor->direction = STOP;
    Motor->lastMs = 0;
}

void forward(L298N_TypeDef *Motor)
{
    // Motor->pinIN1.GPIO_Portx->BRR = Motor->pinIN1.GPIO_Pinx;
    // Motor->pinIN2.GPIO_Portx->BSRR = Motor->pinIN2.GPIO_Pinx;
    GPIO_WriteBit(Motor->pinIN1.GPIO_Portx, Motor->pinIN1.GPIO_Pinx, Bit_SET);
    GPIO_WriteBit(Motor->pinIN2.GPIO_Portx, Motor->pinIN2.GPIO_Pinx, Bit_RESET);

    // Motor->pwmVal = 749;

    Motor->direction = FORWARD;
    Motor->isMoving = true;
}

void backward(L298N_TypeDef *Motor)
{
    // Motor->pinIN1.GPIO_Portx->BRR = Motor->pinIN1.GPIO_Pinx;
    // Motor->pinIN2.GPIO_Portx->BSRR = Motor->pinIN2.GPIO_Pinx;
    GPIO_WriteBit(Motor->pinIN1.GPIO_Portx, Motor->pinIN1.GPIO_Pinx, Bit_RESET);
    GPIO_WriteBit(Motor->pinIN2.GPIO_Portx, Motor->pinIN2.GPIO_Pinx, Bit_SET);
    
    /* analogWrite */
    
    Motor->direction = BACKWARD;
    Motor->isMoving = true;
}

void stop(L298N_TypeDef *Motor)
{
    // Motor->pinIN1.GPIO_Portx->BRR = Motor->pinIN1.GPIO_Pinx;
    // Motor->pinIN2.GPIO_Portx->BRR = Motor->pinIN2.GPIO_Pinx;
    GPIO_WriteBit(Motor->pinIN1.GPIO_Portx, Motor->pinIN1.GPIO_Pinx, Bit_RESET);
    GPIO_WriteBit(Motor->pinIN2.GPIO_Portx, Motor->pinIN2.GPIO_Pinx, Bit_RESET);

    /* analogWrite */

    Motor->direction = STOP;
    Motor->isMoving = false;
}

void run(L298N_TypeDef *Motor, Direction direction)
{
    switch (direction)
    {
    case BACKWARD:
        backward(Motor);
        break;
    case FORWARD:
        forward(Motor);
        break;
    case STOP:
        stop(Motor);
        break;
    }
}