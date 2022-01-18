#include "power_saving.h"
#include <stdio.h>
#include <time.h>
#include "lv_pocket_router/res/values/styles.h"
#include "lv_pocket_router/src/display/display.h"
#include "lv_pocket_router/src/power_saving/battery_optimize.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/high_speed.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/page_anim.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/power_manager.h"
#include "lv_pocket_router/src/util/util.h"

#define MAX_LISTE              5

int scn_off_options[] = { TIME_15S, TIME_30S, TIME_60S, TIME_120S, TIME_10MIN };
int dura_options[] = { TIME_10MIN, TIME_20MIN, TIME_30MIN };

typedef struct {
    char * title;
    int sec;
} time_list;

time_list list_items[MAX_LISTE];
lv_obj_t * liste_screen_off_time;
lv_obj_t * screen_off_time_label;
lv_obj_t * wifi_duration;
lv_obj_t * wifi_duration_label;
lv_obj_t * wifi_dura_enable;
lv_obj_t * wifi_dura_enable_sw;
lv_obj_t * high_speed_liste;

int time_label_len;
int select_id = -1;
int dura_select_id = -1;
lv_obj_t * liste_img[MAX_LISTE];

lv_task_t * power_saving_loading_task;
lv_obj_t * loading_mbox;
int power_saving_loading;
int btn_type;

static const int SCREEN_OFF_TIME = 0;
static const int WIFI_CLOSE_DURA = 1;

// Add compile option as brightness not support due to HW limitation
#ifdef FEATURE_BRIGHTNESS
static lv_res_t brightness_change_action(lv_obj_t * slider) {
    int brightness = lv_slider_get_value(slider);
    set_brightness(brightness);
    return LV_RES_OK;
}

lv_obj_t * lv_liste_ps_brightness(lv_obj_t * list) {
    static lv_style_t style_btn_rel;
    lv_style_copy(&style_btn_rel, &lv_style_btn_rel);
    style_btn_rel.body.main_color = LV_COLOR_WHITE;
    style_btn_rel.body.grad_color = LV_COLOR_WHITE;
    style_btn_rel.body.border.color = LV_COLOR_SILVER;
    style_btn_rel.body.border.width = 2;
    style_btn_rel.body.border.opa = LV_OPA_COVER;
    style_btn_rel.body.border.part = LV_BORDER_BOTTOM;
    style_btn_rel.body.radius = 0;

    static lv_style_t style_btn_pr;
    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.body.border.color = LV_COLOR_BASE;
    style_btn_pr.body.border.opa = LV_OPA_COVER;
    style_btn_pr.body.border.width = 3;

    static lv_style_t style_bar;
    lv_style_copy(&style_bar, &lv_style_pretty);
    style_bar.body.main_color = LV_COLOR_NOBEL;
    style_bar.body.grad_color = LV_COLOR_NOBEL;
    style_bar.body.radius = LV_RADIUS_CIRCLE;
    style_bar.body.border.color = LV_COLOR_WHITE;

    static lv_style_t style_indic;
    lv_style_copy(&style_indic, &lv_style_plain);
    style_indic.body.grad_color =  LV_COLOR_BASE;
    style_indic.body.main_color=  LV_COLOR_BASE;
    style_indic.body.opa = LV_OPA_COVER;
    style_indic.body.radius = LV_RADIUS_CIRCLE;
    style_indic.body.padding.left = 0;
    style_indic.body.padding.right = 0;
    style_indic.body.padding.top = 0;
    style_indic.body.padding.bottom = 0;

    lv_obj_t * liste;
    liste = lv_btn_create(list, NULL);
    lv_btn_set_style(liste, LV_BTN_STYLE_REL, &style_btn_rel);
    lv_btn_set_style(liste, LV_BTN_STYLE_PR, &style_btn_pr);

    lv_page_glue_obj(liste, true);
    lv_btn_set_layout(liste, LV_LAYOUT_OFF);
    lv_btn_set_fit(liste, false, true);
    lv_obj_set_protect(liste, LV_PROTECT_PRESS_LOST);
    lv_obj_set_click(liste, false);
    lv_obj_set_size(liste, LISTE_X, LISTE_Y);

    //Set brightness slider
    lv_obj_t * slider = lv_slider_create(liste, NULL);
    lv_obj_set_size(slider, 200, 10);
    lv_slider_set_range(slider, BACKLIGHT_MIN, BACKLIGHT_MAX);
    lv_slider_set_value(slider, get_brightness());
    lv_slider_set_action(slider, brightness_change_action);

    lv_bar_set_style(slider, LV_BAR_STYLE_BG, &style_bar);
    lv_bar_set_style(slider, LV_BAR_STYLE_INDIC, &style_indic);
    lv_obj_align(slider, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * img = lv_img_create(liste, NULL);
    lv_img_set_src(img, &ic_list_brightness_l);
    lv_obj_align(img, slider, LV_ALIGN_IN_LEFT_MID, -40, 0);

    lv_obj_t * img1 = lv_img_create(liste, NULL);
    lv_img_set_src(img1, &ic_list_brightness_h);
    lv_obj_set_click(img1, false);
    lv_obj_align(img1, slider, LV_ALIGN_IN_RIGHT_MID, 40, 0);
    return liste;
}
#endif

void scn_btn_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    int id = lv_obj_get_user_data(btn);
    lv_img_set_src(liste_img[select_id], &btn_list_radio_n);
    lv_img_set_src(liste_img[id], &btn_list_radio_p);
    select_id = id;
}

