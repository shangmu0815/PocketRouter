#include "battery_info.h"
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "lv_pocket_router/src/display/display.h"
#include "lv_pocket_router/src/util/battery_log.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/high_speed.h"
#include "lv_pocket_router/src/util/led.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/power_menu.h"
#include "lv_pocket_router/src/util/usb_compositions.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/about/device_information.h"

#define ERROR_VALUE             -1
#define USB_ONLINE              1

#define POWER_CAPACITY_PATH         "/sys/class/power_supply/battery/capacity"
//SMx3 node
#define POWER_USB_ONLINE_PATH       "/sys/class/power_supply/usb/present"

#define POWER_FULL_CAPACITY_PATH    "/sys/class/power_supply/bms/charge_full"

#define POWER_TEMPERATURE_PATH      "/sys/class/power_supply/battery/temp"

#define CHARGING_CURRENT_PATH       "/sys/class/power_supply/bms/current_now"

#define POWER_VOLTAGE_PATH          "/sys/class/power_supply/bms/voltage_now"

#define CPU_TEMPERATURE_PATH        "/sys/class/thermal/thermal_zone40/temp"

#define BATTERY_TEMP_MONITOR_STATE  "/sys/module/smb138x_charger/parameters/batt_temp_monitor_state"

#define REGISTER_1340_PATH          "/sys/class/power_supply/battery/input_suspend"

#define BATTERY_PRESENT             "/sys/devices/platform/838000.i2c/i2c-4/4-0044/838000.i2c:qcom,smb138x@44:qcom,smb1381-charger@1000/power_supply/battery/present"

#define CHARGER_FV_ENABLE_PATH      "/sys/module/smb138x_charger/parameters/change_fv_enabled"

#define CHARGING_TYPE_PATH          "/sys/class/power_supply/usb/real_type"
#define QC_CHARGING_TYPE            "USB_HVDCP"
#define USB_CHARGING_TYPE           "USB"
#define UNKNOWN_CHARGING_TYPE       "Unknown"

#define READ_NODE_INTERVAL          10000

#define MAX_POWER_VALUE             50

#define GREEN_LIGHT_LEVEL           90

#define BATTERY_OPTIMIZE_REMINDED   "/tmp/battery_optimize_reminded"
#define BATTERY_CHARGING_TIME       "/tmp/battery_charging_t"  // charging timestamp

typedef enum {
    PROTECT_INACTIVE,
    PROTECT_PARTIAL_CHARGE,
    PROTECT_STOP_CHARGE
} protect_type_prompt_t;

static int battery_level;
static int new_battery_level;
static bool usb;
static bool battery_removed = false;
static int emergency_high_temp = -1;
static protect_type_prompt_t protect_type_prompt = PROTECT_INACTIVE;

#ifdef BATTERY_OPTIMIZE_SUPPORT
static bool optimize_remind_flag = false; // for main.c to check if need to show optimize prompt
static lv_obj_t * bat_opt_obj = NULL;
#endif

// for Quick Charge prompt
static lv_task_t * lv_qc_prompt_task = NULL;
static bool quick_charge_shown_b = false; // flag to control only show prompt once per boot
static bool show_quick_charge_b = false;

int read_node(char* path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
#ifdef FEATURE_ROUTER
        log_e("open node error: %s", path);
#endif
        return ERROR_VALUE;
    }

    char buf[MAX_POWER_VALUE];
    int len = read(fd, buf, sizeof(buf));
    if (len < 0) {
#ifdef FEATURE_ROUTER
        log_e("read value errno %d %s: %s", errno, strerror(errno), path);
#endif
        close(fd);
        return ERROR_VALUE;
    }

    close(fd);

    char value[MAX_POWER_VALUE];
    memset(value, 0, sizeof(value));
    memcpy(value, buf, len);
    return atoi(value);
}

