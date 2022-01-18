#include "lv_pocket_router/src/status_bar.h"
#include "lv_pocket_router/src/util/high_speed.h"
#include "lv_pocket_router/src/util/power_menu.h"

#define THERMAL_PORTABLE_MODE               1
#define THERMAL_FIX_MODE                    2


#define THERMAL_ERROR_VALUE                 -1
#define MAX_THERMAL_VALUE                   10
#define MMWAVE_DISABLED_VALUE               "-273000"

// Portable Mode threshold
#define PORTABLE_MODE_BAT_TEMP_HIGH         55
#define PORTABLE_MODE_BAT_TEMP_LOW          40

// Fixed Mode threshold
#define FIXED_MODE_CPU_TEMP_HIGH            110
#define FIXED_MODE_CPU_TEMP_HIGH_WO_BAT     105
#define FIXED_MODE_CPU_TEMP_LOW_WO_BAT      90
#define FIXED_MODE_MMWAVE_TEMP_HIGH         90
#define FIXED_MODE_MMWAVE_TEMP_LOW          70
#define FIXED_MODE_MMWAVE_TEMP_SHUTDOWN     95

#define MMWAVE_TEMP_NODE_LEN                4
static char * mmWave_temp_path[] = {
    "/sys/devices/virtual/thermal/thermal_zone2/temp",
    "/sys/devices/virtual/thermal/thermal_zone3/temp",
    "/sys/devices/virtual/thermal/thermal_zone4/temp",
    "/sys/devices/virtual/thermal/thermal_zone5/temp"
};

// states for high temp monitor
typedef enum {
    MMWAVE_ENABLED_STATE,
    MMWAVE_DISABLING_STATE,
    MMWAVE_DISABLED_STATE,
    SHUTDOWN_STATE
} hs_mmwave_state_t;

typedef enum {
    NO_PROMPT = 0,
    FIXED_MODE_PROMPT = ID_HIGH_SPEED_DEVICE_TEMP_PROMPT,
    PORTABLE_MODE_PROMPT = ID_HIGH_SPEED_BATTERY_TEMP_PROMPT
} hs_temp_monitor_prompt_t;

static hs_temp_monitor_prompt_t hs_temp_monitor_prompt_id = NO_PROMPT;
static hs_mmwave_state_t mmWave_state = MMWAVE_ENABLED_STATE;

static bool high_speed_remind_flag = false; // for main.c to check if need to show high speed prompt
static bool high_speed_shutdown_flag = false; // for main.c to check if need to show shutdown


void mmwave_enable(bool enable) {
    static bool state_b = true;
    if (state_b == enable) return;
    if (enable) {
        log_d("HighSpeed: mmWave enable");
        systemCmd("atcli at+bconfig=5,0,4000,0,0,1");
    } else {
        log_d("HighSpeed: mmWave disable");
        systemCmd("atcli at+bconfig=5,0,4000,0,0,0");
    }
    state_b = enable;
}

void set_modem_thermal_config(int mode) {
    if (mode == THERMAL_PORTABLE_MODE) {
        log_d("HighSpeed: config to modem thermal Portable mode");
        systemCmd("atcli at+ceitherm=4,0");
        systemCmd("atcli at+ceitherm=5,0");
        systemCmd("atcli at+ceitherm=6,0");
        systemCmd("atcli at+ceitherm=7,0");
    } else if (mode == THERMAL_FIX_MODE) {
        log_d("HighSpeed: config to modem thermal FIX mode");
        systemCmd("atcli at+ceitherm=4,1");
        systemCmd("atcli at+ceitherm=5,1");
        systemCmd("atcli at+ceitherm=6,1");
        systemCmd("atcli at+ceitherm=7,1");
    }
}

void high_speed_battery_reconfig(bool batt_present) {
    log_d("HighSpeed: Battery present state changed to %d", batt_present);
    if (ds_get_bool(DS_KEY_HIGH_SPEED)) {
        if (!batt_present) { //remove battery
            mmwave_enable(true);
        } else { // insert battery
            mmwave_enable(false);
            high_speed_reminder();
        }
    }
}

void init_high_speed() {
    bool enable = ds_get_bool(DS_KEY_HIGH_SPEED);
    if (enable) {
#ifdef HIGH_SPEED_WIFI_DOWN
        log_d("HighSpeed: init by bring down wifi");
        wifi_down();
#endif
        if (get_battery_present() == 0) {
            mmwave_enable(true);
        }
    }
}

