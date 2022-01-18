#include "launcher.h"
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h> // memcpy

#include "status_bar.h"
#include "status_bar_view.h"
#include "uevent.h"
#include "../res/values/string_value.h"
#include "../res/values/styles.h"

#include "lv_pocket_router/src/about/about.h"
#include "lv_pocket_router/src/conn_guide/guide.h"
#include "lv_pocket_router/src/power_saving/power_saving.h"
#include "lv_pocket_router/src/ipc/socket_client.h"
#include "lv_pocket_router/src/ipc/socket_server.h"
#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/settings/settings.h"
#include "lv_pocket_router/src/speedtest/speedtest.h"
#include "lv_pocket_router/src/speedtest/downloadtest.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/ssid/ssid.h"
#include "lv_pocket_router/src/sms/sms.h"
#include "lv_pocket_router/src/util/page_anim.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"

static void tab_slider_event_handler(lv_obj_t * tv, lv_event_t event);

static lv_obj_t * tv;
lv_obj_t * status_bar_root;
lv_obj_t * data_usage_bar;
lv_obj_t * data_usage_space;

static lv_style_t style_logo_bg;
static lv_style_t style_bold_font;
static lv_style_t style_white_bg;
static lv_style_t style_btn_pr;
static lv_style_t style_sms_num;
static lv_style_t style_bar;
static lv_style_t style_indic;
static lv_style_t style_indic_warning;
static lv_style_t data_font_style;
static lv_style_t data_font_style_warning;

#ifndef FEATURE_ROUTER // for simulator testing
#define SPEED_TEST
#endif

#define ITEM_PER_TV         3

static char * tv_name[] = {"First", "Second", "third"};

#ifdef CUST_DLINK
//for app btn click anim
const void * app_anim_map[]={&ic_launcher_btn_pressed_01, &ic_launcher_btn_pressed_02};
int app_anim_len = sizeof(app_anim_map) / sizeof(void *);
int app_anim_cnt;
lv_obj_t * app_anim_img;
#endif
lv_task_t * app_anim_task;
bool app_click = false;
void app_set_img_src(lv_obj_t * img, const void * src_img, bool enable);

//page indicator icon
lv_obj_t * ind1;
lv_obj_t * ind2;
#ifdef SPEED_TEST
lv_obj_t * ind3;
#endif
//for sms unread number
static lv_obj_t * sms_num_img;
static lv_obj_t * sms_num_label;
lv_task_t * update_sms_num_task;

lv_obj_t * app_guide;
lv_obj_t * img1;
lv_obj_t * app_guide_txt;
lv_obj_t * app_power_saving;
lv_obj_t * img2;
lv_obj_t * app_power_saving_txt;
lv_obj_t * app_ssid;
lv_obj_t * img3;
lv_obj_t * app_ssid_txt;
lv_obj_t * app_sms;
lv_obj_t * img4;
lv_obj_t * app_sms_txt;
lv_obj_t * app_settings;
lv_obj_t * img5;
lv_obj_t * app_settings_txt;
lv_obj_t * img6;
lv_obj_t * app_about;
lv_obj_t * app_about_txt;
lv_obj_t * app_speed_test;
lv_obj_t * img7;
lv_obj_t * app_speed_test_txt;
#if DOWNLOAD_TEST == 1
lv_obj_t * app_download_test;
lv_obj_t * img8;
lv_obj_t * app_download_test_txt;
#endif
lv_obj_t * company_logo;
#ifdef CUST_DLINK
lv_obj_t * home_btn;
lv_obj_t * home_img;
#endif