void clear_list_info() {
    int i;
    for (i = 0; i < MAX_LISTE; i++) {
        if (list_items[i].title != NULL) {
            lv_mem_free(list_items[i].title);
        }
    }
}

void set_list_info(time_list * list, int cnt, /*int list_info[]*/int *list_info) {
    //add list element in order
    int i;
    for (i = 0; i < cnt; i++) {
        list[i].title = lv_mem_alloc(time_label_len);
        memset(list[i].title, 0, time_label_len);
        get_time_string(list_info[i], list[i].title);
        list[i].sec = list_info[i];
    }
}

void get_time_string(int t, char * time){
    if(t >= TIME_10MIN){
        //in minutes
        sprintf(time, "%d%s", t/60/1000, get_string(ID_MINUTES));
    } else {
        //in seconds
        sprintf(time, "%d%s", t/1000, get_string(ID_SECONDS));
    }
}

void display_timeout_string(char * time) {
    int timeout = get_screen_timeout();
    get_time_string(timeout, time);
}

void lv_scn_ok_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    //show loading anim popup
    btn_type = SCREEN_OFF_TIME;
    power_saving_loading_anim(&btn_type);

    set_screen_timeout(list_items[select_id].sec);

    char time_label[time_label_len];
    memset(time_label, 0, time_label_len);
    display_timeout_string(&time_label);
    lv_label_set_text(screen_off_time_label, time_label);
    lv_liste_w_arrow_align_scroll(liste_screen_off_time, ID_PWR_SAVE_SCN_OFF_TIME, time_label);
}

void select_win_close_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;
    clear_list_info();
}

void scn_off_time_create() {
    liste_style_create();
    lv_obj_t * win = modify_list_header(lv_scr_act(), get_string(ID_PWR_SAVE_SCN_OFF_TIME),
        lv_scn_ok_action, select_win_close_action);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    memset(list_items, 0, sizeof(list_items));
    set_list_info(list_items, sizeof(scn_off_options)/sizeof(int), &scn_off_options);

    int i;
    int timeout = get_screen_timeout();
    select_id = -1;
    for (i = 0; i < sizeof(scn_off_options)/sizeof(int); i++) {
        if (timeout == list_items[i].sec) {
            select_id = i;
        }
        lv_obj_t * l = lv_liste_w_cbox(list, list_items[i].title,
            (select_id == i), scn_btn_action, i);
        liste_img[i] = lv_obj_get_child(l, NULL);
    }
}

//for screen off time action
void scn_off_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;
    scn_off_time_create();
}

//get wifi auto-close duration time in ms
int get_auto_close_dura() {
    return ds_get_int(DS_KEY_WIFI_AUTO_CLOSE_DURA);
}

