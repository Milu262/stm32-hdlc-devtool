#include "i2c_handle.h"
#include "i2c_init.h"
#include "board.h"
#include <stdio.h>
static const Error_Code i2c_Errors[] = {
    {0, "I2C start signal transmission failed!\r\n"},
    {1, "I2C Device address No response!\r\n"},
    {2, "I2C Write the wrong register address!\r\n"},
    {3, "I2C Write data error!\r\n"},
    {4, "I2C send read register address error!\r\n"},
    {5, "I2C read data error!\r\n"},
    {10, "I2C bus is busy!\r\n"}
    // 可以根据需要添加更多错误信息
};
// 定义错误信息数组的大小
#define I2C_ERROR_COUNT (sizeof(i2c_Errors) / sizeof(i2c_Errors[0]))

/**
 * @brief 这是一个用户定义的I2C超时回调函数。
 * @param errorCode 这是一个错误码，用于标识超时原因。
 * @return -1 表示超时发生。
 * @note 该函数会在I2C操作超时后被调用。
 */
static int I2C_TIMEOUT_UserCallback(uint8_t errorCode)
{
    /* 使用串口 printf 输出错误信息，方便调试 */
    // 遍历错误信息数组，查找并打印对应的错误信息
    for (size_t i = 0; i < I2C_ERROR_COUNT; i++)
    {
        if (i2c_Errors[i].errorCode == errorCode)
        {
            printf("%s", i2c_Errors[i].errorMessage);
            break;
        }
    }

    // 如果没有找到对应的错误信息，则不打印任何内容（或者可以打印一个默认的错误信息）

    // 生成停止条件
    I2C_GenerateSTOP(I2C1, ENABLE);

    return -1;
}

int I2C_ByteWrite(uint8_t slave_adress, uint8_t WriteAddr, uint8_t pBuffer)
{

    uint32_t I2CTimeout;
    /* 产生 I2C 起始信号 */
    I2C_GenerateSTART(I2C1, ENABLE);
    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* 检测 EV5 事件并清除标志 */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(0);
    }

    /* 发送设备地址 */
    I2C_Send7bitAddress(I2C1, slave_adress, I2C_Direction_Transmitter);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* 检测 EV6 事件并清除标志 */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(1);
    }
    /* 发送要写入的寄存器地址*/
    I2C_SendData(I2C1, WriteAddr);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* 检测 EV8 事件并清除标志 */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(2);
    }
    /* 发送一字节要写入的数据 */
    I2C_SendData(I2C1, pBuffer);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* 检测 EV8 事件并清除标志 */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(3);
    }

    /* 发送停止信号 */
    I2C_GenerateSTOP(I2C1, ENABLE);

    return 0;
}

uint32_t I2C_BufferWrite(uint8_t slave_adress, uint8_t WriteAddr, uint8_t *pBuffer, uint16_t NumByteToWrite)
{
    uint16_t i;
    uint8_t res;

    /* 每写一个字节调用一次 I2C_EE_ByteWrite 函数 */
    for (i = 0; i < NumByteToWrite; i++)
    {
        /* 等待 EEPROM 准备完毕 */
        // I2C_EE_WaitEepromStandbyState();
        /* 按字节写入数据 */
        res = I2C_ByteWrite(slave_adress, WriteAddr++, *pBuffer++);
        if (res == 0)
        {
            printf("I2C 8bit Register Address Buffer write error!\r\n");
            return 0;
        }
    }
    return res;
}

uint32_t I2C_BufferRead(uint8_t slave_adress, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t NumByteToRead)
{
    uint32_t I2CTimeout = I2CT_LONG_TIMEOUT;

    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(10);
    }
    /* Send START condition */
    I2C_GenerateSTART(I2C1, ENABLE);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(0);
    }

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(I2C1, slave_adress, I2C_Direction_Transmitter);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(1);
    }
    /* Clear EV6 by setting again the PE bit */
    I2C_Cmd(I2C1, ENABLE);

    /* Send the EEPROM's internal address to write to */
    I2C_SendData(I2C1, ReadAddr);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV8 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(4);
    }
    /* Send STRAT condition a second time */
    I2C_GenerateSTART(I2C1, ENABLE);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(0);
    }
    /* Send EEPROM address for read */
    I2C_Send7bitAddress(I2C1, slave_adress, I2C_Direction_Receiver);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(1);
    }
    /* While there is data to be read */

    while (NumByteToRead)
    {

        if (NumByteToRead == 1)
        {
            /* Disable Acknowledgement */
            I2C_AcknowledgeConfig(I2C1, DISABLE);

            /* Send STOP Condition */
            I2C_GenerateSTOP(I2C1, ENABLE);
        }

        I2CTimeout = I2CT_LONG_TIMEOUT;
        while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) == 0)
        {
            if ((I2CTimeout--) == 0)
                return I2C_TIMEOUT_UserCallback(5);
        }

        /* Read a byte from the device */
        *pBuffer = I2C_ReceiveData(I2C1);

        /* Point to the next location where the byte read will be saved */
        pBuffer++;

        /* Decrement the read bytes counter */
        NumByteToRead--;
    }

    /* Enable Acknowledgement to be ready for another reception */
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return 1;
}