// note: when deleting objs, last obj in list will be delete first.
// add obj to list in order they were created, so when delete,
// child objects can be delete before parent obj
lv_obj_t ** launcher_obj_list[] = { &tv, &status_bar_root, &data_usage_bar,
           &data_usage_space, &ind1, &ind2,
#ifdef SPEED_TEST
           &ind3,
#endif
           &app_guide, &img1,
           &app_guide_txt, &app_power_saving, &img2, &app_power_saving_txt,
           &app_ssid, &img3, &app_ssid_txt, &app_sms, &img4, &app_sms_txt,
           &app_settings, &img5, &app_settings_txt, &app_about, &img6, &app_about_txt,
           &app_speed_test, &img7, &app_speed_test_txt, &sms_num_img, &sms_num_label,
#if DOWNLOAD_TEST == 1
           &app_download_test, &img8, &app_download_test_txt,
#endif
           &company_logo,
#ifdef CUST_DLINK
	   &home_btn, &home_img
#endif
};

int btn_click_index;
int tmp_click_index = -1;
lv_point_t point_last;

void app_btn_hidden(bool hidden);
lv_point_t p_press;

lv_obj_t ** main_menu_btn[] = {
        &app_guide, &app_power_saving, &app_ssid,
        &app_sms, &app_settings, &app_about, 
#ifdef SPEED_TEST
        &app_speed_test
#endif
        };
lv_obj_t ** main_menu_img[] = {
        &img1, &img2, &img3, &img4, &img5, &img6, &img7};
lv_obj_t ** main_menu_label[] = {
        &app_guide_txt, &app_power_saving_txt,
        &app_ssid_txt, &app_sms_txt, &app_settings_txt,
        &app_about_txt, &app_speed_test_txt};
const void * main_menu_img_src[] = {
        &ic_launcher_guide, &ic_launcher_power, &ic_launcher_ssid,
        &ic_launcher_sms, &ic_launcher_setting, &ic_launcher_about,
        &ic_launcher_speedtest};
int main_menu_id[] = {
        INDEX_CONN_GUIDE, INDEX_POWER_SAVING, INDEX_SSID_SETTING,
        INDEX_SMS, INDEX_SETTINGS, INDEX_ABOUT, INDEX_SPEEDTEST};
int main_label_id[] = {
        ID_LAUNCHER_GUIDE, ID_LAUNCHER_POWER, ID_LAUNCHER_SSID, ID_LAUNCHER_SMS,
        ID_LAUNCHER_SETTINGS, ID_LAUNCHER_ABOUT, ID_LAUNCHER_SPEEDTEST};

void launcher_style_create(){

#if defined(HIGH_RESOLUTION)
    lv_style_copy(&style_bold_font, &lv_style_plain);
    style_bold_font.text.font = get_font(font_w_bold, font_h_40);
    style_bold_font.text.color = LV_COLOR_GREYISH_BROWN;
#else
    lv_style_copy(&style_bold_font, &lv_style_plain);
    style_bold_font.text.font = get_font(font_w_bold, font_h_22);
    style_bold_font.text.color = LV_COLOR_GREYISH_BROWN;
    style_bold_font.text.letter_space = 0;
#endif

    lv_style_copy(&style_white_bg, &lv_style_plain);
    style_white_bg.body.main_color = LV_COLOR_WHITE;
    style_white_bg.body.grad_color = LV_COLOR_WHITE;

    lv_style_copy(&style_btn_pr, &style_white_bg);
    style_btn_pr.image.color = LV_COLOR_BASE;
    style_btn_pr.image.intense = LV_OPA_COVER;

    //add sms unread message number
    lv_style_copy(&style_sms_num, &lv_style_plain);
    style_sms_num.text.font = get_font(font_w_bold, font_h_16);
    style_sms_num.text.color = LV_COLOR_WHITE;
    style_sms_num.text.letter_space = 0;

    // Data usage counter
    lv_style_copy(&style_bar, &lv_style_plain);
    style_bar.body.main_color = LV_COLOR_GRAY;
    style_bar.body.grad_color = LV_COLOR_GRAY;
    style_bar.body.radius = LV_RADIUS_CIRCLE;
    style_bar.body.border.color = LV_COLOR_GRAY;

    lv_style_copy(&style_indic, &lv_style_plain);
    style_indic.body.grad_color =  LV_COLOR_BASE;
    style_indic.body.main_color =  LV_COLOR_BASE;
    style_indic.body.radius = LV_RADIUS_CIRCLE;
    style_indic.body.padding.left = 0;
    style_indic.body.padding.right = 0;
    style_indic.body.padding.top = 0;
    style_indic.body.padding.bottom = 0;

    lv_style_copy(&style_indic_warning, &style_indic);
    style_indic_warning.body.grad_color =  LV_COLOR_RED;
    style_indic_warning.body.main_color =  LV_COLOR_RED;

    //for adding company logo
    lv_style_copy(&style_logo_bg, &lv_style_plain);
    style_logo_bg.image.color = LV_COLOR_GREYISH_BROWN;
    style_logo_bg.image.intense = LV_OPA_COVER;

    lv_style_copy(&data_font_style, &lv_style_plain);
    data_font_style.text.font = get_font(font_w_bold, font_h_16);
    data_font_style.text.color = LV_COLOR_GRAY;

    lv_style_copy(&data_font_style_warning, &data_font_style);
    data_font_style_warning.text.color = LV_COLOR_RED;
}

