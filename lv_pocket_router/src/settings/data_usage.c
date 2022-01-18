#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "lv_pocket_router/src/keyboard/num_kb_col.h"
#include "lv_pocket_router/src/keyboard/num_kb_box.h"
#include "lv_pocket_router/src/keyboard/en_kb.h"
#include "lv_pocket_router/src/keyboard/basic_kb.h"
#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/settings/data_usage.h"
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/res/values/styles.h"
#include "lv_pocket_router/res/values/string_value.h"
#define UNIT_MAX_LISTE 2
lv_obj_t * liste_data_usage_monitor;
lv_obj_t * liste_display_data_usage;
lv_obj_t * liste_max_data_usage;
lv_obj_t * liste_unit;
lv_obj_t * liste_start_date;
lv_obj_t * liste_remind_data_usage;
lv_obj_t * liste_remind_threshold;
static lv_obj_t * usage_label;

char max_str[64];
char remind_str[10];
char data_usage_info1[30];
char data_usage_info2[250];
char max_data_usage_info[120];

lv_style_t style_bar;
lv_style_t style_indic;
lv_style_t style_knob;

lv_obj_t * max_data_usage_value_label;
lv_obj_t * unit_label;
lv_obj_t * start_date_value_label;
lv_obj_t * the_remind_threshold_label;
lv_obj_t * threshold_percent_label;
lv_obj_t * unit_liste_img[UNIT_MAX_LISTE];
lv_obj_t * unit_liste[UNIT_MAX_LISTE];
lv_obj_t * num_col_root;

lv_task_t * data_usage_refresh_task;

int unit_map[2] = { ID_GB, ID_MB };
int unit_type;
int min_slider_bar_vale;
int max_slider_bar_vale;

char* get_data_usage_content1() {
    double usage;
    const char *units = get_string(ID_MB);

#if defined (FEATURE_ROUTER)
    usage = get_data_usage();
    //to avoid showing negative value while data too large
    if(usage < 0) usage = 0;
#else
    usage = (rand() % 100);
#endif
    if(ds_get_int(DS_KEY_DATA_USAGE_UNIT) == DATA_USAGE_UNIT_GB) {
        usage = usage / 1024;
        units = get_string(ID_GB);
        snprintf(data_usage_info1, sizeof(data_usage_info1), "%.2f%s", usage, units);
    }else{
        snprintf(data_usage_info1, sizeof(data_usage_info1), "%.0f%s", usage, units);
    }

    return data_usage_info1;
}

char* get_data_usage_content2() {

    const char * reset_time = get_string(ID_DATA_USAGE_LAST_TIME_RESET);
    char *time = getLastTimeResetDateTime();
    const char *note = get_string(ID_DATA_USAGE_MAIN_NOTE);
    snprintf(data_usage_info2, sizeof(data_usage_info2), "%s: %s  %s",
            reset_time, time, note);

    return data_usage_info2;
}

//for data usage main page back/home btn action, will delete task and close win
void info_page_header_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    if (data_usage_refresh_task != NULL) {
        lv_task_del(data_usage_refresh_task);
        data_usage_refresh_task = NULL;
    }
    lv_win_close_event_cb(btn, LV_EVENT_RELEASED);
}

void update_data_usage_label(){
    char* content = get_data_usage_content1();
    if(usage_label != NULL) {
        lv_label_set_text(usage_label, content);
    }
}

void reset_all_data_usage_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(mbox);

    if (strcmp(txt, get_string(ID_OK)) == 0) { //ok
        reset_data_usage(true);

        //update date after reset data usage
        lv_obj_t * label = lv_obj_get_child(lv_obj_get_parent(usage_label), NULL);
        lv_label_set_text(label, get_data_usage_content2());
    }
    close_popup();
}

void data_usage_main_page_action(lv_obj_t * btnm, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(btnm);

    if (strcmp(txt, get_string(ID_DATA_USAGE_CLEAR)) == 0) { //clear
        static const char * btns[3];
        btns[0] = get_string(ID_CANCEL);
        btns[1] = get_string(ID_OK);
        btns[2] = "";
        popup_anim_que_create(get_string(ID_DATA_USAGE_RESET_CONFIRM_PROMPT), btns,
                reset_all_data_usage_action, NULL);
    } else if (strcmp(txt, get_string(ID_SETTINGS)) == 0) { //settings
        data_usage_create();
    }
}

