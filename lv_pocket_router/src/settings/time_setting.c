#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "lv_pocket_router/src/keyboard/num_kb_box.h"
#include "lv_pocket_router/src/keyboard/en_kb.h"
#include "lv_pocket_router/src/keyboard/basic_kb.h"
#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/res/values/styles.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/settings/time_setting.h"

lv_obj_t * liste_time_year;
lv_obj_t * time_year_label;
lv_obj_t * liste_time_month_date;
lv_obj_t * time_month_date_label;
lv_obj_t * liste_time_hour_minute;
lv_obj_t * time_hour_minute_label;
lv_obj_t * liste_time_am_pm;
lv_obj_t * time_am_pm_label;
lv_obj_t * year_root_view;
lv_obj_t * month_date_root_view;
lv_obj_t * hour_minute_root_view;

#define AM_PM_MAX_LISTE 2
lv_obj_t * am_pm_liste_img[AM_PM_MAX_LISTE];
lv_obj_t * am_pm_liste[AM_PM_MAX_LISTE];
int am_pm_type;
int am_pm_map[2] = { ID_TIME_SETTING_AM, ID_TIME_SETTING_PM };
int setting_type = UNKNOWN_TYPE;
char current_year[5];
char current_month[3];
char current_date[3];
char current_hour[3];
char current_hour_24hformat[3];
char current_minute[3];
char current_sec[3];
char current_am_pm[3];
char current_year_last2_digits[3];
char cmd[50];
char month_date_str[6];
char hour_minute_str[6];
time_t t;
lv_task_t * refresh_task;
struct time new_time;

enum AM_PM_IDS {
    ID_AM,
    ID_PM
};

char* get_month_date() {
    memset(month_date_str, 0, sizeof(month_date_str));
    sprintf(month_date_str, "%s/%s", get_current_month(), get_current_date());
    return month_date_str;
}

char* get_hour_minute() {
    memset(hour_minute_str, 0, sizeof(hour_minute_str));
    sprintf(hour_minute_str, "%s:%s", get_current_hour(), get_current_minute());
    return hour_minute_str;
}

void year_kb_action(lv_obj_t * btnm, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;
    uint32_t index = lv_obj_get_user_data(btnm);
    if (index == KB_LEFT_BTN) {
        num_kb_time_box_close();
    } else if (index == KB_RIGHT_BTN) {
        if(get_tick_btn_ina_state(get_kb_tick_btn(year_root_view))) {
            return;
        }
        //set new datetime
        char year[5];//year
        memset(year, '\0', sizeof(year));
        sprintf(year, "%s%s%s%s", get_num_kb_box(INDEX_COLIMN_1),
                get_num_kb_box(INDEX_COLIMN_2), get_num_kb_box(INDEX_COLIMN_3),
                get_num_kb_box(INDEX_COLIMN_4));
        if (check_date_valid(atoi(year), atoi(get_current_month()), atoi(get_current_date())))
        {
            memset(&new_time, 0, sizeof(new_time));
            new_time.year = atoi(year);
            new_time.mon = atoi(get_current_month());
            new_time.day = atoi(get_current_date());
            new_time.hour = atoi(get_current_hour_24hformat());
            new_time.min = atoi(get_current_minute());
            new_time.sec = atoi(get_current_sec());
            set_time(&new_time);
            num_kb_time_box_close();
        } else {
            time_setting_err_notice(YEAR_TYPE);
        }
    }
}