void update_data_usage_bar() {
    if (data_usage_bar == NULL && data_usage_space == NULL) {
        return ;
    }
    if (!ds_get_bool(DS_KEY_DATA_USAGE_MONITOR)) {
        lv_obj_set_hidden(data_usage_space, true);
        lv_obj_set_hidden(data_usage_bar, true);
        return;
    }

    //Use MB to calculate
    int max = ds_get_int(DS_KEY_MAX_DATA_USAGE);
    double usage = get_data_usage();
    log_d("launcher data usage: %.2f, max: %d", usage, max);

    float space = 0.0;
    if (usage >= max) {
        lv_bar_set_value(data_usage_bar, 0, false);
        space = 0;
    } else if (usage <= 0) {
        lv_bar_set_value(data_usage_bar, 100, false);
        space = max;
    } else {
        int percentage = (int)((usage/(float)max) * 100);
        lv_bar_set_value(data_usage_bar, (100 - percentage), false);
        space = max-usage;
    }

    //check if need set data usage bar/txt to warning color
    if(ds_get_bool(DS_KEY_DATA_USAGE_REMIND) &&
            space <= (max * (ds_get_int(DS_KEY_DATA_USAGE_REMIND_VALUE)) / 100)){
        lv_bar_set_style(data_usage_bar, LV_BAR_STYLE_INDIC, &style_indic_warning);
        lv_obj_set_style(data_usage_space, &data_font_style_warning);
    }else{
        lv_bar_set_style(data_usage_bar, LV_BAR_STYLE_INDIC, &style_indic);
        lv_obj_set_style(data_usage_space, &data_font_style);
    }

    if (ds_get_bool(DS_KEY_SHOW_DATA_USAGE_ON_HOME)) {
        if (lv_obj_get_hidden(data_usage_space)) {
            lv_obj_set_hidden(data_usage_space, false);
            lv_obj_set_hidden(data_usage_bar, false);
        }
        char total[64];
        memset(total, '\0', sizeof(total));
        int unit = ds_get_int(DS_KEY_DATA_USAGE_UNIT);
        if (unit == DATA_USAGE_UNIT_MB) {
            snprintf(total, sizeof(total), "%.1f%s", space, get_string(ID_MB));
        } else {
            space = space / 1024;
            snprintf(total, sizeof(total), "%.2f%s", space, get_string(ID_GB));
        }
        lv_label_set_text(data_usage_space, total);
        lv_obj_align(data_usage_space, data_usage_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    } else if (!lv_obj_get_hidden(data_usage_space)) {
        lv_obj_set_hidden(data_usage_space, true);
        lv_obj_set_hidden(data_usage_bar, true);
    }
}

void statusbar_create(void) {
    status_bar_root = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(status_bar_root, LV_HOR_RES_MAX, STATUS_BAR_HEIGH);
    lv_obj_set_style(status_bar_root, &style_white_bg);

    statusbar_reset();
    create_statusbar(status_bar_root, LV_COLOR_GREYISH_BROWN, LV_COLOR_WHITE, LAUNCHER_BG);
}

void launcher_task() {

    if (btn_click_index == INDEX_CONN_GUIDE) {
        conn_guide_create();
    } else if (btn_click_index == INDEX_POWER_SAVING) {
        power_saving_create();
    } else if (btn_click_index == INDEX_SSID_SETTING) {
        init_ssid();
    } else if (btn_click_index == INDEX_SMS) {
        init_sms(true);
    } else if (btn_click_index == INDEX_SETTINGS) {
        show_settings_list();
    } else if (btn_click_index == INDEX_ABOUT) {
        show_about_list();
    } else if (btn_click_index == INDEX_SPEEDTEST) {
#ifdef SPEED_TEST
        speedtest_create();
#endif
#if DOWNLOAD_TEST == 1
    } else if (btn_click_index == INDEX_DOWNLOADTEST) {
        downloadtest_create();
#endif
    }
    //reset app btn to rel style
    app_set_img_src(*main_menu_img[btn_click_index], main_menu_img_src[btn_click_index], false);
    app_click = false;
}

//TODO may need to change below part later
//update sms unread number every 1000ms for now
void update_sms_num() {
    if (sms_num_img == NULL || sms_num_label == NULL) {
        return;
    }
    char * num = get_sms_unread_num();
    //show unread number if any
    if (strcmp(num, "0") != 0) {
        lv_obj_set_hidden(sms_num_img, 0);
        lv_obj_set_hidden(sms_num_label, 0);
        lv_label_set_text(sms_num_label, num);
        //re-align
        lv_obj_align(sms_num_label, sms_num_img, LV_ALIGN_CENTER, 0, 0);
    } else {
        lv_obj_set_hidden(sms_num_img, 1);
        lv_obj_set_hidden(sms_num_label, 1);
    }
}

void launcher_destroy() {
    int i;
    int len = sizeof(launcher_obj_list) / sizeof(lv_obj_t *);

    statusbar_reset();

    // delete child objects before parent obj
    for(i = len-1; i >= 0; i--) {
        if (*launcher_obj_list[i] != NULL) {
            lv_obj_del(*launcher_obj_list[i]);
            *launcher_obj_list[i] = NULL;
        }
    }

    if(update_sms_num_task != NULL) {
        lv_task_del(update_sms_num_task);
        update_sms_num_task = NULL;
    }
}

//to remove launcher if launched via dashboard, should only be called from clean_task
void launcher_cleanup() {
    lv_obj_del(tv);
    lv_obj_del(status_bar_root);
    lv_obj_del(data_usage_bar);
    lv_obj_del(data_usage_space);
    lv_obj_del(ind1);
    lv_obj_del(ind2);
#ifdef SPEED_TEST
    lv_obj_del(ind3);
#endif
    lv_obj_del(company_logo);
#ifdef SHOW_COMPANY_LOGO
    lv_obj_del(company_logo);
#endif
#ifdef CUST_DLINK
    lv_obj_del(home_btn);
    lv_obj_del(home_img);
#endif
}

void app_set_img_src(lv_obj_t * img, const void * src_img, bool enable){
    lv_obj_set_style(img, enable ? &style_btn_pr : &style_white_bg);
    lv_img_set_src(img, src_img);
}

void app_click_impl(){
    //close all win while enter launcher
    close_all_lists(0);
    close_all_pages();

    //show page to page entrance part
    page_anim_enter();
    lv_task_t * anim_task = lv_task_create(launcher_task, ANIM_TIME, LV_TASK_PRIO_MID, NULL);
    lv_task_once(anim_task);

    //should only called when launched by dashboard
    if(ds_get_bool(DS_KEY_LAUNCHER_CLEANUP)) {
        lv_task_t * clean_task = lv_task_create(launcher_cleanup, DELAY_DEL_LAUNCHER, LV_TASK_PRIO_MID, NULL);
        lv_task_once(clean_task);
    }
}

void app_click_anim_impl(){
#ifdef CUST_DLINK
    //show app btn click anim for dlink
    if (app_anim_cnt < app_anim_len) {
        lv_obj_set_hidden(app_anim_img, 0);
        app_set_img_src(app_anim_img, app_anim_map[app_anim_cnt], true);
        app_anim_cnt++;
    } else if(app_anim_cnt == app_anim_len){
        if (app_anim_img != NULL) {
            lv_obj_del(app_anim_img);
            app_anim_img = NULL;
        }
        app_anim_cnt++;
    } else {
        if (app_anim_task) {
            lv_task_del(app_anim_task);
            app_anim_task = NULL;
        }
        app_anim_cnt = 0;
        app_click_impl();
    }
#else
    //set app btn to pr style for 100ms
    app_set_img_src(*main_menu_img[btn_click_index], main_menu_img_src[btn_click_index], true);
    app_click_impl();
#endif
}

void app_click_anim(lv_obj_t * btn){
    app_click = true;
#ifdef CUST_DLINK
    app_anim_cnt = 0;
    app_anim_img = lv_img_create(*main_menu_img[btn_click_index], NULL);
    lv_obj_set_size(app_anim_img, MAIN_MENU_ICON_HOR, MAIN_MENU_ICON_VER);
    lv_img_set_src(app_anim_img, app_anim_map[0]);
    lv_obj_set_hidden(app_anim_img, 1);
#endif
    app_anim_task = lv_task_create(app_click_anim_impl, 100, LV_TASK_PRIO_LOW, NULL);
#ifndef CUST_DLINK
    lv_task_once(app_anim_task);
#endif
}
#ifdef CUST_DLINK
void home_btn_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;
    //skip launcher home btn action if app click start
    if(!app_click){
        launch_home_behaviour();
    }
}
#endif

