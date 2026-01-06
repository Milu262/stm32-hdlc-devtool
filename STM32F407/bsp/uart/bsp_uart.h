

#ifndef __BSP_UART_H__
#define __BSP_UART_H__
// #include "DMA_Init.h"
#include "stm32f4xx.h"

// #include "response.h"

// #define BSP_USART USART1
// #define BSP_USART_RCC RCC_APB2Periph_USART1

// #define BSP_USART_TX_RCC RCC_AHB1Periph_GPIOA
// #define BSP_USART_RX_RCC RCC_AHB1Periph_GPIOA


// #define BSP_USART_TX_PORT GPIOA
// #define BSP_USART_TX_PIN GPIO_Pin_9
// #define BSP_USART_RX_PORT GPIOA
// #define BSP_USART_RX_PIN GPIO_Pin_10
// #define BSP_USART_AF GPIO_AF_USART1
// #define BSP_USART_TX_AF_PIN GPIO_PinSource9
// #define BSP_USART_RX_AF_PIN GPIO_PinSource10

//-------------串口2----------------
#define BSP_USART USART2
#define BSP_USART_RCC RCC_APB1Periph_USART2
#define BSP_USART_AF GPIO_AF_USART2

#define BSP_USART_TX_PORT GPIOD
#define BSP_USART_TX_PIN GPIO_Pin_5
#define BSP_USART_TX_AF_PIN GPIO_PinSource5
#define BSP_USART_TX_RCC RCC_AHB1Periph_GPIOD

#define BSP_USART_RX_PORT GPIOA
#define BSP_USART_RX_PIN GPIO_Pin_3
#define BSP_USART_RX_AF_PIN GPIO_PinSource3
#define BSP_USART_RX_RCC RCC_AHB1Periph_GPIOA

typedef enum {
  RX_STATE_IDLE = 0,      // 接收状态为空闲
  RX_STATE_RECEIVING = 1, // 接收状态为已接收,由串口中断来改变状态
  RX_STATE_OVERFLOW = 2   // 接收状态为溢出
} RxState;                // 接收状态


//  extern uint8_t DMA_Uart_SendBuff[SENDBUFF_SIZE];	// 定义发送缓冲区，用于存储要发送的数据;//发送缓冲区

/**
 * @brief  Initializes the UART1 peripheral with the specified baud rate.
 * @param  __Baud: The baud rate to be set for UART1.
 * @retval None
 */
void uart1_init(uint32_t __Baud);
/**
 * @brief  Sends a single data byte through UART.
 * @param  ucch: The data byte to be sent.
 * @retval None
 */
void usart_send_data(uint8_t ucch);
/**
 * @brief  通过UART发送字符串数据.
 * @param  ucstr: 要发送的字符串指针.
 * @retval None
 */
void usart_send_String(uint8_t *ucstr);

/**
 * @brief  设置接收数据状态为已接收.
 */
void enter_rx_Receive(void);

/**
 * @brief  设置接收数据状态为IDLE.
 */
void enter_rx_IDLE(void);

/**
 * @brief  设置接收数据状态为Overflow.
 */
void enter_rx_Overflow(void);

/**
 * @brief  获取串口接收数据状态.
 * @param  None
 * @retval The current state of the receive buffer.
 */
RxState get_rx_status(void);
#endif
