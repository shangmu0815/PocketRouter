
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#if !defined (FEATURE_ROUTER)
#define SDL_MAIN_HANDLED        /*To fix SDL's "undefined reference to WinMain" issue*/
#include <SDL2/SDL.h>
#endif
#include "lvgl/lvgl.h"
#include "lv_drivers/display/monitor.h"
#include "lv_drivers/indev/mouse.h"
#include "lv_drivers/indev/mousewheel.h"
#include "lv_drivers/indev/keyboard.h"
#include "lv_examples/lv_apps/demo/demo.h"
#include "lv_examples/lv_apps/benchmark/benchmark.h"
#include "lv_examples/lv_examples.h"

#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "lvgl/src/lv_hal/Arduino.h"
#include "lv_pocket_router/src/dashboard.h"
#include "lv_pocket_router/src/launcher.h"
#include "lv_pocket_router/src/status_bar.h"
#include "lv_pocket_router/src/status_bar_view.h"
#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/wlan/hostapd_conf.h"
#include "lv_pocket_router/src/display/display.h"
#include "lv_pocket_router/src/util/power_manager.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/default_settings.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/high_speed.h"
#include "lv_pocket_router/src/util/led.h"
#include "lv_pocket_router/src/util/usb_compositions.h"
#include "lv_pocket_router/src/settings/update.h"
#include "lv_pocket_router/src/sms/sms_store.h"
#include "lv_pocket_router/src/ipc/socket_server.h"
#include "lv_pocket_router/src/settings/pin_management.h"
#include "lv_pocket_router/src/settings/profile_management.h"
#include "lv_pocket_router/src/settings/network_settings.h"
#include "lv_pocket_router/src/util/power_menu.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/battery/battery_charging.h"
#include "lv_pocket_router/src/battery/battery_info.h"
#include "lv_pocket_router/src/util/battery_log.h"
#include "lv_pocket_router/src/display/display.h"
#include "lv_pocket_router/src/settings/sim_network_controller.h"
#include "lv_pocket_router/src/settings/preference_network.h"

#if USE_EVDEV
#include "lv_drivers/indev/hw_events.h"
#else
#include "lv_drivers/display/monitor.h"
#include "lv_drivers/indev/mouse.h"
#include "lv_drivers/indev/mousewheel.h"
#include "lv_drivers/indev/keyboard.h"
#endif  /*USE_EVDEV*/

#if FEATURE_ROUTER

#define CHARGE_MODE_LONGPRESS_PERIOD 2000
#ifdef CUST_LUXSHARE
#define LONGPRESS_PERIOD 3000 // 3sec
#else
#define LONGPRESS_PERIOD 800 // 800ms
#endif
#define REPEAT_PERIOD 200    // 200ms
#define DOWN 1
#define UP   0

int key_period = LONGPRESS_PERIOD;
static uint32_t pw_key_timestamp;
static int emergency_shutdown_id = 0;

#if BATTERY_EXTRA_TIME_OUT
#define ES_EXTRA_TIME_OUT   30000
static int ES_TIME_OUT_STATUS = ES_TIMEOUT_NONE;
static uint32_t es_timestamp = 0;
#endif

static bool pw_key_down = false;
lv_task_t * pw_task = NULL;
lv_task_t * charging_pw_task = NULL;
static bool screen_on_flag = false;
static bool turning_screen_on = false;
static usb_state_t usb_state_g = USB_STATE_UNKNOWN;
static bool wifi_reboot_flag = false;
static int prev_sim_state = UNKNOWN;
static bool potential_sim_insert = false;

//when ril_init done, ril_init_done will be set to true
lv_task_t * monitor_ril_init_task;
bool ril_init_done;
int ril_init_state = RIL_ERROR_UNKNOWN;

static bool power_menu_enable = false;
static bool fota_result_notify = false;

static void sig_hdlr(int signal)
{
    switch (signal)
    {
    case SIGTERM:
        log_d("sig_hdlr SIGTERM!");
        break;
    case SIGINT:
        log_d("sig_hdlr SIGINT!");
        break;
    case SIGSEGV:
        log_d("sig_hdlr SIGSEGV!");
        trace_dump();
        break;
    default:
        log_d("Received unexpected signal %d", signal);
        trace_dump();
        break;
    }

    // de-init
    ril_deinit();
    exit(0);
}

void screen_on() {
    if (get_power_state() == SCREEN_OFF) {
        turning_screen_on = true;
        config_locale();
        dump_battery_level();
        dashboard_create();
        sim_pin_check();
        turn_on_screen();
        sync_data_storage_apn();
        if (ril_init_done == true) {
            check_airplane_mode();
        }
        key_period = REPEAT_PERIOD; //start to read key events

        screen_on_flag = false;
        turning_screen_on = false;
    }
}

void lunch_power_menu() {
    reset_screen_timeout();
    key_period = REPEAT_PERIOD;
    //Entry point for create power menu popup
    power_menu_popup_create();
}

