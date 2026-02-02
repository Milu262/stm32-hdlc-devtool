#ifndef __SPI_SCREEN_INIT_H__
#define __SPI_SCREEN_INIT_H__

#include "SPI_init.h"
// 屏幕的SPI接口定义在SPI_init.h中定义

// 屏幕使用SPI1
/**屏幕使用的管脚定义
 * - SCK:  PA5
 * - MOSI: PA7
 * - CS(NSS): PE9
 * - DC:  PC4
 * - RES: PA1
 * - BLK: PA2
 */

// #define LCD_SPI SPI1
// #define LCD_SPI_CS_GPIO_PORT GPIOE
// #define LCD_SPI_CS_PIN GPIO_Pin_9

#define LCD_SPI SPIScreen
#define LCD_SPI_CS_GPIO_PORT CS_PIN_GPIO_PORT
#define LCD_SPI_CS_PIN CS_PIN

#define LCD_BLK_GPIO_PORT GPIOA
#define LCD_BLK_PIN GPIO_Pin_2
#define LCD_BLK_GPIO_CLK RCC_AHB1Periph_GPIOA

#define LCD_DC_GPIO_PORT GPIOC
#define LCD_DC_PIN GPIO_Pin_4
#define LCD_DC_GPIO_CLK RCC_AHB1Periph_GPIOC

#define LCD_RES_GPIO_PORT GPIOA
#define LCD_RES_PIN GPIO_Pin_1
#define LCD_RES_GPIO_CLK RCC_AHB1Periph_GPIOA

//竖屏--------------
#define LCD_WIDTH 240
#define LCD_HIDTH 280

//横屏--------------
// #define LCD_WIDTH 280
// #define LCD_HIDTH 240

#define COLOR_WHITE 0xFFFF

#define USE_HORIZONTAL 1 // 设置屏幕方向，0或1为竖屏，2或3为横屏

#define LCD_SPI_CS_ON(x) GPIO_WriteBit(LCD_SPI_CS_GPIO_PORT, LCD_SPI_CS_PIN, x ? Bit_SET : Bit_RESET)

// 屏幕的背光控制
#define LCD_BLK_ON(x) GPIO_WriteBit(LCD_BLK_GPIO_PORT, LCD_BLK_PIN, x ? Bit_SET : Bit_RESET)
// 屏幕的指令控制
#define LCD_DC_ON(x) GPIO_WriteBit(LCD_DC_GPIO_PORT, LCD_DC_PIN, x ? Bit_SET : Bit_RESET)

// 屏幕的复位控制
#define LCD_RES_ON(x) GPIO_WriteBit(LCD_RES_GPIO_PORT, LCD_RES_PIN, x ? Bit_SET : Bit_RESET)


void LCD_Screen_Init(void);

/**
 * @brief 设置显示区域的起始和结束地址
 *
 * @param x1 列的起始地址
 * @param y1 行的起始地址
 * @param x2 列的结束地址
 * @param y2 行的结束地址
 * @return void 无返回值
 */
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/**
 * @brief 填充指定区域的颜色
 *
 * @param xsta 列的起始地址
 * @param ysta 行的起始地址
 * @param xend 列的结束地址
 * @param yend 行的结束地址
 * @param color 填充颜色值
 * @return void 无返回值
 */
void LCD_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color);

uint8_t LCD_Pixel_Cycle(void);
uint8_t LCD_Write_Data_FullDuplex(uint16_t *dat, uint16_t len);
uint8_t LCD_Write_Data_1Line_Tx(uint16_t *dat, uint16_t len);
void LCD_Write_Repeat_Data(uint16_t dat, uint32_t len);

/**
 * @brief 画一个点
 *
 * @param x 列地址
 * @param y 行地址
 * @param color 颜色值
 * @return void 无返回值
 */
void LCD_DrawPoint(u16 x,u16 y,u16 color);
#endif