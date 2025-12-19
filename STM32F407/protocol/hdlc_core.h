#ifndef HDLC_CORE_H
#define HDLC_CORE_H

#include <stdint.h>
#include "crc16.h"


// CRC16-CCITT (0xFFFF init, poly 0x1021)
// uint16_t crc16_ccitt(const uint8_t* data, uint16_t len);


// ====== 【协议常量】======
#define HDLC_FLAG       0x7E
#define HDLC_ESCAPE     0x7D
#define HDLC_XOR        0x20


#define CMD_WRITE_FLASH_BLOCK   0x0101
#define CMD_FLASH_READ          0x0102
#define CMD_FLASH_READ_RESULT   0x0103
#define CMD_WRITE_FLASH_ACK     0x0104
#define CMD_FLASH_SECTOR_ERASE  0x0105
#define CMD_FLASH_BLOCK_ERASE32  0x0106
#define CMD_FLASH_BLOCK_ERASE64  0x0107
#define CMAD_FLASH_CHIP_ERASE   0x0108
#define CMD_FLASH_ERASE_ACK     0x0120

#define CMD_I2C_READ_REG        0x0201
#define CMD_I2C_16READ_REG      0x0202
#define CMD_I2C_READ_BUFFER_REG 0x0203
#define CMD_I2C_16READ_BUFFER_REG 0x0204
#define CMD_I2C_ADDRESS_FIND    0x0209
#define CMD_I2C_READ_RESULT     0x0210

#define CMD_I2C_WRITE_REG       0x0221
#define CMD_I2C_16WRITE_REG     0x0222
#define CMD_I2C_WRITE_BUFFER_REG 0x0223
#define CMD_I2C_16WRITE_BUFFER_REG 0x0224
#define CMD_I2C_WRITE_ACK       0x0230

#define CMD_SPI_READ_REG        0x0301
#define CMD_SPI_READ_RESULT     0x0303
#define CMD_SPI_WRITE_REG       0x0302
#define CMD_SPI_WRITE_ACK       0x0304

// 命令处理函数原型
typedef int (*cmd_handler_t)(const uint8_t* payload, uint16_t payload_len);


// ====== 【主入口函数】======

/**
 * @brief 处理 HDLC 流数据
 * @param data 流数据
 * @param len 流数据长度
 */
void hdlc_process_stream(const uint8_t *data, uint16_t len);


/**
 * @brief 发送 HDLC 帧（CMD_ID + Payload）
 * @param cmd_id 命令 ID
 * @param payload 负载数据
 * @param payload_len 负载长度
 */
void hdlc_send_frame(uint16_t cmd_id, const uint8_t *payload, uint16_t payload_len);

#endif