void month_date_kb_action(lv_obj_t * btnm, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;
    uint32_t index = lv_obj_get_user_data(btnm);
    if (index == KB_LEFT_BTN) {
        num_kb_time_box_close();
    } else if (index == KB_RIGHT_BTN) {
        if(get_tick_btn_ina_state(get_kb_tick_btn(month_date_root_view))) {
            return;
        }
        //set new datetime
        char month[3];
        memset(month, '\0', sizeof(month));
        snprintf(month, sizeof(month), "%s%s", get_num_kb_box(INDEX_COLIMN_1),get_num_kb_box(INDEX_COLIMN_2));
        char day[3];
        memset(day, '\0', sizeof(day));
        snprintf(day, sizeof(day), "%s%s", get_num_kb_box(INDEX_COLIMN_3),get_num_kb_box(INDEX_COLIMN_4));
        if (check_date_valid(atoi(get_current_year()), atoi(month), atoi(day)))
        {
            memset(&new_time, 0, sizeof(new_time));
            new_time.year = atoi(get_current_year());
            new_time.mon = atoi(month);
            new_time.day = atoi(day);
            new_time.hour = atoi(get_current_hour_24hformat());
            new_time.min = atoi(get_current_minute());
            new_time.sec = atoi(get_current_sec());
            set_time(&new_time);
            num_kb_time_box_close();
        } else {
            time_setting_err_notice(MONTH_DATE_TYPE);
        }
    }
}

void hour_minute_kb_action(lv_obj_t * btnm, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;
    uint32_t index = lv_obj_get_user_data(btnm);
    if (index == KB_LEFT_BTN) {
        num_kb_time_box_close();
    } else if (index == KB_RIGHT_BTN) {
        if(get_tick_btn_ina_state(get_kb_tick_btn(hour_minute_root_view))) {
            return;
        }
        //set new datetime
        char hr[3];
        memset(hr, '\0', sizeof(hr));
        sprintf(hr, "%s%s", get_num_kb_box(INDEX_COLIMN_1),get_num_kb_box(INDEX_COLIMN_2));
        char min[3];
        memset(min, '\0', sizeof(min));
        snprintf(min, sizeof(min), "%s%s", get_num_kb_box(INDEX_COLIMN_3),get_num_kb_box(INDEX_COLIMN_4));
        memset(&new_time, 0, sizeof(new_time));
        new_time.year = atoi(get_current_year());
        new_time.mon = atoi(get_current_month());
        new_time.day = atoi(get_current_date());
        new_time.hour = hour_convert_check(atoi(hr),((strcmp(get_current_am_pm(), "AM") == 0) ? ID_AM : ID_PM));
        new_time.min = atoi(min);
        new_time.sec = atoi(get_current_sec());
        set_time(&new_time);
        num_kb_time_box_close();
    }
}

void year_create() {
    year_root_view = num_kb_time_box_create(KB_YEAR, get_string(ID_TIME_SETTING_YEAR), year_kb_action);
}

void month_date_create() {
    month_date_root_view = num_kb_time_box_create(KB_MONTH_DATE, get_string(ID_TIME_SETTING_MONTH_DATE), month_date_kb_action);
}

void hour_minute_create() {
    hour_minute_root_view = num_kb_time_box_create(KB_HOUR_MINUTE, get_string(ID_TIME_SETTING_HOUR_MINUTE), hour_minute_kb_action);
}

void am_pm_btn_action(lv_obj_t * btn, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;
    int i;
    am_pm_type = lv_obj_get_user_data(btn);
    for (i = 0; i < AM_PM_MAX_LISTE; i++){
        lv_img_set_src(am_pm_liste_img[i], &btn_list_radio_n);
    }
    lv_img_set_src(am_pm_liste_img[am_pm_type], &btn_list_radio_p);
    log_d("am_pm_btn_action am_pm_type:%d", am_pm_type);
}

void am_pm_confirm_action(lv_obj_t * btn, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;
    memset(&new_time, 0, sizeof(new_time));
    new_time.year = atoi(get_current_year());
    new_time.mon = atoi(get_current_month());
    new_time.day = atoi(get_current_date());
    new_time.hour = hour_convert_check(atoi(get_current_hour()), am_pm_type);
    new_time.min = atoi(get_current_minute());
    new_time.sec = atoi(get_current_sec());
    set_time(&new_time);
}

