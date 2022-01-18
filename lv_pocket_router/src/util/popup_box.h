#ifndef LV_POCKET_ROUTER_SRC_UTIL_POPUP_BOX_H_
#define LV_POCKET_ROUTER_SRC_UTIL_POPUP_BOX_H_

#include "../../../lvgl/lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t * popup_img_create(const void * src_img, const void * text, const char ** btns, lv_event_cb_t action);
lv_obj_t * popup_img_create_impl(const void * src_img, const void * text, const char ** btns, lv_event_cb_t action, lv_event_cb_t action2);
lv_obj_t * popup_scrl_create(const void * title, const void * text, const char ** btns, lv_event_cb_t action);
lv_obj_t * popup_scrl_create_impl(const void * title, const void * text, const char ** btns, lv_event_cb_t action, lv_event_cb_t action2);
lv_obj_t * popup_anim_int_create(const void * text, const char ** btns, lv_event_cb_t action);
lv_obj_t * popup_anim_int_create_impl(const void * text, const char ** btns, lv_event_cb_t action, lv_event_cb_t action2);
lv_obj_t * popup_anim_loading_create(const void * text, const void * message);
lv_obj_t * popup_anim_not_create(const void * text, const char ** btns, lv_event_cb_t action, lv_event_cb_t action2);
lv_obj_t * popup_anim_not_long_create(const void * text, const char ** btns, lv_event_cb_t action, lv_event_cb_t action2);
lv_obj_t * popup_anim_que_create(const void * text, const char ** btns, lv_event_cb_t action, lv_event_cb_t action2);
lv_obj_t * popup_anim_que_long_create(const void * text, const char ** btns, lv_event_cb_t action, lv_event_cb_t action2);
lv_obj_t * popup_anim_not_plain_create(const void * text, int32_t time);
lv_task_t * popup_loading_task_create(lv_task_cb_t task_cb, uint32_t period, lv_task_prio_t prio, void * user_data);
void hide_loading_bar(lv_obj_t * box);
void update_loading_bar(lv_obj_t * box, int16_t value);
void update_anim_loading_msg_font(lv_obj_t * box, uint8_t font_weight, uint8_t font_height);
void set_static_popup(bool is_static);
void close_static_popup();
bool is_static_popup();
void move_static_popup_to_foreground();
void set_popup_cb(void (*callback)());
void close_popup();
void create_static_popup(void (*callback)());

#endif /* LV_POCKET_ROUTER_SRC_UTIL_POPUP_BOX_H_ */