void init_data_usage(void) {
    static const char * btns[3];
    btns[0] = get_string(ID_DATA_USAGE_CLEAR);
    btns[1] = get_string(ID_SETTINGS);
    btns[2] = "";
    usage_label = info_page_create_btmn_label_refresh(lv_scr_act(),
            get_string(ID_DATA_USAGE),
            get_data_usage_content1(),
            get_data_usage_content2(),
            btns, info_page_header_action,
            data_usage_main_page_action);

    //update data usage every 3000ms for now
    data_usage_refresh_task = lv_task_create(update_data_usage_label, 3000, LV_TASK_PRIO_LOW, NULL);
}

char* getLastTimeResetDateTime() {
    return ds_get_value(DS_KEY_DATA_USAGE_RESET_TIME);
}

char* getMaxDataUsage() {
    const int unit = ds_get_int(DS_KEY_DATA_USAGE_UNIT);
    int max = ds_get_int(DS_KEY_MAX_DATA_USAGE);

    memset(max_str, '\0', sizeof(max_str));
    float value = (unit == DATA_USAGE_UNIT_GB)? 
        (max/1024.00) : (max);
    if (value > 0 && value < 1) {
        sprintf(max_str, "%.2f", value);
    } else {
        sprintf(max_str, "%.f", value);
    }
    return max_str;
}

const char* getUnit() {
    const int unit = ds_get_int(DS_KEY_DATA_USAGE_UNIT);
    return (unit == DATA_USAGE_UNIT_GB)? get_string(ID_GB) : get_string(ID_MB);
}

char* getStartDate() {
    return "1";
}

char* getRemindThreshold() {
    memset(remind_str, '\0', sizeof(remind_str));
    sprintf(remind_str, "%d%s", ds_get_int(DS_KEY_DATA_USAGE_REMIND_VALUE), "%");
    return remind_str;
}

void remind_data_usage_action(lv_obj_t * sw, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    //liste_remind_data_usage
#ifdef CUST_SWITCH
    set_cust_switch_state(liste_remind_data_usage, DS_KEY_DATA_USAGE_REMIND);
    bool enable = ds_get_bool(DS_KEY_DATA_USAGE_REMIND);
    lv_obj_set_hidden(liste_remind_threshold, !enable);
#else
    bool remind = lv_sw_get_state(sw);
    ds_set_bool(DS_KEY_DATA_USAGE_REMIND, remind);
    lv_obj_set_hidden(liste_remind_threshold, !remind);
#endif
}

void remind_threshold_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    if (ds_get_bool(DS_KEY_DATA_USAGE_REMIND)) {
        remind_threshold_create();
    }
}

void start_date_keyboard_action(lv_obj_t * btnm, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    uint32_t index = lv_obj_get_user_data(btnm);
    log_d("start_date_keyboard_action index %d",index);
    if (index == KB_RIGHT_BTN){
        //return if tick btn is in inactive state
        if(get_tick_btn_ina_state(get_kb_tick_btn(num_col_root))) return;

        char* value = num_col_get_lable();
        if (value[0] != '\0') {
            ds_set_value(DS_KEY_DATA_USAGE_START_DATE, value);
            lv_label_set_text(start_date_value_label, value);
            lv_liste_w_arrow_align(start_date_value_label);

            reset_date_usage_start_date();
        }
    }
    num_col_close_win();
}

void start_date_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    start_date_value_label = lv_obj_get_child(liste_start_date, NULL);
    num_col_root = num_col_create(get_string(ID_DATA_USAGE_START_DATE), false, start_date_keyboard_action);
    num_col_set_lable(lv_label_get_text(start_date_value_label));
    num_kb_col_set_lable_len(2);
    set_date_keyboard_style(true);
}

void unit_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    unit_create();
}

void max_data_usage_keyboard_action(lv_obj_t * btnm, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;

    uint32_t index = lv_obj_get_user_data(btnm);
    log_d("max_data_usage_keyboard_action index %d",index);

    if (index == KB_RIGHT_BTN) { //ok
        //return if tick btn is in inactive state
        if(get_tick_btn_ina_state(get_kb_tick_btn(num_col_root))) return;

        char* str = num_col_get_lable();
        int value = 0;
        if (str != NULL && str[0] != '\0') {
            value = atoi(str);
        }

        if (value > 0) {
            ds_set_int(DS_KEY_MAX_DATA_USAGE, 
                (ds_get_int(DS_KEY_DATA_USAGE_UNIT) == DATA_USAGE_UNIT_MB)?
                    value : value*1024);
            lv_label_set_text(max_data_usage_value_label, get_max_data_usage_content());
            lv_liste_w_arrow_align(max_data_usage_value_label);
        }
    }
    update_data_usage_bar();
    num_col_close_win();
}

