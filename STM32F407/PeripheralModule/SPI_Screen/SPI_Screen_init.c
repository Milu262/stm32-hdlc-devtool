#include "SPI_Screen_init.h"
#include "SPI_Control.h"
#include "board.h"
#include <stdio.h>
static void LCD_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能GPIO 时钟 */
    RCC_AHB1PeriphClockCmd(LCD_BLK_GPIO_CLK | LCD_DC_GPIO_CLK | LCD_RES_GPIO_CLK, ENABLE);

    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = LCD_BLK_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(LCD_BLK_GPIO_PORT, &GPIO_InitStructure);

    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = LCD_DC_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(LCD_DC_GPIO_PORT, &GPIO_InitStructure);

    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = LCD_RES_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(LCD_RES_GPIO_PORT, &GPIO_InitStructure);

    LCD_RES_ON(0); // LCD屏幕复位
    delay_ms(100);
    LCD_RES_ON(1);
    delay_ms(100);

    LCD_BLK_ON(1); // 打开LCD背光
    delay_ms(100);
}

/**
  * @brief       向LCD屏幕写入一个字节数据
  * @param       dat: 要写入的字节数据
  * @retval      返回写入结果，1表示失败，0表示成功
  */
static uint8_t LCD_Writ_Bus(uint8_t dat)
{
    LCD_SPI_CS_ON(0);
    uint16_t spiTimeoutCounter = SPIT_FLAG_TIMEOUT;
    /* 等待发送缓冲区为空，TXE 事件 */
    while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET)
    {
        if ((spiTimeoutCounter--) == 0)
        {
            printf("SPI Timeout\r\n");
            LCD_SPI_CS_ON(1);
            return 1;
        }
    }
    SPI_I2S_SendData(LCD_SPI, dat);
    while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET)
        ;
    while (SET == SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY))
        ;
    // /* 等待接收缓冲区不空，RNXE 事件 */
    // while (RESET == SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_RXNE))
    //     ;
    // SPI_I2S_ReceiveData(SPI2);

    LCD_SPI_CS_ON(1);
    return 0;
}

/**
  * @brief       向LCD屏幕写入一个字节命令
  * @param       reg: 要写入的命令
  * @retval      返回写入结果，1表示失败，0表示成功
  */
static uint8_t LCD_WR_REG(uint8_t reg)
{
    uint8_t err = 0;
    LCD_DC_ON(0); // DC=0表示写入命令
    err = LCD_Writ_Bus(reg);
    // printf("LCD Write reg: 0x%02x\r\n", reg);
    LCD_DC_ON(1); // DC=1表示写入数据
    return err;
}

/**
  * @brief       向LCD屏幕写入一个字节数据
  * @param       data: 要写入的字节数据
  * @retval      返回写入结果，1表示失败，0表示成功
  */
static uint8_t LCD_WR_DATA8(uint8_t data)
{
    uint8_t err = 0;
    err = LCD_Writ_Bus(data);
    // printf("LCD Write Data: 0x%02x\r\n", data);
    return err;
}

/**
  * @brief       向LCD屏幕写入一个16位数据
  * @param       dat: 要写入的16位数据
  * @retval      无返回值
  */
static void LCD_WR_DATA(uint16_t dat)
{
    SPI_Cmd(LCD_SPI, DISABLE);
    // 设置选择 16 位数据帧格式
    SPI_DataSizeConfig(LCD_SPI, SPI_DataSize_16b);
    SPI_Cmd(LCD_SPI, ENABLE);
    LCD_SPI_CS_ON(0);
    while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET)
        ;
    SPI_I2S_SendData(LCD_SPI, dat);
    while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET)
        ;
    while (SET == SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY))
        ;
    LCD_SPI_CS_ON(1);

    SPI_Cmd(LCD_SPI, DISABLE);
    // 设置选择 8 位数据帧格式
    SPI_DataSizeConfig(LCD_SPI, SPI_DataSize_8b);
    SPI_Cmd(LCD_SPI, ENABLE);
    
    // LCD_Writ_Bus(dat >> 8);
    // LCD_Writ_Bus(dat);
    // printf("LCD Write Data: %04x\r\n", dat);
}