void update_led(bool charging) {
#ifndef CUST_LUXSHARE
    if (charging) {
        if (battery_level >= GREEN_LIGHT_LEVEL) {
            led_green();
        } else {
            led_red();
        }
    } else {
        led_off();
    }
#endif
}

bool qc_prompt_checking() {
    if (lv_qc_prompt_task != NULL)
        return true;
    else
        return false;
}

void stop_qc_prompt_task() {
    if (lv_qc_prompt_task != NULL) {
        lv_task_del(lv_qc_prompt_task);
        lv_qc_prompt_task = NULL;
    }
}

void quick_charge_prompt() {
    static const char * btns[2];
    btns[0] = get_string(ID_OK);
    btns[1] = "";

    if (get_battery_present()) {
        popup_scrl_create(get_string(ID_ABOUT_CHARGING),
              get_string(ID_QC_W_BATTERY_PROMPT), btns, NULL);
    } else {
        popup_scrl_create(get_string(ID_ABOUT_CHARGING),
              get_string(ID_QC_WO_BATTERY_PROMPT), btns, NULL);
    }
    show_quick_charge_b = false;
    quick_charge_shown_b = true;
}

bool show_quick_charge() {
    if (quick_charge_shown_b) return false;
    return show_quick_charge_b;
}

void check_quick_charge_impl() {
    char type[MAX_POWER_VALUE];

    stop_qc_prompt_task();

    if(usb && 0 == read_node_value(CHARGING_TYPE_PATH, type, MAX_POWER_VALUE)) {
        log_d("QC Prompt: charging type is %s", type);
        // skip if charge type is USB
        if (strlen(type) == 4 && strncmp(type, USB_CHARGING_TYPE , strlen(USB_CHARGING_TYPE)) == 0) {
            log_d("QC Prompt: charging type is %s, no need to show QC prompt", type);
            return;
        }

        int current = get_charging_current();
        int battery_temp = get_battery_temperature();
        int cpu_temp = get_cpu_temperature();
        int voltage = get_battery_voltage();
        log_d("QC Prompt: current: %d, battery temp: %d, cpu temp: %d, voltage: %d",
                  current, battery_temp, cpu_temp, voltage);
        if (current > 1500 || battery_temp > 45 || cpu_temp > 45 || voltage >= 4300000) {
            log_d("QC Prompt: skip Quick Charge warning");
            return;
        }

        if (strncmp(type, UNKNOWN_CHARGING_TYPE , strlen(UNKNOWN_CHARGING_TYPE)) == 0) {
            log_d("QC Prompt: charge type is Unknown, wait another 5 sec");
            check_quick_charge();
            return;
        }
        if (strncmp(type, QC_CHARGING_TYPE , strlen(QC_CHARGING_TYPE)) != 0 &&
                   read_usb_state() == USB_STATE_NOT_ATTACHED) {
            show_quick_charge_b = true;
            log_d("QC Prompt: Prompt Quick Charge warning, charge type is %s", type);
        }
    }
}

void check_quick_charge() {
    if (quick_charge_shown_b) return;

    if (!lv_qc_prompt_task) {
        lv_qc_prompt_task = lv_task_create(check_quick_charge_impl, QC_PROMPT_CHECK_INTERVAL * 1000, LV_TASK_PRIO_LOW, NULL);
        lv_task_once(lv_qc_prompt_task);
    }
}

void dump_usb_state() {
    usb = (read_node(POWER_USB_ONLINE_PATH) == USB_ONLINE);
}

bool is_charging() {
    static bool charging = false;
    //bool usb = (read_node(POWER_USB_ONLINE_PATH) == USB_ONLINE);
    bool ac = false;// TODO

    if ((usb || ac) != charging) {
        update_led(usb || ac);
#ifdef BATTERY_OPTIMIZE_SUPPORT
        update_charging_timer(usb || ac);
#endif
#ifdef BATTERY_LOG
        battery_log_plugin(!charging);
#endif
        if (usb) check_quick_charge();
    }

    charging = usb || ac;

    return charging;
}

