#include "tim.h"
#include "stm32f4xx.h"


void TIM_Mode_Config(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	// 开启 TIMx_CLK
	RCC_APB1PeriphClockCmd(BASIC_TIM_CLK, ENABLE);
	/* 累计 TIM_Period 个后产生一个更新或者中断 */
	// 当定时器从 0 计数到 999，即为 1000 次，为一个定时周期
	TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
	// 定时器时钟源 TIMxCLK = 2 * PCLK1
	//  PCLK1 = HCLK / 4
	//  => TIMxCLK=HCLK/2=SystemCoreClock/2=84MHz
	//  设定定时器频率为 =TIMxCLK/(TIM_Prescaler+1)=10000Hz
	TIM_TimeBaseStructure.TIM_Prescaler = 8400 - 1;

	//1ms
	// 初始化定时器 TIMx
	TIM_TimeBaseInit(BASIC_TIM, &TIM_TimeBaseStructure);

	// 清除定时器更新中断标志位
	TIM_ClearFlag(BASIC_TIM, TIM_FLAG_Update);

	// 开启定时器更新中断
	TIM_ITConfig(BASIC_TIM, TIM_IT_Update, ENABLE);

	// 使能定时器
	TIM_Cmd(BASIC_TIM, ENABLE);
}

