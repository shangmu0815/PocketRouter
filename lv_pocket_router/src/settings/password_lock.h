/*
 * password_lock.h
 *
 *  Created on: Mar 27, 2019
 *      Author: joseph
 */

#ifndef LV_POCKET_ROUTER_SRC_SETTINGS_PASSWORD_LOCK_H_
#define LV_POCKET_ROUTER_SRC_SETTINGS_PASSWORD_LOCK_H_

void enable_password_lock_action(lv_obj_t * sw, lv_event_cb_t event_cb);
void pw_lock_modified_action(lv_obj_t * btn, lv_event_cb_t event_cb);
void pw_lock_num_kb_box_btnm_action(lv_obj_t * btn, lv_event_cb_t event_cb);
void pw_input_done_action();
void password_lock_create(void);
void pw_lock_clear_config();

#endif /* LV_POCKET_ROUTER_SRC_SETTINGS_PASSWORD_LOCK_H_ */
