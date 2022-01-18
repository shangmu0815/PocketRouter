#ifndef LV_POCKET_ROUTER_SRC_KEYBOARD_NUM_KB_BOX_H_
#define LV_POCKET_ROUTER_SRC_KEYBOARD_NUM_KB_BOX_H_

#include "../../../lvgl/lvgl.h"
#include "../../../lv_ex_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t * num_kb_box_create(void (*fp)(char *), const void * headline, bool hashomekey, lv_event_cb_t action);
void num_pwd_set_lable(const void * pwd);
void num_pwd_close_win();
void unlock_pin_draw_bg(bool draw);
void num_kb_refresh_title(const char * title);
void num_kb_box_cleanup();
void num_kb_reset_val();
void num_kb_header_style_default(lv_event_cb_t action, lv_obj_t * r_btn,
        lv_obj_t * l_btn, lv_obj_t * r_img, lv_obj_t * l_img);
lv_obj_t * num_kb_time_box_create(int time_box_style, const void * headline,
        lv_event_cb_t action);
void num_kb_time_box_year_default_val();
void num_kb_time_box_action(lv_obj_t * btnm, lv_event_t event);
void time_box_del_listener(lv_obj_t * imgbtn, lv_event_t event);
void set_num_kb_time_box_style(int style);
void num_kb_time_box_close();
const char* get_num_kb_box(int index);
bool check_date_valid(int year, int month, int date);
void num_kb_box_enable_tick_btn(bool enable);
void num_kb_box_close();

enum BOX_KB_COL{
    INDEX_COLIMN_1 = 1,
    INDEX_COLIMN_2,
    INDEX_COLIMN_3,
    INDEX_COLIMN_4,
};
enum TIME_BOX_KB_STYLE {
    KB_UNKNOWN = -1,
    KB_YEAR,
    KB_MONTH_DATE,
    KB_HOUR_MINUTE,
};
#endif /* LV_POCKET_ROUTER_SRC_KEYBOARD_NUM_KB_BOX_H_ */
