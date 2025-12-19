#ifndef CMD_HANDLERS_H
#define CMD_HANDLERS_H

#include <stdint.h>

// 命令处理函数类型（与 hdlc_core 共享）
typedef int (*cmd_handler_t)(const uint8_t* payload, uint16_t payload_len);

// 声明各个命令处理函数（供 hdlc_core.c 使用）

/**
 * @brief 处理 FLASH 读取命令
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 */
int handle_flash_read(const uint8_t* payload, uint16_t len);

/**
 * @brief 处理 FLASH 写入命令
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 */
int handle_flash_write(const uint8_t* payload, uint16_t len);

/**
 * @brief 处理 FLASH 扇区擦除命令
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 */
int handle_flash_SectionErase(const uint8_t *payload, uint16_t len);

/**
 * @brief 处理 FLASH 块擦除命令(32K)
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 */
int handle_flash_BlockErase32(const uint8_t *payload, uint16_t len);

/**
 * @brief 处理 FLASH 块擦除命令(64K)
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 */
int handle_flash_BlockErase64(const uint8_t *payload, uint16_t len);

/**
 * @brief 处理 FLASH 芯片擦除命令
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 */
int handle_flash_ChipErase(const uint8_t *payload, uint16_t len);

/**
 * @brief 处理 I2C 读取寄存器命令
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 */
int handle_i2c_read_reg(const uint8_t* payload, uint16_t len);

/**
 * @brief 处理 I2C 读取16位寄存器命令
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 */
int handle_i2c_read_reg_16(const uint8_t *payload, uint16_t len);
/**
 * @brief 处理 I2C 写入寄存器命令
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 */
int handle_i2c_write_reg(const uint8_t* payload, uint16_t len);

/**
 * @brief 处理 I2C 读取16位寄存器命令
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 */
int handle_i2c_write_reg_16(const uint8_t *payload, uint16_t len);

/**
 * @brief I2C读多个数据
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 * @return 0: 成功,-1: 失败
 */
int handle_i2c_read_buffer_reg(const uint8_t *payload, uint16_t len);

/**
 * @brief I2C 写多个数据
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 * @return 0: 成功,-1: 失败
 */
int handle_i2c_write_buffer_reg(const uint8_t *payload, uint16_t len);

/**
 * @brief I2C 读多个数据（16-bit 地址）
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 * @return 0: 成功,-1: 失败
 */
int handle_i2c_read_buffer_reg_16(const uint8_t *payload, uint16_t len);

/**
 * @brief I2C 写多个数据（16-bit 地址）
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 * @return 0: 成功,-1: 失败
 */
int handle_i2c_write_buffer_reg_16(const uint8_t *payload, uint16_t len);

/**
 * @brief 寻找I2C设备
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 * @return 0: 找到,-1: 未找到
 */
int handle_i2c_address_find(const uint8_t *payload, uint16_t len);




/****************SPI 命令处理函数（供 hdlc_core.c 使用） ***************** */

/**
 * @brief 处理 SPI 读取寄存器命令
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 */
int handle_spi_read_reg(const uint8_t* payload, uint16_t len);
/**
 * @brief 处理 SPI 写入寄存器命令
 * @param payload: 命令负载指针
 * @param len: 负载长度（字节）
 */
int handle_spi_write_reg(const uint8_t* payload, uint16_t len);

#endif // CMD_HANDLERS_H