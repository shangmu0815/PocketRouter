#ifndef LV_POCKET_ROUTER_SRC_KEYBOARD_EN_KB_H_
#define LV_POCKET_ROUTER_SRC_KEYBOARD_EN_KB_H_

#include "../../../lvgl/lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t * en_kb_create_impl(const void * headline, int btn_click_id,
        void (*callback)(int id, const char* txt), lv_event_cb_t r_action, lv_event_cb_t l_action);
lv_obj_t * en_kb_create(const void * headline, int btn_click_id, void (*callback)(int id, const char* txt));
lv_obj_t * en_kb_reuse_create(const void * headline, int btn_click_id, void (*callback)(int id, const char* txt), lv_event_cb_t l_action);
void en_kb_set_lable(const void * txt);
void en_kb_set_tip(const char * tip);
void en_kb_input_allow_empty();
void en_kb_set_tip_impl(bool enable);
void en_kb_set_lable_length(int length);
void en_kb_set_cb_id(int id);
void en_kb_hide_cancel_btn(lv_obj_t * kb);
void ur_kb_adjust(int curr);
void enable_tick_btn(bool enable);
void en_kb_set_pwd_lable(const char* txt);
void update_pwd_lable_task();
void en_kb_close_cb(lv_obj_t * kb, lv_event_t event);
bool get_tip_mode();
bool get_tick_btn_ina_state(lv_obj_t * tick_btn);
lv_obj_t * get_kb_tick_btn(lv_obj_t * root);
void en_kb_set_profile_font_style(bool enable);
void profile_del_btn_action(lv_obj_t * btn, lv_event_t event);
#endif /* LV_POCKET_ROUTER_SRC_KEYBOARD_EN_KB_H_ */