//for config popup extra setting
void reboot_popup_extra_setting(int strid){
    if(strid == ID_SHUTDOWN_CONFIG_REBOOT){
        ds_set_bool(DS_KEY_WIFI_REBOOT_FLAG, true);
        log_d("wifi finish loading, reboot device");
    }
}

//charging only mode long press action
void charge_mode_power_key_longpress_timeout(){
    if (charging_pw_task != NULL) {
        //device reboot if long press in charging only mode
        key_period = LONGPRESS_PERIOD;
        charging_only_restart();
    }
}

//charging only mode event callback
int lvgl_ev_callback_charge_mode(int fd, uint32_t epevents) {
    struct input_event ev;
    if (ev_get_input(fd, epevents, &ev) == -1) {
        return -1;
    }

    if (ev.type == EV_KEY && ev.code == KEY_POWER) {
        if (ev.value == UP) {
            //check elapsed time
            uint32_t t = lv_tick_elaps(pw_key_timestamp);
            if (t < LONGPRESS_PERIOD) {
                //pw key short press
                if (get_power_state() == SCREEN_OFF) {
                    key_period = REPEAT_PERIOD;
                    dump_battery_level();
                    charge_mode_turn_on_screen();

                } else if (get_power_state() == SCREEN_ON) {
                    key_period = -1; //stop read key events
                    charge_mode_turn_off_screen();
                }
            }
            if(charging_pw_task != NULL){
                lv_task_del(charging_pw_task);
                charging_pw_task = NULL;
            }
        } else if (ev.value == DOWN) {
            key_period = CHARGE_MODE_LONGPRESS_PERIOD;
            pw_key_timestamp = lv_tick_get();
            charging_pw_task = lv_task_create(charge_mode_power_key_longpress_timeout, CHARGE_MODE_LONGPRESS_PERIOD, LV_TASK_PRIO_LOW, NULL);
            lv_task_once(charging_pw_task);
        }
    }
    return 0;
}

void power_key_longpress_timeout(){
    if (pw_key_down) {
        pw_key_down = false;
        key_period = LONGPRESS_PERIOD;
#ifdef CUST_LUXSHARE
        // let power off to be handle by led, so it can finish led task
        // before shutdown command kill us while still performing power off led flash
        set_reboot_screen_on(true);
        update_custom_led();
#else
        //launch power menu if screen on and pass 800ms
        if(get_power_state() == SCREEN_ON){
            power_menu_enable = true;
        }
#endif
    }
}

int lvgl_ev_callback(int fd, uint32_t epevents) {
    struct input_event ev;
    if (ev_get_input(fd, epevents, &ev) == -1) {
        return -1;
    }

    if (ev.type == EV_KEY && ev.code == KEY_POWER) {
        power_menu_enable = false;
        if (ev.value == UP) {
            pw_key_down = false;
            //check elapsed time
            uint32_t t = lv_tick_elaps(pw_key_timestamp);
            log_d("power key timestamp: %d", t);
            if (t < LONGPRESS_PERIOD) {
                if (get_power_state() == SCREEN_OFF) {
                    // if set screen_on_flag for main thread to do screen on
                    // will cause device resume performance issue
                    screen_on();
                } else if (get_power_state() == SCREEN_ON) {
                    key_period = -1; //stop read key events
                    if (turning_screen_on) {
                        log_d("turning on screen, skip power key event");
                    } else if(get_reboot_screen_on()){
                        log_d("power off/restart device, skip power key event");
                    } else {
                        turn_off_screen();
                    }
                } else {
                    log_d("skip power key event during state transition");
                }
            }
        } else if (ev.value == DOWN) {
            key_period = LONGPRESS_PERIOD;
            //start counting power key pressing elapsed time to
            //decide weather to do suspend/resume/power menu launch
            pw_key_timestamp = lv_tick_get();
            pw_key_down = true;
            pw_task = lv_task_create(power_key_longpress_timeout, LONGPRESS_PERIOD, LV_TASK_PRIO_LOW, NULL);
            lv_task_once(pw_task);
        }
    }
    return 0;
}

static void* InputThreadLoop(void* param) {
    //polling key events
    while (true) {
        if (!ev_wait(key_period)) {
            ev_dispatch(); // call ev_callback
        }
    }
}

#if defined (DEBUG_MEM)
/**
 * Print the memory usage periodically
 * @param param
 */
static void memory_monitor(void * param)
{
    (void) param; /*Unused*/

    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);

    if (mon.used_pct > 50) {
        log_d("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d\n", (int)mon.total_size - mon.free_size,
           mon.used_pct,
           mon.frag_pct,
           (int)mon.free_biggest_size);
    }
}
#endif

bool isNumeric(const char *str){
    while(*str != '\0'){
        if(*str < '0' || *str > '9')
            return false;
        str++;
    }
    return true;
}