void max_data_usage_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    num_col_root = num_col_create(get_string(ID_DATA_USAGE_MAX_DATA_USAGE), false, max_data_usage_keyboard_action);
    int value = atoi(getMaxDataUsage());
    log_d("max_data_usage_action value:%d",value);
    if (value == 0) {
        num_col_set_lable("0");
    } else {
        num_col_set_lable(getMaxDataUsage());
    }
    //set data usage input limit to 3 digits in GB(999GB)
    //set data usage input limit to 6 digits in MB(999999MB)
    int unit = ds_get_int(DS_KEY_DATA_USAGE_UNIT);
    if(unit == DATA_USAGE_UNIT_GB){
        num_kb_col_set_lable_len(3);
    }else{
        num_kb_col_set_lable_len(6);
    }
}

void data_usage_monitor_action(lv_obj_t * sw, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

#ifdef CUST_SWITCH
    set_cust_switch_state(liste_data_usage_monitor, DS_KEY_DATA_USAGE_MONITOR);
#else
    bool value = lv_sw_get_state(sw);
    ds_set_bool(DS_KEY_DATA_USAGE_MONITOR, value);
#endif
    reset_data_usage(false);
    //refresh_data_usage_settings
    refresh_data_usage_settings(ds_get_bool(DS_KEY_DATA_USAGE_MONITOR));
}

void display_data_usage_on_home_screen_action(lv_obj_t * sw, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

#ifdef CUST_SWITCH
    lv_obj_t * img = lv_obj_get_child(liste_display_data_usage, NULL);
    bool value = ds_get_bool(DS_KEY_SHOW_DATA_USAGE_ON_HOME);
    ds_set_bool(DS_KEY_SHOW_DATA_USAGE_ON_HOME, !value);
#ifdef CUST_DLINK
    if (value) {
        //enable "No show on display" and set show_data_usage_on_home as false
        lv_img_set_src(img, &ic_list_checkbox_selected);
    } else {
        //disable "No show on display" and set show_data_usage_on_home as true
        lv_img_set_src(img, &ic_list_checkbox);
    }
#else
    if (value) {
        lv_img_set_src(img, &ic_list_checkbox);
    } else {
        lv_img_set_src(img, &ic_list_checkbox_selected);
    }
#endif /* CUST_DLINK */
#else
    bool value = lv_sw_get_state(sw);
#ifdef CUST_DLINK
    //set show_data_usage_on_home as false when enable "No show on display"
    value = !value;
#endif
    ds_set_bool(DS_KEY_SHOW_DATA_USAGE_ON_HOME, value);
#endif
    update_data_usage_bar();
}