void dump_battery_level() {
#if !defined(NO_BATTERY)
    if (get_battery_present()) {
        new_battery_level = read_node(POWER_CAPACITY_PATH);
    } else {
        new_battery_level = 0;
    }
#endif
}

int get_battery_info() {
    static uint32_t timestamp;
    if (new_battery_level == 0) { //force to read node every 10 sec if level is zero
        uint32_t t = lv_tick_elaps(timestamp);
        if (t > READ_NODE_INTERVAL) {
            dump_battery_level();
            timestamp = lv_tick_get();
        }
    } else {
        timestamp = lv_tick_get();
    }

    if ((battery_level < GREEN_LIGHT_LEVEL && new_battery_level >= GREEN_LIGHT_LEVEL) ||
        (battery_level >= GREEN_LIGHT_LEVEL && new_battery_level < GREEN_LIGHT_LEVEL)) {
        battery_level = (new_battery_level == ERROR_VALUE) ? battery_level : new_battery_level;
        update_led(is_charging());
    }
    battery_level = (new_battery_level == ERROR_VALUE) ? battery_level : new_battery_level;
#ifdef BATTERY_LOG
    battery_log_level(battery_level);
#endif
    return battery_level;
}

int get_battery_capacity() {
#if !defined(NO_BATTERY)
    if (get_battery_present())
        return read_node(POWER_FULL_CAPACITY_PATH);
    else
        return 0;
#else
    return 0;
#endif
}

int get_battery_temperature() {
#if !defined(NO_BATTERY)
    static uint32_t timestamp;
    static int temp;
    int pre_temp = temp;
    if (get_battery_present()) {
        uint32_t t = lv_tick_elaps(timestamp);
        if (t > READ_NODE_INTERVAL || temp == 0) {
            temp = read_node(POWER_TEMPERATURE_PATH) / 10;
            timestamp = lv_tick_get();

            if (temp > 55 && pre_temp <= 55) {
                protect_type_prompt = PROTECT_STOP_CHARGE;
            } else if (temp >= 45 && pre_temp < 45) {
                protect_type_prompt = PROTECT_PARTIAL_CHARGE;
            }
        }
        return temp;
    } else {
        return 0;
    }
#else
    return 0;
#endif
}

int get_charging_current() {
#if !defined(NO_BATTERY)
    return read_node(CHARGING_CURRENT_PATH) / 1000;
#else
    return 0;
#endif
}

int get_battery_voltage() {
#if !defined(NO_BATTERY)
    if (get_battery_present())
        return read_node(POWER_VOLTAGE_PATH);
    else
        return 0;
#else
    return 0;
#endif
}

int get_cpu_temperature() {
    int temp = ERROR_VALUE;
    char value[MAX_POWER_VALUE];

    if (0 == read_node_value(CPU_TEMPERATURE_PATH, value, MAX_POWER_VALUE)) {
        temp = atoi(value) / 1000;
    }
    return temp;
}

int get_driver_workqueue_current() {
#if !defined(NO_BATTERY)
    if (get_battery_present())
        return read_node(BATTERY_TEMP_MONITOR_STATE );
    else
        return 0;
#else
    return 0;
#endif
}

int get_register_1340() {
    return read_node(REGISTER_1340_PATH);
}

int get_battery_present() {
#if !defined(NO_BATTERY)
    static uint32_t timestamp;
    static int batt_present = ERROR_VALUE;
    int pre_state = batt_present;
    uint32_t t = lv_tick_elaps(timestamp);
    if (t > READ_NODE_INTERVAL || batt_present == ERROR_VALUE) {
        batt_present = read_node(BATTERY_PRESENT);
        timestamp = lv_tick_get();

#ifdef HIGH_SPEED_SUPPORT
        if (pre_state != batt_present) {
            high_speed_battery_reconfig((batt_present == 1) ? true : false);
        }
#endif
    }
    return batt_present;
#else
    return 0;
#endif
}