//sms call back function, will be called when receive sms
void smscb (ril_sms_pp_ind_msg *ind_data)
{
    log_d("smscb(): add = %s, content = %s, encoding type = %d", ind_data->source_address, ind_data->message_content, ind_data->encoding_type);
    log_d("smscb(): len = %d, class = %d, format = %d, id = %d", ind_data->message_content_length, ind_data->message_class, ind_data->message_format, ind_data->message_id);
    log_d("smscb(): in sim = %d, record_num = %d, timestamp = %s", ind_data->message_storage, ind_data->record_num, ind_data->timestamp);
    log_d("smscb(): read status = %d", ind_data->message_status);

    char current_date[20];
    //add received sms to sms_storage.xml
    SMS_THREAD thread;
    thread.number  = ind_data->source_address;
    thread.content = ind_data->message_content;
    //save sms encoding type
    char encoding[2];
    memset(encoding, '\0', sizeof(encoding));
    sprintf(encoding, "%d", ind_data->encoding_type);
    thread.encoding_type = encoding;
    //save sim sms id and status
    char id_sim[15];
    memset(id_sim, '\0', sizeof(id_sim));
    sprintf(id_sim, "%d", ind_data->record_num);
    thread.rec_num = id_sim;
    if(ind_data->message_storage) {
        thread.in_sim  = "true";
    } else {
        thread.in_sim  = "false";
    }
    if((ind_data->message_status == RIL_SMS_MESSAGE_STATUS_READ)
            || (ind_data->message_status == RIL_SMS_MESSAGE_STATUS_SENT)) {
        thread.read  = "true";
    } else {
        thread.read  = "false";
    }

    char* time_stamp = ind_data->timestamp;
    if(isNumeric(time_stamp) &&
            (strlen(time_stamp) == RIL_SMS_MAX_TIMESTAMP_LENGTH)){
        //get current time for sim sms
        //translate current time format from 19071512365732 to 2019/07/15 12:36
        char year[5];
        char month[3];
        char day[3];
        char hr[3];
        char min[3];
        char tmp[5];
        char* time_list[] = { &year, &month, &day, &hr, &min};
        int len = sizeof(time_list) / sizeof(char *);
        int i;
        for(i = 0; i < len; i++){
            memset(tmp, '\0', sizeof(tmp));
            strncpy(tmp, time_stamp, 2);
            if(i == 0){
                sprintf(time_list[0], "20%s", tmp);
            } else {
                sprintf(time_list[i], "%s", tmp);
            }
            time_stamp = time_stamp + 2;
        }
        sprintf(current_date, "%s/%s/%s %s:%s", year, month, day, hr, min);
    }else{
        //get current time for non sim sms
        time_t t = time(NULL);
        struct tm *timeinfo = localtime(&t);
        memset(current_date, '\0', sizeof(current_date));
        strftime(current_date, sizeof(current_date), "%Y/%m/%d %H:%M", timeinfo);
    }
    thread.date = current_date;
    write_new_sms(thread);
}

void sim_pin_check()
{
    // if device will do config reboot later, do not show pin input
    if (!wifi_reboot_flag) return;

    int sim_state = get_sim_state();
    log_d("sim_pin_check sim_state:%d", sim_state);
    if (sim_state == PIN_REQUIRED) {
        if (is_silent_reboot()) {
            log_d("silent reboot sim verify");
            verify_sim_pin(get_sim_pin());
        } else {
            pin_puk_retry_left_info_page(SIM_PIN, get_sim_pin1_retries());
        }
    } else if (sim_state == PUK_REQUIRED) {
        pin_puk_retry_left_info_page(SIM_PUK, get_sim_puk1_retries());
    }
    if (is_silent_reboot()) {
        reset_silent_reboot();
        turn_off_screen();
    }
}

void sim_pin_req_cb(sim_pin_req_enum  sim_pin_req, int error, int retry_left)
{
    switch(sim_pin_req)
    {
        case SIM_PIN_REQ_VERIFY_PIN:
            log_d("sim_pin_req_cb, SIM_PIN_REQ_VERIFY_PIN, error=%d, retry left=%d", error, retry_left);
            break;
        case SIM_PIN_REQ_CHANGE_PIN:
            log_d("sim_pin_req_cb, SIM_PIN_REQ_CHANGE_PIN, error=%d, retry left=%d", error, retry_left);
            break;
        case SIM_PIN_REQ_ENABLE_PIN:
            log_d("sim_pin_req_cb, SIM_PIN_REQ_ENABLE_PIN, error=%d, retry left=%d", error, retry_left);
            break;
        case SIM_PIN_REQ_DISABLE_PIN:
            log_d("sim_pin_req_cb, SIM_PIN_REQ_DISABLE_PIN, error=%d, retry left=%d", error, retry_left);
            break;
        case SIM_PIN_REQ_VERIFY_PUK:
            log_d("sim_pin_req_cb, SIM_PIN_REQ_VERIFY_PUK, error=%d, retry left=%d", error, retry_left);
            break;
        default:
            break;
    }
    pin_puk_verify_result_cb(sim_pin_req, error, retry_left);
}

