#ifndef LV_POCKET_ROUTER_SRC_UTIL_HIGH_SPEED_H_
#define LV_POCKET_ROUTER_SRC_UTIL_HIGH_SPEED_H_

#include "lv_pocket_router/src/util/util.h"

void init_high_speed();
bool get_high_speed_notify_flag();
void high_speed_reminder();
void high_speed_config();
void high_speed_prompt();
void high_speed_battery_reconfig(bool batt_present);
void high_speed_temp_monitor();
bool high_speed_shutdown();
void mmWave_disable_prompt();
bool show_high_speed_shutdown();

#endif /* LV_POCKET_ROUTER_SRC_UTIL_HIGH_SPEED_H_ */
