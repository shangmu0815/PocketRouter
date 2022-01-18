#include "wps.h"
#include <stdio.h>
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/wlan/hostapd_conf.h"
#include "lv_pocket_router/src/keyboard/num_kb_col.h"
#include "lv_pocket_router/src/keyboard/basic_kb.h"
#include "lv_pocket_router/res/values/string_value.h"

#define FOUR_DIGITS                     4
#define EIGHT_DIGITS                    8
#define INTERVAL_MSEC                   2000
#define MAX_TASK_COUNT                  (120000 / INTERVAL_MSEC)
static const int WPS_CONN_TIMEOUT = 120;
static const int WPS_CONN_POPUP_TIMEOUT = 3000;
static uint32_t last_popup_run;

lv_obj_t * liste_wps_enable;
lv_task_t * wps_task;
lv_task_t * wps_popup_task;
lv_obj_t * mbox;
lv_obj_t * root_view;
lv_obj_t * wps_enable_sw;
lv_obj_t * liste_wps_config;

int startUsers = 0;
int taskCount = 0;

//callback func from popup after close_popup() 
void wps_cb_destroy() {
    if (wps_task != NULL) {
        lv_task_del(wps_task);
        wps_task = NULL;
    }
    if (wps_popup_task != NULL) {
        lv_task_del(wps_popup_task);
        wps_popup_task = NULL;
    }
}

void wps_destroy() {
    wps_cb_destroy();
    close_popup();
}

//wps connection count down time popup
void wps_connect_task() {
    if (taskCount >= MAX_TASK_COUNT || startUsers < get_connected_number()) {
        wps_destroy();
        if (startUsers < get_connected_number()) {
            mbox = popup_anim_not_plain_create(get_string(ID_CONN_GUIDE_WPS_CONN_SUCCESS), WPS_CONN_POPUP_TIMEOUT);
            set_popup_cb(wps_cb_destroy);
        }
    }
    taskCount++;
}

//show wps connection out of time popup
lv_res_t wps_conn_oot_action() {
    wps_destroy();

    //close the popup in 3000 ms for now
    mbox = popup_anim_not_plain_create(get_string(ID_CONN_GUIDE_WPS_CONN_OOT), WPS_CONN_POPUP_TIMEOUT);
    set_popup_cb(wps_cb_destroy);

    return LV_RES_OK;
}

void wps_close_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    wps_destroy();
}

void wps_pin_action(lv_obj_t * btnm, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * input = num_col_get_lable();
    uint32_t index = lv_obj_get_user_data(btnm);
    if (index == KB_LEFT_BTN){
        num_col_close_win();

    } else if (index == KB_RIGHT_BTN){
        if(get_tick_btn_ina_state(get_kb_tick_btn(root_view))) return;

        int length = strlen(input);
        if(length == FOUR_DIGITS || length == EIGHT_DIGITS){
            //length check ok, start wps conn task
            wps_conn_create(input);
            num_col_close_win();
        } else {
            //show digit incorrect popup
            static const char *btns[2];
            btns[1] = "";
            btns[0] = get_string(ID_OK);
            popup_anim_not_create(get_string(ID_CONN_GUIDE_WPS_LEN_ERR_PROMPT), btns, wps_close_action, NULL);
        }
    }
}

void est_wps_conn_popup_task(){
    int32_t t = lv_tick_elaps(last_popup_run);
    char info[128];

    int32_t time = WPS_CONN_TIMEOUT - (t/1000);
    if (time > 0) {
        reset_screen_timeout();
        sprintf(info,"%s\n%s %d%s", get_string(ID_CONN_GUIDE_WPS_CONN_NOTE),
                get_string(ID_CONN_GUIDE_WPS_CONN_TIME), time, get_string(ID_SECONDS));
        popup_scrl_update(info);
    } else {
        wps_conn_oot_action();
    }
}

//show wps conn popup, update time remain every second
void est_wps_conn_popup(){
    static const char *btns[2];
    char info[128];
    btns[0] = get_string(ID_CANCEL);
    btns[1] = "";

    sprintf(info,"%s\n%s %d%s", get_string(ID_CONN_GUIDE_WPS_CONN_NOTE),
            get_string(ID_CONN_GUIDE_WPS_CONN_TIME), WPS_CONN_TIMEOUT, get_string(ID_SECONDS));
    last_popup_run = lv_tick_get();
    mbox = popup_scrl_create(get_string(ID_CONN_GUIDE_WPS_CONN), info, btns, wps_close_action);
    set_popup_cb(wps_cb_destroy);

    wps_popup_task = lv_task_create(est_wps_conn_popup_task, 1000, LV_TASK_PRIO_LOW, NULL);
}

