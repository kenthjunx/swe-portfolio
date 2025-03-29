# **My Journey in Software Engineering**

## **My Projects**

### **Embedded Systems Projects**

#### 1. [Robot Car Project](./embedded/RobotCar)

This fun project is meant to simulate basic Car controls, such as forward and backward, using the available peripherals of STM32F1 microcontroller (BluePill). The project uses an I2C LCD as display, timer for the PWM control of the motors in L298N driver, another timer for the ultrasonic sensor for sensing an obstacle in front, and a systick timer for the car's heartbeat.

#### 2. [LCD Module Driver Development](./embedded/STM32F103_I2C_LCD1602)

I made this project to learn about I2C communication protocol, driver development of sensors and module, and RTC or Real Time Clock. I chose the builtin RTC module of the STM32F1 microcontroller, to be displayed on an I2C LCD.

#### 3. [RTOS Digital Watch](./embedded/RTOS/DigitalWatch)

A fun, little project that demonstrates real-time operating system (FreeRTOS) concepts while building a fully functional digital watch with an STM32F4 microcontroller. Some features of the watch are:
* 4 modes (idle, alarm, stopwatch, time and date settings)
* alarm buzzer
* snooze function
* LCD backlight toggle
* LCD backlight timeout
* I2C LCD as display
* 4 buttons as inputs

### Python Projects

#### 1. [Extract Table From HTML](./python/htmltocsv)

A Python web scraping tool that extracts exchange rate tables from a financial website, specifically converting Vietnamese Dong (VND) to Philippine Peso (PHP) based on user-provided month and year inputs. I developed this project for expense reporting in my previous workplace to reduce manual travel allowance calculations.