void nw_resp_cb(ril_nw_resp_enum nw_resp, int error, unsigned int resp_data_size, void *resp_data)
{
    /*** each response event has its own response data. ***/
    ril_nw_scan_result_t *scan_result;
    ril_nw_service_state *nw_date_reg_state;
    ril_nw_nitz_time_info_t *nitz_time;

    switch(nw_resp)
    {
        case RIL_NW_RESP_NW_SCAN:
            scan_result = (ril_nw_scan_result_t *)resp_data;
            log_d("RIL_NW_RESP_NW_SCAN, error=%d, resp_data_size=%d, length=%d", error, resp_data_size, scan_result->entry_len);
            //start parsing data in network settings
            manual_search_cb(scan_result);

            //we call below in network settings based on which NS user choose
            //set_nw_selection_manual(&scan_result->entry[0]);
            break;
        case RIL_NW_RESP_NW_SELECTION:
            log_d("RIL_NW_RESP_NW_SELECTION, error=%d, resp_data_size=%d", error, resp_data_size);
            break;
        case RIL_NW_IND_NITZ_TIME:
            log_d("RIL_NW_IND_NITZ_TIME, error=%d, resp_data_size=%d", error, resp_data_size);
            nitz_time = (ril_nw_nitz_time_info_t *)resp_data;
            if (nitz_time != NULL) {
                set_device_time(nitz_time->nitz_time);
            }
            break;
        case RIL_NW_RESP_DATA_REG_STATE:
            nw_date_reg_state = (ril_nw_service_state *)resp_data;
            log_d("RIL_NW_RESP_DATA_REG_STATE, current reg state=%d", *nw_date_reg_state);
            break;
        default:
            break;
    }
}

#if BATTERY_EXTRA_TIME_OUT
int get_battery_extra_to(){
    return ES_TIME_OUT_STATUS;
}
void reset_battery_extra_to(){
    log_d("battery low extra timeout reset");
    es_timestamp = 0;
    ES_TIME_OUT_STATUS = ES_TIMEOUT_NONE;
}
#endif

//check regularly if device met the condition to do emergency shutdown
void check_emergency_shutdown(){
    int temp = get_battery_temperature();
    int level = get_battery_info();

    if(!battery_ever_removed() && level == 0 && !is_charging()){
#if BATTERY_EXTRA_TIME_OUT
        if((get_battery_extra_to() == ES_TIMEOUT_NONE)
                && !emergency_shutdown_popup_exist()){
            log_d("battery low extra timeout start");
            es_timestamp = lv_tick_get();
            ES_TIME_OUT_STATUS = ES_TIMEOUT_START;
        } else if(get_battery_extra_to() == ES_TIMEOUT_START){
            uint32_t t = lv_tick_elaps(es_timestamp);
            if(t > ES_EXTRA_TIME_OUT){
                ES_TIME_OUT_STATUS = ES_TIMEOUT_OVER;
            }
        } else if(get_battery_extra_to() == ES_TIMEOUT_OVER){
            log_d("battery low extra timeout finish");
            reset_battery_extra_to();
            emergency_shutdown_id = ID_SHUTDOWN_BATT_LOW;
            screen_on_flag = true;
        }
#else
        log_d("*** power low emergency_shutdown ***");
        emergency_shutdown_id = ID_SHUTDOWN_BATT_LOW;
        screen_on_flag = true;
#endif
    } else if(get_battery_present() && temp <= EMERGENCY_TEMP_LOW){
        log_d("*** temperature low emergency_shutdown ***");
        emergency_shutdown_id = ID_SHUTDOWN_BATT_LOW_TEMP;
        screen_on_flag = true;
    } else if(get_battery_present() && temp >= get_emergency_temp_high()){
        log_d("*** temperature high emergency_shutdown ***");
        emergency_shutdown_id = ID_SHUTDOWN_BATT_HIGH_TEMP;
        screen_on_flag = true;
    }
#if BATTERY_EXTRA_TIME_OUT
      else if(get_battery_extra_to() == ES_TIMEOUT_START && ((level > 0) || is_charging())
              || (get_battery_extra_to() == ES_TIMEOUT_OVER)){
        reset_battery_extra_to();
    }
#endif
}

//check sim state to detect sim insert/remove
void check_sim_insert_remove(){
    bool screen_on = (get_power_state() == SCREEN_ON)? true:false;
    int state = get_sim_state();
    if(state != prev_sim_state){
        save_reboot_state(state, prev_sim_state, false);
        log_d("sim state change, prev state: %d, curr state: %d ", prev_sim_state, state);
    }
    if((prev_sim_state == ABSENT) || (prev_sim_state == CARD_IO_ERROR)){
        potential_sim_insert = true;
    } else if(potential_sim_insert && state == READY){
        log_d("*** sim card insert ***");
        potential_sim_insert = false;
        sim_change_impl(screen_on ? SCREEN_ON_SIM_INSERT:SCREEN_OFF_SIM_INSERT);
    } else if(prev_sim_state != ABSENT && state == ABSENT){
        log_d("*** sim card remove ***");
        sim_change_impl(screen_on ? SCREEN_ON_SIM_REMOVE:SCREEN_OFF_SIM_REMOVE);
    }
    prev_sim_state = state;
}

