#ifndef LV_POCKET_ROUTER_SRC_SETTINGS_NETWORK_SETTINGS_H_
#define LV_POCKET_ROUTER_SRC_SETTINGS_NETWORK_SETTINGS_H_

void search_for_network();
void search_for_network_task_animation(void);
void regist_the_network();
void regist_the_network_task_animation(void);
void init_network_settings(void);
void network_settings_create_with_no_sim(void);
void choose_network_ok_action(lv_obj_t * btn, lv_event_t event);
void choose_network_close_action(lv_obj_t * btn, lv_event_t event);
void search_for_network_create(void);
void search_for_network_action(lv_obj_t * btn, lv_event_t event);
void search_mode_create(void);
void support_4g_action(lv_obj_t * sw, lv_event_t event);
void network_settings_create(void);
void update_pref_network_lable();

char* match_nw_state(int state);
const char* match_nw_rat(int rat);
void airplane_mode_action(lv_obj_t * sw, lv_event_t event);

void airplane_mode_result_popup_close_action(lv_obj_t * mbox, lv_event_t event);
void set_airplane_mode_result_popup(bool res);
#endif /* LV_POCKET_ROUTER_SRC_SETTINGS_NETWORK_SETTINGS_H_ */
