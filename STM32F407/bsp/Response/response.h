#ifndef __RESPONSE_H__
#define __RESPONSE_H__
#include <stdint.h>
// 定义最大数据长度
#define MaxDataSize 256

#define hdlc_data_size 256 + 32	 // 数据长度
#define Frame_header 0x55	 // 接收帧头
#define SendFrameHeader 0xFA // 发送帧头

#define MODE_UART 0x01
#define MODE_I2C_8BIT_ADDR 0x02
#define MODE_I2C_16BIT_ADDR 0x03
#define MODE_SPI 0x04

typedef struct
{
	unsigned int flag; // 标志位
	// unsigned int err;		   // 数据类型
	unsigned int DataOverflow; // 数据溢出
	unsigned int len;
	uint8_t data[hdlc_data_size];	 // 接收数据
	uint8_t SendData[hdlc_data_size]; // 发送数据

} usart_data_typed;

typedef enum
{
	ReceiveHeader = Frame_header, // 接收帧头
	SendHeader = SendFrameHeader, // 发送帧头
} header_typed;

/**
 * @brief 通信模式枚举类型
 * @details 定义了系统支持的不同通信接口模式
 */
typedef enum
{
	UartMode,			 // UART串口通信模式
	I2C_8BIT_ADDR_Mode,	 // I2C通信模式，8位地址
	I2C_16BIT_ADDR_Mode, // I2C通信模式，16位地址
	SPI_Mode,			 // SPI通信模式
} usart_mode_typed;		 // 通信模式枚举类型名

// 定义一个结构体，用于存储串口接收的数据帧,包含帧头、模式、长度、校验和、数据
typedef struct
{
	header_typed header;
	usart_mode_typed mode;
	uint8_t len;
	uint8_t data[MaxDataSize];
	uint8_t checksum;

} usart_frame_typed;

// 声明一个外部变量 test_data，其类型为 usart_data_typed
// 这意味着 test_data 变量定义在别的源文件中，但在此文件中可以访问和使用
extern usart_data_typed test_data;
extern uint8_t Frame_it;

/**
 * @brief 处理接收到的数据
 * @param udata 指向包含接收数据的结构体的指针
 * @return uint8_t 处理结果，0表示失败，1表示成功
 */
uint8_t do_process(usart_data_typed *udata);

/**
 * @brief 处理串口接收数据
 * @param RxCount 接收数据的长度
 * @return int 处理结果，0表示成功，-1表示失败，-2表示数据溢出
 */
int response_handle(uint16_t RxCount);
#endif