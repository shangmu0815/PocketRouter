/*
 * connected_users.h
 *
 *  Created on: Apr 15, 2019
 *      Author: joseph
 */

#ifndef LV_POCKET_ROUTER_SRC_SETTINGS_CONNECTED_USERS_H_
#define LV_POCKET_ROUTER_SRC_SETTINGS_CONNECTED_USERS_H_

void connected_users_with_bt(void);
void init_connected_users(void);
void connected_users_with_no_bt();
void blacklist();
void add_to_blacklist_popup_warning_action(lv_obj_t * mbox, lv_event_t event);
void any_existed_blacklist_action(lv_obj_t * btn, lv_event_t event);
void add_to_blacklist_popup_warning(char * text);

#endif /* LV_POCKET_ROUTER_SRC_SETTINGS_CONNECTED_USERS_H_ */
