#include "hdlc_core.h"
#include <string.h>
#include <stdarg.h>
#include "cmd_handlers.h"
#include "./protocol_driver/uart_driver.h"

#define FRAME_BUF_SIZE 266
#define TX_BUF_SIZE 256
// 假设最大有效载荷为 520 字节（根据你的协议设计）
#define HDLC_MAX_PAYLOAD_LEN 520

// 命令分发表（按 CMD_ID 排序，支持二分查找；但这里用线性足够）
typedef struct
{
    uint16_t cmd_id;
    cmd_handler_t handler;
    uint16_t expected_min_len; // 可选：用于快速校验
    uint16_t cmd_ack;
} cmd_entry_t;

static const cmd_entry_t cmd_table[] = {
    {CMD_FLASH_READ, handle_flash_read, 6},
    {CMD_WRITE_FLASH_BLOCK, handle_flash_write, 6},
    {CMD_FLASH_SECTOR_ERASE, handle_flash_SectionErase, 4},
    {CMD_FLASH_BLOCK_ERASE32, handle_flash_BlockErase32, 4},
    {CMD_FLASH_BLOCK_ERASE64, handle_flash_BlockErase64, 4},
    {CMAD_FLASH_CHIP_ERASE, handle_flash_ChipErase, 0},
    {CMD_I2C_READ_REG, handle_i2c_read_reg, 2},
    {CMD_I2C_WRITE_REG, handle_i2c_write_reg, 3},
    {CMD_I2C_16READ_REG, handle_i2c_read_reg_16, 3},
    {CMD_I2C_16WRITE_REG, handle_i2c_write_reg_16, 4},
    {CMD_I2C_READ_BUFFER_REG, handle_i2c_read_buffer_reg, 4},
    // 注意 write buffer不需要payload里包含需要读的字节数,可以根据payload_len计算得到
    {CMD_I2C_WRITE_BUFFER_REG, handle_i2c_write_buffer_reg, 3},
    {CMD_I2C_16READ_BUFFER_REG, handle_i2c_read_buffer_reg_16, 5},
    // 注意 write buffer不需要payload里包含需要读的字节数
    {CMD_I2C_16WRITE_BUFFER_REG, handle_i2c_write_buffer_reg_16, 4},
    {CMD_I2C_ADDRESS_FIND, handle_i2c_address_find, 0},
    {CMD_SPI_READ_REG, handle_spi_read_reg, 1},
    {CMD_SPI_WRITE_REG, handle_spi_write_reg, 2},
    // 新命令在这里追加 ↓
    // { CMD_NEW_CMD,         handle_new_cmd,         4 },
};

#define CMD_TABLE_SIZE (sizeof(cmd_table) / sizeof(cmd_table[0]))

// static uint8_t tx_buffer[TX_BUF_SIZE];

// 全局发送缓冲区（足够容纳最坏转义情况）
#define TX_FRAME_BUF_SIZE 520
static uint8_t tx_frame_buf[TX_FRAME_BUF_SIZE];

// ────────────────────────────────────────
// 1. 将 CMD_ID + Payload 写入原始缓冲区
// ────────────────────────────────────────
/**
 * @brief 构建原始 HDLC 帧（CMD_ID + Payload）
 * @param buf 目标缓冲区
 * @param buf_size 缓冲区大小
 * @param cmd_id 命令 ID
 * @param payload 负载数据
 * @param payload_len 负载长度
 * @return uint16_t 实际写入长度（0 表示失败）
 */
static uint16_t build_raw_frame(uint8_t *buf, uint16_t buf_size,
                                uint16_t cmd_id, const uint8_t *payload, uint16_t payload_len)
{
    // 检查输出缓冲区是否足够存放 CMD ID（至少 2 字节）
    if (buf == NULL || buf_size < 2)
    {
        return 0;
    }

    // 限制最大 payload 长度（根据协议设计）
    if (payload_len > 256)
    {
        return 0;
    }

    // 关键安全检查：若 payload_len > 0，则 payload 不能为 NULL
    if (payload_len > 0 && payload == NULL)
    {
        return 0; // 非法参数组合
    }

    // 写入命令 ID（大端）
    buf[0] = (cmd_id >> 8) & 0xFF;
    buf[1] = cmd_id & 0xFF;

    // 写入 payload（如果存在）
    if (payload_len > 0)
    {
        if (2 + payload_len > buf_size)
        {
            return 0; // 缓冲区不足
        }
        memcpy(&buf[2], payload, payload_len);
    }

    return 2 + payload_len;
}