bool battery_ever_removed() {
    if (!battery_removed && get_battery_present() == 0) {
        log_d("Battery detected not presented");
        battery_removed = true;
    }
    return battery_removed;
}

bool get_battery_protect_flag() {
    if (protect_type_prompt == PROTECT_INACTIVE) {
        return false;
    } else {
        return true;
    }
}

void battery_protect_prompt() {
    if (protect_type_prompt == PROTECT_INACTIVE) return;

    if(is_static_popup()){
        log_d("BatProtect: static popup exist, skip reminder prompt");
        return;
    }

    if (protect_type_prompt == PROTECT_PARTIAL_CHARGE) {
        popup_anim_not_create(get_string(ID_BATTERY_PROTECT_PARTIAL_CHARGE), NULL, NULL, NULL);
    } else {
        popup_anim_not_create(get_string(ID_BATTERY_PROTECT_STOP_CHARGE), NULL, NULL, NULL);
    }
    set_static_popup(true);
    lv_task_t * task = lv_task_create(close_static_popup, 5000, LV_TASK_PRIO_MID, NULL);
    lv_task_once(task);

    protect_type_prompt = PROTECT_INACTIVE;
}

void reset_emergency_temp_high() {
    emergency_high_temp = -1;
}

int get_emergency_temp_high_impl(){
    int temperature = EMERGENCY_TEMP_HIGH;
#if (0) // remove below to config mmWave high temp shutdown to 60 degree
#if FEATURE_ROUTER
    //read Customer id from /oem/cust_id
    //id >= 0R50, set high temperature to 70, set to 60 degree otherwise
    char buf[3];
    memset(buf, '\0', 3);
    readVersion("/oem/cust_id", buf);
    if(buf != NULL && strlen(buf) > 0 && atoi(buf) >= 50){
        log_d("mmWave ver detected, update shutdown high temp to %d", EMERGENCY_QC_TEMP_HIGH);
        temperature = EMERGENCY_QC_TEMP_HIGH;
    } else {
        //read Quick Charge type from CHARGING_TYPE_PATH
        //if is QC charging type, set high temperature to 70, set to 60 degree otherwise
        char value[MAX_POWER_VALUE];
        if(0 == read_node_value(CHARGING_TYPE_PATH, value, MAX_POWER_VALUE)) {
            if (strncmp(value, QC_CHARGING_TYPE , strlen(QC_CHARGING_TYPE)) == 0) {
                log_d("Quick Charge charging type detected, update shutdown high temp to %d", EMERGENCY_QC_TEMP_HIGH);
                temperature = EMERGENCY_QC_TEMP_HIGH;
            }
        }
    }
#endif
#endif
    log_d("emergency_high_temp set to %d", temperature);
    return temperature;
}

int get_emergency_temp_high(){
    if(emergency_high_temp < 0){
        emergency_high_temp = get_emergency_temp_high_impl();
    }
    return emergency_high_temp;
}

