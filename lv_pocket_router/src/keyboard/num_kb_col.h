#ifndef LV_POCKET_ROUTER_SRC_KEYBOARD_NUM_KB_COL_H_
#define LV_POCKET_ROUTER_SRC_KEYBOARD_NUM_KB_COL_H_

#include <stdbool.h>
#include "../../../lvgl/lvgl.h"
#include "../../../lv_ex_conf.h"
#include "../../res/values/styles.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t * num_col_create(const void * headline, bool default_ic, lv_event_cb_t action);
void num_col_set_lable(const void * lable);
void num_col_close_win();
void set_date_keyboard_style(bool state);
void num_kb_col_set_lable_len(int length);
void num_kb_set_tip(const char * tip);
void num_kb_enable_tick_btn(bool enable);
void pin_mng_r_action(lv_obj_t * btn, lv_event_t event);
void num_kb_col_pin_mng(const void * headline, int btn_click_id, void (*callback)(int id, const char* txt), lv_event_cb_t action);
void num_kb_col_set_select_id(int btn_click_id);
bool check_input_len_fit_4_to_8(const char * input);
bool check_input_len_fit_8(const char * input);
bool check_valid_date(const char * input);
const char* num_col_get_lable(void);
void set_kb_close_btn_hidden();
bool get_kb_close_btn_hidden();

#endif /* LV_POCKET_ROUTER_SRC_KEYBOARD_NUM_KB_COL_H_ */
