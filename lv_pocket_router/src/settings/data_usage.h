/*
 * data_usage.h
 *
 *  Created on: Apr 1, 2019
 *      Author: joseph
 */

#ifndef LV_POCKET_ROUTER_SRC_SETTINGS_DATA_USAGE_H_
#define LV_POCKET_ROUTER_SRC_SETTINGS_DATA_USAGE_H_

char* get_data_usage_content();
void init_data_usage(void);
char* getDataUsageInfo();
char* getLastTimeResetDateTime();
char* getMaxDataUsage();
const char* getUnit();
char* getStartDate();
char* getRemindThreshold();
void data_usage_create(void);
void unit_create(void);
void remind_threshold_create(void);
void data_usage_monitor_action(lv_obj_t * sw, lv_event_t event);
void refresh_data_usage_settings(bool value);
char* get_max_data_usage_content();
#define USAGE_HOR_SPACE 14

#endif /* LV_POCKET_ROUTER_SRC_SETTINGS_DATA_USAGE_H_ */