void data_usage_create(void) {
    liste_style_create();
    lv_obj_t * win = default_list_header (lv_scr_act(), get_string(ID_DATA_USAGE), lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    //liste_data_usage_monitor
    bool data = ds_get_bool(DS_KEY_DATA_USAGE_MONITOR);
#ifdef CUST_SWITCH
    liste_data_usage_monitor = lv_liste_cust_switch(list,
            get_string(ID_DATA_USAGE_MONITOR),
            data_usage_monitor_action, data);
#else
    liste_data_usage_monitor = lv_liste_w_switch(list, get_string(ID_DATA_USAGE_MONITOR), data_usage_monitor_action);
    lv_obj_t * data_usage_monitor_sw = lv_obj_get_child(liste_data_usage_monitor, NULL);
    if (data) {
        lv_sw_on(data_usage_monitor_sw, LV_ANIM_OFF);
    } else {
        lv_sw_off(data_usage_monitor_sw, LV_ANIM_OFF);
    }
#endif

    //liste_display_data_usage
    const char * title;
    bool display = ds_get_bool(DS_KEY_SHOW_DATA_USAGE_ON_HOME);
#ifdef CUST_DLINK
    display = !display;
    title = get_string(ID_DATA_USAGE_NO_SHOW_ON_DISPLAY);
#else
    title = get_string(ID_DATA_USAGE_DISPLAY_DATA_USAGE_ON_HOME_SCREEN);
#endif
    log_d("init display %d", display);
#ifdef CUST_SWITCH
    liste_display_data_usage = lv_liste_cust_switch(list, title,
            display_data_usage_on_home_screen_action, display);
#else
    liste_display_data_usage = lv_liste_w_switch(list, title,
            display_data_usage_on_home_screen_action);
    lv_obj_t * display_data_usage_sw = lv_obj_get_child(liste_display_data_usage, NULL);
    if (display) {
        lv_sw_on(display_data_usage_sw, LV_ANIM_OFF);
    } else {
        lv_sw_off(display_data_usage_sw, LV_ANIM_OFF);
    }
#endif

    //liste_max_data_usage
    liste_max_data_usage = lv_liste_w_arrow(list, get_string(ID_DATA_USAGE_MAX_DATA_USAGE),
            get_max_data_usage_content(), max_data_usage_action);
    max_data_usage_value_label = lv_obj_get_child(liste_max_data_usage, NULL);

    //liste_unit
    unit_type = ds_get_int(DS_KEY_DATA_USAGE_UNIT);
    liste_unit = lv_liste_w_arrow(list, get_string(ID_DATA_USAGE_UNIT), getUnit(), unit_action);

    //liste_start_date
    liste_start_date = lv_liste_w_arrow(list, get_string(ID_DATA_USAGE_START_DATE),
            ds_get_value(DS_KEY_DATA_USAGE_START_DATE), start_date_action);
#ifndef CUST_DLINK
    //liste_remind_data_usage
    bool remind = ds_get_bool(DS_KEY_DATA_USAGE_REMIND);
#ifdef CUST_SWITCH
    liste_remind_data_usage = lv_liste_cust_switch(list,
            get_string(ID_DATA_USAGE_REMIND_SWITCH_TITLE),
            remind_data_usage_action, remind);
#else
    liste_remind_data_usage = lv_liste_w_switch(list,
            get_string(ID_DATA_USAGE_REMIND_SWITCH_TITLE),
            remind_data_usage_action);

    lv_obj_t * remind_data_usage_sw = lv_obj_get_child(liste_remind_data_usage, NULL);
    if (remind) {
        lv_sw_on(remind_data_usage_sw, LV_ANIM_OFF);
    } else {
        lv_sw_off(remind_data_usage_sw, LV_ANIM_OFF);
    }
#endif

    //liste_remind_threshold
    liste_remind_threshold = lv_liste_w_arrow(list, get_string(ID_DATA_USAGE_REMIND_THRESHOLD),
            getRemindThreshold(), remind_threshold_action);
    lv_obj_set_hidden(liste_remind_threshold, !remind);
#endif
    //refresh_data_usage_settings
    refresh_data_usage_settings(ds_get_bool(DS_KEY_DATA_USAGE_MONITOR));
}

void unit_confirm_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    unit_label = lv_obj_get_child(liste_unit, NULL);
    lv_label_set_text(unit_label, get_string(unit_map[unit_type]));
    lv_liste_w_arrow_align(unit_label);

    ds_set_int(DS_KEY_DATA_USAGE_UNIT, unit_type);
    lv_label_set_text(max_data_usage_value_label, get_max_data_usage_content());
    lv_liste_w_arrow_align(max_data_usage_value_label);
    //lv_obj_t * win = lv_win_get_from_btn(btn);
    //lv_obj_del(win);

    update_data_usage_bar();
}

void unit_btn_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    int i;
    unit_type = lv_obj_get_user_data(btn);

    for (i = 0; i < UNIT_MAX_LISTE; i++){
        lv_img_set_src(unit_liste_img[i], &btn_list_radio_n);
    }
    lv_img_set_src(unit_liste_img[unit_type], &btn_list_radio_p);

    log_d("unit_btn_action unit_type:%d",unit_type);
}

