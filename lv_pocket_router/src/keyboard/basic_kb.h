#ifndef LV_POCKET_ROUTER_SRC_KEYBOARD_BASIC_KB_H_
#define LV_POCKET_ROUTER_SRC_KEYBOARD_BASIC_KB_H_

#include "../../../lvgl/lvgl.h"
#include "../../../lv_ex_conf.h"

lv_obj_t * basic_kb_create(const void * headline);
void basic_num_kb(lv_obj_t * par, lv_event_cb_t kb_action, lv_event_cb_t del_action);
void close_kb();
void kb_home_action();
void kb_update_headline(const char * headline);
void kb_tip_adjust(lv_obj_t * ta, const char * tip, int obj_x);

enum INDEX_BASIC_KB{
    KB_TITLE = 2,
    KB_LEFT_BTN,
    KB_RIGHT_BTN,
    KB_DEL_BTN,
    KB_NUM_BTNM,
    KB_HOME_BTN,
};

#endif /* LV_POCKET_ROUTER_SRC_KEYBOARD_BASIC_KB_H_ */
