#include "I2C.h"

// Global structures as handles for peripheral devices:
GPIO_InitTypeDef I2C_GPIOConfigStruct;
I2C_InitTypeDef I2C_ConfigStruct;

/**
 * @brief Initializes and enables the I2C1
 * @param None
 * @retval None
 */
void init_I2C1(void)
{
    // Enable the GPIOB, I2C1 and AFIO modules.
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    // Setup the I2C BUS properties through the defined "I2C" structure:
    I2C_ConfigStruct.I2C_ClockSpeed = 100000; // Bus clock speed 100kbps
    I2C_ConfigStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_ConfigStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    // Setup I2C BUS master properties:
    I2C_ConfigStruct.I2C_OwnAddress1 = 0x00;
    I2C_ConfigStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_ConfigStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    // Finally initialize the I2C1 unit using the "i2c" structure.
    I2C_Init(I2C1, &I2C_ConfigStruct);

    // Configure the GPIO pins shared between the I2C module
    I2C_GPIOConfigStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    I2C_GPIOConfigStruct.GPIO_Mode = GPIO_Mode_AF_OD;
    I2C_GPIOConfigStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &I2C_GPIOConfigStruct);

    // Enable the I2C1 module:
    I2C_Cmd(I2C1, ENABLE);
}

/**
 * @brief Starts the I2C coomunication
 *
 * @param I2Cx where x can be 1 or 2 to select the I2C peripheral.
 * @param transmissionDirection: specifies whether the I2C device will be a
 *   Transmitter or a Receiver. This parameter can be one of the following values
 *     @arg I2C_Direction_Transmitter: Transmitter mode
 *     @arg I2C_Direction_Receiver: Receiver mode
 * @param slaveAddress: specifies the slave address which will be transmitted
 * @retval None
 */
void I2C_StartTransmission(I2C_TypeDef *I2Cx, uint8_t transmissionDirection, uint8_t slaveAddress)
{
    // Wait until I2C module is idle:
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
        ;

    // Generate the start condition
    I2C_GenerateSTART(I2Cx, ENABLE);
    //
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
        ;

    // Send the address of the slave to be contacted:
    I2C_Send7bitAddress(I2Cx, slaveAddress << 1, transmissionDirection);

    // If this is a write operation, set I2C for transmit
    if (transmissionDirection == I2C_Direction_Transmitter)
    {
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
            ;
    }
    // Or if a read operation, set i2C for receive
    if (transmissionDirection == I2C_Direction_Receiver)
    {
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
            ;
    }
}

/**
 * @brief Sends a data byte through the I2Cx peripheral.
 *
 * @param I2Cx: where x can be 1 or 2 to select the I2C peripheral.
 * @param data: Data to be transmitted.
 * @retval None
 */
void I2C_WriteData(I2C_TypeDef *I2Cx, uint8_t data)
{
    // Write the data on the bus
    I2C_SendData(I2Cx, data);
    // Wait until transmission is complete:
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        ;
}

/**
 * @brief Returns the most recent received data by the I2Cx peripheral.
 *
 * @param I2Cx: where x can be 1 or 2 to select the I2C peripheral.
 * @return uint8_t raw data read from the I2C device
 */
uint8_t I2C_ReadData(I2C_TypeDef *I2Cx)
{
    // Wait until receive is completed
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
        ;
    uint8_t data;
    // Read one byte of data from I2c
    data = I2C_ReceiveData(I2Cx);
    return data;
}
