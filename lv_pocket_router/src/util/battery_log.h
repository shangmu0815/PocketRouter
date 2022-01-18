#ifndef LV_POCKET_ROUTER_SRC_BATTERY_LOG_H_
#define LV_POCKET_ROUTER_SRC_BATTERY_LOG_H_

#include "../../../lvgl/lvgl.h"
#include "lv_pocket_router/src/util/debug_log.h"

enum {
    PLUG_IN,
    PLUG_OUT
};

void init_battery_log();
void battery_log_level(int level);
void battery_log_plugin(bool charging);
void battery_log_charge_info();

#endif /* LV_POCKET_ROUTER_SRC_BATTERY_LOG_H_ */