uint8_t LCD_Write_Data_FullDuplex(uint16_t *dat, uint16_t len)
{
    LCD_SPI_CS_ON(0);
    uint16_t spiTimeoutCounter = SPIT_FLAG_TIMEOUT;
    /* 等待发送缓冲区为空，TXE 事件 */
    while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET)
    {
        if ((spiTimeoutCounter--) == 0)
        {
            printf("SPI Timeout\r\n");
            LCD_SPI_CS_ON(1);
            return 0;
        }
    }

    while (len--)
    {
        SPI_I2S_SendData(LCD_SPI, *dat++);
        /* 等待接收缓冲区不空，RNXE 事件 */
        while (RESET == SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_RXNE))
            ;
        SPI_I2S_ReceiveData(SPI2);
    }
    LCD_SPI_CS_ON(1);
    return 1;
}

uint8_t LCD_Write_Data_1Line_Tx(uint16_t *dat, uint16_t len)
{
    LCD_SPI_CS_ON(0);
    while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET)
        ;
    while (len--)
    {
        SPI_I2S_SendData(LCD_SPI, *dat++);
        while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET)
            ;
    }
    while (SET == SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY))
        ;
    LCD_SPI_CS_ON(1);
    return 1;
}

void LCD_Write_Repeat_Data(uint16_t dat, uint32_t len)
{
    SPI_Cmd(LCD_SPI, DISABLE);
    // 设置选择 16 位数据帧格式
    SPI_DataSizeConfig(LCD_SPI, SPI_DataSize_16b);
    SPI_Cmd(LCD_SPI, ENABLE);
    LCD_SPI_CS_ON(0);
    while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET)
        ;
    while (len--)
    {
        SPI_I2S_SendData(LCD_SPI, dat);
        while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET)
            ;
    }
    while (SET == SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY))
        ;
    LCD_SPI_CS_ON(1);
    SPI_Cmd(LCD_SPI, DISABLE);
    // 设置选择 8 位数据帧格式
    SPI_DataSizeConfig(LCD_SPI, SPI_DataSize_8b);
    SPI_Cmd(LCD_SPI, ENABLE);
}


void LCD_Screen_Init(void)
{
    LCD_GPIO_Init();
    LCD_WR_REG(0x11); // 进入休眠模式
    delay_ms(120);
    LCD_WR_REG(0x36);
    // 设置显示方向
    if (USE_HORIZONTAL == 0)
        LCD_WR_DATA8(0x00);
    else if (USE_HORIZONTAL == 1)
        LCD_WR_DATA8(0xC0);
    else if (USE_HORIZONTAL == 2)
        LCD_WR_DATA8(0x70);
    else
        LCD_WR_DATA8(0xA0);

    LCD_WR_REG(0x3A);
    LCD_WR_DATA8(0x05);

    LCD_WR_REG(0xB2);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x33);

    LCD_WR_REG(0xB7);
    LCD_WR_DATA8(0x35);

    LCD_WR_REG(0xBB);
    LCD_WR_DATA8(0x32); // Vcom=1.35V

    LCD_WR_REG(0xC2);
    LCD_WR_DATA8(0x01);

    LCD_WR_REG(0xC3);
    LCD_WR_DATA8(0x15); // GVDD=4.8V 颜色深度

    LCD_WR_REG(0xC4);
    LCD_WR_DATA8(0x20); // VDV, 0x20:0v

    LCD_WR_REG(0xC6);
    LCD_WR_DATA8(0x0F); // 0x0F:60Hz

    LCD_WR_REG(0xD0);
    LCD_WR_DATA8(0xA4);
    LCD_WR_DATA8(0xA1);

    LCD_WR_REG(0xE0);
    LCD_WR_DATA8(0xD0);
    LCD_WR_DATA8(0x08);
    LCD_WR_DATA8(0x0E);
    LCD_WR_DATA8(0x09);
    LCD_WR_DATA8(0x09);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x31);
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x48);
    LCD_WR_DATA8(0x17);
    LCD_WR_DATA8(0x14);
    LCD_WR_DATA8(0x15);
    LCD_WR_DATA8(0x31);
    LCD_WR_DATA8(0x34);

    LCD_WR_REG(0xE1);
    LCD_WR_DATA8(0xD0);
    LCD_WR_DATA8(0x08);
    LCD_WR_DATA8(0x0E);
    LCD_WR_DATA8(0x09);
    LCD_WR_DATA8(0x09);
    LCD_WR_DATA8(0x15);
    LCD_WR_DATA8(0x31);
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x48);
    LCD_WR_DATA8(0x17);
    LCD_WR_DATA8(0x14);
    LCD_WR_DATA8(0x15);
    LCD_WR_DATA8(0x31);
    LCD_WR_DATA8(0x34);
    LCD_WR_REG(0x21);

    LCD_WR_REG(0x29);
    printf("LCD Screen Init Over\r\n");
}

