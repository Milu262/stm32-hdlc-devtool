/**
 ******************************************************************************
 * @file    Project/STM32F4xx_StdPeriph_Templates/stm32f4xx_it.c
 * @author  MCD Application Team
 * @version V1.8.1
 * @date    27-January-2022
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and
 *          peripherals interrupt service routine.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2016 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "bsp_uart.h"
#include "DMA_Init.h"
#include "response.h"
#include <stdint.h>

/** @addtogroup Template_Project
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief  This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
void SVC_Handler(void)
{
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief  This function handles PendSVC exception.
 * @param  None
 * @retval None
 */
void PendSV_Handler(void)
{
}

/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
// void USART1_IRQHandler(void)
// {

//   // if (USART_GetITStatus(BSP_USART, USART_IT_RXNE) != RESET)//触发接收中断
//   // {
//   //   test_data.data[test_data.len] = USART_ReceiveData(BSP_USART);

//   //   if (++test_data.len > data_size - 2)
//   //   {
//   //     test_data.len = 0;
//   //   }
//   // }

//   // if (USART_GetITStatus(BSP_USART, USART_IT_IDLE) != RESET)
//   // {

//   //   USART_ReceiveData(BSP_USART);
//   //   test_data.data[test_data.len] = '\0';
//   //   test_data.flag = 1;
//   // }

//   // 以下是DMA的方式
//   if (USART_GetITStatus(BSP_USART, USART_IT_IDLE) != RESET) // 触发空闲中断
//   {
//     test_data.flag = 1;                                                                 // 接收完成标志
//     DMA_Cmd(DEBUG_USART_RX_DMA_STREAM, DISABLE);                                       // 暂时关闭接收串口的DMA，数据待处理
//     usart1_rx_len = USART_MAX_LEN - DMA_GetCurrDataCounter(DEBUG_USART_RX_DMA_STREAM); // 计算接收到的数据长度
//     DMA_ClearFlag(DEBUG_USART_RX_DMA_STREAM, DMA_FLAG_TCIF5);                          // 清除接收完成标志
//     // 做自己的事,例如将接收到的数据回显
//     // usart_send_String_DMA(DMA_USART1_RX_BUF, usart1_rx_len); // 回显
//     if (usart1_rx_len > data_size - 2)
//     {
//       test_data.DataOverflow = 1; // 接收数据溢出
//     }
//     // else
//     // {
//     //   memcpy(test_data.data, DMA_USART1_RX_BUF, usart1_rx_len); // 将接收到的数据复制到test_data.data
//     //   test_data.data[usart1_rx_len] = '\0';
//     // }

//     DMA_SetCurrDataCounter(DEBUG_USART_RX_DMA_STREAM, USART_MAX_LEN); // 重新设置DMA的缓冲区大小
//     DMA_Cmd(DEBUG_USART_RX_DMA_STREAM, ENABLE);                       // 重新开启接收串口的DMA
//     USART_ReceiveData(BSP_USART);                                      // 清除接收中断标志
//   }

//   if (USART_GetITStatus(BSP_USART, USART_IT_TC) != RESET) // 串口发送完成
//   {

//     USART_ClearITPendingBit(BSP_USART, USART_IT_TC); // 清除串口发送完成标志位
//     USART_ITConfig(USART1, USART_IT_TC, DISABLE);    // 关闭串口发送完成中断
//     // usart1_rx_len = 0;                            // 清空接收数据长度
//     //  printf("send ok\r\n");
//   }
// }