ril_init_data init_data;

int bg_thread() {
    // workaround to fix mobileap_cfg.xml unsync after upgrade from C2 to CS9
    convert_mobileapcfg();

    //init wlan load config
    init_wlan_data();

    // For low power boot up failed issue, change default power on with
    // WLAN OFF and airplane mode ON (by wlan driver and modem), we check if need to
    // prompt low power shutdown or QC Prompt before do wlan & ril init (which will
    // do WLAN ON and airplane mode OFF)
#if BATTERY_EXTRA_TIME_OUT
    check_emergency_shutdown();
    if (get_battery_extra_to() != ES_TIMEOUT_NONE) {
        log_d("LowPowerBootUp: low battery, wait 30 second before next attempt of shutdown check");
        sleep(30);
        check_emergency_shutdown(); // call again to check if still meet shutdown criteria
        check_emergency_shutdown(); // call again to check if shutdown id need to be set
        if (emergency_shutdown_id) {
            log_d("LowPowerBootUp: meet shutdown criteria, sleep another 30sec incase 2nd time checked pass");
            sleep(30);
        }
    } else
#endif
    if (is_charging() && qc_prompt_checking()) {
        log_d("LowPowerBootUp: wait 10sec for QC charger prompt check");
        sleep(QC_PROMPT_CHECK_INTERVAL * 2);
    }

    memset(&init_data, 0, sizeof(init_data));

    init_data.sms_init.sms_ind_cb = smscb;
    init_data.sim_pin_cb = sim_pin_req_cb;
    init_data.nw_cb = nw_resp_cb;
    ril_init_state = ril_init(&init_data);
    ril_init_done = true;
    //load airplane mode
    init_airplane_mode();
    //set preference network default value
    init_pref_network();
    qcmap_client_init();
    //init wlan parameters
    init_wlan();
#ifdef HIGH_SPEED_SUPPORT
    init_high_speed(); // do this after wlan init because it might disable wlan
#endif

#ifndef CUST_DLINK
    reload_systemctl_daemon(); // For fota timer service
#endif

    if (need_to_show_fota_result()) {
        if(get_power_state() == SCREEN_OFF){
            screen_on_flag = true;
        }
        fota_result_notify = true;
    }

    static int nthSec = 0;
    while (1) {
        usleep(1000000);
        nthSec++;

        // 3 sec
        if (nthSec % 3 == 0) {
            check_blacklist_sim();
            data_usage_check();
#if !defined(NO_BATTERY)
            if (get_power_state() == SCREEN_OFF && battery_protect_shutdown()) {
                screen_on_flag = true;
            }
            //check if device need emergency shutdown
            check_emergency_shutdown();
#endif
#ifdef HIGH_SPEED_TEMP_MONITOR
            high_speed_temp_monitor();
#endif
            //check sim state to detect sim insert/remove
            check_sim_insert_remove();
            init_apn_monitor_task();
#ifdef CUST_LUXSHARE
            update_custom_led();
#endif
            if (is_task_debug_enabled()) lv_task_dump();
        }

        // one min
        if (nthSec % 60 == 0) {
            dump_data_usage();
#if !defined(NO_BATTERY)
            battery_log_charge_info();
#endif
            nthSec = 0; // reset counter
        }
    }

    return 0;
}

void monitor_ril_init(){
    if(ril_init_done){
        sim_pin_check();
        if (monitor_ril_init_task != NULL) {
            lv_task_del(monitor_ril_init_task);
            monitor_ril_init_task = NULL;
        }
        // start to allow key event after most of init done
        turning_screen_on = false;
    }
}

int init_status_check() {
    usleep(180000000); // 3 mins
    if ((ril_init_state != RIL_SUCCESS) || !wlan_initialized()) {
        log_e("init failed reboot, ril_init_state: %d, wlan_initialized: %d", ril_init_state, wlan_initialized());
        set_reboot_popup_enable(ID_SHUTDOWN_INIT_FAILED_REBOOT, 0);
        //systemCmd("timeout -t 10 logcat > /data/misc/log.txt &");
    }
    return 0;
}

