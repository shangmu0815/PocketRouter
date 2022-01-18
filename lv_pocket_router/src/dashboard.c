#include "dashboard.h"
#include <time.h>
#include "status_bar.h"
#include "status_bar_view.h"
#include "lv_pocket_router/src/battery/battery_info.h"
#include "lv_pocket_router/src/wlan/hostapd_conf.h"
#include "lv_pocket_router/src/display/display.h"
#include "lv_pocket_router/src/keyboard/basic_kb.h"
#include "lv_pocket_router/src/keyboard/num_kb_box.h"
#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/util/page_anim.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/popup_box.h"

#define DB_STATUS_BAR_SHIFT_VER 12
#define DB_STATUS_BAR_SHIFT_HOR 65
#define MAX_CONN_DEVICE         MAX_CONNECTED_USERS

//for bootup animation
//#define BOOT_ANIMATION
//#define BOOT_ANIMATION_TIME   6000
lv_obj_t * boot_anim;
static bool boot_anim_completed = false;
uint32_t delay_anim;

static lv_obj_t * root;
static lv_obj_t * tv;
//callback tabview
static lv_obj_t * tv_bc;
int pb_convert(int p);
void set_default_inner_pb(int pos, int p);

#if defined (CUST_DLINK)
lv_obj_t * data_limitation;
lv_obj_t * data_usage;
lv_obj_t * slider_time;
lv_obj_t * slider_img;
void refresh_right_dlink();
void refresh_left_dlink();
#endif

#if defined (CUST_ZYXEL)
//variables for Zyxel DB
lv_obj_t * z_data_left;
lv_obj_t * z_data_main;
lv_obj_t * z_data_main_img;
lv_obj_t * z_data_unit;
lv_obj_t * z_device_main;
lv_obj_t * z_device_unit;
lv_obj_t * z_device_connected;
lv_obj_t * z_power;
lv_obj_t * z_power_main;
lv_obj_t * z_power_unit;
//variables for Zyxel DB end
#else
//variables for default DB
//main circle
lv_obj_t * data_count;
lv_obj_t * time_label;
lv_obj_t * user_number;
lv_obj_t * device;
lv_obj_t * battery_info;
lv_obj_t * battery;
//for animation
int arc_m_cnt;
int arc_m_bg_cnt;
int arc_l_cnt;
int arc_r_cnt;
int arc_r_pb_cnt;
int arc_l_pb_cnt;
int arc_m_pb_cnt;
int oth_txt_cnt;
//count progress bar percentage
int battery_pb = 0;
int dev_count_pb = 0;
int data_count_pb = 0;

//outer circle image for dash board
#if defined (CUST_DLINK)
const void * o_cir_map[]={&ic_dash_battery_remain_1, &ic_dash_battery_remain_2, &ic_dash_battery_remain_3
        , &ic_dash_battery_remain_4, &ic_dash_battery_remain_5, &ic_dash_battery_remain_6, &ic_dash_battery_remain_7};
#else
const void * o_cir_map[]={&ic_dash_user_number_1, &ic_dash_user_number_2, &ic_dash_user_number_3
        , &ic_dash_user_number_4, &ic_dash_user_number_5, &ic_dash_user_number_6, &ic_dash_user_number_7};
#endif
//inner circle image for left and right circle
//we add ic_dash_data_count_7(no inner circle) for pb 0%
#if defined (CUST_DLINK)
const void * inner_cir_map[]={&ic_dash_data_level_12, &ic_dash_data_level_11, &ic_dash_data_level_10,
        &ic_dash_data_level_9, &ic_dash_data_level_8, &ic_dash_data_level_7, &ic_dash_data_level_6,
        &ic_dash_data_level_5, &ic_dash_data_level_4, &ic_dash_data_level_3, &ic_dash_data_level_2,
        &ic_dash_data_level_1};
#else
const void * inner_cir_map[]={&ic_dash_data_count_7, &ic_dash_data_level_1, &ic_dash_data_level_2, &ic_dash_data_level_3,
        &ic_dash_data_level_4, &ic_dash_data_level_5, &ic_dash_data_level_6, &ic_dash_data_level_7,
        &ic_dash_data_level_8, &ic_dash_data_level_9, &ic_dash_data_level_10, &ic_dash_data_level_11,
        &ic_dash_data_level_12};
#endif

//background for middle circle data count
const void * data_remain_map[]={&ic_dashboard_data_5, &ic_dashboard_data_25, &ic_dashboard_data_50,
        &ic_dashboard_data_75, &ic_dashboard_data_100};

const float data_level_interval = 8.333;//100/inner_cir_map's len
//variables for default DB end
#endif

//for display date in the bottom
lv_obj_t * data_text;
static int day = -1;
int retry = 0;

lv_obj_t * arc_m;
lv_obj_t * arc_l;
lv_obj_t * arc_r;
lv_obj_t * arc_m_pb;
lv_obj_t * arc_l_pb;
lv_obj_t * arc_r_pb;
lv_obj_t * arc_m_bg;
lv_obj_t * arc_l_bg;
lv_obj_t * arc_r_bg;
lv_obj_t * l_icon;
lv_obj_t * r_icon;
lv_obj_t * slider_text;
lv_obj_t * num_box_root;
lv_obj_t * mbox;
lv_task_t * anim_cir_task;
lv_task_t * refresh_task;
uint32_t last_db_run;
bool destory = false;

static lv_style_t style_launcher_bg;
static lv_style_t style_db_bg;
static lv_style_t bold_32_font_style;
static lv_style_t semi_16_font_style;
static lv_style_t semi_16_w_font_style;
static lv_style_t semi_22_font_style;
static lv_style_t reg_22_font_style;
static lv_style_t reg_22_font_style_warning;
static lv_style_t semi_10_font_style;
static lv_style_t style_logo_bg;
static lv_style_t black_bg;

void db_info_refresh();

