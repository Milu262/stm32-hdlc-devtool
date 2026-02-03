
#include <stdio.h>
#include <string.h>
#include "../bsp/DMA/DMA_Init.h"
#include "../board/board.h"
#include "../bsp/uart/bsp_uart.h"
#include "hdlc_core.h"
#include "../bsp/Response/response.h"
#include "../bsp/TIM/tim.h"
#include "SPI_Screen_init.h"


#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"


// #define SHT30_SENSOR_ADDR 0x44 /*!< Slave address of the SHT30 sensor */

static uint8_t ReceiveData[USART_MAX_LEN] = {0};
static uint16_t RxCount = 0;
int main(void)
{

	uint8_t error = 0;
	error = hardware_init();


	lv_init();//lvgl初始化
	lv_port_disp_init();//注册lvgl的显示任务
	lv_port_indev_init();//注册lvgl的输入任务

	//按钮
	lv_obj_t *btn = lv_btn_create(lv_scr_act());
	lv_obj_set_pos(btn, 10, 10);//设置按钮的位置
	lv_obj_set_size(btn, 100, 50);//设置按钮的大小

	lv_obj_t *label_btn = lv_label_create(btn);//创建一个标签,父对象为按钮
	lv_obj_align(label_btn, LV_ALIGN_CENTER, 0, 0);//对齐于父对象
	lv_label_set_text(label_btn, "Button");//设置标签的文本

	lv_obj_t *label = lv_label_create(lv_scr_act());//创建文本标签，父对象为当前屏幕
	lv_label_set_text(label, "Hello World!");
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_align_to(btn,label , LV_ALIGN_OUT_TOP_MID, 0, -20);

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
