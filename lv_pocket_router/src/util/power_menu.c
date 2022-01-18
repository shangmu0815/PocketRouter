#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/high_speed.h"
#include "lv_pocket_router/src/util/power_menu.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/battery/battery_info.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/display/display.h"

#define ES_TIME_OUT         30000
#define ES_CHECK_INTERVAL   500
#define REBOOT_POPUP_TO     5000
#define DELAY_POWER_OFF     1000

static lv_obj_t * mbox = NULL;
static lv_obj_t * popup_box = NULL;

//for gray background
static lv_style_t style_gray_bg;
static lv_style_t style_bg;
static lv_style_t style_ta;

//for power off popup
static lv_style_t style_pw_pr;
static lv_style_t style_pw_rel;

#define LOG_FILE           DEFAULT_DATA_STORE_PATH "/reboot_reason.txt"
#define LOG_BACKUP_FILE    DEFAULT_DATA_STORE_PATH "/reboot_reason_bak.txt"

//for emergency shutdown
static uint32_t es_timestamp;
int anim_cnt;
lv_task_t * es_task;
lv_task_t * es_anim_task;
lv_obj_t *  es_bg = NULL;
lv_obj_t *  es_img = NULL;

//for basic reboot popup
static int reboot_popup_strid = -1;
static int reboot_popup_timeout = -1;

void close_es_popup();
static const void * anim_notice_map[]={&ic_pop_notice_1, &ic_pop_notice_2, &ic_pop_notice_3
        , &ic_pop_notice_4, &ic_pop_notice_5, &ic_pop_notice_6, &ic_pop_notice_7
        , &ic_pop_notice_8, &ic_pop_notice_9, &ic_pop_notice_10};

void power_menu_style(){
    //mbox bg style
    lv_style_copy(&style_bg, &lv_style_plain);
    style_bg.body.main_color = LV_COLOR_WHITE;
    style_bg.body.grad_color = LV_COLOR_WHITE;
    style_bg.text.font = get_font(font_w_bold, font_h_18);
    style_bg.body.radius = 2;

    lv_style_copy(&style_gray_bg, &lv_style_plain);
    style_gray_bg.body.main_color = LV_COLOR_GREYISH_BROWN;//#greyish-brown
    style_gray_bg.body.grad_color = LV_COLOR_GREYISH_BROWN;
    style_gray_bg.body.opa = LV_OPA_60;

    //set power off style
    lv_style_copy(&style_pw_rel, &lv_style_btn_rel);
    style_pw_rel.body.main_color = LV_COLOR_WHITE;
    style_pw_rel.body.grad_color = LV_COLOR_WHITE;
    style_pw_rel.body.border.color = LV_COLOR_SILVER;
    style_pw_rel.body.border.width = 1;
    style_pw_rel.body.border.opa = LV_OPA_COVER;
    style_pw_rel.body.border.part = LV_BORDER_TOP;
    style_pw_rel.body.radius = 2;
    style_pw_rel.body.padding.left = 10;
    style_pw_rel.body.padding.right = 10;
    style_pw_rel.body.padding.top = 0;
    style_pw_rel.body.padding.bottom = 0;
    style_pw_rel.body.padding.inner = 15;
    style_pw_rel.text.color = LV_COLOR_MATTERHORN;
    style_pw_rel.text.font = get_font(font_w_bold, font_h_18);

    lv_style_copy(&style_pw_pr, &style_pw_rel);
    style_pw_pr.body.main_color = LV_COLOR_WHITE;
    style_pw_pr.body.grad_color = LV_COLOR_WHITE;
    style_pw_pr.body.border.color = LV_COLOR_BASE;
    style_pw_pr.body.border.width = 3;
    style_pw_pr.text.color = LV_COLOR_BASE;
    style_pw_pr.image.color = LV_COLOR_BASE;
    style_pw_pr.image.intense = LV_OPA_COVER;

    lv_style_copy(&style_ta, &lv_style_plain);
    style_ta.body.main_color = LV_COLOR_WHITE;
    style_ta.body.grad_color = LV_COLOR_WHITE;
    style_ta.text.font = get_font(font_w_bold, font_h_16);
    style_ta.text.color = LV_COLOR_GREYISH_BROWN;
    style_ta.text.letter_space = 1;
}

