

#include "bsp_uart.h"
#include "stdio.h"
#include "NVIC_Init.h"
#include "DMA_Init.h"
// #include "response.h"
void uart1_init(uint32_t __Baud)
{
	USART_DeInit(BSP_USART); // 复位串口
	GPIO_InitTypeDef GPIO_InitStructure;

	// RCC_APB2PeriphClockCmd(BSP_USART_RCC, ENABLE);//串口1
	RCC_APB1PeriphClockCmd(BSP_USART_RCC, ENABLE); // 串口2

	RCC_AHB1PeriphClockCmd(BSP_USART_TX_RCC, ENABLE);
	RCC_AHB1PeriphClockCmd(BSP_USART_RX_RCC, ENABLE);

	GPIO_PinAFConfig(BSP_USART_TX_PORT, BSP_USART_TX_AF_PIN, BSP_USART_AF);
	GPIO_PinAFConfig(BSP_USART_RX_PORT, BSP_USART_RX_AF_PIN, BSP_USART_AF);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = BSP_USART_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(BSP_USART_TX_PORT, &GPIO_InitStructure);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = BSP_USART_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(BSP_USART_RX_PORT, &GPIO_InitStructure);

	USART_InitTypeDef USART_InitStructure;

	// USART_DeInit(BSP_USART);
	// USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate = __Baud;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(BSP_USART, &USART_InitStructure);
	// USART_ClearFlag(BSP_USART, USART_FLAG_RXNE);
	// USART_ITConfig(BSP_USART, USART_IT_RXNE, ENABLE); // 使能接收中断
	USART_ITConfig(BSP_USART, USART_IT_IDLE, ENABLE); // 使能空闲中断

	USART_DMACmd(BSP_USART, USART_DMAReq_Tx, ENABLE); // 使能串口1的DMA发送
	USART_DMACmd(BSP_USART, USART_DMAReq_Rx, ENABLE); // 使能串口1的DMA接收

	// DMA_Uart1_Init_Config(); // DMA初始化
	USART_Cmd(BSP_USART, ENABLE); // 使能串口1
}

void usart_send_data(uint8_t ucch)
{
	USART_SendData(BSP_USART, (uint8_t)ucch);

	while (RESET == USART_GetFlagStatus(BSP_USART, USART_FLAG_TXE))
	{
	}
}

void usart_send_String(uint8_t *ucstr)
{
	while (ucstr && *ucstr)
	{
		usart_send_data(*ucstr++);
	}
}
#ifdef USE_GCC
// 使用GCC编译器时，需要使用以下函数

#endif

#ifdef USE_Keil
// 使用Keil编译器时，需要使用以下函数
#if !defined(__MICROLIB)

#if (__ARMCLIB_VERSION <= 6000000)

struct __FILE
{
	int handle;
};
#endif

FILE __stdout;

void _sys_exit(int x)
{
	x = x;
}
#endif

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{

	usart_send_String_DMA((uint8_t *)&ch, 1); // 使用DMA发送单个字符

	return ch;
}

int fgetc(FILE *f)
{

	while (USART_GetFlagStatus(BSP_USART, USART_FLAG_RXNE) == RESET)
		;

	return (int)USART_ReceiveData(BSP_USART);
}
#endif

static volatile RxState _RxStatus = RX_STATE_IDLE;//定义接收状态

void enter_rx_Receive(void)
{
	_RxStatus = RX_STATE_RECEIVING;
}

void enter_rx_IDLE(void)
{
	_RxStatus = RX_STATE_IDLE;
}

void enter_rx_Overflow(void)
{
	_RxStatus = RX_STATE_OVERFLOW;
}
RxState get_rx_status(void)
{
	return _RxStatus;
}
