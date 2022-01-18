/*
 * profile_management.h
 *
 *  Created on: Apr 11, 2019
 *      Author: joseph
 */

#ifndef LV_POCKET_ROUTER_SRC_SETTINGS_PROFILE_MANAGEMENT_H_
#define LV_POCKET_ROUTER_SRC_SETTINGS_PROFILE_MANAGEMENT_H_
enum PROFILE_IDS {
    ID_PROFILE_NAME,
    ID_APN,
    ID_USER_NAME,
    ID_PROFILE_PASSWORD,
    ID_PDP_TYPE
};
void profile_name_close_action(lv_obj_t * btn, lv_event_t event);
void profile_name_keyboard_action(int id, const char* profile_info);
void profile_management_loading(void);
void profile_management_loading_task_animation(int *btn_type);
void profile_loading_action_for_create(lv_obj_t * btn, lv_event_t event);
void profile_loading_action_for_edit(lv_obj_t * btn, lv_event_t event);
void select_pdp_type_close_action(lv_obj_t * btn, lv_event_t event);
void select_pdp_type_confirm_action(lv_obj_t * btn, lv_event_t event);
void pdp_type_btn_action(lv_obj_t * btn, lv_event_t event);
void select_pdp_type_page();
void confirm_profile_info(int id);
void profile_management_page_action(lv_obj_t * btnm, lv_event_t event);
void init_profile_management_loading(void);
void init_profile_management_loading_task_animation(void);
void init_profile_management(void);
void query_profile_management_page(void);
char* getProfileName();
char* getApnName();
char* getUserName();
char* getPassword();
char* getPdpType();
void profile_name_data_confirm_action(lv_obj_t * btn, lv_event_t event);
void profile_btn_action(lv_obj_t * btn, lv_event_t event);
void select_profile_name_page(void);
void delete_selected_items_action(lv_obj_t * btnm, lv_event_t event);
void delete_profile_name_data_action(lv_obj_t * btn, lv_event_t event);
void delete_profile_btn_action(lv_obj_t * btn, lv_event_t event);
void delete_profile_name_page(void);
void profile_edit_action(lv_obj_t * btn, lv_event_t event);
char* get_all_profile_data();
void reset_profile_state_config();
void reset_profile_temp_save_config();
char* get_pdp_type(int type);
void init_default_apn_profile();
void init_apn_info();
void close_wwan_check_task();
void check_wwan_status();
int check_wwan_Thread();
void check_wwan_status_task();
void init_apn_monitor_task();
char* pdp_type_str(char* pdp_type);
void sync_data_storage_apn();
void get_mcc_mnc_info();
void set_mcc_mnc_default_profile_name(char* mcc_mnc, char* profile_name);
char* get_mcc_mnc_default_profile_name(char* mcc_mnc);
int modify_modem_apn(char *apn, char *username, char *password, char *pdp_type);
void profiles_reached_limit_close_action(lv_obj_t * mbox, lv_event_t event);
void style_profile_text_create(void);
int check_contain_escapechar_num(const char * input);
void warning_popup_close(lv_obj_t * mbox, lv_event_t event);
void warning_popup(int id);
void set_orig_mcc(char* mcc);
char* get_orig_mcc();
void set_orig_mnc(char* mnc);
char* get_orig_mnc();
void set_mccmnc_default_apn_to_modem(char* profile_name, char* mcc, char* mnc);
void reestablish_data(void *arg);
bool set_reestablish_data_state(bool state);
bool get_reestablish_data_state();
void reestablish_data_task();

#endif /* LV_POCKET_ROUTER_SRC_SETTINGS_PROFILE_MANAGEMENT_H_ */