void check_logfile_size(){
    struct stat buffer;
    int exist = stat(LOG_FILE, &buffer);
    log_d("reboot reason log file size %d", buffer.st_size);

    if (buffer.st_size > 32000) {
        char cmd[100];
        sprintf(cmd, "mv %s %s", LOG_FILE, LOG_BACKUP_FILE);
        systemCmd(cmd);
        log_d("mv reboot reason log to backup file");
        exist = stat(LOG_FILE, &buffer);
    }
}

void get_reboot_reason(char* info, int reason, char* t){
    switch(reason)
    {
        case SCREEN_ON_SIM_INSERT:
            sprintf(info, "%s:%s", t, "[Sim change] screen on sim insert");
            break;
        case SCREEN_ON_SIM_REMOVE:
            sprintf(info, "%s:%s", t, "[Sim change] screen on sim remove");
            break;
        case SCREEN_OFF_SIM_INSERT:
            sprintf(info, "%s:%s", t, "[Sim change] screen off sim insert");
            break;
        case SCREEN_OFF_SIM_REMOVE:
            sprintf(info, "%s:%s", t, "[Sim change] screen off sim remove");
            break;
        case CONFIG_CHANGE_REBOOT:
            sprintf(info, "%s:%s", t, "[Reboot] configuration change");
            break;
        case POWER_MENU_REBOOT:
            sprintf(info, "%s:%s", t, "[Reboot] power menu reboot");
            break;
        case POWER_MENU_POWER_OFF:
            sprintf(info, "%s:%s", t, "[PowerOff] power menu power off");
            break;
        case BATTERY_LOW_POWER_OFF:
            sprintf(info, "%s:%s", t, "[PowerOff] battery low emergency shutdown");
            break;
        case BATTERY_HIGH_TEMP_POWER_OFF:
            sprintf(info, "%s:%s", t, "[PowerOff] battery high temperature emergency shutdown");
            break;
        case BATTERY_LOW_TEMP_POWER_OFF:
            sprintf(info, "%s:%s", t, "[PowerOff] battery low temperature emergency shutdown");
            break;
        case CHARGING_ONLY_POWER_OFF:
            sprintf(info, "%s:%s", t, "[PowerOff] charging only mode power off");
            break;
        case CHARGING_ONLY_REBOOT:
            sprintf(info, "%s:%s", t, "[Reboot] charging only mode reboot");
            break;
        default:
            break;
    }
}

void get_reboot_state(char* info, int state, int prev_state, bool screen_on, char* t){
    if(screen_on){
        sprintf(info, "%s:[SimState] screen on prev state: %d, sim state: %d ", t, prev_state, state);
    }else{
        sprintf(info, "%s:[SimState] screen off prev state: %d, sim state: %d ", t, prev_state, state);
    }
}

void save_reboot_reason_impl(int str_id, int reason, int state, int prev_state, bool screen_on){

    char info[256];
    memset(info, 0, sizeof(info));
    check_logfile_size();

    FILE *f = fopen(LOG_FILE, "a");
    if (f == NULL){
        log_e("save_reboot_reason fopen error!");
        return;
    }

    time_t now;
    (void)time(&now);
    char t[30];
    strftime(t, 30, "%Y-%m-%d %T ", localtime(&now));

    if(reason >= 0){
        get_reboot_reason(info, reason, t);
    } else if(state >= 0){
        get_reboot_state(info, state, prev_state, screen_on, t);
    } else if(str_id >= 0){
        sprintf(info, "%s:%s%s", t, "[Reboot] ", get_string_locale(str_id, EN));
    }

    fprintf(f, "%s\n", info);
    fclose(f);
}
void save_reboot_reason_strid(int strid){
    save_reboot_reason_impl(strid, -1, -1, -1, false);
}
void save_reboot_reason(int reason){
    save_reboot_reason_impl(-1, reason, -1, -1, false);
}
void save_reboot_state(int state, int prev_state, bool screen_on){
    save_reboot_reason_impl(-1, -1, state, prev_state, screen_on);
}

