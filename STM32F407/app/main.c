
#include <stdio.h>
#include <string.h>
#include "../bsp/DMA/DMA_Init.h"
#include "../board/board.h"
#include "../bsp/uart/bsp_uart.h"
#include "hdlc_core.h"
#include "../bsp/Response/response.h"

// #define SHT30_SENSOR_ADDR 0x44 /*!< Slave address of the SHT30 sensor */

static uint8_t ReceiveData[USART_MAX_LEN] = {0};
static uint16_t RxCount = 0;
int main(void)
{

	uint8_t error = 0;
	error = hardware_init();

	while (1)
	{

		if (get_rx_status() != RX_STATE_RECEIVING)
			continue;
		enter_rx_IDLE();
		RxCount = GetUsartRxCount();
		uart_copy_receive_data(ReceiveData, RxCount);
		hdlc_process_stream(ReceiveData, RxCount);
	}
}