int uevent_listener() {
    int status = uevent_init();
    if (!status) {
        log_e("Failed to initialize uevent handler.");
        return;
    }

    const char* usb_uevent_path = "change@/devices/virtual/android_usb/android0";
    const char* charger_uevent_path = "change@/devices/platform/838000.i2c/i2c-4/4-0044/838000.i2c:qcom,smb138x@44:qcom,smb1381-charger@1000/power_supply/usb";
    const char* battery_uevent_path = "change@/devices/platform/838000.i2c/i2c-4/4-0044/838000.i2c:qcom,smb138x@44:qcom,smb1381-charger@1000/power_supply/battery";
    char buffer[1024];
    bool is_charge_mode = charge_mode();

    // read value after boot up for case usb plugin at boot up
    if (!is_charge_mode) usb_state_g = read_usb_state();
    dump_usb_state();
#ifdef BATTERY_OPTIMIZE_SUPPORT
    update_charging_timer(is_charging());
#endif

    for (;;) {
        int length = uevent_next_event(buffer, sizeof(buffer) - 1);
        if (length <= 0) {
            return;
        }
        buffer[length] = '\0';

#if !defined(NO_BATTERY)
        //log_d("Received uevent message: %s", buffer);
        if (strcmp(buffer, battery_uevent_path) == 0) {
            dump_battery_level();
            if ((get_power_state() == SCREEN_OFF && battery_protect_shutdown()) && !is_charge_mode) {
                screen_on();
            }
            continue; /* event processed */
        }
#endif
        if (strcmp(buffer, usb_uevent_path) && strcmp(buffer, charger_uevent_path)) {
            continue; /* event not for us */
        }
        if (is_charge_mode) {
            dump_usb_state();
            if (!is_charging()) {
                log_d("device is not charging, power off");
                charging_only_power_off();
            }
        } else {
            if ((get_power_state() == SCREEN_OFF)) {
                screen_on_flag = true;
            }
            usb_state_g = read_usb_state();
            dump_usb_state();
            reset_emergency_temp_high();
#ifdef CUST_LUXSHARE
            update_custom_led();
#endif
        }
    }

    return 0;
}

int charge_mode_bg_thread() {
    // qmi and qcmap init
    ril_init(NULL);
    qcmap_client_init();

    if (DisableWLAN()) {
        log_d("DisableWLAN success");
    } else {
        log_d("DisableWLAN failed");
    }

    //airplane on by at command
    int ret = systemCmd("atcli at+cfun=4");
    log_d("airplane on cmd atcli at+cfun=4 status:%d", ret);
    pthread_exit(NULL);
    return 0;
}