//init style
void init_dashboard_style(void){
    lv_style_copy(&style_launcher_bg, &lv_style_plain);
    style_launcher_bg.body.main_color = LV_COLOR_WHITE;
    style_launcher_bg.body.grad_color = LV_COLOR_WHITE;
    style_launcher_bg.body.padding.top = 0;

    lv_style_copy(&bold_32_font_style, &lv_style_plain);
    bold_32_font_style.text.font = get_locale_font(EN, font_w_bold, font_h_32);
    bold_32_font_style.text.color = LV_COLOR_WHITE_SMOKE;
    bold_32_font_style.text.letter_space = 0;

    lv_style_copy(&semi_10_font_style, &bold_32_font_style);
    semi_10_font_style.text.font = get_font(font_w_bold, font_h_12);

    lv_style_copy(&reg_22_font_style, &bold_32_font_style);
    reg_22_font_style.text.font = get_font(font_w_regular, font_h_22);

    lv_style_copy(&reg_22_font_style_warning, &reg_22_font_style);
    reg_22_font_style_warning.text.color = LV_COLOR_RED;

    lv_style_copy(&semi_16_font_style, &lv_style_plain);
    semi_16_font_style.text.font = get_font(font_w_bold, font_h_16);
    semi_16_font_style.text.color = LV_COLOR_GREYISH_BROWN;

    lv_style_copy(&semi_16_w_font_style, &lv_style_plain);
    semi_16_w_font_style.text.font = get_font(font_w_bold, font_h_16);
#ifdef CUST_DLINK // DLink request strings in white in dashboard
    semi_16_w_font_style.text.color = LV_COLOR_WHITE;
#else
    semi_16_w_font_style.text.color = LV_COLOR_NOBEL;
#endif

    lv_style_copy(&semi_22_font_style, &semi_16_w_font_style);
    semi_22_font_style.text.font = get_font(font_w_bold, font_h_22);

    lv_style_copy(&style_db_bg, &lv_style_plain);
#if defined (CUST_DLINK)
    style_db_bg.body.main_color = LV_COLOR_DARK_GRAY;
    style_db_bg.body.grad_color = LV_COLOR_DARK_GRAY;
#else
    style_db_bg.body.main_color = LV_COLOR_BLACK;
    style_db_bg.body.grad_color = LV_COLOR_BLACK;
#endif
    style_db_bg.body.padding.top = 0;

    lv_style_copy(&style_logo_bg, &style_db_bg);
    style_logo_bg.image.color = LV_COLOR_WHITE_SMOKE;
    style_logo_bg.image.intense = LV_OPA_COVER;

    lv_style_copy(&black_bg, &lv_style_plain);
    black_bg.body.main_color = LV_COLOR_BLACK;
    black_bg.body.grad_color = LV_COLOR_BLACK;
}

//password lock page back btn action
void num_box_close_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    //if number box page close with back btn, we close everything and start dashboard again
    num_kb_box_close();

    //reload dashboard
    lv_tabview_ext_t * ext = lv_obj_get_ext_attr(tv_bc);
    if(ext->tab_cnt == 2) {
        ext->tab_cur = 1;
        lv_tabview_set_tab_act(tv_bc, 1, false);
    }

}
void delay_close_static_popup(){
    //close 30s delay popup
    close_static_popup();
    retry = 0;
    num_kb_box_close();
    num_box_root = num_kb_box_create(check_password, get_string(ID_PW_LOCK_PWD_UNLOCK), false, num_box_close_action);
}

void db_callback(lv_obj_t * tabview, uint16_t act_id){
    destory = true;
    char * key = ds_get_value(DS_KEY_PASSWORD_LOCK_VALUE);
    if (strlen(key) == 4) {
        //open password check page
        tv_bc = tabview;
        num_box_root = num_kb_box_create(check_password, get_string(ID_PW_LOCK_PWD_UNLOCK),
                false, num_box_close_action);
    } else {
        //no password lock, call launcher
        launcher_create();
        // Bug1047 remove delay to prevent static tv obj got created a new instance before
        // the old instance can be delete at delay timeup
        db_cleanup();
        //lv_task_t * clean = lv_task_create(db_cleanup, DELAY_DEL_DB, LV_TASK_PRIO_MID, NULL);
        //lv_task_once(clean);
    }
}

//dashboard static popup callback
void db_check_static_popup_cb(){
    //create warning popup, close in 30s
    mbox = popup_anim_not_create(get_string(ID_PW_LOCK_INCORRECT_5TIMES_PROMPT), NULL, NULL, NULL);
    //make 30s delay popup a static one i.e. will not be closed during suspend/resume
    set_static_popup(true);
    lv_task_t * task = lv_task_create(delay_close_static_popup, 30000, LV_TASK_PRIO_MID, NULL);
    lv_task_once(task);
}

void check_password(char * pwd){
    retry++;

    if (strcmp(pwd, ds_get_value(DS_KEY_PASSWORD_LOCK_VALUE)) == 0) {
        //pwd check success, call launcher
        launcher_create();
        num_kb_box_close();
        retry = 0;
        // Bug1047 remove delay to prevent static tv obj got created a new instance before
        // the old instance can be delete at delay timeup
        db_cleanup();
        //lv_task_t * clean = lv_task_create(db_cleanup, DELAY_DEL_DB, LV_TASK_PRIO_MID, NULL);
        //lv_task_once(clean);

    } else {
        //pwd check failed, retry
        if (retry == 5) {
            create_static_popup(db_check_static_popup_cb);
        } else {
            num_kb_box_close();
            num_box_root = num_kb_box_create(check_password, get_string(ID_PW_LOCK_PW_INCORRECT), false, num_box_close_action);
        }
    }
}

void db_destroy(){
    statusbar_reset();
    db_cleanup();
}

void db_cleanup(){
    if (tv != NULL) {
        lv_obj_del(tv);
        tv = NULL;
    }
    //close anim task
    if (anim_cir_task != NULL) {
        lv_task_del(anim_cir_task);
        anim_cir_task = NULL;
    }
    if (refresh_task != NULL) {
        lv_task_del(refresh_task);
        refresh_task = NULL;
    }
}

void db_slider_event_handler(lv_obj_t * slider, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        db_callback(slider, 0);
    }
}

