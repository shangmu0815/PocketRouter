/*
 * bluetooth_settings.h
 *
 *  Created on: Apr 9, 2019
 *      Author: joseph
 */

#ifndef LV_POCKET_ROUTER_SRC_SETTINGS_BLUETOOTH_SETTINGS_H_
#define LV_POCKET_ROUTER_SRC_SETTINGS_BLUETOOTH_SETTINGS_H_

void bt_list_action(lv_obj_t * list_btn, lv_event_cb_t event_cb);
void bluetooth_settings_sw_release_action(lv_obj_t * sw, lv_event_cb_t event_cb);
void init_bluetooth_settings(void);
void init_router_bluetooth_info(void);
char* getBluetoothName();
char* getBluetoothMacAddress();

#endif /* LV_POCKET_ROUTER_SRC_SETTINGS_BLUETOOTH_SETTINGS_H_ */