bool get_high_speed_notify_flag() {
#ifdef HIGH_SPEED_TEMP_MONITOR
    if (high_speed_remind_flag || hs_temp_monitor_prompt_id != NO_PROMPT ||
             high_speed_shutdown()) {
        return true;
    } else {
        return false;
    }
#else
    return high_speed_remind_flag;
#endif
}

void high_speed_reminder() {
    if (ds_get_bool(DS_KEY_HIGH_SPEED) && get_battery_present()) {
        high_speed_remind_flag = true;
    }
}

void high_speed_config() {
#ifdef FEATURE_ROUTER
    bool enabled = ds_get_bool(DS_KEY_HIGH_SPEED);
    log_d("HighSpeed: Mode config to %s", (enabled) ? "Fixed Mode" : "Portable Mode");

    wlan_fixed_mode(!enabled);

    if (enabled) {
        set_modem_thermal_config(THERMAL_FIX_MODE);
#ifdef HIGH_SPEED_WIFI_DOWN
        log_d("HighSpeed: Enabling by bring down wifi");
        wifi_down();
#endif
        if (get_battery_present()) {
            log_d("HighSpeed: Fixed mode with battery present, turn off mmWave");
            mmwave_enable(false);
            high_speed_remind_flag = true;
            high_speed_prompt();
        } else {
            mmwave_enable(true);
            popup_anim_not_plain_create(get_string(ID_LOADING), 3000);
        }
    } else {
        set_modem_thermal_config(THERMAL_PORTABLE_MODE);
#ifdef HIGH_SPEED_WIFI_DOWN
        log_d("HighSpeed: Disabling by bring up wifi");
        wifi_up();
#endif
        mmwave_enable(true);
        popup_anim_not_plain_create(get_string(ID_LOADING), 3000);
    }
#ifdef HIGH_SPEED_WIFI_DOWN
    update_ssid();
#endif
#ifdef HIGH_SPEED_TEMP_MONITOR
    mmWave_state = MMWAVE_ENABLED_STATE;
#endif
#else //simulator
    bool enabled = ds_get_bool(DS_KEY_HIGH_SPEED);
    if (enabled) {
        high_speed_remind_flag = true;
        high_speed_prompt();
    } else {
        popup_anim_not_plain_create(get_string(ID_LOADING), 3000);
    }
    update_ssid();
#endif
}

void high_speed_prompt() {
    if (!high_speed_remind_flag) return;
    high_speed_remind_flag = false;

    if(is_static_popup()){
        log_d("HighSpeed: static popup exist, skip reminder prompt");
        return;
    }

    popup_anim_not_plain_create(get_string(ID_HIGH_SPEED_PROMPT), 5000);
}

int get_mmWave_temp() {
    int temp = THERMAL_ERROR_VALUE;
    char value[MAX_THERMAL_VALUE];

    // get the highest temperature value from mmWave temp node list
    int i;
    for (i = 0; i < MMWAVE_TEMP_NODE_LEN; i++) {
        if (0 == read_node_value(mmWave_temp_path[i], value, MAX_THERMAL_VALUE)) {
            if(strcmp(value, MMWAVE_DISABLED_VALUE) == 0) {
                return temp;
            }
            int val = atoi(value) / 1000;
            temp = (temp < val) ? val:temp;
        }
    }
    return temp;
}

bool high_speed_shutdown() {
    if (mmWave_state == SHUTDOWN_STATE) {
        return true;
    } else {
        return false;
    }
}

bool show_high_speed_shutdown() {
    if (high_speed_shutdown_flag == true) {
        emergency_shutdown_popup(ID_SHUTDOWN_BATT_HIGH_TEMP);
        high_speed_shutdown_flag = false;
    }
}