uint32_t I2C_byteRead(uint8_t slave_adress, uint8_t ReadAddr, uint8_t *pBuffer)
{
    uint32_t I2CTimeout = I2CT_LONG_TIMEOUT;

    //*((u8 *)0x4001080c) |=0x80;
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(10);
    }
    /* Send START condition */
    I2C_GenerateSTART(I2C1, ENABLE);
    //*((u8 *)0x4001080c) &=~0x80;

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(0);
    }

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(I2C1, slave_adress, I2C_Direction_Transmitter);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if ((I2CTimeout--) == 0)
        {
            return I2C_TIMEOUT_UserCallback(1);
        }
    }
    // printf("I2C1 EV6\n");
    /* Clear EV6 by setting again the PE bit */
    I2C_Cmd(I2C1, ENABLE);

    /* Send the EEPROM's internal address to write to */
    I2C_SendData(I2C1, ReadAddr);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV8 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(4);
    }
    /* Send STRAT condition a second time */
    I2C_GenerateSTART(I2C1, ENABLE);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(0);
    }
    /* Send EEPROM address for read */
    I2C_Send7bitAddress(I2C1, slave_adress, I2C_Direction_Receiver);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(1);
    }
    /* While there is data to be read */

    /* Disable Acknowledgement */
    I2C_AcknowledgeConfig(I2C1, DISABLE);

    /* Send STOP Condition */
    I2C_GenerateSTOP(I2C1, ENABLE);

    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) == 0)
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(5);
    }

    /* Read a byte from the device */
    *pBuffer = I2C_ReceiveData(I2C1);

    /* Enable Acknowledgement to be ready for another reception */
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return 1;
}

uint32_t I2C_Write_16addr(uint8_t slave_adress, uint16_t WriteAddr, uint8_t pBuffer)
{

    uint32_t I2CTimeout;
    /* 产生 I2C 起始信号 */
    I2C_GenerateSTART(I2C1, ENABLE);
    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* 检测 EV5 事件并清除标志 */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(0);
    }

    /* 发送设备地址 */
    I2C_Send7bitAddress(I2C1, slave_adress, I2C_Direction_Transmitter);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* 检测 EV6 事件并清除标志 */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(1);
    }

    /* 发送要写入的寄存器地址*/
    I2C_SendData(I2C1, WriteAddr >> 8);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* 检测 EV8 事件并清除标志 */
    while (!I2C_CheckEvent(I2C1,
                           I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(2);
    }

    I2C_SendData(I2C1, WriteAddr);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* 检测 EV8 事件并清除标志 */
    while (!I2C_CheckEvent(I2C1,
                           I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(2);
    }
    /* 发送一字节要写入的数据 */
    I2C_SendData(I2C1, pBuffer);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* 检测 EV8 事件并清除标志 */
    while (!I2C_CheckEvent(I2C1,
                           I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(3);
    }

    /* 发送停止信号 */
    I2C_GenerateSTOP(I2C1, ENABLE);

    return 1;
}

uint32_t I2C_Read16addr(uint8_t slave_adress, uint16_t ReadAddr, uint8_t *pBuffer)
{
    uint32_t I2CTimeout = I2CT_LONG_TIMEOUT;

    //*((u8 *)0x4001080c) |=0x80;
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(10);
    }
    /* Send START condition */
    I2C_GenerateSTART(I2C1, ENABLE);
    //*((u8 *)0x4001080c) &=~0x80;

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(0);
    }

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(I2C1, slave_adress, I2C_Direction_Transmitter);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(1);
    }
    /* Clear EV6 by setting again the PE bit */
    I2C_Cmd(I2C1, ENABLE);

    /* Send the EEPROM's internal address to write to */
    I2C_SendData(I2C1, ReadAddr >> 8);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV8 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(4);
    }

    I2C_SendData(I2C1, ReadAddr);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV8 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(4);
    }
    /* Send STRAT condition a second time */
    I2C_GenerateSTART(I2C1, ENABLE);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(0);
    }
    /* Send EEPROM address for read */
    I2C_Send7bitAddress(I2C1, slave_adress, I2C_Direction_Receiver);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(1);
    }
    /* While there is data to be read */

    /* Disable Acknowledgement */
    I2C_AcknowledgeConfig(I2C1, DISABLE);

    /* Send STOP Condition */
    I2C_GenerateSTOP(I2C1, ENABLE);

    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) == 0)
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(5);
    }

    /* Read a byte from the device */
    *pBuffer = I2C_ReceiveData(I2C1);

    /* Enable Acknowledgement to be ready for another reception */
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return 1;
}

