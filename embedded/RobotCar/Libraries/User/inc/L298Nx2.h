#ifndef L298NX2_H
#define L298NX2_H

#include "L298N.h"

void MotorA_Init(GPIO_Pin *pinEN, GPIO_Pin *pinIN1, GPIO_Pin *pinIN2);
void forwardA(void);
void backwardA(void);
void stopA(void);
void runA(Direction direction);
void setSpeedA(uint16_t pwmVAl);
bool isMovingA(void);
Direction getDirectionA(void);

void MotorB_Init(GPIO_Pin *pinEN, GPIO_Pin *pinIN1, GPIO_Pin *pinIN2);
void forwardB(void);
void backwardB(void);
void stopB(void);
void runB(Direction direction);
void setSpeedB(uint16_t pwmVAl);
bool isMovingB(void);
Direction getDirectionB(void);

#endif