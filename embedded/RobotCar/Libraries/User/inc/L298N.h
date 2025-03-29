#ifndef L298N_H
#define L298N_H

#include "stm32f10x.h"
#include <stdbool.h>

typedef enum
{
    FORWARD = 0,
    BACKWARD = 1,
    STOP = -1
} Direction;

typedef struct
{
    GPIO_TypeDef *GPIO_Portx;
    uint16_t GPIO_Pinx;
} GPIO_Pin;

typedef struct
{
    GPIO_Pin pinEN;
    GPIO_Pin pinIN1;
    GPIO_Pin pinIN2;
    uint16_t pwmVal;
    unsigned long lastMs;
    bool canMove;
    bool isMoving;
    Direction direction;
} L298N_TypeDef;

void L298N_Init(L298N_TypeDef *Motor, GPIO_Pin *pinEN, GPIO_Pin *pinIN1, GPIO_Pin *pinIN2);
void forward(L298N_TypeDef *Motor);
void backward(L298N_TypeDef *Motor);
void stop(L298N_TypeDef *Motor);
void run(L298N_TypeDef *Motor, Direction direction);

#endif /* L298N_H */