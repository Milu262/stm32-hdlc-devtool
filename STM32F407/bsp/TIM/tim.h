#ifndef __TIM_H__
#define __TIM_H__ 


#define BASIC_TIM TIM6
#define BASIC_TIM_CLK RCC_APB1Periph_TIM6
#define BASIC_TIM_IRQHandler TIM6_DAC_IRQHandler

/**
 * @brief 定时器初始化
 */
void TIM_Mode_Config(void);
#endif