bool battery_protect_shutdown() {
    // This function just check and return if should shutdown for case like
    // screen off where status bar not exist, actual shutdown in status_bar
    int temp = get_battery_temperature();
#if BATTERY_EXTRA_TIME_OUT
    if(!battery_ever_removed() && (get_battery_info() == 0) && !is_charging()
            && ((get_battery_extra_to() != ES_TIMEOUT_START))){
#else
    if(!battery_ever_removed() && (get_battery_info() == 0) && !is_charging()){
#endif
        return true;
    } else if(get_battery_present() && temp <= EMERGENCY_TEMP_LOW){
        return true;
    } else if(get_battery_present() && temp >= get_emergency_temp_high()){
        return true;
    }
    return false;
}

#ifdef BATTERY_OPTIMIZE_SUPPORT
void init_charging_fv() {
#ifdef FEATURE_ROUTER
    bool enable = ds_get_bool(DS_KEY_BATTERY_OPTIMIZE);
    set_charging_fv(enable);
#endif
}

void set_charging_fv(bool enable) {
#ifdef FEATURE_ROUTER
    if (enable) {
        systemCmd("echo 1 > /sys/module/smb138x_charger/parameters/change_fv_enabled");
    } else {
        systemCmd("echo 0 > /sys/module/smb138x_charger/parameters/change_fv_enabled");
    }
#endif
}

// Only need to show user Optmize Reminder prompt once in each boot cycle
// touch BATTERY_OPTIMIZE_REMINDED file in tmp to set a flag to know if
// prompt have been shown to user in this boot cycle
void set_optimize_reminded() {
#ifdef FEATURE_ROUTER
    char cmd[50];
    sprintf(cmd, "touch %s", BATTERY_OPTIMIZE_REMINDED);
    systemCmd(cmd);
#endif
}

bool optimize_reminded() {
    static bool reminded = false;

    // no need to enable battery optimize reminder timer if Battery Optimize
    // already config to be applied
    if (ds_get_bool(DS_KEY_BATTERY_OPTIMIZE)) {
        log_d("BatOpt: Battery Optimize already config to be applied");
        return true;
    }
#ifdef FEATURE_ROUTER
    if (reminded) {
        log_d("BatOpt: Battery Optimized prompt already been shown to user in this boot cycle");
    } else {
        struct stat buffer;
        reminded = (stat(BATTERY_OPTIMIZE_REMINDED, &buffer) == 0);
    }
#endif
    return reminded;
}

bool get_bat_opt_notify_flag() {
    if (optimize_remind_flag) {
        optimize_remind_flag = false;
        return true;
    } else {
        return false;
    }
}

void set_bat_opt_notify_flag() {
    optimize_remind_flag = true;
}

void optimize_remind_action(lv_obj_t * mbox, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;

    log_d("BatOpt: Reminder confirmed by user");
    set_optimize_reminded();
    bat_opt_obj = NULL;
    close_popup();
}

void optimize_remind_close(void) {
    if (bat_opt_obj != NULL) {
        bat_opt_obj = NULL;
        close_popup();
    }
}

void battery_optimize_prompt() {
    if(is_static_popup()){
        log_d("BatOpt: static popup exist, skip battery opt remind prompt");
        return;
    }
    if (bat_opt_obj == NULL) {
        static const char * btns[2];
        btns[0] = get_string(ID_OK);
        btns[1] = "";
        bat_opt_obj = popup_scrl_create_impl(get_string(ID_BATTERY_OPTIMIZE),
                            get_string(ID_BATTERY_OPTIMIZE_PROMPT), btns,
                            optimize_remind_action, NULL);
        set_popup_cb(optimize_remind_close);
    }
}

void battery_optimize_reminder() {
    if (!is_charging() || bat_opt_obj != NULL || optimize_reminded()) {
        return;
    }

    time_t now = time(NULL);
    int charged_time = now - read_node(BATTERY_CHARGING_TIME);
    log_d("BatOpt: Battery have been charging for %d seconds", charged_time);

    if (charged_time > 172800) { // 48 hours
        set_bat_opt_notify_flag();
    }
}

void update_charging_timer(bool charging) {
    if (optimize_reminded()) return;

    char cmd[50];
    if (charging) {
        int charged_time = read_node(BATTERY_CHARGING_TIME);
        if (charged_time == 0 || charged_time == ERROR_VALUE) {
            sprintf(cmd, "echo %d > %s", time(NULL), BATTERY_CHARGING_TIME);
            systemCmd(cmd);
        }
    } else {
        sprintf(cmd, "echo 0 > %s", BATTERY_CHARGING_TIME);
        systemCmd(cmd);
        optimize_remind_close();
    }
}
#endif /* BATTERY_OPTIMIZE_SUPPORT */