void am_pm_create() {
    liste_style_create();
    lv_obj_t * win = modify_list_header(lv_scr_act(), get_string(ID_TIME_SETTING_AM_PM),
            am_pm_confirm_action, lv_win_close_event_cb);

    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);
    //add list element in order
    am_pm_type = (strcmp(get_current_am_pm(), "AM") == 0) ? ID_AM : ID_PM;
    log_d("am_pm_create am_pm_type:%d",am_pm_type);
    int i;
    for (i = 0; i < AM_PM_MAX_LISTE; i++) {
        am_pm_liste[i] = lv_liste_w_cbox(list, get_string(am_pm_map[i]), am_pm_type == i,am_pm_btn_action, i);
        am_pm_liste_img[i] = lv_obj_get_child(am_pm_liste[i], NULL);
    }
}

void year_action(lv_obj_t * btn, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;
    year_create();
}

void month_date_action(lv_obj_t * btn, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;
    month_date_create();
}

void hour_minute_action(lv_obj_t * btn, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;
    hour_minute_create();
}

void am_pm_action(lv_obj_t * btn, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;
    am_pm_create();
}

void update_year_label() {
    time_year_label = lv_obj_get_child(liste_time_year, NULL);
    lv_label_set_text(time_year_label, get_current_year());
    lv_liste_w_arrow_align(time_year_label);
}

void update_month_date_label() {
    time_month_date_label = lv_obj_get_child(liste_time_month_date, NULL);
    lv_label_set_text(time_month_date_label, get_month_date());
    lv_liste_w_arrow_align(time_month_date_label);
}

void update_hour_minute_label() {
    time_hour_minute_label = lv_obj_get_child(liste_time_hour_minute, NULL);
    lv_label_set_text(time_hour_minute_label, get_hour_minute());
    lv_liste_w_arrow_align(time_hour_minute_label);
}

void update_am_pm_label() {
    time_am_pm_label = lv_obj_get_child(liste_time_am_pm, NULL);
    lv_label_set_text(time_am_pm_label, get_current_am_pm());
    lv_liste_w_arrow_align(time_am_pm_label);
}

void time_setting_label_refresh() {
    update_year_label();
    update_month_date_label();
    update_hour_minute_label();
    update_am_pm_label();
}

//start task for regular time setting update
void time_setting_label_refresh_task() {
    if (refresh_task == NULL) {
        refresh_task = lv_task_create(time_setting_label_refresh, 1000, LV_TASK_PRIO_LOW, NULL);
    }
}

void time_setting_close_cb(lv_obj_t * btn, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;
    log_d("time_setting_close_cb");
    if (refresh_task != NULL) {
        lv_task_set_period(refresh_task, LV_TASK_PRIO_OFF);
        lv_task_del(refresh_task);
        refresh_task = NULL;
    }
}

//create Date&Time page
void time_setting_create() {
    liste_style_create();
    lv_obj_t * win = default_list_header(lv_scr_act(), get_string(ID_TIME_SETTING_TITLE), time_setting_close_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);
    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);
    //liste_datetime_year
    liste_time_year = lv_liste_w_arrow(list, get_string(ID_TIME_SETTING_YEAR), get_current_year(), year_action);
    //liste_time_month_date
    liste_time_month_date = lv_liste_w_arrow(list, get_string(ID_TIME_SETTING_MONTH_DATE), get_month_date(), month_date_action);
    //liste_time_hour_minute
    liste_time_hour_minute = lv_liste_w_arrow(list, get_string(ID_TIME_SETTING_HOUR_MINUTE), get_hour_minute(), hour_minute_action);
    //liste_time_am_pm
    liste_time_am_pm = lv_liste_w_arrow(list, get_string(ID_TIME_SETTING_AM_PM), get_current_am_pm(), am_pm_action);
    time_setting_label_refresh_task();
}

//ex:2020
char* get_current_year() {
    memset(current_year, '\0', sizeof(current_year));
    t = time(NULL);
    struct tm *timeinfo = localtime(&t);
    strftime(current_year, sizeof(current_year), "%Y", timeinfo);
    return current_year;
}

char* get_current_month() {
    memset(current_month, '\0', sizeof(current_month));
    t = time(NULL);
    struct tm *timeinfo = localtime(&t);
    strftime(current_month, sizeof(current_month), "%m", timeinfo);
    return current_month;
}

