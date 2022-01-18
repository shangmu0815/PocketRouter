/*
 * pin_management.h
 *
 *  Created on: Apr 1, 2019
 *      Author: joseph
 */

#ifndef LV_POCKET_ROUTER_SRC_SETTINGS_PIN_MANAGEMENT_H_
#define LV_POCKET_ROUTER_SRC_SETTINGS_PIN_MANAGEMENT_H_
//#define FEATURE_ROUTER
enum PIN_MANAGEMENT_CHK_TYPES {
    SIM_PIN,
    SIM_PUK
};
enum PIN_MANAGEMENT_BTN_IDS {
    ID_ENABLE_PIN,
    ID_ENABLE_PUK,
    ID_ENTER_NEW_PIN,
    ID_CONFIRM_PIN
};
char* get_retry_left_info_page_string(int type, int retry_left);
void retry_left_page_action(lv_obj_t * btnm, lv_event_t event);
void pin_puk_retry_left_info_page(int type, int retry_left);
void retry_left_info_page_str_free_alloc();
void init_pin_management(void);
void pin_management_with_sim_not_ready(void);
void pin_management_with_sim_ready(void);
void set_switch_state();
void enable_pin_switch_error_action(lv_obj_t * mbox/*btn*/, lv_event_t event);
void enable_pin_switch_error_notice();
void enable_pin_switch(lv_obj_t * sw, lv_event_t event);
void pin_puk_verify_result_cb(int sim_pin_req, int error, int retry_left);
void verify_fail_notice(int type, int retry_left);
void verify_fail_error_action(lv_obj_t * mbox/*btn*/, lv_event_t event);
void incorrect_pin_threes_times_action(lv_obj_t * mbox, lv_event_t event);
void retry_puk_error_10_times_action(lv_obj_t * mbox, lv_event_t event);
char* get_pin_puk_verify_fail_string(int type, int retry_left);
void verify_fail_str_free_alloc();
void pin_puk_kb_close_action(lv_obj_t * btn, lv_event_t event);
void sim_pin_verify(const char *pin1);
void sim_puk_verify(char *puk1, char *new_pin1);
void new_pin_not_match_action(lv_obj_t * mbox, lv_event_t event);
void new_pin_not_match_notice();
void pin_puk_kb_cb(int id, const char* pin_puk_info);
void pin_puk_kb_create(int kb_type, bool with_close_btn);
void enter_puk_info_page_action(lv_obj_t * btnm, lv_event_t event);
void len_err_action(lv_obj_t * mbox, lv_event_t event);
void len_err_notice(int type);
#if !defined (FEATURE_ROUTER)
void reset_retry_state();
#endif
void set_pin_puk_kb_state(int kb_type);
void ril_error_action(lv_obj_t * mbox, lv_event_t event);
void ril_error_notice();
void close_delay_popup();
#endif /* LV_POCKET_ROUTER_SRC_SETTINGS_PIN_MANAGEMENT_H_ */