uint32_t I2C_BufferWrite_16addr(uint8_t slave_adress, uint16_t WriteAddr, uint8_t *pBuffer, uint16_t NumByteToWrite)
{
    uint16_t i;
    uint8_t res;

    /* 每写一个字节调用一次 I2C_EE_ByteWrite 函数 */
    for (i = 0; i < NumByteToWrite; i++)
    {
        /* 等待 EEPROM 准备完毕 */
        // I2C_EE_WaitEepromStandbyState();
        /* 按字节写入数据 */
        res = I2C_Write_16addr(slave_adress, WriteAddr++, *pBuffer++);
        if (res == 0)
        {
            printf("I2C 16bit Register Address Buffer write error!\r\n");
            return 0;
        }
    }
    return res;
}

uint32_t I2C_BufferRead_16addr(uint8_t slave_adress, uint16_t ReadAddr, uint8_t *pBuffer, uint16_t NumByteToRead)
{
    uint32_t I2CTimeout = I2CT_LONG_TIMEOUT;

    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(10);
    }
    /* Send START condition */
    I2C_GenerateSTART(I2C1, ENABLE);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(0);
    }

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(I2C1, slave_adress, I2C_Direction_Transmitter);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(1);
    }
    I2C_Cmd(I2C1, ENABLE);

    /* Send the EEPROM's internal address to write to */
    I2C_SendData(I2C1, ReadAddr >> 8);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV8 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(4);
    }

    I2C_SendData(I2C1, ReadAddr);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV8 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(4);
    }
    /* Send STRAT condition a second time */
    I2C_GenerateSTART(I2C1, ENABLE);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV5 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(0);
    }
    /* Send EEPROM address for read */
    I2C_Send7bitAddress(I2C1, slave_adress, I2C_Direction_Receiver);

    I2CTimeout = I2CT_FLAG_TIMEOUT;

    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(1);
    }
    /* While there is data to be read */
    while (NumByteToRead)
    {
        if (NumByteToRead == 1)
        {
            /* Disable Acknowledgement */
            I2C_AcknowledgeConfig(I2C1, DISABLE);

            /* Send STOP Condition */
            I2C_GenerateSTOP(I2C1, ENABLE);
        }

        I2CTimeout = I2CT_LONG_TIMEOUT;
        while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) == 0)
        {
            if ((I2CTimeout--) == 0)
                return I2C_TIMEOUT_UserCallback(5);
        }
        {
            /* Read a byte from the device */
            *pBuffer = I2C_ReceiveData(I2C1);

            /* Point to the next location where the byte read will be saved */
            pBuffer++;

            /* Decrement the read bytes counter */
            NumByteToRead--;
        }
    }

    /* Enable Acknowledgement to be ready for another reception */
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return 1;
}

int i2c_device_adress_find(uint8_t slave_adress)
{
    uint32_t I2CTimeout;
    /* 产生 I2C 起始信号 */
    I2C_GenerateSTART(I2C1, ENABLE);
    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* 检测 EV5 事件并清除标志 */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(0);
    }

    /* 发送设备地址 */
    I2C_Send7bitAddress(I2C1, slave_adress, I2C_Direction_Transmitter);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* 检测 EV6 事件并清除标志 */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if ((I2CTimeout--) == 0)
        {
            I2C_GenerateSTOP(I2C1, ENABLE);
            return -1;
        }
    }
    I2C_GenerateSTOP(I2C1, ENABLE);
    return 0;
}

void Find_i2c_device(void)
{
    uint8_t i2c_device = 0;

    for (uint8_t i = 0x01; i < 0x80; i++)
    {
        if (i2c_device_adress_find(i << 1))
        {
            printf("find i2c device address: 0x%x(Write address 8bit)\r\n", i << 1);
            i2c_device++;
        }
        // delay_ms(10);//延时
    }
    if (!i2c_device)
    {
        printf("No device is connected to the I2C bus!\r\n");
    }
}