char* get_current_date() {
    memset(current_date, '\0', sizeof(current_date));
    t = time(NULL);
    struct tm *timeinfo = localtime(&t);
    strftime(current_date, sizeof(current_date), "%d", timeinfo);
    return current_date;
}

char* get_current_hour() {
    memset(current_hour, '\0', sizeof(current_hour));
    t = time(NULL);
    struct tm *timeinfo = localtime(&t);
    strftime(current_hour, sizeof(current_hour), "%I", timeinfo);
    return current_hour;
}

char* get_current_hour_24hformat() {
    memset(current_hour_24hformat, '\0', sizeof(current_hour_24hformat));
    t = time(NULL);
    struct tm *timeinfo = localtime(&t);
    strftime(current_hour_24hformat, sizeof(current_hour_24hformat), "%H", timeinfo);
    return current_hour_24hformat;
}

char* get_current_minute() {
    memset(current_minute, '\0', sizeof(current_minute));
    t = time(NULL);
    struct tm *timeinfo = localtime(&t);
    strftime(current_minute, sizeof(current_minute), "%M", timeinfo);
    return current_minute;
}

char* get_current_sec() {
    memset(current_sec, '\0', sizeof(current_sec));
    t = time(NULL);
    struct tm *timeinfo = localtime(&t);
    strftime(current_sec, sizeof(current_sec), "%S", timeinfo);
    return current_sec;
}

char* get_current_am_pm() {
    memset(current_am_pm, '\0', sizeof(current_am_pm));
    t = time(NULL);
    struct tm *timeinfo = localtime(&t);
    strftime(current_am_pm, sizeof(current_am_pm), "%p", timeinfo);
    return current_am_pm;
}

int hour_convert_check(int hr, int am_pm_type) {
    int res;
    if (am_pm_type == ID_PM && (hr > 0 && hr < 12)) {
        res = hr + 12;
    } else if (am_pm_type == ID_AM && (hr == 12)) {
        res = hr - 12;
    } else {
        res = hr;
    }
    log_d("hour_convert_check res:%d",res);
    return res;
}

void set_time(struct time *datetime) {
    memset(cmd, '\0', sizeof(cmd));
    //sprintf(cmd, "timedatectl set-time \"%04d-%02d-%02d %02d:%02d:%02d\"",
    //        datetime->year, datetime->mon, datetime->day,
    //        datetime->hour, datetime->min, datetime->sec);
    sprintf(cmd, "dci -t \"%02d-%02d-%02d %02d:%02d:%02d\"",
            datetime->year % 100, datetime->mon, datetime->day,
            datetime->hour, datetime->min, datetime->sec);
    log_d("set_time cmd:%s", cmd);
    #if !defined (FEATURE_ROUTER)
        //packet_router no need to timedatectl set-ntp no
        //int set_ntp_res1 = systemCmd("timedatectl set-ntp no");
        //log_d("set_ntp_res1:%d",set_ntp_res1);
    #endif
    int res = systemCmd(cmd);
    log_d("%s result: %d", cmd, res);
}

void time_setting_err_action(lv_obj_t * mbox, lv_event_t event) {
    if (event != LV_EVENT_CLICKED)
        return;
    log_d("time_setting_err_action setting_type:%d", setting_type);
    const char* msg =
            (setting_type == YEAR_TYPE) ?
                    get_string(ID_TIME_SETTING_YEAR) :
                    get_string(ID_TIME_SETTING_MONTH_DATE);
    num_kb_refresh_title(msg);
    if (setting_type == YEAR_TYPE) {
        num_kb_time_box_year_default_val();
    }
    num_kb_box_enable_tick_btn(false);
    close_popup();
}

void time_setting_err_notice(int type) {
    setting_type = type;
    static const char *btns[2];
    btns[0] = get_string(ID_OK);
    btns[1] = "";
    popup_anim_not_create(get_string(ID_TIME_SETTING_ERR), btns,
            time_setting_err_action,
            NULL);
}
