/*
 * wifi.h
 *
 *  Created on: Mar 22, 2019
 *      Author: joseph
 */

#ifndef LV_POCKET_ROUTER_SRC_SETTINGS_WIFI_H_
#define LV_POCKET_ROUTER_SRC_SETTINGS_WIFI_H_

void wifi_bandwidth_confirm_action(lv_obj_t * btn, lv_event_t event);
void wifi_bandwidth_btn_action(lv_obj_t * btn, lv_event_t event);
void wifi_bandwidth_create(void);
void wifi_popup_comfirm_action(lv_obj_t * mbox, lv_event_t event);
void popup_wifi_band_comfirm_action(lv_obj_t * btn, lv_event_t event);
void wifi_band_confirm_action(lv_obj_t * btn, lv_event_t event);
void wifi_band_btn_action(lv_obj_t * btn, lv_event_t event);
void wifi_band_close_action(lv_obj_t * btn, lv_event_t event);
void wifi_band_create(void);
void wifi_enable_action(lv_obj_t * sw, lv_event_t event);
void wifi_pmf_action(lv_obj_t * sw, lv_event_t event);
void wifi_bandwidth_action(lv_obj_t * btn, lv_event_t event);
void wifi_band_action(lv_obj_t * btn, lv_event_t event);
void hide_ssid_action(lv_obj_t * sw, lv_event_t event);
void wifi_init(int band);


#endif /* LV_POCKET_ROUTER_SRC_SETTINGS_WIFI_H_ */
