#ifndef LV_POCKET_ROUTER_SRC_UTIL_POWER_MENU_H_
#define LV_POCKET_ROUTER_SRC_UTIL_POWER_MENU_H_
#include "../../../lvgl/lvgl.h"

void power_menu_popup_create(void);
void close_power_menu();
void emergency_shutdown_popup(int res_id);
bool emergency_shutdown_popup_exist();
void sim_change_impl(int reason);
void save_reboot_state(int state, int prev_state, bool screen_on);
void set_reboot_popup_enable(int strid, int timeout);
void reboot_popup_create();
int get_reboot_popup_enable();
void charging_only_power_off();
void charging_only_restart();
void power_off();

static const int EMERGENCY_TEMP_HIGH = 60;
static const int EMERGENCY_QC_TEMP_HIGH = 70;
static const int EMERGENCY_TEMP_LOW = -18;

enum INDEX_POWER_MENU{
    INDEX_POWER_OFF,
    INDEX_RESTART,
    INDEX_CANCEL
};

enum reboot_reason_list{
    SCREEN_ON_SIM_INSERT,
    SCREEN_ON_SIM_REMOVE,
    SCREEN_OFF_SIM_INSERT,
    SCREEN_OFF_SIM_REMOVE,
    CONFIG_CHANGE_REBOOT,
    POWER_MENU_REBOOT,
    POWER_MENU_POWER_OFF,
    BATTERY_LOW_POWER_OFF,
    BATTERY_HIGH_TEMP_POWER_OFF,
    BATTERY_LOW_TEMP_POWER_OFF,
    CHARGING_ONLY_POWER_OFF,
    CHARGING_ONLY_REBOOT,
};

#if BATTERY_EXTRA_TIME_OUT
//for battery low emergency shutdown extra timeout
enum ES_TIMEOUT_STATUS {
    ES_TIMEOUT_NONE,
    ES_TIMEOUT_START,
    ES_TIMEOUT_OVER
};
#endif
#endif /* LV_POCKET_ROUTER_SRC_UTIL_POWER_MENU_H_ */