void set_auto_close_dura(int value /* ms */) {
    int duration = (value > 0)? value : WIFI_DURA_DEFAULT;
    ds_set_int(DS_KEY_WIFI_AUTO_CLOSE_DURA, duration);
    wifi_close_task_refresh();
}

void display_auto_close_dura_string(char * time) {
    int duration = get_auto_close_dura();
    get_time_string(duration, time);
}

//wifi auto-close duration choosing page ok action
void lv_dur_ok_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    //show loading anim popup
    btn_type = WIFI_CLOSE_DURA;
    power_saving_loading_anim(&btn_type);

    //TODO save selected time to xml, need set to real function in the future
    set_auto_close_dura(list_items[dura_select_id].sec);

    char time_label[time_label_len];
    memset(time_label, 0, time_label_len);
    display_auto_close_dura_string(&time_label);
    lv_label_set_text(wifi_duration_label, time_label);
    lv_liste_w_arrow_align(wifi_duration_label);
}

void dura_btn_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    int id = lv_obj_get_user_data(btn);
    lv_img_set_src(liste_img[dura_select_id], &btn_list_radio_n);
    lv_img_set_src(liste_img[id], &btn_list_radio_p);
    dura_select_id = id;
}

//create wifi auto-close duration choosing page
void wifi_close_dur_create(){

    liste_style_create();

    lv_obj_t * win = modify_list_header(lv_scr_act(), get_string(ID_PWR_SAVE_AUTO_CLOSE_WIFI_DURA),
            lv_dur_ok_action, select_win_close_action);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    memset(list_items, 0, sizeof(list_items));
    set_list_info(list_items, sizeof(dura_options)/sizeof(int), &dura_options);

    int i;
    int duration = get_auto_close_dura();
    dura_select_id = -1;
    for (i = 0; i < sizeof(dura_options)/sizeof(int); i++) {
        if (duration == list_items[i].sec) {
            dura_select_id = i;
        }
        lv_obj_t * l = lv_liste_w_cbox(list, list_items[i].title,
            (dura_select_id == i), dura_btn_action, i);
        liste_img[i] = lv_obj_get_child(l, NULL);
    }
}

//for auto-close wifi action
void auto_close_wifi_action(lv_obj_t * sw, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

#ifdef CUST_SWITCH
    set_cust_switch_state(wifi_dura_enable, DS_KEY_WIFI_AUTO_CLOSE);
    bool enable = ds_get_bool(DS_KEY_WIFI_AUTO_CLOSE);
    lv_obj_set_hidden(wifi_duration, !enable);
#else
    if (lv_sw_get_state(sw)) {
        ds_set_bool(DS_KEY_WIFI_AUTO_CLOSE, true);
        lv_obj_set_hidden(wifi_duration, 0);
    } else {
        ds_set_bool(DS_KEY_WIFI_AUTO_CLOSE, false);
        lv_obj_set_hidden(wifi_duration, 1);
    }
#endif
    wifi_close_task_refresh();
}

//for wifi auto-close duration action
void wifi_close_dur_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    wifi_close_dur_create();
}

void power_saving_loading_action() {
    if (power_saving_loading <= 100) {
        update_loading_bar(loading_mbox, power_saving_loading);
        power_saving_loading = power_saving_loading + 3;
    } else {
        if (*(int *)(power_saving_loading_task->user_data) == SCREEN_OFF_TIME) {
            //may need to do something later
        } else if(*(int *)(power_saving_loading_task->user_data) == WIFI_CLOSE_DURA){
            //may need to do something later
        }
        //close task and popup when finish
        close_popup();
    }
}

void power_saving_loading_anim(int *type){
    const char *str;
    power_saving_loading = 0;

    if (*type == SCREEN_OFF_TIME){
        str = get_string(ID_PWR_SAVE_SCN_OFF_TIME);
    } else if (*type == WIFI_CLOSE_DURA){
        str = get_string(ID_PWR_SAVE_AUTO_CLOSE_WIFI_DURA);
    }
    loading_mbox = popup_anim_loading_create(str, get_string(ID_LOADING));
    power_saving_loading_task = popup_loading_task_create(power_saving_loading_action, 50, LV_TASK_PRIO_MID, type);
}

