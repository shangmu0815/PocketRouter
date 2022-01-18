#ifndef LV_POCKET_ROUTER_SRC_SMS_SMS_H_
#define LV_POCKET_ROUTER_SRC_SMS_SMS_H_

void init_sms(bool show_anim);
char * get_sms_unread_num ();
void update_sms_list_title();
void refresh_sms_list();
void sms_long_press_action(lv_obj_t * btn, lv_event_t event);
void sms_short_press_action(lv_obj_t * btn, lv_event_t event);
void sms_list_home_action(lv_obj_t * btn, lv_event_t event);
void sms_list_back_action(lv_obj_t * btn, lv_event_t event);
void sms_open_action(lv_obj_t * btn, lv_event_t event);
void sms_del_action(lv_obj_t * list_btn, lv_event_t event);
void sms_pop_del_comfirm_action(lv_obj_t * mbox, lv_event_t event);
void sms_pop_del_action(lv_obj_t * mbox, lv_event_t event);
void enable_sms_reload();
void enable_reload_static_popup();

enum SMS_MSG_STATE_IDS {
    CHECK,
    UNCHECK,
};

#endif /* LV_POCKET_ROUTER_SRC_SMS_SMS_H_ */