//for wps btn
void wps_conn_create(const char * input) {
    est_wps_conn_popup();
    startUsers = get_connected_number();
    taskCount = 0;
    start_wps(input);
    wps_task = lv_task_create(wps_connect_task, INTERVAL_MSEC, LV_TASK_PRIO_HIGH, NULL);
}

//for wps pin btn
void wps_conn_pin_create(void) {
    root_view = num_col_create(get_string(ID_CONN_GUIDE_WPS_PIN_KB_HEADER), false, wps_pin_action);
    num_kb_set_tip(get_string(ID_KB_4OR8_DIGIT_TIP));
    num_kb_col_set_lable_len(EIGHT_DIGITS);
}

//wps_create page btnm action
void wps_action(lv_obj_t * btnm, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(btnm);
    if(strcmp(txt, get_string(ID_WPS)) == 0) {
        //WPS btn action
        wps_conn_create(NULL);
    } else if (strcmp(txt, get_string(ID_CONN_GUIDE_WPS_PIN)) == 0) {
        //WPS PIN btn action
        wps_conn_pin_create();
    }
}

void wps_enable_action(lv_obj_t * sw, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

#ifdef CUST_SWITCH
    lv_obj_t * img = lv_obj_get_child(liste_wps_enable, NULL);
    if (get_wlan_wps_state()){
        //disable
        if (write_wlan_wps_state(false)) {
            lv_img_set_src(img, &ic_list_checkbox);
        }
        lv_obj_set_hidden(liste_wps_config, true);
    } else {
        //enable
        if (write_wlan_wps_state(true)) {
            lv_img_set_src(img, &ic_list_checkbox_selected);
        }
        lv_obj_set_hidden(liste_wps_config, false);
    }
#else
    if (lv_sw_get_state(sw)) {
        if (!write_wlan_wps_state(true)) {
            lv_sw_off(wps_enable_sw, LV_ANIM_OFF);
        }
        lv_obj_set_hidden(liste_wps_config, false);
    } else {
        if (!write_wlan_wps_state(false)) {
            lv_sw_on(wps_enable_sw, LV_ANIM_OFF);
        }
        lv_obj_set_hidden(liste_wps_config, true);
    }
#endif
}

void wps_config_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    wps_connect_create();
}

void wps_connect_create(void) {
    static const char *btns[3];
    btns[0] = get_string(ID_WPS);
    btns[1] = get_string(ID_CONN_GUIDE_WPS_PIN);
    btns[2] = "";
    if (wps_support()) {
        info_page_create_btmn(lv_scr_act(), get_string(ID_WPS),
                get_string(ID_CONN_GUIDE_WPS), btns, wps_action);
    } else {
        info_page_create(lv_scr_act(), get_string(ID_CONN_GUIDE_WPS_CANNOT_USE_TITLE),
                get_string(ID_CONN_GUIDE_WPS_CANNOT_USE_PROMPT));
    }
}

//entering point for wps function
void wps_create(void) {
    lv_obj_t * win = default_list_header (lv_scr_act(),
                             get_string(ID_WPS), lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    bool enable = get_wlan_wps_state();
#ifdef CUST_SWITCH
    liste_wps_enable = lv_liste_cust_switch(list,
            get_string(ID_CONN_GUIDE_WPS_ENABLE), wps_enable_action, enable);
#else
    liste_wps_enable = lv_liste_w_switch(list, get_string(ID_CONN_GUIDE_WPS_ENABLE),
                                         wps_enable_action);
    wps_enable_sw = lv_obj_get_child(liste_wps_enable, NULL);
#endif
    liste_wps_config = lv_liste_w_arrow(list, get_string(ID_CONN_GUIDE_WPS_CONFIG),
                                        "", wps_config_action);

#ifdef CUST_SWITCH
    if (!enable)
        lv_obj_set_hidden(liste_wps_config, true);
#else
    if (get_wlan_wps_state()) {
        lv_sw_on(wps_enable_sw, LV_ANIM_OFF);
    } else {
        lv_sw_off(wps_enable_sw, LV_ANIM_OFF);
        lv_obj_set_hidden(liste_wps_config, true);
    }
#endif
}