void btn_click_listener(lv_obj_t * imgbtn, lv_event_t event) {
    int id = lv_obj_get_user_data(imgbtn);
    lv_indev_t * indev = lv_indev_get_act();
    if (!indev) return;

    lv_point_t point_act;
    lv_indev_get_point(indev, &point_act);
    lv_coord_t x_diff = point_act.x - p_press.x;

    //to fix launcher slide performance issue, we hide btn while
    //user dragging to improve slide performance
    if(x_diff != 0 && event == LV_EVENT_PRESSING){
        app_btn_hidden(true);
    }else{
        app_btn_hidden(false);
    }

    if(event == LV_EVENT_PRESSED){
        lv_indev_get_point(indev, &p_press);

        //for fixing drag and click main menu issue
        //we save point when received pressed event and count
        //the x difference to point when received click event
        if(tmp_click_index < 0){
            tmp_click_index = id;
            lv_indev_get_point(indev, &point_last);
        }

    } else if (event == LV_EVENT_CLICKED) {
        x_diff = point_act.x - point_last.x;

        //x_diff > 0 means drag to right, x_diff < 0 means drag to left
        //we only do "click" when x_diff == 0
        if(x_diff != 0 || id > INDEX_COUNT){
            log_d("skip launcher main menu click x_diff:%d", x_diff);
            return;
        }
        //let user click once at a time
        if (app_click) return;
        btn_click_index = id;
        app_click_anim(imgbtn);

    } else if(event == LV_EVENT_PRESS_LOST || event == LV_EVENT_RELEASED) {
        tmp_click_index = -1;
    }
}

