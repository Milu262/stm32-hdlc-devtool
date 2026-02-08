#include <board.h>

#include "../libraries/CMSIS/Device/ST/STM32F4xx/Include/stm32f4xx.h"
#include "bsp_uart.h"
#include "NVIC_Init.h"

#include "i2c_init.h"
#include "i2c_handle.h"
#include "response.h"
#include "spi_init.h"
#include "SPI_Control.h"
#include "DMA_Init.h"
#include "SPI_Screen_init.h"
#include "OV2640_init.h"

#include "Test_code.h"
// static __IO uint32_t g_system_tick = 0;

/**
 * @brief 板级初始化,适用于LCKFB的STM32F407开发板
 *
 */
static void board_init(void)
{
    /* NVIC Configuration */
#define NVIC_VTOR_MASK 0x3FFFFF80
#ifdef VECT_TAB_RAM
    /* Set the Vector Table base location at 0x10000000 */
    SCB->VTOR = (0x10000000 & NVIC_VTOR_MASK);
#else /* VECT_TAB_FLASH  */
    /* Set the Vector Table base location at 0x08000000 */
    SCB->VTOR = (0x08000000 & NVIC_VTOR_MASK);
#endif

    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    SysTick->LOAD = 0xFFFF;                   // ��ռ�����
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // ��ʼ����

    //	RCC_ClocksTypeDef rcc;
    //	RCC_GetClocksFreq(&rcc);//��ȡϵͳʱ��Ƶ��
}

#define usart_baud_rate 2000000

int hardware_init(void)
{
    board_init();                // 板级初始化
    NVIC_Configuration();        // 中断优先级初始化
    uart1_init(usart_baud_rate); // 串口1初始化
    DMA_Uart1_Init_Config();     // DMA串口1初始化
    User_I2C_Init();             // I2C初始化
    SPI_FLASH_BUS_Init();        // FLASH总线初始化
    // test_SPI_Flash();		 // 测试FLASH
    SPI_Screen_BUS_Init();       // SPI屏幕总线初始化

    OV2640_Hardware_Init();
    LCD_Screen_Init();                                   // LCD屏幕初始化
    LCD_Address_Set(0, 0, LCD_WIDTH - 1, LCD_HIDTH - 1); // 设置显示区
    if (USE_HORIZONTAL == 0 || USE_HORIZONTAL == 1)      // 竖屏
        LCD_Fill(0, 0, LCD_WIDTH, LCD_HIDTH, 0x0000);
    else
        LCD_Fill(0, 0, LCD_HIDTH, LCD_WIDTH, 0x0000);




#ifdef USE_OV2640
    OV2640_Image_Config(); // OV2640图像配置
    SPI_Cmd(LCD_SPI, DISABLE); // 设置前先关闭SPI
    // 设置选择 16 位数据帧格式
    SPI_DataSizeConfig(LCD_SPI, SPI_DataSize_16b);

    SPI_Cmd(LCD_SPI, ENABLE);
    LCD_SPI_CS_ON(0); // 使能SPI的数据传输

    DMA_Cmd(DMA2_Stream1, ENABLE); // DMA2,Stream1
    DCMI_Cmd(ENABLE);              // DCMI启动
    DCMI_CaptureCmd(ENABLE);       // DCMI采集启动
#endif
    return 0;
}

void delay_us(uint32_t _us)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;

    ticks = _us * (SystemCoreClock / 1000000);

    told = SysTick->VAL;

    while (1)
    {
        tnow = SysTick->VAL;

        if (tnow != told)
        {
            if (tnow < told)
                tcnt += told - tnow;
            else
                tcnt += SysTick->LOAD - tnow + told;

            told = tnow;

            if (tcnt >= ticks)
                break;
        }
    }
}

void delay_ms(uint32_t _ms) { delay_us(_ms * 1000); }

void delay_1ms(uint32_t ms) { delay_us(ms * 1000); }

void delay_1us(uint32_t us) { delay_us(us); }
