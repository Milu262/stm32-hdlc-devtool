#include "key.h"
#include "stm32f4xx.h"
#include "lvgl.h"


void key_init(void)
{
    /* 开启时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
 * @brief 读取按键输入电平
 *
 * @return true 按键按下
 * @return false 按键未按下
 */
static bool key_read_pin(void)
{
    // 高电平=按下（根据你的硬件电路修改）
    return (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == SET);
}


// 枚举类型用于表示按键状态
typedef enum {
    KEY_STATE_IDLE,           // 空闲状态
    KEY_STATE_DEBOUNCING,     // 消抖阶段
    KEY_STATE_PRESSED,        // 已按下（消抖完成）
    KEY_STATE_LONG_PRESS_TRIG,// 长按已触发
    KEY_STATE_SHORT_PRESS_TRIG// 短按已触发
} key_state_t;

uint32_t key_polling_handle(void)
{
    // 静态变量：保存跨轮询周期的状态
    static key_state_t key_state = KEY_STATE_IDLE;       // 当前按键状态
    static uint16_t key_debounce_cnt = 0;               // 消抖计数
    static uint16_t key_long_press_cnt = 0;             // 长按累计计数

    uint32_t ret_key = KEY_RETURN_NONE;                 // 默认返回值
    bool key_cur_state = key_read_pin();                // 读取当前按键电平

    // 异常处理：检测无效输入
    if (key_cur_state != true && key_cur_state != false) {
        return KEY_RETURN_NONE; // 输入无效，直接返回
    }

    switch (key_state) {
        case KEY_STATE_IDLE:
            if (key_cur_state) {
                key_state = KEY_STATE_DEBOUNCING;
                key_debounce_cnt = 1;
            }
            break;

        case KEY_STATE_DEBOUNCING:
            if (key_cur_state) {
                key_debounce_cnt++;
                if (key_debounce_cnt >= KEY_DEBOUNCE_THRESHOLD) {
                    key_state = KEY_STATE_PRESSED;
                    key_long_press_cnt = 0;
                }
            } else {
                key_state = KEY_STATE_IDLE; // 提前释放，重置状态
                key_debounce_cnt = 0;
            }
            break;

        case KEY_STATE_PRESSED:
            if (key_cur_state) {
                key_long_press_cnt++;
                if (key_long_press_cnt >= KEY_LONG_PRESS_THRESHOLD) {
                    key_state = KEY_STATE_LONG_PRESS_TRIG;
                    ret_key = LV_KEY_ENTER;
                }
            } else {
                key_state = KEY_STATE_SHORT_PRESS_TRIG;
                ret_key = LV_KEY_NEXT;
            }
            break;

        case KEY_STATE_LONG_PRESS_TRIG:
            if (!key_cur_state) {
                key_state = KEY_STATE_IDLE;
                key_debounce_cnt = 0;
                key_long_press_cnt = 0;
            }
            break;

        case KEY_STATE_SHORT_PRESS_TRIG:
            if (!key_cur_state) {
                key_state = KEY_STATE_IDLE;
                key_debounce_cnt = 0;
                key_long_press_cnt = 0;
            }
            break;

        default:
            key_state = KEY_STATE_IDLE; // 容错处理
            break;
    }

    return ret_key;
}