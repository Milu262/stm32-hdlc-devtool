#include "lvgl_gui_main.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"

#include "gui_guider.h"
#include "events_init.h"

#include "stm32f4xx.h"

	// 全局变量：保存键盘输入设备句柄、组句柄（方便后续修改组配置）
lv_indev_t *g_keypad_indev = NULL;
lv_group_t *g_keypad_group = NULL;
extern lv_indev_t *indev_keypad;
lv_ui guider_ui;
void lvgl_gui_main(void)
{
	lv_init();//lvgl初始化
	lv_port_disp_init();//注册lvgl的显示任务
	lv_port_indev_init();//注册lvgl的输入任务

	setup_ui(&guider_ui);
	events_init(&guider_ui);

    g_keypad_group = lv_group_create();
    lv_group_add_obj(g_keypad_group, guider_ui.screen_btn_1);
	lv_group_add_obj(g_keypad_group, guider_ui.screen_btn_2);
    lv_indev_set_group(indev_keypad, g_keypad_group);
    lv_group_set_wrap(g_keypad_group, true); // 开启焦点循环（焦点到最后一个控件后，再按方向键回到第一个）
	//按钮
	// lv_obj_t *btn = lv_btn_create(lv_scr_act());
	// lv_obj_set_pos(btn, 10, 10);//设置按钮的位置
	// lv_obj_set_size(btn, 100, 50);//设置按钮的大小

	// lv_obj_t *label_btn = lv_label_create(btn);//创建一个标签,父对象为按钮
	// lv_obj_align(label_btn, LV_ALIGN_CENTER, 0, 0);//对齐于父对象
	// lv_label_set_text(label_btn, "Button");//设置标签的文本

	// lv_obj_t *label = lv_label_create(lv_scr_act());//创建文本标签，父对象为当前屏幕
	// lv_label_set_text(label, "Hello World!");
	// lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
	// lv_obj_align_to(btn,label , LV_ALIGN_OUT_TOP_MID, 0, -20);


}

