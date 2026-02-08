#ifndef __KEY_H__
#define __KEY_H__

#define KEY_DEBOUNCE_THRESHOLD    4     // 消抖阈值：按下持续4次（轮询周期×4）判定为真实按下
#define KEY_LONG_PRESS_THRESHOLD  10   // 长按阈值：按下持续100次（10ms轮询=1秒长按）
#define KEY_RETURN_NONE           0     // 无触发时返回值


#include "stdint.h"
/**
 * @brief 初始化按键PA0
 */
void key_init(void);

/**
 * @brief 按键轮询处理
 * 
 * @return 短按返回聚焦下一个，长按模拟点击，无触发返回0
 */
uint32_t key_polling_handle(void);

#endif