//create dash board status bar
void db_statusbar_create(void) {

    //Create Root View
    tv = lv_tabview_create(lv_scr_act(), NULL);
    lv_obj_set_style(tv, &style_launcher_bg);
    //workaround for dashboard will left shift when slide from right to left issue
    lv_obj_set_size(tv, LV_HOR_RES_MAX + 30, LV_VER_RES_MAX + 70);
    lv_obj_set_event_cb(tv, db_slider_event_handler);
    lv_tabview_set_anim_time(tv, 0);
    lv_obj_set_pos(tv, -DB_STATUS_BAR_SHIFT_VER, -65);

    // add a blank tab on the left side of the real dashboard
    lv_tabview_cust_add_tab(tv, "First");

    // Tab for dash board
    root = lv_tabview_cust_add_tab(tv, "Second");
    lv_obj_align(root, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style(root, &style_db_bg);
    lv_cont_set_fit(root, LV_FIT_TIGHT);

    //create status bar root
    lv_obj_t * status_bar = lv_cont_create(root, NULL);
    lv_obj_set_size(status_bar, LV_HOR_RES_MAX, STATUS_BAR_HEIGH);
    lv_obj_set_style(status_bar, &style_db_bg);

    //create status bar
    statusbar_reset();
#ifdef CUST_DLINK // DLink request strings in white in dashboard
    create_statusbar(status_bar, LV_COLOR_WHITE, LV_COLOR_DARK_GRAY, DASHBOARD_BG);
#else
    create_statusbar(status_bar, LV_COLOR_GREYISH_BROWN, LV_COLOR_BLACK, DASHBOARD_BG);
#endif
    log_d("setup db status bar done\n");
}

void db_refresh_date_info() {
    time_t t = time(NULL);
    struct tm *timeinfo = localtime(&t);

    if (day != timeinfo->tm_mday) {
        char abbr_weekday_str[4];
        memset(abbr_weekday_str, '\0', sizeof(abbr_weekday_str));
        strftime(abbr_weekday_str, sizeof(abbr_weekday_str), "%a", timeinfo);//Mon,Tue..
        char current_date[30];
        memset(current_date, '\0', sizeof(current_date));
#if defined (CUST_ONDA)
        strftime(current_date, sizeof(current_date), "%d/%m/%Y", timeinfo);
#else
        strftime(current_date, sizeof(current_date), "%Y/%m/%d", timeinfo);
#endif
        strcat(current_date, "(");
        strcat(current_date, abbr_weekday_name(abbr_weekday_str));
        strcat(current_date, ")");
        lv_label_set_text(data_text, current_date);
#ifndef CUST_DLINK
        lv_obj_align(data_text, arc_m, LV_ALIGN_OUT_BOTTOM_MID, 0, 25);
#endif
        day = timeinfo->tm_mday;
    }
}

int update_conn_user(lv_obj_t *label){
    int users = 0;
    char str[3];
    memset(str, '\0', sizeof(str));

#if defined (FEATURE_ROUTER)
    users = get_connected_number();
#else
    users = (rand() % MAX_CONN_DEVICE);
#endif
    snprintf(str, sizeof(str), "%02d", users);
    lv_label_set_text(label, str);
    update_hotspot_user_counter();

    return users;
}

int get_battery_info_data(){
    int battery;
#if defined (FEATURE_ROUTER)
    battery = get_battery_info();
#else
    battery = (rand() % 100);
#endif

    return battery;
}


#if defined (CUST_ZYXEL)
void refresh_mid_zyxel(){
    update_conn_user(z_device_main);
    lv_obj_align(z_device_main, NULL, LV_ALIGN_CENTER, 0, -3);
    lv_obj_set_hidden(arc_m, false);
}

void refresh_left_zyxel(){
    int max = ds_get_int(DS_KEY_MAX_DATA_USAGE);
    double usage = get_data_usage();

    int remain;
    if(usage >= max){
        remain = 0;
    }else if(usage <= 0){
        remain = max;
    }else{
        remain = max - usage;
    }

    if(!ds_get_bool(DS_KEY_DATA_USAGE_MONITOR)) {
        lv_label_set_text(z_data_unit, get_string(ID_DB_UNLIMITED));
        lv_obj_set_hidden(z_data_main_img, 0);
        lv_obj_set_hidden(z_data_main, 1);
        db_set_img_src(arc_l_pb, &ic_dash_data_status_normal);
        db_set_img_src(arc_l_bg, &ic_dash_data_level05);
    }else{
        char data[10];
        memset(data, '\0', sizeof(data));
        float space_gb = 0.0;

        lv_obj_set_hidden(z_data_main_img, 1);
        lv_obj_set_hidden(z_data_main, 0);

        int unit = ds_get_int(DS_KEY_DATA_USAGE_UNIT);
        if (unit == DATA_USAGE_UNIT_MB && (remain <= 999)) {
            lv_label_set_text(z_data_unit, get_string(ID_MB));
            snprintf(data, sizeof(data), "%d", remain);
        } else {
            lv_label_set_text(z_data_unit, get_string(ID_GB));
            space_gb = (float)remain / 1024;
            snprintf(data, sizeof(data), "%.1f", space_gb);
        }
        lv_label_set_text(z_data_main, data);

        if(remain > 1126){//1.1GB
            db_set_img_src(arc_l_pb, &ic_dash_data_status_normal);
            db_set_img_src(arc_l_bg, &ic_dash_data_level05);
        } else if(remain > 500 && remain <=1126){
            db_set_img_src(arc_l_pb, &ic_dash_data_status_alert);
            db_set_img_src(arc_l_bg, &ic_dash_data_level02);
        } else{
            db_set_img_src(arc_l_pb, &ic_dash_data_status_warning);
            db_set_img_src(arc_l_bg, &ic_dash_data_level01);
        }
    }
    //arc_l_bg was set hidden in db_main_zyxel to fix showing No Data issue
    //we set it back here before updating image above
    lv_obj_set_hidden(arc_l_bg, false);
    lv_obj_align(z_data_main, arc_l, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(z_data_unit, z_data_main, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
}

void refresh_right_zyxel(){
    char data[5];

    int battery = get_battery_info_data();
    sprintf(data, "%d", battery);

    //set img for battery unit icon in the bottom of battery circle
    if(is_charging()){
        db_set_img_src(z_power_unit, &ic_dash_bettary_charging);
    }else{
        if (battery == 100) {
            db_set_img_src(z_power_unit, &ic_dash_bettary_level04);
        } else if (battery < 100 && battery >= 70) {
            db_set_img_src(z_power_unit, &ic_dash_bettary_level03);
        } else if (battery < 70  && battery >= 40) {
            db_set_img_src(z_power_unit, &ic_dash_bettary_level02);
        } else if (battery < 40  && battery >= 10) {
            db_set_img_src(z_power_unit, &ic_dash_bettary_level01);
        } else if (battery < 10) {
            db_set_img_src(z_power_unit, &ic_dash_bettary_low);
        }
    }
    //set img for otter circle and background of battery circle
    if(battery <= 20){
        db_set_img_src(arc_r_pb, &ic_dash_data_status_warning);
        db_set_img_src(arc_r_bg, &ic_dash_data_level01);
    }else if(battery > 50){
        db_set_img_src(arc_r_pb, &ic_dash_data_status_normal);
        db_set_img_src(arc_r_bg, &ic_dash_data_level05);
    }else{
        db_set_img_src(arc_r_pb, &ic_dash_data_status_alert);
        db_set_img_src(arc_r_bg, &ic_dash_data_level02);
    }
    //arc_r_bg was set hidden in db_main_zyxel to fix showing No Data issue
    //we set it back here before updating image above
    lv_obj_set_hidden(arc_r_bg, false);
    lv_label_set_text(z_power_main, data);
    lv_obj_align(z_power_main, arc_r, LV_ALIGN_CENTER, 0, -3);
    lv_obj_align(z_power_unit, z_power_main, LV_ALIGN_OUT_BOTTOM_MID, 0, -3);
}
//create customized obj for zyxel dashboard
void db_main_zyxel(){
    //draw main circle
    db_set_img_src(arc_l, &ic_dash_data_bg);
    db_set_img_src(arc_m, &ic_dash_data_bg);
    db_set_img_src(arc_r, &ic_dash_data_bg);
    lv_obj_set_hidden(arc_m_bg, 1);

    // hide bg circle before set img src for fixnig showing No Data issue
    lv_obj_set_hidden(arc_l_bg, true);
    lv_obj_set_hidden(arc_r_bg, true);

    //draw outer circle
    db_set_img_src(arc_l_pb, &ic_dash_data_status_normal);
    db_set_img_src(arc_m_pb, &ic_dash_data_status_normal);
    db_set_img_src(arc_r_pb, &ic_dash_data_status_normal);

    //draw left circle start
    z_data_main = lv_label_create(arc_l, NULL);
    lv_label_set_text(z_data_main, "");
    lv_obj_set_style(z_data_main, &bold_32_font_style);
    lv_obj_align(z_data_main, arc_l, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_align(z_data_main, LV_LABEL_ALIGN_CENTER);

    z_data_main_img = lv_img_create(arc_l, NULL);
    lv_obj_set_size(z_data_main_img, 40, 20);
    db_set_img_src(z_data_main_img, &ic_dash_data_unlimited);
    lv_obj_align(z_data_main_img, arc_l, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_hidden(z_data_main_img, 1);

    z_data_left = lv_label_create(arc_l, NULL);
    lv_obj_set_style(z_data_left, &semi_10_font_style);
    lv_label_set_text(z_data_left, get_string(ID_DB_DATA_LEFT));
    lv_obj_align(z_data_left, z_data_main, LV_ALIGN_OUT_TOP_MID, 0, 0);

    z_data_unit = lv_label_create(arc_l, z_data_left);
    lv_label_set_text(z_data_unit, get_string(ID_MB));
    lv_obj_align(z_data_unit, z_data_main, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    //draw left circle end

    //draw middle circle start
    z_device_main = lv_label_create(arc_m, z_data_main);
    lv_obj_align(z_device_main, arc_m, LV_ALIGN_CENTER, 0, -3);

    z_device_unit = lv_img_create(arc_m, z_data_left);
    lv_obj_set_size(z_device_unit, 26, 26);
    lv_obj_set_style(z_device_unit, &style_db_bg);
    lv_img_set_src(z_device_unit, &ic_dash_device_connected);
    lv_obj_align(z_device_unit, z_device_main, LV_ALIGN_OUT_BOTTOM_MID, 0, -3);

    z_device_connected = lv_label_create(arc_m, z_data_left);
    lv_label_set_text(z_device_connected, get_string(ID_DB_CONNECTED));
    lv_obj_align(z_device_connected, z_device_main, LV_ALIGN_OUT_TOP_MID, 0, 0);
    //draw middle circle end

    //draw right circle start
    z_power_main = lv_label_create(arc_r, z_data_main);
    lv_obj_align(z_power_main, arc_r, LV_ALIGN_CENTER, 0, -3);

    z_power = lv_label_create(arc_r, z_data_left);
    lv_label_set_text(z_power, get_string(ID_DB_POWER));
    lv_obj_align(z_power, z_power_main, LV_ALIGN_OUT_TOP_MID, 0, 0);

    z_power_unit = lv_img_create(arc_r, z_device_unit);
    lv_img_set_src(z_power_unit, &ic_dash_bettary_level04);
    lv_obj_align(z_power_unit, z_power_main, LV_ALIGN_OUT_BOTTOM_MID, 0, -3);
    lv_label_set_text(z_power_main, "");
    //draw right circle end

    refresh_mid_zyxel();
    refresh_left_zyxel();
    refresh_right_zyxel();
}
#elif defined (CUST_DLINK)
void db_main_dlink(){
    //draw right circle start
    data_count = lv_label_create(arc_r, NULL);
    lv_obj_set_style(data_count, &bold_32_font_style);
    lv_obj_align(data_count, NULL, LV_ALIGN_CENTER, 0, 22);
    lv_obj_set_hidden(data_count, 1);
    data_usage = lv_label_create(arc_r, data_count);
    lv_obj_align(data_usage, NULL, LV_ALIGN_CENTER, 0, -22);
    lv_obj_set_hidden(data_usage, 1);
    data_limitation = lv_img_create(arc_r, device);
    lv_obj_align(data_limitation, NULL, LV_ALIGN_CENTER, 0, -22);
    db_set_img_src(data_limitation, &ic_dashboard_data_remain);
    lv_obj_set_hidden(data_limitation, 1);
    //draw right circle end

    //draw left circle start
    battery_info = lv_label_create(arc_l, data_count);
    lv_obj_align(battery_info, NULL, LV_ALIGN_CENTER, 0, 22);
    lv_obj_set_hidden(battery_info, 1);
    battery = lv_img_create(arc_l, device);
    lv_obj_align(battery, NULL, LV_ALIGN_CENTER, 0, -22);
    lv_obj_set_hidden(battery, 1);
    //draw left circle end

    //refresh default value for all circle
    refresh_left_dlink();
    refresh_right_dlink();
    //refresh default value for all circle end
}

//both battery and data usage should have color rings,
// >60% green, 20%~59% yellow, <19% red
void set_inner_pb_dlink(int p, lv_obj_t * arc){
    if(p < 20){
        db_set_img_src(arc, &ic_dash_battery_remain_alert);
    }else if(p >= 20 && p < 60){
        db_set_img_src(arc, &ic_dash_battery_remain_normal);
    }else{
        db_set_img_src(arc, &ic_dash_battery_remain_charging);
    }
}

void refresh_left_dlink(){
    char percent[5];
    memset(percent, 0, sizeof(percent));
    int batt = get_battery_info_data();

    snprintf(percent, sizeof(percent), "%d%s", batt, "%");
    lv_label_set_text(battery_info, percent);
    lv_obj_align(battery_info, arc_l, LV_ALIGN_CENTER, 0, 22);

    //update data usage inner circle
    set_inner_pb_dlink(batt, arc_l);

    //set img for battery unit icon in the bottom of battery circle
    if(is_charging()){
        db_set_img_src(battery, &ic_dash_battery_charging);
    }else{
        if (batt <= 20) {
            db_set_img_src(battery, &ic_dash_battery_low);
        }else{
            db_set_img_src(battery, &ic_dash_battery);
        }
    }
    //to set inner circle progress bar
    battery_pb = pb_convert(batt);
    set_default_inner_pb(DB_LEFT_CIR, batt);

    lv_obj_align(battery_info, NULL, LV_ALIGN_CENTER, 0, 22);
    lv_obj_align(battery, NULL, LV_ALIGN_CENTER, 0, -22);
}

void refresh_right_dlink(){
    char total[64];
    memset(total, '\0', sizeof(total));
    char remain[5];
    memset(remain, '\0', sizeof(remain));
    int space = 0;
    float p = 0;
    float space_gb = 0.0;
    double usage = get_data_usage();
    int max = ds_get_int(DS_KEY_MAX_DATA_USAGE);

    if (usage >= max) {
        space = 0;
        p = 0;
    } else if (usage <= 0) {
        space = max;
        p = 100;
    } else {
        space = max-usage;
        p = ((float)space/max) * 100;
    }

    if (!ds_get_bool(DS_KEY_DATA_USAGE_MONITOR)) {
        //right pb stay in 100% if disable data limitation
        space = usage;
        p = 100;
        db_set_img_src(arc_r, &ic_dash_battery_remain_7);
        lv_obj_align(data_limitation, NULL, LV_ALIGN_CENTER, 0, -22);
    }else if(!ds_get_bool(DS_KEY_SHOW_DATA_USAGE_ON_HOME)) {
        p = 100;
        db_set_img_src(arc_r, &ic_dash_battery_remain_7);
        lv_obj_align(data_limitation, NULL, LV_ALIGN_CENTER, 0, 0);
    }else{
        //update data usage inner circle
        set_inner_pb_dlink(p, arc_r);
        sprintf(remain, "%.0f%s", p, "%");
        lv_label_set_text(data_usage, remain);
        lv_obj_align(data_usage, NULL, LV_ALIGN_CENTER, 0, -22);
    }

    int unit = ds_get_int(DS_KEY_DATA_USAGE_UNIT);
    if (unit == DATA_USAGE_UNIT_MB && (space <= 999)) {
        snprintf(total, sizeof(total), "%d%s", space, get_string(ID_MB));
    } else {
        space_gb = (float)space / 1024;
        snprintf(total, sizeof(total), "%.1f%s", space_gb, get_string(ID_GB));
    }
    lv_label_set_text(data_count, total);

    //to set inner circle progress bar
    data_count_pb = pb_convert(p);
    set_default_inner_pb(DB_RIGHT_CIR, p);

    lv_obj_align(data_count, NULL, LV_ALIGN_CENTER, 0, 22);
}

void anim_cir_dlink(){
    uint32_t DELAY_OTH_CIR = 350 + delay_anim;
    uint32_t DELAY_PB = 560 + delay_anim;
    //count elapsed time
    uint32_t t = lv_tick_elaps(last_db_run);
    if(t < delay_anim) return;

    //outer circle map length
    int o_cir_len = sizeof(o_cir_map) / sizeof(void *);

    if (t > DELAY_OTH_CIR) {
        lv_obj_set_hidden(arc_l, 0);
        lv_obj_set_hidden(arc_r, 0);

        if (arc_l_cnt < o_cir_len && arc_r_cnt < o_cir_len) {
            db_set_img_src(arc_l, o_cir_map[arc_l_cnt]);
            db_set_img_src(arc_r, o_cir_map[arc_r_cnt]);
            arc_l_cnt++;
            arc_r_cnt++;
        } else {
            //delay showing left and right part of the text
            if (!ds_get_bool(DS_KEY_DATA_USAGE_MONITOR)) {
                lv_obj_set_hidden(data_usage, 1);
                lv_obj_set_hidden(data_limitation, 0);
                lv_obj_set_hidden(data_count, 0);
            }else{
                if(!ds_get_bool(DS_KEY_SHOW_DATA_USAGE_ON_HOME)){
                    lv_obj_set_hidden(data_limitation, 0);
                    lv_obj_set_hidden(data_usage, 1);
                    lv_obj_set_hidden(data_count, 1);
                    lv_obj_align(data_limitation, NULL, LV_ALIGN_CENTER, 0, 0);
                }else{
                    lv_obj_set_hidden(data_limitation, 1);
                    lv_obj_set_hidden(data_usage, 0);
                    lv_obj_set_hidden(data_count, 0);
                }
            }
            lv_obj_set_hidden(battery_info, 0);
            lv_obj_set_hidden(battery, 0);

            //change task period to 100ms
            lv_task_set_period(anim_cir_task, 100);
        }
    }

    //show left and right circle progress bar animation
    if (t > DELAY_PB &&
            (!lv_obj_get_hidden(data_limitation) || !lv_obj_get_hidden(data_usage) || !lv_obj_get_hidden(data_count)) &&
            (!lv_obj_get_hidden(battery_info) && !lv_obj_get_hidden(battery))) {
        lv_obj_set_hidden(arc_l_pb, 0);
        lv_obj_set_hidden(arc_r_pb, 0);
        if (arc_r_pb_cnt <= data_count_pb) {
            db_set_img_src(arc_r_pb, inner_cir_map[arc_r_pb_cnt]);
            arc_r_pb_cnt++;
        }
        if (arc_l_pb_cnt <= battery_pb) {
            db_set_img_src(arc_l_pb, inner_cir_map[arc_l_pb_cnt]);
            arc_l_pb_cnt++;
        }
        if((arc_l_pb_cnt > battery_pb)
                && (arc_r_pb_cnt > data_count_pb)) {
            //refresh battery status/data usage again
            refresh_left_dlink();
            refresh_right_dlink();
            //stop self
            if (anim_cir_task) {
                lv_task_del(anim_cir_task);
                anim_cir_task = NULL;
            }
            //start regular data update
            db_info_refresh();
        }
    }
}

void refresh_slider_time_dlink(){
    time_t t = time(NULL);
    struct tm *timeinfo = localtime(&t);

    char current_time[9];
    strftime(current_time, sizeof(current_time), "%H:%M", timeinfo);
    lv_label_set_text(slider_time, current_time);
}

void db_slider_dlink_create(void){
    //draw slider time
    slider_time = lv_label_create(root, NULL);
    lv_obj_set_size(slider_time, 65, 27);
    lv_obj_set_style(slider_time, &bold_32_font_style);
    refresh_slider_time_dlink();

    //draw slider date
    data_text = lv_label_create(root, NULL);
    lv_obj_set_size(data_text, 320, 18);
    lv_obj_set_style(data_text, &semi_16_w_font_style);

    day = -1;
    db_refresh_date_info();

    //draw slider image
    slider_img = lv_img_create(root, NULL);
    lv_obj_set_size(slider_img, 124, 12);
    db_set_img_src(slider_img, &ic_dashboard_slide);

    lv_obj_align(slider_time, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -45);
    lv_obj_align(data_text, slider_time, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);
    lv_obj_align(slider_img, data_text, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
}
#else /* elif defined (CUST_DLINK) */
//refresh data usage percentage
void refresh_mid_default(){
    // refresh remain data usage
    char remain[5];
    int p;
    memset(remain, '\0', sizeof(remain));
    int max = ds_get_int(DS_KEY_MAX_DATA_USAGE);
    double usage = get_data_usage();

    if(usage >= max){
        p = 0;
        sprintf(remain, "%s", "0%");
    }else if((int)usage == 0){
        p = 100;
        sprintf(remain, "%s", "100%");
    }else{
        p = (float)((max - usage)/max) * 100;
        snprintf(remain, sizeof(remain), "%.0f%s", ((max - usage)/max)*100, "%");
    }
    //to set inner circle progress bar
    data_count_pb = pb_convert(p);
    set_default_inner_pb(DB_MID_CIR, p);

    lv_label_set_text(data_count,
            (ds_get_bool(DS_KEY_DATA_USAGE_MONITOR) ? remain : ""));
    lv_obj_align(data_count, arc_m, LV_ALIGN_CENTER, 0, 15);

    //check if need set data count warning color
    if(ds_get_bool(DS_KEY_DATA_USAGE_REMIND)
            && (p <= ds_get_int(DS_KEY_DATA_USAGE_REMIND_VALUE))){
        lv_obj_set_style(data_count, &reg_22_font_style_warning);
    }else{
        lv_obj_set_style(data_count, &reg_22_font_style);
    }

    //update middle circle data count background
    if(!ds_get_bool(DS_KEY_DATA_USAGE_MONITOR)) lv_obj_set_hidden(arc_m_bg, 1);
    int r = atoi(remain);
    if(r <= 5){
        db_set_img_src(arc_m_bg, data_remain_map[0]);
    }else if(r > 5 && r <= 25){
        db_set_img_src(arc_m_bg, data_remain_map[1]);
    }else if(r > 25 && r <= 50){
        db_set_img_src(arc_m_bg, data_remain_map[2]);
    }else if(r > 50 && r <= 75){
        db_set_img_src(arc_m_bg, data_remain_map[3]);
    }else{
        db_set_img_src(arc_m_bg, data_remain_map[4]);
    }
    lv_obj_align(arc_m_bg, arc_m, LV_ALIGN_CENTER, 0, 0);

    //update time label
    if (statusbar_left[INDEX_TIME].slot.con != NULL) {
            lv_label_set_text(time_label,
                lv_label_get_text(statusbar_left[INDEX_TIME].slot.con));
        lv_obj_align(time_label, arc_m, LV_ALIGN_CENTER, 0,
                (ds_get_bool(DS_KEY_DATA_USAGE_MONITOR) ? -10 : 0));
    }
}

void refresh_left_default() {
    int users = update_conn_user(user_number);
    lv_obj_align(user_number, NULL, LV_ALIGN_CENTER, 0, 15);

    //update data level background image
    int p;
    if(users > MAX_CONN_DEVICE){
        p = 100;
    }else{
        p = (float)users / MAX_CONN_DEVICE * 100.0;
    }

    //to set inner circle progress bar
    dev_count_pb = pb_convert(p);
    set_default_inner_pb(DB_LEFT_CIR, p);
}

//update battery charging icon for the right circle
void refresh_right_default_cust(int batt){
    if(is_charging()){
        db_set_img_src(battery, &ic_launcher_info_charging);
        //db_set_img_src(arc_r, &ic_dash_battery_charging);
    }else{
        if(batt < 10){
            db_set_img_src(battery, &ic_launcher_info_low);
            //db_set_img_src(arc_r, &ic_dash_battery_alert);
        }else{
            db_set_img_src(battery, &ic_launcher_info_charger);
            //db_set_img_src(arc_r, &ic_dash_data_count_7);
        }
    }
    lv_obj_align(battery, NULL, LV_ALIGN_CENTER, 0, -10);
}

void refresh_right_default() {
    char percent[5];
    memset(percent, 0, sizeof(percent));
    int battery = get_battery_info_data();

    //to set inner circle progress bar
    battery_pb = pb_convert(battery);
    set_default_inner_pb(DB_RIGHT_CIR, battery);

    snprintf(percent, sizeof(percent), "%d%s", battery, "%");
    lv_label_set_text(battery_info, percent);
    lv_obj_align(battery_info, arc_r, LV_ALIGN_CENTER, 0, 14);

    //customization UI changes
    refresh_right_default_cust(battery);

    update_battery_label();
}

//create customized obj for default dashboard
void db_main_default(){
    //draw middle circle start
    data_count = lv_label_create(arc_m, NULL);
    lv_obj_set_style(data_count, &reg_22_font_style);
    lv_obj_set_hidden(data_count, 1);

    time_label = lv_label_create(arc_m, data_count);
    lv_obj_set_hidden(time_label, 1);
    //draw middle circle end

    //draw left circle start
    user_number = lv_label_create(arc_l, data_count);
    lv_obj_set_hidden(user_number, 1);

    device = lv_img_create(arc_l, NULL);
    lv_obj_set_style(device, &style_db_bg);
    lv_obj_set_size(device, 24, 12);
    lv_img_set_src(device, &ic_launcher_info_device);
    lv_obj_align(device, NULL, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_hidden(device, 1);
    //draw left circle end

    //draw right circle start
    battery_info = lv_label_create(arc_r, data_count);
    lv_obj_set_hidden(battery_info, 1);

    battery = lv_img_create(arc_r, device);
    lv_img_set_src(battery, &ic_launcher_info_charger);
    lv_obj_align(battery, NULL, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_hidden(battery, 1);
    //draw right circle end

    //refresh default value for all circle
    refresh_mid_default();
    refresh_left_default();
    refresh_right_default();
    //refresh default value for all circle end
}

//show circles animation
void anim_cir() {
    uint32_t DELAY_M_CIR = 350 + delay_anim;
    uint32_t DELAY_OTH_CIR = 560 + delay_anim;
    uint32_t DELAY_M_TXT = 530 + delay_anim;
    uint32_t DELAY_OTH_TXT = 800 + delay_anim;
    uint32_t DELAY_PB = 1100 + delay_anim;
    //count elapsed time
    uint32_t t = lv_tick_elaps(last_db_run);
    if(t < delay_anim) return;

    //outer circle map len
    int o_cir_len = sizeof(o_cir_map) / sizeof(void *);
    int m_bg_len = sizeof(data_remain_map) / sizeof(void *);

    lv_obj_t * oth_txt_map[] = {user_number, device, battery_info, battery};
    int oth_map_len = sizeof(oth_txt_map) / sizeof(lv_obj_t *);

    if (t > DELAY_M_CIR) {
        lv_obj_set_hidden(arc_m, 0);
        if (arc_m_cnt < o_cir_len) {
            db_set_img_src(arc_m, o_cir_map[arc_m_cnt]);
            arc_m_cnt++;
        }
    }

    if (t > DELAY_M_TXT) {
        lv_obj_set_hidden(data_count, 0);
        lv_obj_set_hidden(time_label, 0);
    }

    if (t > DELAY_OTH_CIR) {
        lv_obj_set_hidden(arc_l, 0);
        lv_obj_set_hidden(arc_r, 0);

        if (arc_l_cnt < o_cir_len && arc_r_cnt < o_cir_len) {
            db_set_img_src(arc_l, o_cir_map[arc_l_cnt]);
            db_set_img_src(arc_r, o_cir_map[arc_r_cnt]);
            arc_l_cnt++;
            arc_r_cnt++;
        } else {
            //change task period to 100ms
            lv_task_set_period(anim_cir_task, 100);
        }
    }

    if (t > DELAY_OTH_TXT) {
        //draw middle circle bg
        lv_obj_set_hidden(arc_m_bg,
                (ds_get_bool(DS_KEY_DATA_USAGE_MONITOR) ? 0 : 1));
        if (arc_m_bg_cnt < m_bg_len) {
            db_set_img_src(arc_m_bg, data_remain_map[arc_m_bg_cnt]);
            arc_m_bg_cnt++;
        }
        //delay showing left and right part of the text
        if (oth_txt_cnt < oth_map_len) {
            lv_obj_set_hidden(oth_txt_map[oth_txt_cnt], 0);
            lv_obj_set_hidden(oth_txt_map[oth_txt_cnt + 1], 0);
            oth_txt_cnt = oth_txt_cnt + 2;

        }
        //show left and right circle animation part2
        if (arc_l_cnt < o_cir_len) {
            db_set_img_src(arc_l, o_cir_map[arc_l_cnt]);
            arc_l_cnt++;
        } else if (arc_r_cnt < o_cir_len) {
            db_set_img_src(arc_r, o_cir_map[arc_r_cnt]);
            arc_r_cnt++;
        }
    }

    //show left and right circle progress bar animation
    if (t > DELAY_PB) {
        lv_obj_set_hidden(arc_l_pb, 0);
        lv_obj_set_hidden(arc_r_pb, 0);
        lv_obj_set_hidden(arc_m_pb, 0);
        if (arc_r_pb_cnt <= battery_pb) {
            db_set_img_src(arc_r_pb, inner_cir_map[arc_r_pb_cnt]);
            arc_r_pb_cnt++;
        }
        if (arc_l_pb_cnt <= dev_count_pb) {
            db_set_img_src(arc_l_pb, inner_cir_map[arc_l_pb_cnt]);
            arc_l_pb_cnt++;
        }
        if (arc_m_pb_cnt <= data_count_pb) {
            db_set_img_src(arc_m_pb, inner_cir_map[arc_m_pb_cnt]);
            arc_m_pb_cnt++;
        }

        if((arc_r_pb_cnt > battery_pb)
                && (arc_l_pb_cnt > dev_count_pb)
                && (arc_m_pb_cnt > data_count_pb)) {
            //stop self
            if (anim_cir_task) {
                lv_task_del(anim_cir_task);
                anim_cir_task = NULL;
            }
            //start regular data update
            db_info_refresh();
        }
    }
}
#endif

void db_slider_create(void){
    //draw date
    data_text = lv_label_create(root, NULL);
    lv_obj_set_size(data_text, 320, 18);
    lv_obj_set_style(data_text, &semi_16_w_font_style);

    day = -1;
    db_refresh_date_info();

    //draw slider text
    slider_text =  lv_label_create(root, NULL);
    lv_obj_set_size(slider_text, 140, 34);

    lv_label_set_text(slider_text, get_string(ID_DB_SLIDE_TO_UNLOCK));
    lv_obj_set_style(slider_text, &semi_22_font_style);
    lv_obj_align(slider_text, data_text, LV_ALIGN_OUT_BOTTOM_MID, 0, 3);

    //draw left slider icon
    l_icon = lv_cont_create(root, NULL);
    lv_obj_set_size(l_icon, 18, 10);
#ifndef LV_USE_ARABIC_PERSIAN_CHARS
    lv_obj_align(l_icon, slider_text, LV_ALIGN_OUT_LEFT_MID, (is_ltr() ? -6 : 6), 0);
#else
    lv_obj_align(l_icon, slider_text, LV_ALIGN_OUT_LEFT_MID, -6, 0);
#endif
    lv_obj_set_style(l_icon, &style_db_bg);

    lv_obj_t * l_icon_img = lv_img_create(l_icon, NULL);
    lv_obj_set_size(l_icon_img, 100, 100);
    db_set_img_src(l_icon_img, &ic_dashboard_slide);
    lv_obj_align(l_icon_img, l_icon, LV_ALIGN_CENTER, 0, 0);

    //draw right slider icon
    r_icon = lv_cont_create(root, l_icon);
#ifndef LV_USE_ARABIC_PERSIAN_CHARS
    lv_obj_align(l_icon, slider_text, LV_ALIGN_OUT_RIGHT_MID, (is_ltr() ? 6 : -6), 0);
#else
    lv_obj_align(l_icon, slider_text, LV_ALIGN_OUT_RIGHT_MID, 6, 0);
#endif

    lv_obj_t * r_icon_img = lv_img_create(r_icon, NULL);
    lv_obj_set_size(r_icon_img, 100, 100);
    db_set_img_src(r_icon_img, &ic_dashboard_slide);
    lv_obj_align(r_icon_img, r_icon, LV_ALIGN_CENTER, 0, 0);

    //for adding a company logo
    lv_obj_t * company_logo = lv_img_create(root, NULL);
    db_set_img_src(company_logo, &Zyxel_logo_2016);
    lv_obj_set_style(company_logo, &style_logo_bg);
    lv_obj_align(company_logo, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -25, -12);

    //company logo default set to hidden
#ifndef SHOW_COMPANY_LOGO
    lv_obj_set_hidden(company_logo, true);
#else
    lv_obj_set_hidden(company_logo, false);
#endif
}

#if !defined (CUST_ZYXEL)
//convert progress bar percentage
//return value decide how many inner circle img will be shown for animation here
int pb_convert(int p){
    int len = sizeof(inner_cir_map) / sizeof(lv_obj_t *);
    int pb;

#if defined (CUST_DLINK)
    if(p >= 100){
        pb = 0;
    }else if(p <= 0){
        pb = len - 1;
    }else{
        pb = len - (p * (len - 1)/ 100) - 1;
    }
#else
    if(p >= 100){
        pb = len - 1;
    }else if(p <= 0){
        pb = 0;
    }else{
        pb = (p * (len - 1)/ 100) + 1;
    }
#endif
    return pb;
}

//update inner circle progress bar
void set_default_inner_pb(int pos, int p){
    //TODO we did not apply animation on data level update
    //on inner circle, may need to change later
    int pb = pb_convert(p);
    if(pos == DB_LEFT_CIR){
        db_set_img_src(arc_l_pb, inner_cir_map[pb]);
    }else if (pos == DB_MID_CIR){
        db_set_img_src(arc_m_pb, inner_cir_map[pb]);
    }else if (pos == DB_RIGHT_CIR){
        db_set_img_src(arc_r_pb, inner_cir_map[pb]);
    }
}

//Start animation task here
void db_anim_create(void){
    //hide obj because we will apply animation now
    lv_obj_set_hidden(arc_l, 1);
    lv_obj_set_hidden(arc_r, 1);
    lv_obj_set_hidden(arc_l_pb, 1);
    lv_obj_set_hidden(arc_r_pb, 1);
    arc_l_cnt = 0;
    arc_r_cnt = 0;
    arc_l_pb_cnt = 0;
    arc_r_pb_cnt = 0;
#if !defined (CUST_DLINK)
    lv_obj_set_hidden(arc_m, 1);
    lv_obj_set_hidden(arc_m_bg, 1);
    lv_obj_set_hidden(arc_m_pb, 1);
    lv_obj_set_hidden(arc_l_bg, 1);
    lv_obj_set_hidden(arc_r_bg, 1);
    arc_m_cnt = 0;
    arc_m_bg_cnt = 0;
    arc_m_pb_cnt = 0;
    oth_txt_cnt = 0;
#endif

    //set delay_anim due to dashboard create before screen turn on
    //we need to set a delay so the slider animation will show
    delay_anim = 800;

#if defined (BOOT_ANIMATION)
    if (boot_anim_completed == false) {
        delay_anim = BOOT_ANIMATION_TIME;
    }
#endif

    //reset anim elapsed time for last_task_run
    lv_anim_reset_tick();

#if !defined (CUST_DLINK)
    //set slider animation
    //time:0~133ms
    page_animate_impl(slider_text, PAGE_ANIM_FLOAT_BOTTOM, 133, delay_anim, false);
    page_animate_impl(r_icon, PAGE_ANIM_FLOAT_BOTTOM, 133, delay_anim, false);
    page_animate_impl(l_icon, PAGE_ANIM_FLOAT_BOTTOM, 133, delay_anim, false);
    //time:0~217ms
    page_animate_impl(data_text, PAGE_ANIM_FLOAT_BOTTOM, 217, delay_anim, false);

    last_db_run = lv_tick_get();
    anim_cir_task = lv_task_create(anim_cir, 30, LV_TASK_PRIO_HIGH, NULL);
#else
    //set slider animation
    //time:0~133ms
    page_animate_impl(slider_time, PAGE_ANIM_FLOAT_BOTTOM, 133, delay_anim, false);
    //time:0~217ms
    page_animate_impl(data_text, PAGE_ANIM_FLOAT_BOTTOM, 217, delay_anim, false);
    page_animate_impl(slider_img, PAGE_ANIM_FLOAT_BOTTOM, 217, delay_anim, false);
    last_db_run = lv_tick_get();
    anim_cir_task = lv_task_create(anim_cir_dlink, 30, LV_TASK_PRIO_HIGH, NULL);
#endif
}
#endif

//define items that we'll do regular update
void db_refresh_task() {
    //refresh db main circle start
#if defined (CUST_ZYXEL)
    refresh_mid_zyxel();
    refresh_left_zyxel();
    refresh_right_zyxel();
#elif defined (CUST_DLINK)
    refresh_left_dlink();
    refresh_right_dlink();
    refresh_slider_time_dlink();
#else
    refresh_mid_default();
    refresh_left_default();
    refresh_right_default();
#endif
    //refresh db main circle end

    //refresh statusbar info start
    db_refresh_date_info();
    update_unread_message();
    //refresh statusbar info end
}

//start task for regular db data update
void db_info_refresh(){
    refresh_task = lv_task_create(db_refresh_task, 2000, LV_TASK_PRIO_LOW, NULL);
}

//for UI customization
void db_set_img_src(lv_obj_t * img, const void * src_img){
    lv_img_set_src(img, src_img);
}

void db_main_create(void){
#if defined (CUST_DLINK)
    //we draw dashboard 2 basic circle for DLINK
    //draw main circle
    arc_l = lv_img_create(root, NULL);
    lv_obj_set_size(arc_l, 120, 120);
    lv_obj_align(arc_l, NULL, LV_ALIGN_IN_LEFT_MID, 25, 0);
    arc_r = lv_img_create(root, arc_l);
    lv_obj_align(arc_r, NULL, LV_ALIGN_IN_RIGHT_MID, -30, 0);
    //draw progress bar
    arc_l_pb = lv_img_create(arc_l, NULL);
    lv_obj_set_size(arc_l_pb, 120, 120);
    lv_obj_align(arc_l_pb, arc_l, LV_ALIGN_CENTER, 0, 0);
    arc_r_pb = lv_img_create(arc_r, arc_l_pb);
    lv_obj_align(arc_r_pb, arc_r, LV_ALIGN_CENTER, 0, 0);
#else
    //we draw dashboard 3 basic circle and add customization obj accordingly in the end
    //draw main circle
    arc_m = lv_img_create(root, NULL);
    lv_obj_set_size(arc_m, 100, 100);
    lv_obj_align(arc_m, root, LV_ALIGN_CENTER, 0, -12);
    arc_l = lv_img_create(root, arc_m);
    lv_obj_align(arc_l, arc_m, LV_ALIGN_OUT_LEFT_MID, 0, 0);
    arc_r = lv_img_create(root, arc_m);
    lv_obj_align(arc_r, arc_m, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    //draw progress bar
    arc_l_pb = lv_img_create(arc_l, NULL);
    lv_obj_set_size(arc_l_pb, 100, 100);
    lv_obj_align(arc_l_pb, arc_l, LV_ALIGN_CENTER, 0, 1);
    arc_r_pb = lv_img_create(arc_r, arc_l_pb);
    lv_obj_align(arc_r_pb, arc_r, LV_ALIGN_CENTER, 0, 1);
    arc_m_pb = lv_img_create(arc_m, arc_l_pb);
    lv_obj_align(arc_m_pb, arc_m, LV_ALIGN_CENTER, 0, 1);
    //draw circle background
    arc_m_bg = lv_img_create(arc_m, NULL);
    lv_obj_set_size(arc_m_bg, 100, 100);
    lv_obj_align(arc_m_bg, arc_m, LV_ALIGN_CENTER, 0, 0);
    arc_l_bg = lv_img_create(arc_l, arc_m_bg);
    lv_obj_align(arc_l_bg, arc_l, LV_ALIGN_CENTER, 0, 0);
    arc_r_bg = lv_img_create(arc_r, arc_m_bg);
    lv_obj_align(arc_r_bg, arc_r, LV_ALIGN_CENTER, 0, 0);
#endif

#if defined (CUST_ZYXEL)
    //draw circle info
    db_main_zyxel();
    //start regular data update
    db_info_refresh();
#elif defined (CUST_DLINK)
    db_main_dlink();
#else
    db_main_default();
#endif
}

/* called when BOOT_ANIMATION defined
 * bootup animation choice of images as below:
 * bootup_3/bootup_dlink/bootup_zyxel
 */
#if defined (BOOT_ANIMATION)
void bootup_anim(){
    log_d("db boot animation start");

    boot_anim = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(boot_anim, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_t * img = lv_img_create(boot_anim, NULL);
    lv_obj_set_size(img, LV_HOR_RES_MAX, LV_VER_RES_MAX);

#ifdef CUST_DLINK
    lv_img_set_src(img, &bootup_dlink);
#elif defined (CUST_ZYXEL)
    lv_img_set_src(img, &bootup_zyxel);
    //need to set black background for 3 operator
    //lv_img_set_src(img, &bootup_3);
    //lv_obj_set_style(boot_anim, &black_bg);
#endif

    lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_task_t * clean = lv_task_create(boot_anim_cleanup, BOOT_ANIMATION_TIME, LV_TASK_PRIO_LOW, NULL);
    lv_task_once(clean);
}

void boot_anim_cleanup(){
    if(boot_anim != NULL){
        log_d("db boot animation finish");
        lv_obj_del(boot_anim);
        boot_anim = NULL;
    }
}
#endif

void dashboard_create(void){

    destory = false;
    init_dashboard_style();
    db_statusbar_create();
    db_main_create();
#if defined (CUST_DLINK)
    db_slider_dlink_create();
#else
    db_slider_create();
#endif
#if defined (BOOT_ANIMATION)
    if (boot_anim_completed == false) {
        boot_anim_completed = true;
        bootup_anim();
    }
#endif

#if defined (CUST_ZYXEL)
    //will not run animation in zyxel
#else
    //we trigger db_info_refresh inside anim_cir when it finish
    db_anim_create();
#endif

    //check if there's any static popup need to be moved to foreground
    move_static_popup_to_foreground();
}

const char* abbr_weekday_name(char* weekday_name) {
    const char* res = "";
    if (weekday_name != NULL) {
        if (strcmp(weekday_name, "Mon") == 0) {
            res = get_string(ID_DATE_MON);
        } else if (strcmp(weekday_name, "Tue") == 0) {
            res = get_string(ID_DATE_TUE);
        } else if (strcmp(weekday_name, "Wed") == 0) {
            res = get_string(ID_DATE_WED);
        } else if (strcmp(weekday_name, "Thu") == 0) {
            res = get_string(ID_DATE_THU);
        } else if (strcmp(weekday_name, "Fri") == 0) {
            res = get_string(ID_DATE_FRI);
        } else if (strcmp(weekday_name, "Sat") == 0) {
            res = get_string(ID_DATE_SAT);
        } else if (strcmp(weekday_name, "Sun") == 0) {
            res = get_string(ID_DATE_SUN);
        }
    }
    return res;
}
