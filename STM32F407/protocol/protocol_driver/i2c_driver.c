#include "i2c_driver.h"
#include "i2c_handle.h"
#include <string.h>
int i2c_read_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *value)
{
    if (value == NULL)
    {
        return -1; // 无效指针
    }
    // printf("i2c_read_reg: dev_addr=0x%02X, reg_addr=0x%02X\n", dev_addr, reg_addr);
    // 调用底层驱动
    // I2C_byteRead(0xa0,0x00,value);
    // uint32_t result = 1;
    uint32_t result = I2C_byteRead(dev_addr, reg_addr, value);

    // 注意：I2C_byteRead 成功返回 1，失败返回 0
    return (result == 1) ? 0 : -1;
}

int i2c_read_reg_16(uint8_t dev_addr, uint16_t reg_addr, uint8_t *value)
{
    if (value == NULL)
    {
        return -1; // 无效指针
    }
    uint32_t result = I2C_Read16addr(dev_addr, reg_addr, value);
    // 注意：I2C_byteRead 成功返回 1，失败返回 0
    return (result == 1) ? 0 : -1;
}

int i2c_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t value)
{
    // 调用底层驱动
    return I2C_ByteWrite(dev_addr, reg_addr, value);
}

int i2c_write_reg_16(uint8_t dev_addr, uint16_t reg_addr, uint8_t value)
{
    // 调用底层驱动
    uint32_t result = I2C_Write_16addr(dev_addr, reg_addr, value);

    // I2C_ByteWrite 成功返回 1 → 我们返回 0
    return (result == 1) ? 0 : -1;
}

int i2c_address_find(uint8_t *dev_addr_list)
{
    if (dev_addr_list == NULL)
    {
        return -1; // 无效指针
    }
    uint8_t i2c_device_num = 0;

    for (uint8_t i = 0x01; i < 0x80; i++)
    {
        if (!(i2c_device_adress_find(i << 1)))
        {
            dev_addr_list[i2c_device_num] = (i<<1);
            i2c_device_num++;
        }
    }
    return i2c_device_num;
}