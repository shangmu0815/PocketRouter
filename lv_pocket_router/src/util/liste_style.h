
#include "../../../lvgl/lvgl.h"

#ifndef LV_POCKET_ROUTER_SRC_UTIL_LIST_STYLE_H_
#define LV_POCKET_ROUTER_SRC_UTIL_LIST_STYLE_H_


void liste_style_create(void);
lv_obj_t * lv_get_child_by_index(lv_obj_t * list, int index);
lv_obj_t * lv_liste_w_cbox(lv_obj_t * list, const char * txt, bool enable, lv_event_cb_t action, int id);
lv_obj_t * lv_liste_w_cbox_style(lv_obj_t * list, const char * txt, bool enable, lv_event_cb_t action, int id);
lv_obj_t * lv_liste_w_arrow(lv_obj_t * list, const char * txt, const char * val, lv_event_cb_t action);
lv_obj_t * lv_liste_ps_brightness(lv_obj_t * list);
lv_obj_t * lv_liste_sms(lv_obj_t * list, const char * title, const char * preview, int state, int id, int encoding);
void lv_liste_w_arrow_align(lv_obj_t * lable);
void lv_liste_w_arrow_align_liste(lv_obj_t * liste);
void lv_liste_w_arrow_align_scroll(lv_obj_t * liste, int res_txt, char* val);
lv_obj_t * lv_liste_w_checkbox(lv_obj_t * list, const char * txt, bool enable, lv_event_cb_t action, int id);
lv_obj_t * lv_liste_w_scrl_txt(lv_obj_t * list, const char * txt);
void lv_liste_w_arrow_ina(lv_obj_t * liste, bool enable);
char * lv_liste_conn_usr(lv_obj_t * list, const char * title, const char * preview, lv_event_cb_t action, int id);
char * lv_liste_blk_usr(lv_obj_t * list, const char * title, lv_event_cb_t action, int id);
lv_obj_t * lv_liste_w_switch_with_long_break(lv_obj_t * list, const char * txt, lv_event_cb_t action);
lv_obj_t * lv_liste_w_arrow_w_item_id(lv_obj_t * list, const char * txt, const char * val, lv_event_cb_t action, int id);
lv_obj_t * lv_liste_settings(lv_obj_t * list, const void * img_src,const char * txt, lv_event_cb_t action, int id);
lv_obj_t * lv_liste_setting_info(lv_obj_t * list, const void * img_src, const char * txt, lv_event_cb_t action, int id);
lv_obj_t * lv_liste_setting_double_info(lv_obj_t * list, const char * txt1, const char * txt2);
lv_obj_t * lv_liste_cust_switch(lv_obj_t * list, const char * txt, lv_event_cb_t action, bool enable);
void set_cust_switch_state(lv_obj_t * obj, char * key);
lv_obj_t * lv_liste_w_switch(lv_obj_t * list, const char * txt, lv_event_cb_t action);
lv_obj_t * lv_liste_w_arrow_w_item_id_ssid(lv_obj_t * list, const char * txt, const char * val, lv_event_cb_t action, int id);
lv_obj_t * lv_liste_profile_w_cbox(lv_obj_t * list, const char * txt, bool enable, lv_event_cb_t action, int id);
lv_obj_t * lv_liste_profile_w_checkbox(lv_obj_t * list, const char * txt, bool enable, lv_event_cb_t action, int id);
void lv_liste_w_sw_ina(lv_obj_t * liste);
lv_obj_t * lv_liste_w_scrl_txt_ina(lv_obj_t * list, const char * txt, bool inactive);
void composite_title_adjust(lv_obj_t * title, lv_obj_t * value, const char * txt, const char * val, int extra_x);
#define LISTE_SHIFT 14
#endif /* LV_POCKET_ROUTER_SRC_UTIL_LIST_STYLE_H_ */