void unit_create(void) {
    liste_style_create();
    lv_obj_t * win = modify_list_header(lv_scr_act(), get_string(ID_DATA_USAGE_UNIT),
            unit_confirm_action, lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    //add list element in order
    int unit = ds_get_int(DS_KEY_DATA_USAGE_UNIT);
    int i;
    for (i = 0; i < UNIT_MAX_LISTE; i++) {
        unit_liste[i] = lv_liste_w_cbox(list, get_string(unit_map[i]), unit==i, unit_btn_action, i);
        unit_liste_img[i] = lv_obj_get_child(unit_liste[i], NULL);
    }
}

void threshold_change_action(lv_obj_t * slider, lv_event_t event)
{
    if (event != LV_EVENT_PRESSING && event != LV_EVENT_CLICKED) return;

    int value = lv_slider_get_value(slider);
    int data_usage_remind_value = (is_ltr() ? value : (max_slider_bar_vale - value));
    memset(remind_str, '\0', sizeof(remind_str));
    sprintf(remind_str, "%d%s", data_usage_remind_value, "%");
    //keep update label while user dragging threshold bar
    lv_label_set_text(threshold_percent_label, remind_str);

    //set value to xml once user release
    if(event == LV_EVENT_CLICKED){
        ds_set_int(DS_KEY_DATA_USAGE_REMIND_VALUE, data_usage_remind_value);
    }
}

void remind_threshold_close_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    the_remind_threshold_label = lv_obj_get_child(liste_remind_threshold, NULL);
    lv_label_set_text(the_remind_threshold_label, getRemindThreshold());
    lv_liste_w_arrow_align(the_remind_threshold_label);
    //lv_obj_t * win = lv_win_get_from_btn(btn);
    //lv_obj_del(win);
}