#ifdef BATTERY_OPTIMIZE_SUPPORT
void battery_opt_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    battery_optimize_create();
}
#endif

#ifdef HIGH_SPEED_SUPPORT
void high_speed_sw_action(lv_obj_t * sw, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

#ifdef CUST_SWITCH
    set_cust_switch_state(high_speed_liste, DS_KEY_HIGH_SPEED);
#else
    if (lv_sw_get_state(sw)) {
        ds_set_bool(DS_KEY_HIGH_SPEED, true);
    } else {
        ds_set_bool(DS_KEY_HIGH_SPEED, false);
    }
#endif
    high_speed_config();
}
#endif

//create power saving page
void power_saving_create(void){
    liste_style_create();

    lv_obj_t * win = default_list_header(lv_scr_act(), get_string(ID_PWR_SAVE), lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    //add list element in order
#ifdef FEATURE_BRIGHTNESS
    lv_liste_ps_brightness(list);
#endif
    //count time label length for multi-lang
    int m_len = strlen(get_string(ID_MINUTES));
    int s_len = strlen(get_string(ID_SECONDS));
    if(m_len > s_len) {
        time_label_len = m_len + MAX_TIME_INTEGER + 1;
    } else {
        time_label_len = s_len + MAX_TIME_INTEGER + 1;
    }
    char time_label[time_label_len];
    memset(time_label, 0, time_label_len);
    display_timeout_string(&time_label);
    liste_screen_off_time = lv_liste_w_arrow(list, get_string(ID_PWR_SAVE_SCN_OFF_TIME), time_label, scn_off_action);
    screen_off_time_label = lv_get_child_by_index(liste_screen_off_time, 3);

    bool enable = ds_get_bool(DS_KEY_WIFI_AUTO_CLOSE);
#ifdef CUST_SWITCH
    wifi_dura_enable = lv_liste_cust_switch(list,
            get_string(ID_PWR_SAVE_AUTO_CLOSE_WIFI), auto_close_wifi_action, enable);
#else
    wifi_dura_enable = lv_liste_w_switch(list, get_string(ID_PWR_SAVE_AUTO_CLOSE_WIFI), auto_close_wifi_action);
    wifi_dura_enable_sw = lv_obj_get_child(wifi_dura_enable, NULL);
#endif

    memset(time_label, 0, time_label_len);
    display_auto_close_dura_string(&time_label);
    wifi_duration = lv_liste_w_arrow(list, get_string(ID_PWR_SAVE_AUTO_CLOSE_WIFI_DURA), time_label, wifi_close_dur_action);
    wifi_duration_label = lv_get_child_by_index(wifi_duration, 3);
#ifdef CUST_SWITCH
    if (!enable)
        lv_obj_set_hidden(wifi_duration, 1);
#else
    if (enable) {
        lv_sw_on(wifi_dura_enable_sw, LV_ANIM_OFF);
    } else {
        lv_sw_off(wifi_dura_enable_sw, LV_ANIM_OFF);
        lv_obj_set_hidden(wifi_duration, 1);
    }
#endif

#ifdef BATTERY_OPTIMIZE_SUPPORT
    // Battery Optimize
    lv_liste_w_arrow(list, get_string(ID_BATTERY_OPTIMIZE), "", battery_opt_action);
#endif

    // High Speed
#ifdef HIGH_SPEED_SUPPORT
    enable = ds_get_bool(DS_KEY_HIGH_SPEED);
#ifdef CUST_SWITCH
    high_speed_liste = lv_liste_cust_switch(list, get_string(ID_HIGH_SPEED_FIXED_MODE),
               high_speed_sw_action, enable);
#else
    high_speed_liste = lv_liste_w_switch(list, get_string(ID_HIGH_SPEED_FIXED_MODE), high_speed_sw_action);
    lv_obj_t * high_speed_sw = lv_obj_get_child(high_speed_liste, NULL);
    if (enable) {
        lv_sw_on(high_speed_sw, LV_ANIM_OFF);
    } else {
        lv_sw_off(high_speed_sw, LV_ANIM_OFF);
    }
#endif
#endif

    //show page to page exit part
    page_anim_exit();
}