int main(void) {
#ifndef COREDUMP_DEBUG
    signal(SIGTERM, sig_hdlr);
    signal(SIGINT, sig_hdlr);
    signal(SIGSEGV, sig_hdlr);
#endif

    // LittlevGL init must before turn_on_screen, if not, device will block
    // on boot, because of lv_task create failed on power_manager.c
    lv_init();

    turning_screen_on = true;

    if (is_silent_reboot()) {
        backlight_off();
    }

    update_led(false); // reset led status by turn it off
    start_adbd(); // start adbd in case if were terminated when pocketrouter died

    bool is_charge_mode = charge_mode();
    if (is_charge_mode) {
        ev_init(&lvgl_ev_callback_charge_mode);
        remove_ethernet_kmod();
    } else {
        //load settings
        init_default_settings();
        sms_create_xml(SMS_XML);

        init_data_usage_info();

        // if device will do config reboot later, do not show pin input
        wifi_reboot_flag = ds_get_bool(DS_KEY_WIFI_REBOOT_FLAG);

#ifdef CUST_LUXSHARE
        luxshare_init();
        if (!wifi_reboot_flag) {
            luxshare_usb_composition_init();
        }
#endif

        ev_init(&lvgl_ev_callback);
    }
    pthread_t thread;
    int rc = pthread_create(&thread, NULL, InputThreadLoop, NULL);
    if (rc) log_e("create thread fail\n");

#if USE_FBDEV || USE_ANDROID_FBDEV
    /*Linux frame buffer device init*/
    fbdev_init();
#endif

    /*Create a display buffer*/
    static lv_disp_buf_t disp_buf;
    static lv_color_t buf_1[LV_HOR_RES_MAX * LV_VER_RES_MAX];            /*A screen sized buffer*/
    static lv_color_t buf_2[LV_HOR_RES_MAX * LV_VER_RES_MAX];            /*An other screen sized buffer*/
    lv_disp_buf_init(&disp_buf, buf_1, buf_2, LV_HOR_RES_MAX * LV_VER_RES_MAX);   /*Initialize the display buffer*/

    /*Create a display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.buffer = &disp_buf;
#if USE_FBDEV || USE_ANDROID_FBDEV
    disp_drv.flush_cb = fbdev_flush;    /*Used when `LV_VDB_SIZE != 0` in lv_conf.h (buffered drawing)*/
#endif
    //    disp_drv.hor_res = LV_HOR_RES_MAX;
    //    disp_drv.ver_res = LV_VER_RES_MAX;
    lv_disp_drv_register(&disp_drv);

    init_display(get_screen_timeout(), get_brightness());
    turn_on_screen();

#if USE_FBDEV || USE_ANDROID_FBDEV
    /* initial touch device */
    evdev_init();    
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;  
    indev_drv.read_cb = evdev_read;
    lv_indev_drv_register(&indev_drv);
#endif

#if !defined(NO_BATTERY)
    dump_battery_level();
#endif
#ifdef BATTERY_OPTIMIZE_SUPPORT
    init_charging_fv();
#endif

    //enter charging only UI if device bootup in charging only mode
    if(is_charge_mode) {
        create_charge_mode();

        pthread_t bgThread, ueventThread;

        int res = pthread_create(&bgThread, NULL, &charge_mode_bg_thread, NULL);
        if (res) log_e("create bg thread fail\n");

        res = pthread_create(&ueventThread, NULL, &uevent_listener, NULL);
        if (res) log_e("create uevent thread fail\n");
    } else {
        statusbar_init();
        dashboard_create();

#ifdef BATTERY_OPTIMIZE_SUPPORT
        // popup for warning user about Battery Optimize feature enabled
        battery_optimize_warning();
#endif

        ril_init_done = false;
        monitor_ril_init_task = lv_task_create(monitor_ril_init, 500, LV_TASK_PRIO_LOW, NULL);

        //monitor state
#if !defined(ANDROID_BUILD)
        pthread_t bgThread, initMonitorThread, ueventThread;

        int res = pthread_create(&bgThread, NULL, &bg_thread, NULL);
        if (res) log_e("create bg thread fail\n");

        res = pthread_create(&initMonitorThread, NULL, &init_status_check, NULL);
        if (res) log_e("create init monitor thread fail\n");

        res = pthread_create(&ueventThread, NULL, &uevent_listener, NULL);
        if (res) log_e("create uevent thread fail\n");

#endif
        startRilDataMonitor();
        startMonitorWlan();

        pthread_t socketThread;
        res = pthread_create(&socketThread, NULL, &socket_server, NULL);
        if (res) log_e("create socket thread fail\n");
    }
#if defined (DEBUG_MEM)
    lv_task_create(memory_monitor, 3000, LV_TASK_PRIO_LOWEST, NULL);
#endif

    //reboot popup string id
    int reboot_strid = -1;
    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) { //if device cannot suspend, caused by this while loop?

        if (screen_on_flag && get_power_state() == SCREEN_OFF) {
            screen_on();
        }
        if (!is_charge_mode) {
            //Launch basic reboot popup
            reboot_strid = get_reboot_popup_enable();
            if(reboot_strid >= 0){
                if(get_power_state() == SCREEN_OFF){
                    screen_on();
                }
                reboot_popup_create();
                reboot_popup_extra_setting(reboot_strid);
            }
            //Launch power menu popup
            if(power_menu_enable){
                lunch_power_menu();
                power_menu_enable = false;
            }
            //Launch fota result popup
            if(fota_result_notify && !is_static_popup()){
                fota_result_notify = false;
                fota_result_prompt();
            }
            //Launch emergency shutdown popup
            if(emergency_shutdown_id){
                emergency_shutdown_popup(emergency_shutdown_id);
                emergency_shutdown_id = 0;
            }
            //Launch Battery protect prompt
            if(get_battery_protect_flag()){
                if(get_power_state() == SCREEN_OFF) {
                    screen_on();
                }
                battery_protect_prompt();
            }
#ifdef BATTERY_OPTIMIZE_SUPPORT
            //Launch battery optimize remind popup
            if(get_bat_opt_notify_flag()){
                battery_optimize_prompt();
            }
#endif
#ifdef HIGH_SPEED_SUPPORT
            //Launch high speed remind popup
            if(get_high_speed_notify_flag()){
                high_speed_prompt();
#ifdef HIGH_SPEED_TEMP_MONITOR
                if(get_power_state() == SCREEN_OFF) {
                    screen_on();
                }
                mmWave_disable_prompt();
                show_high_speed_shutdown();
#endif
            }
#endif
            if (show_quick_charge()) {
                quick_charge_prompt();
            }

            if (usb_state_g == USB_STATE_CONFIGURED)
                usb_compositions_create();
            if (usb_state_g == USB_STATE_NOT_ATTACHED)
                usb_compositions_close();
            usb_state_g = USB_STATE_UNKNOWN;
        }

        //Stop execute lv_task when screen off avoid task keep running.
        //But sometimes will bloked at get_power_state() on SMx3 when
        //screen turned on. Need to check ZX5x device, Keep monitor.
#if !USE_FBDEV && !USE_ANDROID_FBDEV
        lv_task_handler();
#else
        if (get_power_state() == SCREEN_ON) {
            lv_task_handler();
        } else {
            key_period = -1; //stop read key events
            if (wifi_close_wait() || charging_pw_task != NULL)
                lv_task_handler();
        }
#endif

        // Sometime will blocked on this line,
        // SMx3 backtrace: bionic/libc/upstream-freebsd/lib/libc/gen/usleep.c:48
        // But ZX5x have no this file, the architecture looks not the same.
        // so might no problem, keep monitor.
        // Test steps: continuous quick to turn off / on screen.
        usleep(5000);
    }

    ev_exit();

    return 0;
}

