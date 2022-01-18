/*
 * wifi_extender.h
 *
 *  Created on: Mar 19, 2019
 *      Author: joseph
 */

#ifndef LV_POCKET_ROUTER_SRC_SETTINGS_WIFI_EXTENDER_H_
#define LV_POCKET_ROUTER_SRC_SETTINGS_WIFI_EXTENDER_H_

void wifi_extender_sw_release_action(lv_obj_t * sw, lv_event_cb_t event_cb);
void init_wifi_extender(void);
void wifi_extender_with_bt(void);
void wifi_extender_with_no_bt(void);
void wifi_extender_loading();
void wifi_extender_loading_animation(void);
void wifi_extender_available_or_unavailable_anim(bool wifi_extender_config);
#endif /* LV_POCKET_ROUTER_SRC_SETTINGS_WIFI_EXTENDER_H_ */