// ────────────────────────────────────────
// 2. 对数据进行 HDLC 转义，并添加首尾 FLAG
// 输入：raw_data[raw_len]
// 输出：写入到 escaped_buf，返回最终长度
// ────────────────────────────────────────

/**
 * @brief 对原始数据进行 HDLC 转义
 * @param raw_data 原始数据
 * @param raw_len 原始数据长度
 * @param escaped_buf 转义后的缓冲区
 * @param escaped_buf_size 转义后的缓冲区大小
 * @return uint16_t 实际写入长度（0 表示失败）
 */
static uint16_t hdlc_escape_frame(const uint8_t *raw_data, uint16_t raw_len,
                                  uint8_t *escaped_buf, uint16_t escaped_buf_size)
{
    // 防御性检查
    if (escaped_buf == NULL)
    {
        return 0;
    }
    if (raw_len > 0 && raw_data == NULL)
    {
        return 0;
    }

    // 最小帧：FLAG + CMD(2) + CRC(2) + FLAG = 6 字节
    if (escaped_buf_size < 6)
    {
        return 0;
    }

    uint16_t idx = 0;

    // 起始 FLAG
    escaped_buf[idx++] = HDLC_FLAG;

    // 转义原始数据（CMD + Payload）
    for (uint16_t i = 0; i < raw_len; i++)
    {
        uint8_t b = raw_data[i];
        if (b == HDLC_FLAG || b == HDLC_ESCAPE)
        {
            if (idx + 2 > escaped_buf_size)
                return 0;
            escaped_buf[idx++] = HDLC_ESCAPE;
            escaped_buf[idx++] = b ^ HDLC_XOR;
        }
        else
        {
            if (idx + 1 > escaped_buf_size)
                return 0;
            escaped_buf[idx++] = b;
        }
    }

    // 计算并转义 CRC（大端）
    uint16_t crc = crc16_ccitt(raw_data, raw_len);
    uint8_t crc_bytes[2] = {(crc >> 8) & 0xFF, crc & 0xFF};
    for (uint8_t i = 0; i < 2; i++)
    {
        uint8_t b = crc_bytes[i];
        if (b == HDLC_FLAG || b == HDLC_ESCAPE)
        {
            if (idx + 2 > escaped_buf_size)
                return 0;
            escaped_buf[idx++] = HDLC_ESCAPE;
            escaped_buf[idx++] = b ^ HDLC_XOR;
        }
        else
        {
            if (idx + 1 > escaped_buf_size)
                return 0;
            escaped_buf[idx++] = b;
        }
    }

    // 结束 FLAG
    if (idx >= escaped_buf_size)
        return 0;
    escaped_buf[idx++] = HDLC_FLAG;

    return idx;
}

// ────────────────────────────────────────
// 3. 主发送函数：协调构建、转义、发送
// ────────────────────────────────────────

void hdlc_send_frame(uint16_t cmd_id, const uint8_t *payload, uint16_t payload_len)
{
    // 安全检查 1: payload 为 NULL 时，payload_len 必须为 0
    if (payload == NULL)
    {
        if (payload_len != 0)
        {
            return; // 非法参数：不能有长度却无数据
        }
    }

    // 安全检查 2: 超出最大 payload 限制
    if (payload_len > HDLC_MAX_PAYLOAD_LEN)
    {
        return;
    }

    // Step 1: 构建原始帧 (CMD + Payload)
    uint8_t raw_buf[2 + HDLC_MAX_PAYLOAD_LEN]; // 2 字节 CMD + 最大 payload
    uint16_t raw_len = build_raw_frame(raw_buf, sizeof(raw_buf), cmd_id, payload, payload_len);
    if (raw_len == 0)
    {
        return;
    }

    // Step 2: HDLC 转义（加 FLAG、CRC、字节填充）
    uint16_t frame_len = hdlc_escape_frame(raw_buf, raw_len, tx_frame_buf, TX_FRAME_BUF_SIZE);
    // printf("[HDLC] Sending frame (%d bytes)\n", frame_len);
    if (frame_len == 0)
    {
        return;
    }

    // Step 3: 通过 DMA 发送
    uart_send_buffer_DMA(tx_frame_buf, frame_len);
}

