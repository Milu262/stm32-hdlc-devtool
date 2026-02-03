#include "NVIC_Init.h"

void NVIC_Configuration(void)
{
  // 配置串口1的NVIC
  NVIC_InitTypeDef NVIC_InitStructure;                      // 定义一个NVIC初始化结构体变量
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);           // 配置中断优先级分组，选择组2（抢占优先级占2位，子优先级占2位）,并且数越小，优先级越高
  NVIC_InitStructure.NVIC_IRQChannel = USART_IRQ;           // 设置USART中断通道
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // 设置USART1抢占优先级为2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        // 设置USART1子优先级为1
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 启用USART1中断通道
  NVIC_Init(&NVIC_InitStructure);                           // 根据配置初始化NVIC

  NVIC_InitStructure.NVIC_IRQChannel = USART_DMA_RX_IRQ;    // 设置串口1接收中断通道为DMA2_Stream5
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // 设置DMA2_Stream5抢占优先级为2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;        // 设置DMA2_Stream5子优先级为2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 启用DMA2_Stream5中断通道
  NVIC_Init(&NVIC_InitStructure);                           // 根据配置初始化NVIC

  NVIC_InitStructure.NVIC_IRQChannel = USART_DMA_TX_IRQ;    // 设置串口1发送中断通道为DMA2_Stream7
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // 设置DMA2_Stream7抢占优先级为2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        // 设置DMA2_Stream7子优先级为3
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 启用DMA2_Stream7中断通道
  NVIC_Init(&NVIC_InitStructure);                           // 根据配置初始化NVIC

  NVIC_InitStructure.NVIC_IRQChannel = DCMI_DMA_IRQ;        // 设置DCMI的DMA中断通道为DMA2_Stream1
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 设置DMA2_Stream1抢占优先级为1
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        // 设置DMA2_Stream1子优先级为3
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 启用DMA2_Stream1中断通道
  NVIC_Init(&NVIC_InitStructure);                           // 根据配置初始化NVIC

  // 设置DCMI的帧中断
  NVIC_InitStructure.NVIC_IRQChannel = DCMI_DCMI_IRQ;       // 设置DCMI帧中断通道
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 设置DCMI帧中断抢占优先级为0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;        // 设置DCMI帧中断子优先级为3
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 启用DCMI帧中断通道
  NVIC_Init(&NVIC_InitStructure);                           // 根据配置初始化NVIC

  NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;       // 设置TIM6中断通道
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 设置TIM6抢占优先级为0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        // 设置TIM6子优先级为3
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // 启用TIM6中断通道
  NVIC_Init(&NVIC_InitStructure);                           // 根据配置初始化NVIC
}