void USART2_IRQHandler(void)
{
  if (USART_GetITStatus(BSP_USART, USART_IT_IDLE) == RESET)
  {
    return;
  }
  // 触发空闲中断
  enter_rx_Receive();
  test_data.flag = 1;                          // 接收完成标志
  DMA_Cmd(DEBUG_USART_RX_DMA_STREAM, DISABLE); // 暂时关闭接收串口的DMA，数据待处理
  uint16_t RxCount = USART_MAX_LEN - DMA_GetCurrDataCounter(DEBUG_USART_RX_DMA_STREAM);
  SetUsartRxCount(RxCount);
  DMA_ClearFlag(DEBUG_USART_RX_DMA_STREAM, DMA_FLAG_TCIF5); // 清除接收完成标志
  if (RxCount > USART_MAX_LEN)
  {
    enter_rx_Overflow();
  }

  DMA_SetCurrDataCounter(DEBUG_USART_RX_DMA_STREAM, USART_MAX_LEN); // 重新设置DMA的缓冲区大小
  DMA_Cmd(DEBUG_USART_RX_DMA_STREAM, ENABLE);                       // 重新开启接收串口的DMA
  USART_ReceiveData(BSP_USART);                                     // 清除接收中断标志
}
/**
 * @brief  该函数处理DMA2 Stream7中断请求。
 * @note   当USART1的DMA发送完成时触发此中断。中断处理程序会清除DMA中断标志，
 *         禁用DMA发送流，并启用USART发送完成中断。
 * @param  无
 * @retval 无
 */
// void DMA2_Stream7_IRQHandler(void)
// {
//   if (DMA_GetITStatus(DEBUG_USART_TX_DMA_STREAM, DMA_IT_TCIF7) != RESET) // 等待DMA发送完成中断
//   {
//     DMA_ClearITPendingBit(DEBUG_USART_TX_DMA_STREAM, DMA_IT_TCIF7); // 清除DMA发送完成中断标志
//     DMA_Cmd(DEBUG_USART_TX_DMA_STREAM, DISABLE);                    // 关闭DMA发送
//     USART_ITConfig(USART1, USART_IT_TC, ENABLE);                     // 使能串口发送完成中断
//   }
// }

void DMA1_Stream6_IRQHandler(void)
{
  if (DMA_GetITStatus(DEBUG_USART_TX_DMA_STREAM, DMA_IT_TCIF6) == RESET)
  {
    return;
  }
  // 等待DMA发送完成中断
  DMA_ClearITPendingBit(DEBUG_USART_TX_DMA_STREAM, DMA_IT_TCIF6); // 清除DMA发送完成中断标志
}
/**
 * @brief  该函数处理 DCMI的DMA 传输中断请求。
 * @note   当 DCMI的DMA 触发中断时，会调用此函数。通常用于处理行接收完成、
 *         溢出、同步错误等事件。在中断服务程序中，需要检查具体的中断标志，
 *         执行相应的处理逻辑，并清除对应的中断挂起位。
 * @param  无
 * @retval 无
 */
// void DMA2_Stream1_IRQHandler(void)
// {
//   if (DMA_GetITStatus(DCMI_DMA_STREAM, DMA_IT_TCIF1) != RESET) // 等待DMA传输完成中断,表示摄像头的一行数据已经传输完成
//   {
//     DMA_ClearITPendingBit(DCMI_DMA_STREAM, DMA_IT_TCIF1); // 清除DMA接收完成中断标志

//     // 做自己的事
//   }
// }
/**
 * @brief  该函数处理 DCMI 中断请求。
 * @note   当 DCMI 外设触发中断时，会调用此函数。通常用于处理帧捕获完成、
 *         溢出、同步错误等事件。在中断服务程序中，需要检查具体的中断标志，
 *         执行相应的处理逻辑，并清除对应的中断挂起位。
 * @param  无
 * @retval 无
 */
void DCMI_IRQHandler(void)
{
  if (DCMI_GetITStatus(DCMI_IT_FRAME) != RESET) // 触发帧中断
  {
    DCMI_ClearITPendingBit(DCMI_IT_FRAME); // 清除帧中断标志

    // 做自己的事，例如让屏幕重新回到(0,0)位置

    Frame_it = 1;

    // 以下为测试代码
    //   DCMI_Cmd(DISABLE);                     // 关闭DCMI
    //   DCMI_ClearFlag(DCMI_FLAG_FRAMERI);     // 清除帧标志
  }
}

void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
 * @brief  This function handles PPP interrupt request.
 * @param  None
 * @retval None
 */
/*void PPP_IRQHandler(void)
{
}*/

/**




  * @}
  */
