
#include <stdio.h>
#include <string.h>
#include "../bsp/DMA/DMA_Init.h"
#include "../board/board.h"
#include "../bsp/uart/bsp_uart.h"
#include "hdlc_core.h"
#include "../bsp/Response/response.h"
#include "../bsp/TIM/tim.h"
#include "SPI_Screen_init.h"

#include "../lvgl/lvgl_gui_main.h"
#include "../lvgl/src/misc/lv_timer.h"

// #define SHT30_SENSOR_ADDR 0x44 /*!< Slave address of the SHT30 sensor */

static uint8_t ReceiveData[USART_MAX_LEN] = {0};
static uint16_t RxCount = 0;
int main(void)
{

	uint8_t error = 0;
	error = hardware_init();
	lvgl_gui_main();
	TIM_Mode_Config();


	while (1)
	{
		delay_ms(5);//延时5ms
		lv_timer_handler();//lvgl任务刷新函数
		if (get_rx_status() != RX_STATE_RECEIVING)
			continue;
		enter_rx_IDLE();
		RxCount = GetUsartRxCount();
		uart_copy_receive_data(ReceiveData, RxCount);
		hdlc_process_stream(ReceiveData, RxCount);
	}
}


/**
 * @brief  TIM6中断服务程序
 * @details TIM6中断服务程序，用于LVGL心跳任务（1ms)
 */
void BASIC_TIM_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
	{
		lv_tick_inc(1);
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	}
}