void delay_poweroff(){
    stop_sleep();
    backlight_off();
    int res = systemCmd("poweroff");
    log_d("power off result: %d", res);
}

void delay_restart(){
    stop_sleep();
    backlight_off();
    int res = systemCmd("reboot");
    log_d("restart result: %d", res);
}

void charging_only_power_off(){
    set_reboot_screen_on(true);
    update_led(false); // turn off led
    backlight_off();
    save_reboot_reason(CHARGING_ONLY_POWER_OFF);
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    delay_poweroff();
#endif
}

void charging_only_restart() {
    set_reboot_screen_on(true);
    log_d("charging only reboot");
    update_led(false); // turn off led
    backlight_off();
    save_reboot_reason(CHARGING_ONLY_REBOOT);
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    delay_restart();
#endif
}

void power_off_impl(){
    set_reboot_screen_on(true);
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    popup_anim_not_create(get_string(ID_PWR_MENU_POWER_OFF_MSG), NULL, NULL, NULL);
    lv_task_t * task = lv_task_create(delay_poweroff, DELAY_POWER_OFF, LV_TASK_PRIO_HIGH, NULL);
    lv_task_once(task);
#endif
}

void reboot_impl(){
    set_reboot_screen_on(true);
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    popup_anim_not_create(get_string(ID_PWR_MENU_RESTART_MSG), NULL, NULL, NULL);
    lv_task_t * task = lv_task_create(delay_restart, DELAY_POWER_OFF, LV_TASK_PRIO_HIGH, NULL);
    lv_task_once(task);
#endif
}

void emergency_shutdown_impl(){
    uint32_t t = lv_tick_elaps(es_timestamp);
    if(t < ES_TIME_OUT){
        if(es_bg != NULL){
            lv_obj_move_foreground(es_bg);
        }
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
        int temp = get_battery_temperature();
        int level = get_battery_info();
        if((!battery_ever_removed() && level == 0 && !is_charging()) ||
                (get_battery_present() && temp <= EMERGENCY_TEMP_LOW) ||
                (get_battery_present() && temp >= get_emergency_temp_high())
#ifdef HIGH_SPEED_TEMP_MONITOR
                || high_speed_shutdown()
#endif
                ){
            //check fail
        } else{
            //check pass
            log_d("2nd time checked pass, close emergency shutdown popup");
            close_es_popup();
            //disable flag because we are no longer in power off process
            set_reboot_screen_on(false);
        }
#endif
    }else{
        log_d("2nd time checked fail, will power off now");
        close_es_popup();
        power_off_impl();
    }
}
void es_anim_notice() {
    int len = sizeof(anim_notice_map) / sizeof(void *);
    lv_obj_set_hidden(es_img, 0);
    if (anim_cnt < len) {
        lv_img_set_src(es_img, anim_notice_map[anim_cnt]);
        anim_cnt++;
    } else {
        //repeat anim
        anim_cnt = 0;
    }
}

