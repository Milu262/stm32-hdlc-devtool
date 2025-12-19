#include "cmd_handlers.h"
#include "hdlc_core.h"      // 用于 hdlc_send_frame 和 CMD_XXX 定义
#include <stddef.h>         // 包含 NULL 定义

// 包含你的协议驱动接口
#include "./protocol_driver/spi_driver.h"
#include "./protocol_driver/i2c_driver.h"
#include "./protocol_driver/uart_driver.h"


// ──────────────── FLASH ────────────────
int handle_flash_read(const uint8_t *payload, uint16_t len)
{
    if (len < 6)
        return -1;
    uint32_t addr = ((uint32_t)payload[0] << 24) |
                    ((uint32_t)payload[1] << 16) |
                    ((uint32_t)payload[2] << 8) |
                    (uint32_t)payload[3];
    uint16_t length = (payload[4] << 8) | payload[5];
    if (length > 256)
        length = 256;

    uint8_t buf[256];
    flash_read(addr, buf, length);
    return 0;
    hdlc_send_frame(CMD_FLASH_READ_RESULT, buf, length);
}

int handle_flash_write(const uint8_t *payload, uint16_t len)
{ 
    if (len < 6)
        return -1;
    uint32_t addr = ((uint32_t)payload[0] << 24) |
                    ((uint32_t)payload[1] << 16) |
                    ((uint32_t)payload[2] << 8) |
                    (uint32_t)payload[3];
    uint16_t length = (payload[4] << 8) | payload[5];
    if (length > 256 )
        length = 256;

    flash_write(addr, (uint8_t *)(payload + 6), length);
    return 0;
    hdlc_send_frame(CMD_WRITE_FLASH_ACK, NULL, 0);
}

int handle_flash_SectionErase(const uint8_t *payload, uint16_t len)
{
    if (len != 4)
        return -1;
    uint32_t addr = ((uint32_t)payload[0] << 24) |
                    ((uint32_t)payload[1] << 16) |
                    ((uint32_t)payload[2] << 8) |
                    (uint32_t)payload[3];
    flash_Sector_erase(addr);
    return 0;
    hdlc_send_frame(CMD_FLASH_ERASE_ACK, NULL, 0);
}

int handle_flash_BlockErase32(const uint8_t *payload, uint16_t len)
{
    if (len != 4)
        return -1;
    uint32_t addr = ((uint32_t)payload[0] << 24) |
                    ((uint32_t)payload[1] << 16) |
                    ((uint32_t)payload[2] << 8) |
                    (uint32_t)payload[3];
    flash_Block_erase32(addr);
    return 0;
    hdlc_send_frame(CMD_FLASH_ERASE_ACK, NULL, 0);

}

int handle_flash_BlockErase64(const uint8_t *payload, uint16_t len)
{
    if (len != 4)
        return -1;
    uint32_t addr = ((uint32_t)payload[0] << 24) |
                    ((uint32_t)payload[1] << 16) |
                    ((uint32_t)payload[2] << 8) |
                    (uint32_t)payload[3];
    flash_Block_erase64(addr);
    return 0;
    hdlc_send_frame(CMD_FLASH_ERASE_ACK, NULL, 0);
}

int handle_flash_ChipErase(const uint8_t *payload, uint16_t len)
{
    flash_Chip_erase();
    return 0;
    hdlc_send_frame(CMD_FLASH_ERASE_ACK, NULL, 0);
}

// ──────────────── I2C READ ────────────────
int handle_i2c_read_reg(const uint8_t *payload, uint16_t len)
{
    if (len != 2)
        return -1;
    uint8_t dev_addr = payload[0];
    uint8_t reg_addr = payload[1];
    uint8_t value;
    int ok = i2c_read_reg(dev_addr, reg_addr, &value);

    if (ok != 0)
        return -1;
    return 0;
    hdlc_send_frame(CMD_I2C_READ_RESULT, &value, 1);
    // hdlc_send_frame(CMD_I2C_READ_RESULT, ok ? &value : NULL, ok ? 1 : 0);
}

int handle_i2c_read_buffer_reg(const uint8_t *payload, uint16_t len)
{

}

int handle_i2c_read_reg_16(const uint8_t *payload, uint16_t len)
{
    if (len != 3)
        return -1;
    uint8_t dev_addr = payload[0];
    uint16_t reg_addr = ((uint16_t)payload[1] << 8) | payload[2];
    uint8_t value;
    int ok = i2c_read_reg_16(dev_addr, reg_addr, &value);
    if (ok != 0)
        return -1;
    return 0;
    hdlc_send_frame(CMD_I2C_READ_RESULT, &value, 1);
}

int handle_i2c_read_buffer_reg_16(const uint8_t *payload, uint16_t len)
{

}

// ──────────────── I2C WRITE ────────────────
int handle_i2c_write_reg(const uint8_t *payload, uint16_t len)
{
    if (len != 3)
        return -1;
    uint8_t dev_addr = payload[0];
    uint8_t reg_addr = payload[1];
    uint8_t value = payload[2];
    int ok = i2c_write_reg(dev_addr, reg_addr, value);
    if (ok != 0)
        return -1;
    return 0;
    hdlc_send_frame(CMD_I2C_WRITE_ACK, NULL, 0);
}

int handle_i2c_write_buffer_reg(const uint8_t *payload, uint16_t len)
{

}

int handle_i2c_write_reg_16(const uint8_t *payload, uint16_t len)
{ 
    if (len != 4)
        return -1;
    uint8_t dev_addr = payload[0];
    uint16_t reg_addr = ((uint16_t)payload[1] << 8) | payload[2];
    uint8_t value = payload[3];
    int ok = i2c_write_reg_16(dev_addr, reg_addr, value);
    if (ok != 0)
        return -1;
    return 0;
    
    hdlc_send_frame(CMD_I2C_WRITE_ACK, NULL, 0);
}

int handle_i2c_write_buffer_reg_16(const uint8_t *payload, uint16_t len)
{

}


int handle_i2c_address_find(const uint8_t *payload, uint16_t len)
{

}
// ──────────────── SPI READ ────────────────
int handle_spi_read_reg(const uint8_t *payload, uint16_t len)
{
    if (len != 1)
        return -1;
    uint8_t reg_addr = payload[0];
    uint8_t value;
    int ok = (spi_read_reg(reg_addr, &value) == 0);
    if (ok != 0)
        return -1;
    return 0;
    
    hdlc_send_frame(CMD_SPI_READ_RESULT, ok ? &value : NULL, ok ? 1 : 0);
}

// ──────────────── SPI WRITE ────────────────
int handle_spi_write_reg(const uint8_t *payload, uint16_t len)
{
    if (len != 2)
        return -1;
    uint8_t reg_addr = payload[0];
    uint8_t value = payload[1];
    spi_write_reg(reg_addr, value);
    return 0;

    hdlc_send_frame(CMD_SPI_WRITE_ACK, NULL, 0);
}