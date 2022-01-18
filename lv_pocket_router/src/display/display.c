#include "display.h"

#include <stdio.h>
#include "wake_lock.h"
#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_pocket_router/src/battery/battery_info.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/high_speed.h"
#include "lv_pocket_router/src/util/util.h"

#define LOCK_NAME           "lvgl_screen"

// For Web UI to sync state of MiFi UI on or off
#define MIFI_UI_ON          "echo 1 > /data/misc/ui_state"
#define MIFI_UI_OFF         "echo 0 > /data/misc/ui_state"

#define BRIGHTNESS_FILE     "/sys/class/leds/lcd-backlight/brightness"
#define BACKLIGHT_OFF   0

lv_task_t * lv_timeout_task;
#ifdef FEATURE_ROUTER
bool keep_on = false;
#else
bool keep_on = true;
#endif
int screen_timeout = 0;
int brightness = 30;
int power_state;

int get_power_state() {
    return power_state;
}

/*
 * Only write valut to node, not save value to xml.
 * Please call power_manager's set_brightness()
 */
void write_brightness(int value) {
#if defined USE_ANDROID_FBDEV
    FILE *f = fopen(BRIGHTNESS_FILE, "w");
    if (f != NULL) {
        if (fprintf(f, "%d\n", brightness) <= 0) {
            log_e("set brightness failed");
        }
        fclose(f);
    }
#endif
}

void reset_timeout_task() {
    if (lv_timeout_task != NULL) {
        lv_task_set_period(lv_timeout_task, LV_TASK_PRIO_OFF);
        lv_task_del(lv_timeout_task);
        lv_timeout_task = NULL;
    }
}

void start_timeout_task() {
    if (!keep_on) {
        reset_timeout_task();
        if(charge_mode()){
            lv_timeout_task = lv_task_create(charge_mode_turn_off_screen, screen_timeout, LV_TASK_PRIO_LOW, NULL);
        }else{
            lv_timeout_task = lv_task_create(turn_off_screen, screen_timeout, LV_TASK_PRIO_LOW, NULL);
        }
    }
}

void notify_brightness_changed(int value) {
    brightness = value;
    write_brightness(brightness);
}

void notify_screen_timeout_changed(int value) {
    screen_timeout = value;
    reset_timeout_task();
    start_timeout_task();
}

void set_keep_screen_on(bool state) {
    keep_on = state;
    if (keep_on) {
        reset_timeout_task();
    } else {
        start_timeout_task();
    }
}

void reset_screen_timeout() {
    if (!keep_on) {
        //log_d("reset_screen_timeout");
        reset_timeout_task();
        start_timeout_task();
    }
}

void backlight_off() {
#if USE_FBDEV || USE_ANDROID_FBDEV
    // turn off backlight
    systemCmd("echo 0xbc46 > /sys/kernel/debug/regmap/spmi0-09/address");
    systemCmd("echo 0x00 > /sys/kernel/debug/regmap/spmi0-09/data");
    systemCmd("echo 0xbc47 > /sys/kernel/debug/regmap/spmi0-09/address");
    systemCmd("echo 0x00 > /sys/kernel/debug/regmap/spmi0-09/data");
#endif
}

void backlight_on() {
#if USE_FBDEV || USE_ANDROID_FBDEV
    if (is_silent_reboot()) return;

    // set backlight level
    systemCmd("echo 0xbc44 > /sys/kernel/debug/regmap/spmi0-09/address");
    systemCmd("echo 0x1e > /sys/kernel/debug/regmap/spmi0-09/data");
    // turn on backlight
    systemCmd("echo 0xbc46 > /sys/kernel/debug/regmap/spmi0-09/address");
    systemCmd("echo 0x80 > /sys/kernel/debug/regmap/spmi0-09/data");
    systemCmd("echo 0xbc47 > /sys/kernel/debug/regmap/spmi0-09/address");
    systemCmd("echo 0x01 > /sys/kernel/debug/regmap/spmi0-09/data");
#endif
}

void turn_on_screen() {
    /*
     * Not allow use FULL_WAKE_LOCK, can check lvgl/lv_pocket_router/src/power/power.c
     * int acquire_wake_lock(int lock, const char* id) {
     * if (lock != PARTIAL_WAKE_LOCK) {
     *    return -EINVAL;
     * }
     */
    acquire_wake_lock(PARTIAL_WAKE_LOCK, LOCK_NAME);
    stop_sleep();

    power_state = SCREEN_ON;

    log_d("turn_on_screen");

    systemCmd(MIFI_UI_ON);

    wifi_wakeup();

#if USE_FBDEV || USE_ANDROID_FBDEV
    Blank(false);

    // use task to delay a bit to stop showing white display flush
    lv_task_t * bl_task = lv_task_create(backlight_on, 60, LV_TASK_PRIO_LOW, NULL);
    lv_task_once(bl_task);
#endif

    write_brightness(brightness);

    start_timeout_task();

    // wlan config might be modified by WebUI while device UI suspend
    // check if need to reload those config that can be modified from WebUI
    hostapd_conf_partial_reload();

    insert_ethernet_module();

#ifdef BATTERY_OPTIMIZE_SUPPORT
    battery_optimize_reminder();
#endif
#ifdef HIGH_SPEED_SUPPORT
    high_speed_reminder();
#endif

    task_debug_refresh();
}

void turn_off_screen() {
    log_d("turn_off_screen");
    power_state = SCREEN_TURNING_OFF;

    systemCmd(MIFI_UI_OFF);

#if USE_FBDEV || USE_ANDROID_FBDEV
    backlight_off();
    Blank(true);
#endif

    write_brightness(BACKLIGHT_OFF);

    reset_timeout_task();

    ui_cleanup();

    power_state = SCREEN_OFF;

    wifi_close_task_refresh(); //run this after set power_state coz it will refer to it

    remove_ethernet_module();

    task_debug_refresh();

    if (!ds_get_bool(DS_KEY_WIFI_AUTO_CLOSE)) {
        start_sleep();
    }
    release_wake_lock(LOCK_NAME);
}

//only turn backlight on/off in charging only mode screen on/off
void charge_mode_turn_on_screen() {
    power_state = SCREEN_ON;
#if USE_FBDEV || USE_ANDROID_FBDEV
    Blank(false);
    backlight_on();
#endif
    start_timeout_task();
}

void charge_mode_turn_off_screen() {
    power_state = SCREEN_TURNING_OFF;
#if USE_FBDEV || USE_ANDROID_FBDEV
    backlight_off();
    Blank(true);
#endif
    reset_timeout_task();
    power_state = SCREEN_OFF;
}

void init_display(int s, int b) {
    screen_timeout = s;
    brightness = b;
}