void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    if (USE_HORIZONTAL == 0)
    {
        LCD_WR_REG(0x2a); // 行地址设置
        LCD_WR_DATA(x1);
        LCD_WR_DATA(x2);
        LCD_WR_REG(0x2b); // 列地址设置
        LCD_WR_DATA(y1 + 20);
        LCD_WR_DATA(y2 + 20);
        LCD_WR_REG(0x2c); // 存储器写
    }
    else if (USE_HORIZONTAL == 1)
    {
        LCD_WR_REG(0x2a);
        LCD_WR_DATA(x1);
        LCD_WR_DATA(x2);
        LCD_WR_REG(0x2b);
        LCD_WR_DATA(y1 + 20);
        LCD_WR_DATA(y2 + 20);
        LCD_WR_REG(0x2c);
    }
    else if (USE_HORIZONTAL == 2)
    {
        LCD_WR_REG(0x2a);
        LCD_WR_DATA(x1 + 20);
        LCD_WR_DATA(x2 + 20);
        LCD_WR_REG(0x2b);
        LCD_WR_DATA(y1);
        LCD_WR_DATA(y2);
        LCD_WR_REG(0x2c);
    }
    else
    {
        LCD_WR_REG(0x2a);
        LCD_WR_DATA(x1 + 20);
        LCD_WR_DATA(x2 + 20);
        LCD_WR_REG(0x2b);
        LCD_WR_DATA(y1);
        LCD_WR_DATA(y2);
        LCD_WR_REG(0x2c);
    }
}

void LCD_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color)
{

    // LCD_Write_Repeat_Data(color,((yend-ysta)*(xend-xsta)));
    uint16_t i, j;
    LCD_Address_Set(xsta, ysta, xend - 1, yend - 1); // 设置显示区

    for (i = ysta; i < yend; i++)
    {
        for (j = xsta; j < xend; j++)
        {
            LCD_WR_DATA(color);
        }
    }
}

uint8_t LCD_Pixel_Cycle(void)
{
    uint16_t color = 0;
    uint16_t len =0;
    while (1)
    {
        // LCD_Address_Set(0, 0, 240 - 1, 280 - 1); // 设置显示区
        len += 8;
        color = len;
        for (int i = 0; i < 280; i++)
        {
            // for (int j = 0; j < 240; j++)
            // {
            //     LCD_WR_DATA(color++);
            //     if (color == 0xffff)
            //         continue;
            // }
            LCD_Write_Repeat_Data(color++,240);

        }
        if (len > 0xfff0)
        {
            break;
        }
        
        // delay_ms(1000);
    }
    return 0;
}

void LCD_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_Address_Set(x,y,x,y);//设置光标位置
	LCD_WR_DATA(color);
} 