static void tab_slider_event_handler(lv_obj_t * tv, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        uint16_t act_id = lv_tabview_get_tab_act(tv);
        log_d("Launcher tab id: %d", act_id);

        if(act_id == 0) {
            lv_img_set_src(ind1, &ic_page_indicator_focus);
            lv_img_set_src(ind2, &ic_indicator_unfocus);
#ifdef SPEED_TEST
            lv_img_set_src(ind3, &ic_indicator_unfocus);
#endif
        } else if (act_id == 1) {
            lv_img_set_src(ind1, &ic_indicator_unfocus);
            lv_img_set_src(ind2, &ic_page_indicator_focus);
#ifdef SPEED_TEST
            lv_img_set_src(ind3, &ic_indicator_unfocus);
#endif
        } else if (act_id == 2) {
            lv_img_set_src(ind1, &ic_indicator_unfocus);
            lv_img_set_src(ind2, &ic_indicator_unfocus);
#ifdef SPEED_TEST
            lv_img_set_src(ind3, &ic_page_indicator_focus);
#endif
        }
    }
}

int tv_page_count() {
    int len = sizeof(main_menu_btn) / sizeof(lv_obj_t *);
    return ceil((float)len/ITEM_PER_TV);
}

void tv_create(void) {
    //Create Root View
    tv = lv_tabview_create(lv_scr_act(), NULL);
    lv_tabview_set_btns_hidden(tv, true);
    lv_obj_set_style(tv, &style_white_bg);
    lv_obj_set_size(tv, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_event_cb(tv, tab_slider_event_handler);
    lv_tabview_set_anim_time(tv, 0);

    int i;
    for(i = 0; i < tv_page_count(); i++) {
        lv_tabview_add_tab(tv, tv_name[i]);
    }

    lv_obj_set_pos(tv, 0, 0);
    lv_obj_align(tv, NULL, LV_ALIGN_IN_TOP_LEFT, 0, -15 * LV_RES_OFFSET);
}

//we draw launcher main menu btn, icon and text here
void app_create_impl(lv_obj_t ** tv_map[], int i){
    //check which tabview should we drawn on
    int tv = i/ITEM_PER_TV;

    if(i == 0){
        app_guide = lv_btn_create(*tv_map[tv], NULL);
#ifdef CUST_DLINK
        lv_obj_set_size(app_guide, MAIN_MENU_ICON_HOR, MAIN_MENU_ICON_VER*2);
#else
        lv_obj_set_size(app_guide, MAIN_MENU_ICON_HOR, MAIN_MENU_ICON_VER);
#endif
        lv_btn_set_style(app_guide, LV_BTN_STYLE_REL, &style_white_bg);
        lv_btn_set_style(app_guide, LV_BTN_STYLE_PR, &style_btn_pr);

        img1 = lv_img_create(*tv_map[tv], NULL);
        lv_obj_set_size(img1, MAIN_MENU_ICON_HOR, MAIN_MENU_ICON_VER);

        app_guide_txt = lv_label_create(*tv_map[i], NULL);
        lv_label_set_long_mode(*main_menu_label[i], LV_LABEL_LONG_BREAK);
        lv_label_set_align(*main_menu_label[i], LV_LABEL_ALIGN_CENTER);
        lv_obj_set_size(app_guide_txt, 84 , 50);
        lv_obj_set_style(app_guide_txt, &style_bold_font);

    } else {
        *main_menu_btn[i] = lv_btn_create(*tv_map[tv], app_guide);
        *main_menu_img[i] = lv_img_create(*tv_map[tv], img1);
        *main_menu_label[i] = lv_label_create(*tv_map[tv], app_guide_txt);
    }
    lv_obj_set_user_data(*main_menu_btn[i], main_menu_id[i]);
    lv_obj_set_event_cb(*main_menu_btn[i], btn_click_listener);
    lv_img_set_src(*main_menu_img[i], main_menu_img_src[i]);
    lv_label_set_text(*main_menu_label[i], get_string(main_label_id[i]));

    //align obj depend on if its the first drawn in tabview (i.e is far left)
    if(i%ITEM_PER_TV == 0){
        lv_obj_align(*main_menu_btn[i], NULL, LV_ALIGN_IN_TOP_LEFT, 16, 72);
        lv_obj_align(*main_menu_img[i], NULL, LV_ALIGN_IN_TOP_LEFT, 16, 72);
#ifdef CUST_DLINK
        lv_obj_align(*main_menu_label[i], *main_menu_btn[i], LV_ALIGN_OUT_BOTTOM_MID, 0, -60);
#else
        lv_obj_align(*main_menu_label[i], *main_menu_btn[i], LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
#endif
    }else{
#ifdef CUST_DLINK
        lv_obj_align(*main_menu_btn[i], *main_menu_btn[i - 1], LV_ALIGN_OUT_RIGHT_MID, 14, 0);
        lv_obj_align(*main_menu_img[i], *main_menu_img[i - 1], LV_ALIGN_OUT_RIGHT_MID, 14, 0);
        lv_obj_align(*main_menu_label[i], *main_menu_btn[i], LV_ALIGN_OUT_BOTTOM_MID, 0, -60);
#else
        lv_obj_align(*main_menu_btn[i], *main_menu_btn[i - 1], LV_ALIGN_OUT_RIGHT_MID, 14, 0);
        lv_obj_align(*main_menu_img[i], *main_menu_btn[i - 1], LV_ALIGN_OUT_RIGHT_MID, 14, 0);
        lv_obj_align(*main_menu_label[i], *main_menu_btn[i], LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
#endif
    }
}

void app_btn_hidden(bool hidden){
    int i;
    int len = sizeof(main_menu_btn) / sizeof(lv_obj_t *);

    if(lv_obj_get_hidden(* main_menu_btn[0]) == hidden){
        return;
    }
    for(i = len-1; i >= 0; i--) {
        lv_obj_set_hidden(* main_menu_btn[i], hidden);
    }
}

void apps_create_tv(lv_obj_t ** tv_map[], int i){
    *tv_map[i] = lv_tabview_get_tab(tv, i);
    lv_obj_set_size(*tv_map[i], LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_pos(*tv_map[i], 0, 0);
    lv_obj_set_style(*tv_map[i], &style_white_bg);
}

void apps_create(void) {
    int i;
    int len = sizeof(main_menu_btn) / sizeof(lv_obj_t *);
    lv_obj_t * tv1;
    lv_obj_t * tv2;
    lv_obj_t * tv3;
    lv_obj_t ** tv_map[] = {&tv1, &tv2, &tv3};

    //create tabview in order
    for(i = 0; i < tv_page_count(); i++){
        apps_create_tv(tv_map, i);
    }

    //draw main icon no1~no7 in order
    for(i = 0; i < len; i++){
        app_create_impl(tv_map, i);
    }

#if defined (CUST_ONDA) || defined(CUST_ZYXEL)
    lv_obj_set_size(app_guide_txt, 95 , 50);
    lv_obj_set_size(app_power_saving_txt, 95 , 50);
    lv_obj_set_size(app_settings_txt, 105 , 50);
    lv_obj_align(app_guide_txt, app_guide, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_obj_align(app_power_saving_txt, app_power_saving, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_obj_align(app_settings_txt, app_settings, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
#endif

    //add sms unread message number bg
    sms_num_img = lv_img_create(img4, img1);
    lv_obj_set_size(sms_num_img, 22, 22);
    lv_img_set_src(sms_num_img, &ic_notifi_badge);
    lv_obj_align(sms_num_img, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -5 * LV_RES_OFFSET, 0);

    sms_num_label = lv_label_create(sms_num_img, NULL);
    lv_label_set_align(sms_num_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_style(sms_num_label, &style_sms_num);
    lv_obj_align(sms_num_label, sms_num_img, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_hidden(sms_num_img, 1);
    lv_obj_set_hidden(sms_num_label, 1);
    update_sms_num_task = lv_task_create(update_sms_num, 1000, LV_TASK_PRIO_LOW, NULL);

#if DOWNLOAD_TEST == 1
    // 8. Download Test
    app_download_test = lv_btn_create(launcher_root_view3, app_guide);
    lv_obj_set_user_data(app_download_test, INDEX_DOWNLOADTEST);
    lv_obj_set_event_cb(app_download_test, btn_click_listener);

    img8 = lv_img_create(app_download_test, NULL);
    lv_obj_set_size(img8, MAIN_MENU_ICON_HOR, MAIN_MENU_ICON_VER);
    lv_img_set_src(img8, &ic_launcher_speedtest);
    lv_obj_align(app_download_test, app_speed_test, LV_ALIGN_IN_TOP_MID, 98 * LV_RES_OFFSET, 0 * LV_RES_OFFSET);

    app_download_test_txt = lv_label_create(launcher_root_view3, app_guide_txt);
    lv_label_set_text(app_download_test_txt, get_string(ID_LAUNCHER_DOWNLOADTEST));
    lv_obj_align(app_download_test_txt, app_speed_test_txt, LV_ALIGN_CENTER, 98 * LV_RES_OFFSET, 0 * LV_RES_OFFSET);
#endif

    data_usage_bar = lv_bar_create(lv_scr_act(), NULL);
    lv_bar_set_range(data_usage_bar, 0, 100);
    lv_bar_set_style(data_usage_bar, LV_BAR_STYLE_BG, &style_bar);
    lv_bar_set_style(data_usage_bar, LV_BAR_STYLE_INDIC, &style_indic);
    lv_obj_set_pos(data_usage_bar, 10, 10);
    lv_obj_set_size(data_usage_bar, 280, 2);
    lv_obj_align(data_usage_bar, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -30);
#ifndef FEATURE_ROUTER
    lv_bar_set_value(data_usage_bar, 40, false);
#endif

    //set data size text
    data_usage_space =  lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(data_usage_space, data_usage_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
#ifndef FEATURE_ROUTER
    lv_label_set_text(data_usage_space, "100KB");
#endif
    lv_obj_set_style(data_usage_space, &data_font_style);

    if (!ds_get_bool(DS_KEY_SHOW_DATA_USAGE_ON_HOME)
            || !ds_get_bool(DS_KEY_DATA_USAGE_MONITOR)) {
        lv_obj_set_hidden(data_usage_bar, true);
        lv_obj_set_hidden(data_usage_space, true);
    }
    update_data_usage_bar();

    //draw page indicator icon that change color while swipe tab
    ind2 = lv_img_create(lv_scr_act(), NULL);
    lv_obj_set_size(ind2, 10, 10);
    lv_img_set_src(ind2, &ic_indicator_unfocus);

    ind1 = lv_img_create(lv_scr_act(), ind2);
    lv_img_set_src(ind1, &ic_page_indicator_focus);

#ifdef SPEED_TEST
    lv_obj_align(ind2, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -42);
    lv_obj_align(ind1, ind2, LV_ALIGN_OUT_LEFT_MID, -8, 0);

    ind3 = lv_img_create(lv_scr_act(), ind1);
    lv_img_set_src(ind3, &ic_indicator_unfocus);
    lv_obj_align(ind3, ind2, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
#else
    lv_obj_align(ind1, NULL, LV_ALIGN_IN_BOTTOM_MID, -8, -42);
    lv_obj_align(ind2, NULL, LV_ALIGN_IN_BOTTOM_MID, 8, -42);
#endif

    company_logo = lv_img_create(lv_scr_act(), NULL);
    lv_obj_set_size(company_logo, 10, 10);
    lv_img_set_src(company_logo, &Zyxel_logo_2016);
    lv_obj_set_style(company_logo, &style_logo_bg);
    lv_obj_align(company_logo, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -15, -12);

    //company logo default set to hidden
#ifndef SHOW_COMPANY_LOGO
    lv_obj_set_hidden(company_logo, true);
#else
    lv_obj_set_hidden(company_logo, false);
#endif

#ifdef CUST_DLINK
    home_btn = lv_btn_create(lv_scr_act(), app_guide);
    lv_obj_set_size(home_btn, HEADER_ICON_SIZE, HEADER_ICON_SIZE);
    lv_obj_set_event_cb(home_btn, home_btn_action);
    lv_obj_align(home_btn, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -10, 0);

    home_img = lv_img_create(home_btn, NULL);
    lv_obj_set_size(home_img, 40, 30);
    lv_img_set_src(home_img, &ic_launcher_return);
#endif
}

void launcher_create(void) {
    // close popup like power menu or usb compositions in case any screen becomes
    // on top of it during screen transition
    close_popup();

    launcher_style_create();
    tv_create();
    apps_create();
    statusbar_create();
}
