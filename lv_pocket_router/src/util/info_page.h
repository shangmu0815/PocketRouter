#ifndef LV_POCKET_ROUTER_SRC_UTIL_INFO_PAGE_H_
#define LV_POCKET_ROUTER_SRC_UTIL_INFO_PAGE_H_

#include "../../../lvgl/lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t *  info_page_create (lv_obj_t * par, const void * headline, const void * text);
lv_obj_t *  info_page_create_btmn (lv_obj_t * par, const void * headline,
    const void * text, const char ** btnm, lv_event_cb_t action);
lv_obj_t * info_page_sms_create(lv_obj_t * par, const void * headline, const void * text, lv_event_cb_t action, int encoding);
lv_res_t close_all_pages(void);
lv_res_t set_sms_header_icon(lv_obj_t * btn);
lv_obj_t * info_page_create_label_align_center (lv_obj_t * par, const void * headline, const void * text);
lv_obj_t * info_page_create_confirm_profile_info(lv_obj_t * par, const void * headline, const void * text,lv_event_cb_t action);
lv_obj_t * info_page_create_btmn_impl_profile_with_edit(lv_obj_t * par, const void * headline,
        const void * text, const char ** btnm, lv_event_cb_t action,
        lv_event_cb_t action2,lv_event_cb_t r2_btn_action);
lv_obj_t * info_page_create_btmn_label_refresh(lv_obj_t * par, const void * headline,
        const void * text1, const void * text2, const char ** btnm, lv_event_cb_t back_action, lv_event_cb_t action);
void info_page_hide_btn(lv_obj_t * win);
void info_page_close_win(lv_obj_t * win);
static void header_action(lv_obj_t * btn, lv_event_t event_cb);
void info_page_set_label(lv_obj_t * label, const void * text);

#endif /* LV_POCKET_ROUTER_SRC_UTIL_INFO_PAGE_H_ */