void high_speed_temp_monitor() {
    if (mmWave_state == SHUTDOWN_STATE || emergency_shutdown_popup_exist()) return;

    int cpu_temp = get_cpu_temperature();
    int mmWave_temp = get_mmWave_temp();
    bool fixed_mode = ds_get_bool(DS_KEY_HIGH_SPEED);

    // shutdown cases
    if (fixed_mode) {
        if (get_battery_present()) {
            if (cpu_temp >= FIXED_MODE_CPU_TEMP_HIGH) {
                log_d("HighSpeed: Fixed mode device temp is %d, shutdown device", cpu_temp);
                mmWave_state = SHUTDOWN_STATE;
            }
        } else {
            if (cpu_temp >= FIXED_MODE_CPU_TEMP_HIGH ||
                    mmWave_temp >= FIXED_MODE_MMWAVE_TEMP_SHUTDOWN) {
                log_d("HighSpeed: Fixed mode device temp is %d and mmWave temp is %d, shutdown device",
                       cpu_temp, mmWave_temp);
                mmWave_state = SHUTDOWN_STATE;
            }
        }
        if (mmWave_state == SHUTDOWN_STATE) {
            // set this flag for main.c do shutdown prompt from main thread
            high_speed_shutdown_flag = true;
            return;
        }
    } else {
        if (get_battery_temperature() >= get_emergency_temp_high()) {
            // set state but no need to call emergency shutdown since its already impl in power_menu
            mmWave_state = SHUTDOWN_STATE;
        }
    }


    if (mmWave_state == MMWAVE_ENABLED_STATE || mmWave_state == MMWAVE_DISABLING_STATE) {
        if (fixed_mode) {
            if (!get_battery_present() &&
                    (cpu_temp >= FIXED_MODE_CPU_TEMP_HIGH_WO_BAT ||
                     mmWave_temp >= FIXED_MODE_MMWAVE_TEMP_HIGH)) {
                log_d("HighSpeed: Fixed mode device temp is %d and mmWave temp is %d, turn off mmWave",
                                  cpu_temp, mmWave_temp);
                hs_temp_monitor_prompt_id = FIXED_MODE_PROMPT;
                mmWave_state = MMWAVE_DISABLING_STATE;
                mmwave_enable(false);
            }
        } else { // Portable Mode
            int battery_temp = get_battery_temperature();
            if (battery_temp >= PORTABLE_MODE_BAT_TEMP_HIGH) {
                log_d("HighSpeed: Portable mode battery temp is %d, turn off mmWave",
                       battery_temp);
                hs_temp_monitor_prompt_id = PORTABLE_MODE_PROMPT;
                mmWave_state = MMWAVE_DISABLING_STATE;
                mmwave_enable(false);
            }
        }
    }
    if (mmWave_state == MMWAVE_DISABLED_STATE || mmWave_state == MMWAVE_DISABLING_STATE) {
        if (fixed_mode) {
            if (!get_battery_present() && (cpu_temp <= FIXED_MODE_CPU_TEMP_LOW_WO_BAT &&
                                           mmWave_temp <= FIXED_MODE_MMWAVE_TEMP_LOW)) {
                log_d("HighSpeed: Fixed mode device temp become %d & mmWave temp become %d, turn on mmWave",
                       cpu_temp, mmWave_temp);
                mmWave_state = MMWAVE_ENABLED_STATE;
                mmwave_enable(true);
            }
        } else { // Portable Mode
            int battery_temp = get_battery_temperature();
            if (battery_temp <= PORTABLE_MODE_BAT_TEMP_LOW &&
                        mmWave_temp <= PORTABLE_MODE_BAT_TEMP_LOW) {
                log_d("HighSpeed: Portable mode battery temp become %d & mmWave temp become %d, turn on mmWave",
                       battery_temp, mmWave_temp);
                mmWave_state = MMWAVE_ENABLED_STATE;
                mmwave_enable(true);
            }
        }
    }
}

void mmWave_disable_action(lv_obj_t * mbox, lv_event_cb_t event_cb) {
    if (event_cb != LV_EVENT_CLICKED) return;

    mmWave_state = MMWAVE_DISABLED_STATE;
    close_popup();
}

void mmWave_disable_prompt() {
    if (hs_temp_monitor_prompt_id == NO_PROMPT) return;

    static const char * btns[2];
    btns[0] = get_string(ID_OK);
    btns[1] = "";

    popup_scrl_create(get_string((hs_temp_monitor_prompt_id == FIXED_MODE_PROMPT) ?
                                  ID_HIGH_SPEED_FIXED_MODE : ID_HIGH_SPEED_PORTABLE_MODE),
                      get_string(hs_temp_monitor_prompt_id), btns, mmWave_disable_action);

    hs_temp_monitor_prompt_id = NO_PROMPT;
}
