#ifndef LV_POCKET_ROUTER_SRC_SSID_SSID_H_
#define LV_POCKET_ROUTER_SRC_SSID_SSID_H_

#include "../../../lvgl/lvgl.h"

char* get_security_info(int band);
char* get_ssid_pwd(int band);
void update_ssid_label(int band);
void update_pw_label(int band);
void update_security_label(int band);
void ssid_keyboard_action(int id, const char* txt);
void ssid_list_action(lv_obj_t * list_btn, lv_event_t event);
void ssid_popup_comfirm_action(lv_obj_t * mbox, lv_event_t event);
void ssid_security_action(lv_obj_t * btn, lv_event_t event);
void init_ssid(void);
void ssid_security_btn_action(lv_obj_t * btn, lv_event_t event);
void ssid_security_create(int band);
void ssid_len_err_action(lv_obj_t * btn, lv_event_t event);
void set_security_type(int select);
bool check_input_fit_len(const char * input, int min, int max);
bool check_input_fit_ascii(const char * input);
bool check_input_contain_space(const char * input);
bool check_input_fit_in_64_len(const char * input);
bool check_input_is_all_space(const char * input);
void ssid_qrcode_action(int band);
#endif /* LV_POCKET_ROUTER_SRC_SSID_SSID_H_ */