/**
 * @brief  等待EEPROM准备好状态。
 *
 * 该函数用于等待EEPROM设备进入准备好状态，通常在写入操作后调用。
 * 通过发送START条件并尝试写入设备地址来检测EEPROM是否准备好。
 *
 * @param  EEPROM_ADDRESS: EEPROM设备的7位地址。
 * @retval 无
 */
static void I2C_EE_WaitEepromStandbyState(uint8_t EEPROM_ADDRESS)
{
    vu16 SR1_Tmp = 0;

    do
    {
        /* Send START condition */
        I2C_GenerateSTART(I2C1, ENABLE);
        /* Read EEPROM_I2C SR1 register */
        SR1_Tmp = I2C_ReadRegister(I2C1, I2C_Register_SR1);
        /* Send EEPROM address for write */
        I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS, I2C_Direction_Transmitter);
    } while (!(I2C_ReadRegister(I2C1, I2C_Register_SR1) & 0x0002));

    /* Clear AF flag */
    I2C_ClearFlag(I2C1, I2C_FLAG_AF);
    /* STOP condition */
    I2C_GenerateSTOP(I2C1, ENABLE);
}

uint8_t I2C_EE_8Addr_PageWrite(uint8_t EEPROM_ADDRESS, uint8_t WriteAddr, uint8_t *Data, uint8_t NumByteToWrite)
{
    uint16_t I2CTimeout = I2CT_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(I2C_Channel, I2C_FLAG_BUSY))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(10);
    }
    I2C_GenerateSTART(I2C_Channel, ENABLE);
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_CheckEvent(I2C_Channel, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(0);
    }
    I2C_Send7bitAddress(I2C_Channel, EEPROM_ADDRESS, I2C_Direction_Transmitter);
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_CheckEvent(I2C_Channel, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(1);
    }
    I2C_SendData(I2C_Channel, WriteAddr);
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_CheckEvent(I2C_Channel, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(2);
    }
    while (NumByteToWrite--)
    {
        I2C_SendData(I2C_Channel, *Data);
        Data++;
        I2CTimeout = I2CT_LONG_TIMEOUT;
        while (!I2C_CheckEvent(I2C_Channel, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        {
            if ((I2CTimeout--) == 0)
                return I2C_TIMEOUT_UserCallback(3);
        }
    }
    I2C_GenerateSTOP(I2C_Channel, ENABLE);
    // 等待EEPROM内部写操作完成
    I2C_EE_WaitEepromStandbyState(EEPROM_ADDRESS);
    return 1;
}

uint8_t I2C_EE_16Addr_PageWrite(uint8_t EEPROM_ADDRESS, uint16_t WriteAddr, uint8_t *Data, uint8_t NumByteToWrite)
{
    uint32_t I2CTimeout;
    /* 产生 I2C 起始信号 */
    I2C_GenerateSTART(I2C1, ENABLE);
    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* 检测 EV5 事件并清除标志 */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(0);
    }

    /* 发送设备地址 */
    I2C_Send7bitAddress(I2C1, EEPROM_ADDRESS, I2C_Direction_Transmitter);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* 检测 EV6 事件并清除标志 */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(1);
    }

    /* 发送要写入的寄存器地址*/
    I2C_SendData(I2C1, WriteAddr >> 8);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* 检测 EV8 事件并清除标志 */
    while (!I2C_CheckEvent(I2C1,
                           I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(2);
    }

    I2C_SendData(I2C1, WriteAddr);

    I2CTimeout = I2CT_FLAG_TIMEOUT;
    /* 检测 EV8 事件并清除标志 */
    while (!I2C_CheckEvent(I2C1,
                           I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((I2CTimeout--) == 0)
            return I2C_TIMEOUT_UserCallback(2);
    }
    /* 发送一字节要写入的数据 */

    while (NumByteToWrite--)
    {
        I2C_SendData(I2C_Channel, *Data);
        Data++;
        I2CTimeout = I2CT_LONG_TIMEOUT;
        while (!I2C_CheckEvent(I2C_Channel, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        {
            if ((I2CTimeout--) == 0)
                return I2C_TIMEOUT_UserCallback(3);
        }
    }

    /* 发送停止信号 */
    I2C_GenerateSTOP(I2C1, ENABLE);
    // 等待EEPROM内部写操作完成
    // I2C_EE_WaitEepromStandbyState(EEPROM_ADDRESS);
    return 1;
}