//create popup for emergency shutdown
lv_obj_t * es_popup_create(const void * text) {
    log_d("Create emergency shutdown popup");
    power_menu_style();

    //draw dark gray background
    lv_obj_t * bg = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(bg, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style(bg, &style_gray_bg);

    //create a container for popup box
    lv_obj_t * box = lv_cont_create(bg, NULL);
    lv_obj_set_size(box, 280, 200);
    lv_obj_set_style(box, &style_bg);
    lv_obj_align(box, NULL, LV_ALIGN_CENTER, 0, 0);

    //add image res on the top of the popup
    es_img = lv_img_create(box, NULL);
    lv_obj_set_size(es_img, 65, 65);
    lv_img_set_src(es_img, anim_notice_map[0]);
    lv_obj_align(es_img, NULL, LV_ALIGN_IN_TOP_MID, 0, 22);
    lv_obj_set_hidden(es_img, 0);

    //draw info main text area
    lv_obj_t * ta = lv_ta_create(box, NULL);
    lv_ta_set_text(ta, text);
    lv_ta_set_text_align(ta, LV_LABEL_ALIGN_CENTER);
    lv_ta_set_style(ta, LV_TA_STYLE_BG, &style_ta);
    lv_ta_set_cursor_type(ta, LV_CURSOR_HIDDEN);
    lv_ta_set_cursor_pos(ta, 0);
    lv_ta_set_sb_mode(ta, LV_SB_MODE_OFF);
    lv_obj_set_size(ta, 240, 50);
    lv_obj_align(ta, NULL, LV_ALIGN_CENTER, 0, 20);

    //adjust UI because this popup did not include btnm
    lv_obj_align(es_img, box, LV_ALIGN_CENTER, 0, -20);
    lv_obj_align(ta, es_img, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    //start animation task
    anim_cnt = 0;
    es_anim_task = lv_task_create(es_anim_notice, 100, LV_TASK_PRIO_MID, NULL);

    return box;
}

void close_es_popup(){
    if(es_bg != NULL){
        log_d("Close emergency shutdown popup");
        if (es_task != NULL) {
            lv_task_del(es_task);
            es_task = NULL;
        }
        if (es_anim_task != NULL) {
            lv_task_del(es_anim_task);
            es_anim_task = NULL;
        }
        lv_obj_del(es_bg);
        es_bg = NULL;
    }
}

bool emergency_shutdown_popup_exist(){
    return es_bg? true:false;
}

/* device will do emergency shutdown if meet below condition
 * temperature > 60 or < -18 or battery level is 0
 */
void emergency_shutdown_popup(int res_id){
    if(es_bg){
        log_d("Skip emergency_shutdown_popup create");
        return;
    }
    set_reboot_screen_on(true);
    if (res_id == ID_SHUTDOWN_BATT_LOW) {
        save_reboot_reason(BATTERY_LOW_POWER_OFF);
    } else if (res_id == ID_SHUTDOWN_BATT_LOW_TEMP){
        save_reboot_reason(BATTERY_LOW_TEMP_POWER_OFF);
    } else if (res_id == ID_SHUTDOWN_BATT_HIGH_TEMP){
        save_reboot_reason(BATTERY_HIGH_TEMP_POWER_OFF);
    }
    es_timestamp = lv_tick_get();
    popup_box = es_popup_create(get_string(res_id));
    es_bg = lv_obj_get_parent(popup_box);
    es_task = lv_task_create(emergency_shutdown_impl, ES_CHECK_INTERVAL, LV_TASK_PRIO_LOW, NULL);
}

void sim_change_impl(int reason){
    save_reboot_reason(reason);
    if(reason == SCREEN_OFF_SIM_REMOVE || reason == SCREEN_ON_SIM_REMOVE){
        delete_sim_sms();
    } else if(reason == SCREEN_OFF_SIM_INSERT || reason == SCREEN_ON_SIM_INSERT){
        //in case of we already called rilSmsRefershSIMSMS, make sure only one copy of sim sms exist in xml
        delete_sim_sms();
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
        rilSmsRefershSIMSMS();
#endif
    }
}

/* for basic reboot popup, the popup will be called from main.c
 * strid: the string will show on reboot popup and saved in reboot reason
 * timeout: reboot popup will display "timeout" ms then reboot
 */
void set_reboot_popup_enable(int strid, int timeout){
    log_d("set_reboot_popup_enable strid: %d, timeout: %d", strid, timeout);
    reboot_popup_strid = strid;
    reboot_popup_timeout = timeout;
}

int get_reboot_popup_enable(){
    return reboot_popup_strid;
}

void reboot_popup_create(){
    if(popup_box){
        log_d("Skip reboot_popup_create");
        return;
    }
    set_reboot_screen_on(true);
    int timeout = (reboot_popup_timeout > 0) ? reboot_popup_timeout : REBOOT_POPUP_TO;
    save_reboot_reason_strid(reboot_popup_strid);
    popup_box = popup_anim_not_plain_create(get_string(reboot_popup_strid), REBOOT_POPUP_TO);
    lv_task_t * task = lv_task_create(reboot_impl, timeout, LV_TASK_PRIO_LOW, NULL);
    lv_task_once(task);
    reboot_popup_strid = -1;
    reboot_popup_timeout = -1;
}

void power_off() {
    //do power off
    log_d("power menu power off clicked");
    save_reboot_reason(POWER_MENU_POWER_OFF);
    power_off_impl();
}

//for power off action
void power_off_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    int id = lv_obj_get_user_data(btn);
    if (id == INDEX_POWER_OFF) {
        power_off();
    } else if (id == INDEX_RESTART)  {
        //do restart
        save_reboot_reason(POWER_MENU_REBOOT);
        log_d("power menu restart clicked");
        reboot_impl();
    } else if (id == INDEX_CANCEL)  {
        //do cancel
        log_d("power menu cancel clicked");
        close_power_menu();
    }
}

//check if power menu already exist, if yes, close it
void close_power_menu(){
    if(mbox != NULL){
        log_d("close power menu");
        lv_obj_t * bg = lv_obj_get_parent(mbox);
        lv_obj_del(bg);
        mbox = NULL;
    }
}

lv_obj_t * power_menu_liste(const void * img_src, const char * txt_src, int id){

    lv_obj_t * liste = lv_btn_create(mbox, NULL);
    lv_obj_set_size(liste, 280, 44);
    lv_btn_set_style(liste, LV_BTN_STYLE_REL, &style_pw_rel);
    lv_btn_set_style(liste, LV_BTN_STYLE_PR, &style_pw_pr);
    lv_btn_set_layout(liste, LV_LAYOUT_OFF);

    lv_obj_t * img = lv_img_create(liste, NULL);
    lv_img_set_src(img, img_src);
    lv_obj_set_size(img, 32, 32);

    lv_obj_t * txt = lv_label_create(liste, NULL);
    lv_label_set_align(txt, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text(txt, txt_src);
    lv_obj_set_size(txt, 80, 44);

    if(is_ltr()){
        lv_obj_align(img, liste, LV_ALIGN_IN_LEFT_MID, 10, 0);
        lv_obj_align(txt, img, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    }else{
        lv_obj_align(img, liste, LV_ALIGN_IN_RIGHT_MID, -10, 0);
        lv_obj_align(txt, img, LV_ALIGN_OUT_LEFT_MID, -10, 0);
    }
    lv_obj_set_event_cb(liste, power_off_action);
    lv_obj_set_user_data(liste, id);

    return liste;
}

//create message box for power off popup
void power_menu_popup_create(void) {
    if(mbox){
        log_d("SKIP power menu popup create");
        return;
    }
    log_d("Create power menu popup");

    //don't show power off if Dlink & charging
    bool show_power_off = true;
/*
#ifdef CUST_DLINK
    if(is_charging())
        show_power_off = false;
#endif
*/
    power_menu_style();

    //draw dark gray background
    lv_obj_t * bg = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(bg, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style(bg, &style_gray_bg);

    //create container for popup box
    mbox = lv_cont_create(bg, NULL);
    lv_obj_set_size(mbox, 280, show_power_off ? 134 : 90);
    lv_obj_set_style(mbox, &style_bg);
    lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * restart = power_menu_liste(&ic_pop_restart, get_string(ID_PWR_MENU_RESTART), INDEX_RESTART);
    lv_obj_t * cancel = power_menu_liste(&ic_pop_cancel, get_string(ID_CANCEL), INDEX_CANCEL);
    if(show_power_off){
        lv_obj_t * power_off = power_menu_liste(&ic_pop_power, get_string(ID_PWR_MENU_POWER_OFF), INDEX_POWER_OFF);
        lv_obj_align(power_off, mbox, LV_ALIGN_IN_TOP_MID, 0, 0);
        lv_obj_align(restart, power_off, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
        lv_obj_align(cancel, restart, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    }else{
        lv_obj_align(restart, mbox, LV_ALIGN_IN_TOP_MID, 0, 0);
        lv_obj_align(cancel, restart, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    }
}