void remind_threshold_create(void) {
    static lv_style_t style;
    lv_style_copy(&style, &lv_style_plain);
    style.text.font = get_font(font_w_bold, font_h_50);
    style.text.color = LV_COLOR_GREYISH_BROWN;

    liste_style_create();
    lv_obj_t * win = default_list_header (lv_scr_act(), get_string(ID_DATA_USAGE_REMIND_THRESHOLD),
            remind_threshold_close_action);
    static lv_style_t threshold_usage_font_style;
    lv_style_copy(&threshold_usage_font_style, &lv_style_plain);
    threshold_usage_font_style.text.font = get_font(font_w_bold, font_h_50);
    threshold_usage_font_style.text.color = LV_COLOR_GREYISH_BROWN;

    lv_obj_t * bg = lv_cont_create(win, NULL);
    lv_obj_set_size(bg, 320, 190);
    lv_obj_set_style(bg, &style);
    lv_obj_align(bg, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    lv_obj_t *  threshold_usage_label =  lv_label_create(bg, NULL);
    char threshold_usage_info[20];
    int unit = ds_get_int(DS_KEY_DATA_USAGE_UNIT);
    snprintf(threshold_usage_info,sizeof(threshold_usage_info),"%s%s",
            getMaxDataUsage(), get_string(unit_map[unit]));
    lv_label_set_text(threshold_usage_label, threshold_usage_info);

    lv_obj_set_size(threshold_usage_label, 280, 50);
    lv_obj_set_style(threshold_usage_label, &threshold_usage_font_style);
    lv_obj_align(threshold_usage_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 18);

    static lv_style_t threshold_percent_font_style;
    lv_style_copy(&threshold_percent_font_style, &lv_style_plain);
    threshold_percent_font_style.text.font = get_font(font_w_bold, font_h_24);
    threshold_percent_font_style.text.color = LV_COLOR_GREYISH_BROWN;

    threshold_percent_label =  lv_label_create(bg, NULL);
    lv_label_set_text(threshold_percent_label, getRemindThreshold());
    lv_obj_set_size(threshold_percent_label, 129, 30);
    lv_obj_align(threshold_percent_label, NULL, LV_ALIGN_IN_TOP_LEFT, 145, 70);
    lv_obj_set_style(threshold_percent_label, &threshold_percent_font_style);

    lv_obj_t * ic_list_usage_l_img = lv_img_create(bg, NULL);
    lv_img_set_src(ic_list_usage_l_img, &ic_list_usage_l);
    lv_obj_align(ic_list_usage_l_img, NULL,
            (is_ltr() ? LV_ALIGN_IN_BOTTOM_LEFT : LV_ALIGN_IN_BOTTOM_RIGHT),
            (is_ltr() ? USAGE_HOR_SPACE : -USAGE_HOR_SPACE), -34);

    lv_obj_t * slider = lv_slider_create(bg, NULL);
    lv_obj_set_size(slider, 200, 16);
    min_slider_bar_vale = 0;
    max_slider_bar_vale = 100;
    lv_slider_set_range(slider, min_slider_bar_vale, max_slider_bar_vale);

    int slider_bar_len = (is_ltr() ?
                    ds_get_int(DS_KEY_DATA_USAGE_REMIND_VALUE) :
                    (max_slider_bar_vale - ds_get_int(DS_KEY_DATA_USAGE_REMIND_VALUE)));
    lv_slider_set_value(slider, slider_bar_len , false);
    lv_obj_set_event_cb(slider, threshold_change_action);
    lv_obj_align(slider, NULL, LV_ALIGN_IN_TOP_LEFT, 60, 140);

    lv_style_copy(&style_bar, &lv_style_pretty);
    style_bar.body.main_color = (is_ltr() ? LV_COLOR_SILVER : LV_COLOR_BASE);
    style_bar.body.grad_color = (is_ltr() ? LV_COLOR_SILVER : LV_COLOR_BASE);
    style_bar.body.radius = LV_RADIUS_CIRCLE;
    style_bar.body.border.color = (is_ltr() ? LV_COLOR_SILVER : LV_COLOR_BASE);

    lv_style_copy(&style_indic, &lv_style_pretty);
    style_indic.body.grad_color = (is_ltr() ? LV_COLOR_BASE : LV_COLOR_SILVER);
    style_indic.body.main_color= (is_ltr() ? LV_COLOR_BASE : LV_COLOR_SILVER);
    style_indic.body.radius = LV_RADIUS_CIRCLE;
    style_indic.body.padding.left = 0;
    style_indic.body.padding.right = 0;
    style_indic.body.padding.top = 0;
    style_indic.body.padding.bottom = 0;

    lv_style_copy(&style_knob, &lv_style_pretty);
    style_knob.body.radius = LV_RADIUS_CIRCLE;
    style_knob.body.opa = LV_OPA_COVER;
    style_knob.body.main_color =  LV_COLOR_GREYISH_BROWN;
    style_knob.body.grad_color =  LV_COLOR_GREYISH_BROWN;

    lv_slider_set_style(slider, LV_SLIDER_STYLE_BG, &style_bar);
    lv_slider_set_style(slider, LV_SLIDER_STYLE_INDIC, &style_indic);
    lv_slider_set_style(slider, LV_SLIDER_STYLE_KNOB, &style_knob);

    lv_obj_t * ic_list_usage_h_img = lv_img_create(bg, NULL);
    lv_img_set_src(ic_list_usage_h_img, &ic_list_usage_h);
    lv_obj_set_click(ic_list_usage_h_img, false);
    lv_obj_align(ic_list_usage_h_img, NULL,
            (is_ltr() ? LV_ALIGN_IN_BOTTOM_RIGHT : LV_ALIGN_IN_BOTTOM_LEFT),
            (is_ltr() ? -USAGE_HOR_SPACE : USAGE_HOR_SPACE), -34);
}

void refresh_data_usage_settings(bool value) {
    if (!value) {
        lv_obj_set_hidden(liste_display_data_usage, 1);
        lv_obj_set_hidden(liste_max_data_usage, 1);
        lv_obj_set_hidden(liste_unit, 1);
        lv_obj_set_hidden(liste_start_date, 1);
#ifndef CUST_DLINK
        lv_obj_set_hidden(liste_remind_data_usage, 1);
        lv_obj_set_hidden(liste_remind_threshold, 1);
#endif
    } else {
        lv_obj_set_hidden(liste_display_data_usage, 0);
        lv_obj_set_hidden(liste_max_data_usage, 0);
        lv_obj_set_hidden(liste_unit, 0);
        lv_obj_set_hidden(liste_start_date, 0);
#ifndef CUST_DLINK
        lv_obj_set_hidden(liste_remind_data_usage, 0);
        lv_obj_set_hidden(liste_remind_threshold,
                ds_get_bool(DS_KEY_DATA_USAGE_REMIND) ? 0 : 1);
#endif
    }
    update_data_usage_bar();
}

char* get_max_data_usage_content() {
    int unit = ds_get_int(DS_KEY_DATA_USAGE_UNIT);
    memset(max_data_usage_info, '\0', sizeof(max_data_usage_info));
    snprintf(max_data_usage_info, sizeof(max_data_usage_info), "%s%s", getMaxDataUsage(), get_string(unit_map[unit]));
    return max_data_usage_info;
}