#else  /* FEATURE_ROUTER */

/*********************
 *      DEFINES
 *********************/

/*On OSX SDL needs different handling*/
#if defined(__APPLE__) && defined(TARGET_OS_MAC)
# if __APPLE__ && TARGET_OS_MAC
#define SDL_APPLE
# endif
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void hal_init(void);
static int tick_thread(void * data);
static void memory_monitor(lv_task_t * param);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int main(int argc, char ** argv)
{
    (void) argc;    /*Unused*/
    (void) argv;    /*Unused*/

    /*Initialize LittlevGL*/
    lv_init();

    /*Initialize the HAL (display, input devices, tick) for LittlevGL*/
    hal_init();

    init_default_settings();
    sms_create_xml(SMS_XML);

    init_data_usage_info();

    init_wlan_data();
    init_wlan();

    // initial style
    init_pocket_router_style();

    statusbar_init();
    //launcher_create();
    dashboard_create();
    init_apn_monitor_task();
    check_blacklist_sim();
    /*Create a demo*/
    //demo_create();

    /*Try the benchmark to see how fast your GUI is*/
    //    benchmark_create();

    /*Check the themes too*/
    //    lv_test_theme_1(lv_theme_night_init(15, NULL));

    //    lv_test_theme_2();
    /*Try the touchpad-less navigation (use the Tab and Arrow keys or the Mousewheel)*/
    //    lv_test_group_1();


    while(1) {
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
        lv_task_handler();
        usleep(5 * 1000);

#ifdef SDL_APPLE
        SDL_Event event;

        while(SDL_PollEvent(&event)) {
#if USE_MOUSE != 0
            mouse_handler(&event);
#endif

#if USE_KEYBOARD
            keyboard_handler(&event);
#endif

#if USE_MOUSEWHEEL != 0
            mousewheel_handler(&event);
#endif
        }
#endif
    }

    return 0;
}

lv_res_t ICON_LVGL_runLength_info(lv_img_decoder_t * decoder, const void * src, lv_img_header_t * header)
{
    (void)decoder; /*Unused*/

    lv_img_src_t src_type = lv_img_src_get_type(src);
    if(src_type == LV_IMG_SRC_VARIABLE) {
        lv_img_cf_t cf = ((lv_img_dsc_t *)src)->header.cf;
        if(cf != LV_IMG_CF_USER_ENCODED_0) return LV_RES_INV;

        header->w  = ((lv_img_dsc_t *)src)->header.w;
        header->h  = ((lv_img_dsc_t *)src)->header.h;
        header->cf = ((lv_img_dsc_t *)src)->header.cf;
        return LV_RES_OK;
    } else {
        return LV_RES_INV;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the Littlev graphics library
 */
static void hal_init(void)
{
    /* Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
    monitor_init();

    /*Create a display buffer*/
    static lv_disp_buf_t disp_buf1;
    static lv_color_t buf1_1[480*10];
    lv_disp_buf_init(&disp_buf1, buf1_1, NULL, 480*10);

    /*Create a display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
    disp_drv.buffer = &disp_buf1;
    disp_drv.flush_cb = monitor_flush;    /*Used when `LV_VDB_SIZE != 0` in lv_conf.h (buffered drawing)*/
    //    disp_drv.hor_res = 200;
    //    disp_drv.ver_res = 100;
    lv_disp_drv_register(&disp_drv);

    /* Add the mouse as input device
     * Use the 'mouse' driver which reads the PC's mouse*/
    mouse_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = mouse_read;         /*This function will be called periodically (by the library) to get the mouse position and state*/
    lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv);

    /*Set a cursor for the mouse*/
    LV_IMG_DECLARE(mouse_cursor_icon);                          /*Declare the image file.*/
    lv_obj_t * cursor_obj =  lv_img_create(lv_disp_get_scr_act(NULL), NULL); /*Create an image object for the cursor */
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);             /*Set the image source*/
    lv_indev_set_cursor(mouse_indev, cursor_obj);               /*Connect the image  object to the driver*/

    /* Tick init.
     * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about how much time were elapsed
     * Create an SDL thread to do this*/
    SDL_CreateThread(tick_thread, "tick", NULL);

    /* Optional:
     * Create a memory monitor task which prints the memory usage in periodically.*/
    lv_task_create(memory_monitor, 3000, LV_TASK_PRIO_MID, NULL);
}

/**
 * A task to measure the elapsed time for LittlevGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void * data)
{
    (void)data;

    while(1) {
        SDL_Delay(5);   /*Sleep for 5 millisecond*/
        lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
}

/**
 * Print the memory usage periodically
 * @param param
 */
static void memory_monitor(lv_task_t * param)
{
    (void) param; /*Unused*/

    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    printf("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d\n", (int)mon.total_size - mon.free_size,
            mon.used_pct,
            mon.frag_pct,
            (int)mon.free_biggest_size);

}
#endif  /* FEATURE_ROUTER */