/**
 * @brief 处理 HDLC 帧
 * @param frame 帧数据
 * @param len 帧长度
 */
static void hdlc_process_frame(const uint8_t *frame, uint16_t len)
{
    if (len < 2)
        return;

    uint16_t cmd_id = (frame[0] << 8) | frame[1];
    const uint8_t *payload = frame + 2;
    uint16_t payload_len = len - 2;

    // 查找命令处理器
    for (uint8_t i = 0; i < CMD_TABLE_SIZE; i++)
    {
        if (cmd_table[i].cmd_id == cmd_id)
        {
            // 可选：快速长度校验
            if (payload_len < cmd_table[i].expected_min_len)
            {
                // debug_printf("[CMD] Invalid len for 0x%04X\n", cmd_id);
                return;
            }
            cmd_table[i].handler(payload, payload_len);
            // TODO 每个handler必须有返回值，且函数返回值必须为0表示正常，其他错误码表示出错
            // TODO 如果函数正常调用没有出错就在下面调用hdlc_send_frame返回ACK帧
            return;
        }
    }

    // 未知命令（可选：返回错误帧）
    // debug_printf("[CMD] Unknown command: 0x%04X\n", cmd_id);
}

static uint8_t hdlc_frame_buf[FRAME_BUF_SIZE];
static uint16_t hdlc_frame_index = 0;
static uint8_t hdlc_escaped = 0;
static uint8_t hdlc_in_frame = 0; // 新增：是否正在接收 HDLC 帧

static void hdlc_handle_frame_end(void)
{
    // 最小有效帧：CMD(2) + CRC(2) = 4 字节
    if (hdlc_frame_index < 4)
    {
        debug_printf("[HDLC] Frame too short\n");
        return;
    }

    uint16_t payload_with_cmd_len = hdlc_frame_index - 2; // 不含 CRC
    uint16_t recv_crc = (hdlc_frame_buf[payload_with_cmd_len] << 8) |
                        hdlc_frame_buf[payload_with_cmd_len + 1];
    uint16_t calc_crc = crc16_ccitt(hdlc_frame_buf, payload_with_cmd_len);

    if (recv_crc == calc_crc)
    {
        hdlc_process_frame(hdlc_frame_buf, payload_with_cmd_len);
    }
    else
    {
        debug_printf("[HDLC] CRC Error\n");
    }
}
void hdlc_process_stream(const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        uint8_t byte = data[i];

        // 收到 FLAG：帧边界
        if (byte == HDLC_FLAG)
        {
            if (hdlc_in_frame)
            {
                hdlc_handle_frame_end(); // 处理完整帧

                // 解包数据正常
                //  usart_send_String_DMA(hdlc_frame_buf,hdlc_frame_index);//测试数据
                hdlc_in_frame = 0;
                return;
            }
            // 开始新帧（即使刚结束，也重置状态）
            hdlc_in_frame = 1;
            hdlc_frame_index = 0;
            hdlc_escaped = 0;
            continue;
        }

        // 非 FLAG 字节：仅在帧内处理
        if (!hdlc_in_frame)
        {
            // 可选：透传非 HDLC 字符（如调试命令）
            // debug_printf("%c", byte);
            continue;
        }

        // 处理解转义
        uint8_t real_byte = byte;
        if (hdlc_escaped)
        {
            real_byte ^= HDLC_XOR;
            hdlc_escaped = 0;
        }
        else if (byte == HDLC_ESCAPE)
        {
            hdlc_escaped = 1;
            continue; // 等待下一个字节
        }

        // 存入缓冲区
        if (hdlc_frame_index < sizeof(hdlc_frame_buf))
        {
            hdlc_frame_buf[hdlc_frame_index++] = real_byte;
        }
        else
        {
            // 溢出：丢弃当前帧
            hdlc_in_frame = 0;
            debug_printf("[HDLC] Buffer overflow\n");
        }
    }
}