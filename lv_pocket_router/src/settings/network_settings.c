#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/settings/network_settings.h"
#include <stdlib.h>
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/res/values/styles.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/settings/preference_network.h"
#include "lv_pocket_router/src/settings/sim_network_controller.h"

enum SEARCH_MODE_IDS {
    ID_AUTO,
    ID_MANUAL,
};

enum SEARCH_NETWORK_IDS {
    ID_TWM_4G,
    ID_TWM_3G,
    ID_T_Star,
};

enum NS_RIL_ERROR {
    ERR_4G,
    ERR_MANUAL,
    ERR_AUTO,
    ERR_SET_MANUAL,
    ERR_NETWORK_BUSY,
};

#define SEARCH_MODE_MAX_LISTE        2
#define SEARCH_NETWORK_MAX_LEN       RIL_NW_OPERATOR_NAME_MAX_LENGTH + 25

#define DS_KEY_SEARCH_MODE           "search_mode"

int search_mode_map[2] = { ID_NW_AUTO, ID_NW_MANUAL };
int search_mode_type;
int manual_search_network;
int task_loading;
int scan_result_num;
bool manual_cb_done;
static char *search_network_map[SEARCH_NETWORK_MAX_LEN];
static const ril_nw_scan_result_t *scan_result;
static lv_obj_t * search_mode_liste_img[SEARCH_MODE_MAX_LISTE];
static lv_obj_t * search_network_liste_img[RIL_NW_SCAN_LIST_MAX_LENGTH];
static lv_obj_t * register_network_task_mbox;
static lv_obj_t * search_for_network_task_mbox;
static lv_obj_t * liste_support_4g_networks;
static lv_obj_t * support_4g_networks_sw;
static lv_obj_t * liste_search_mode;
static lv_obj_t * liste_pref_network;
static lv_obj_t * search_mode_label;
static lv_obj_t * liste_airplane_mode;

void ril_error_popup(int id);
void ril_popup_close_action(lv_obj_t * mbox, lv_event_t event);
int err_popup = -1;

static int prev_sim_state = UNKNOWN;
static int blacklist_sim_file_state = -2;

//receive NS data from main nw_resp_cb, we parse data and save it in search_network_map
void manual_search_cb (ril_nw_scan_result_t* res) {
    int i;

    scan_result = res;
    scan_result_num = res->entry_len;
    char rat[15];
    char operator[10];
    char full_str[SEARCH_NETWORK_MAX_LEN];
    memset(full_str, '\0', sizeof(full_str));

    log_d("manual_search_cb_test len = %d", scan_result_num);
    for(i=0;i < scan_result_num; i++)
    {
        log_d("short name=%s ", scan_result->entry[i].operator_name.short_name);
        log_d("network status=%d ", scan_result->entry[i].network_status);
        log_d("rat=%d ", scan_result->entry[i].rat);

        memset(operator, '\0', sizeof(operator));
        strncpy(operator, scan_result->entry[i].operator_name.short_name ,sizeof(operator)-1);
        char *state = match_nw_state(scan_result->entry[i].network_status);
        memset(rat, '\0', sizeof(rat));
        snprintf(rat, sizeof(rat)-1, "%s", match_nw_rat(scan_result->entry[i].rat));
        if(scan_result->entry[i].is_home_nw){
            snprintf(full_str, sizeof(full_str)-1, "%s %s (%s)(Home)", operator, rat, state);
        }else{
            snprintf(full_str, sizeof(full_str)-1, "%s %s (%s)", operator, rat, state);
        }
        search_network_map[i] = lv_mem_alloc(sizeof(char) * SEARCH_NETWORK_MAX_LEN);
        memset(search_network_map[i], 0, sizeof(char) * SEARCH_NETWORK_MAX_LEN);
        strcpy(search_network_map[i], full_str);
        log_d("search_network_str = %s\n", search_network_map[i]);
    }
    manual_cb_done = true;
}

//to parse NW date state part
char* match_nw_state(int state){
    char* res;
    switch(state)
    {
    case RIL_NW_NETWORK_STATUS_CURRENT_SERVING_V01:
        res = get_string(ID_NW_CURRENT);
        break;
    case RIL_NW_NETWORK_STATUS_PREFERRED_V01:
        res = get_string(ID_NW_PREFERRED);
        break;
    case RIL_NW_NETWORK_STATUS_NOT_PREFERRED_V01:
        res = get_string(ID_NW_NOT_PREFERRED);
        break;
    case RIL_NW_NETWORK_STATUS_AVAILABLE_V01:
        res = get_string(ID_NW_AVAILABLE);
        break;
    case RIL_NW_NETWORK_STATUS_FORBIDDEN_V01:
        res = get_string(ID_NW_FORBIDDEN);
        break;
    default:
        res = get_string(ID_NW_UNKNOWN);
        break;
    }
    return res;
}

//to parse NW date rat part
const char* match_nw_rat(int rat){
    const char* res;
    switch(rat)
    {
    case RIL_NW_RADIO_TECH_GSM_V01:
        res = "GSM";
        break;
    case RIL_NW_RADIO_TECH_HSPAP_V01:
        res = "HSPA+";
        break;
    case RIL_NW_RADIO_TECH_LTE_V01:
        res = "LTE";
        break;
    case RIL_NW_RADIO_TECH_EHRPD_V01:
        res = "EHRPD";
        break;
    case RIL_NW_RADIO_TECH_EVDO_B_V01:
        res = "EVDO B";
        break;
    case RIL_NW_RADIO_TECH_HSPA_V01:
        res = "HSPA";
        break;
    case RIL_NW_RADIO_TECH_HSUPA_V01:
        res = "HSUPA";
        break;
    case RIL_NW_RADIO_TECH_HSDPA_V01:
        res = "HSDPA";
        break;
    case RIL_NW_RADIO_TECH_EVDO_A_V01:
        res = "EVDO A";
        break;
    case RIL_NW_RADIO_TECH_EVDO_0_V01:
        res = "EVDO 0";
        break;
    case RIL_NW_RADIO_TECH_1xRTT_V01:
        res = "1xRTT";
        break;
    case RIL_NW_RADIO_TECH_IS95B_V01:
        res = "IS95B";
        break;
    case RIL_NW_RADIO_TECH_IS95A_V01:
        res = "IS95A";
        break;
    case RIL_NW_RADIO_TECH_UMTS_V01:
        res = "UMTS";
        break;
    case RIL_NW_RADIO_TECH_EDGE_V01:
        res = "EDGE";
        break;
    case RIL_NW_RADIO_TECH_GPRS_V01:
        res = "GPRS";
        break;
    default:
        res = get_string(ID_NW_UNKNOWN);
        break;
    }
    return res;
}

/*
 * task cb function for NS-> Choose network-> Manual
 *if receive cb before timeout, will delete task early
 */
void search_for_network() {
    log_d("loading = %d, manual_cb_done = %d", task_loading, manual_cb_done);
    if (task_loading <= 100 && (!manual_cb_done)) {
        update_loading_bar(search_for_network_task_mbox, task_loading);
        task_loading = task_loading + 2;//loading bar finish in 100/2 = 50s
    } else{
#ifndef FEATURE_ROUTER // for simulator test
        ril_nw_scan_result_t res;
        res.entry_len = 2;
        strcpy(res.entry[0].operator_name.short_name, "Chunghwa");
        res.entry[0].network_status = 1;
        res.entry[0].rat = 15;
        strcpy(res.entry[1].operator_name.short_name, "T Star");
        res.entry[1].network_status = 5;
        res.entry[1].rat = 17;
        manual_search_cb(&res);
#endif
        if(manual_cb_done && (scan_result_num > 0)){
            search_for_network_create();
        }
        //close task and popup when finish
        close_popup();

        //query fail, prompt err popup
        if(scan_result_num == 0){
            static const char *btns[2];
            btns[0] = get_string(ID_OK);
            btns[1] = "";
            popup_anim_not_create(get_string(ID_NW_MANUAL_FAIL_MSG), btns, ril_popup_close_action, NULL);
        }
    }
}

//start anim popup for NS-> Choose network-> Manual
void search_for_network_task_animation(void) {
    task_loading = 0;
    search_for_network_task_mbox = popup_anim_loading_create(get_string(ID_NW_SEARCHING_FOR_NW), get_string(ID_NW_SEARCHING));
    popup_loading_task_create(search_for_network, 1000, LV_TASK_PRIO_LOWEST, NULL);
}

//task cb function for NS-> Support 4G network
void regist_the_network() {
    if (task_loading <= 100) {
        update_loading_bar(register_network_task_mbox, task_loading);
        task_loading++;
    } else {
        //close task and popup when finish
        close_popup();
    }
}

//start anim popup for NS-> Support 4G network
void regist_the_network_task_animation(void) {
    task_loading = 0;
    register_network_task_mbox = popup_anim_loading_create(get_string(ID_NW_SETTINGS), get_string(ID_NW_REGISTERED_WITH_THE_NW));
    popup_loading_task_create(regist_the_network, 50, LV_TASK_PRIO_MID, NULL);
}

//cleanup function for network setting
void network_setting_cleanup(){
    if (scan_result != NULL && scan_result_num > 0) {
        int i;
        for (i = 0; i < scan_result_num; i++){
            if(search_network_map[i] !=  NULL){
                lv_mem_free(search_network_map[i]);
                search_network_map[i] = NULL;
            }
        }
    }
}
/*
 * for "NS-> Search mode-> Choose network-> Manual-> Choose network" ok action
 */
void choose_network_ok_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    log_d("set_nw_selection_manual choose: %d", manual_search_network);
#if defined (FEATURE_ROUTER)
    ril_error_type res = set_nw_selection_manual(&scan_result->entry[manual_search_network]);
    if(res != RIL_SUCCESS) {
        //TODO Error handling
        ril_error_popup(ERR_SET_MANUAL);
        log_e(" set_nw_selection_manual failed:%d", res);
    }
#endif

    //cleanup
    network_setting_cleanup();

    //lv_win_close_event_cb(btn, LV_EVENT_RELEASED);

}

/*
 * for "NS-> Search mode-> Choose network-> Manual-> Choose network" back action
 */
void choose_network_close_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    //cleanup
    network_setting_cleanup();

    //lv_win_close_event_cb(btn, LV_EVENT_RELEASED);
}

/*
 * for "NS-> Search mode-> Choose network-> Manual-> Choose network" LISTE action
 */
void search_for_network_btn_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    int i;
    int item_id = lv_obj_get_user_data(btn);
    log_d("search_for_network_btn_action item_id: %d", item_id);

    if(!(scan_result!= NULL && scan_result_num > 0)){
        log_e("search_for_network_btn_action no content");
    }

    for (i = 0; i < scan_result_num; i++){
        lv_img_set_src(search_network_liste_img[i], &btn_list_radio_n);
    }
    lv_img_set_src(search_network_liste_img[item_id], &btn_list_radio_p);
    manual_search_network = item_id;
}

/*
 * create "NS-> Search mode-> Choose network-> Manual-> Choose network" page
 * will be called when reach timeout or manual_cb_done set to true
 * this page list the result we got from main.c nw_resp_cb()
 */
void search_for_network_create(void) {
    int i;

    liste_style_create();
    log_d("search_for_network_create enter");

    lv_obj_t * win = modify_list_header(lv_scr_act(), get_string(ID_NW_CHOOSE_NW),
            choose_network_ok_action, choose_network_close_action);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    //add list element in order
    if(scan_result_num <= 0){
        log_e("search_for_network_create no content");
        return;
    }

    lv_obj_t * liste;
    for (i = 0; i < scan_result_num; i++) {
        if(search_network_map[i] != NULL &&
                strlen(search_network_map[i]) > 0){
            liste = lv_liste_w_cbox(list, search_network_map[i],
                    i==0, search_for_network_btn_action, i);
            search_network_liste_img[i] = lv_obj_get_child(liste, NULL);
        }
    }
}

/*
 * for "NS-> Search mode-> Choose network" ok action
 * will call scan_network(Manual) or set_nw_selection_auto based(Auto)
 * on which one user choose
 */
void search_for_network_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    printf("search_for_network_action search_mode_type:%d\n", search_mode_type);
    if (search_mode_type == ID_MANUAL) {
        manual_cb_done = false;
        manual_search_network = 0;
        scan_result_num = 0;

        ds_set_int(DS_KEY_SEARCH_MODE, search_mode_type);
        search_mode_label = lv_obj_get_child(liste_search_mode, NULL);
        lv_label_set_text(search_mode_label, get_string(search_mode_map[search_mode_type]));
        lv_liste_w_arrow_align_scroll(liste_search_mode, ID_NW_SEARCH_MODE, get_string(search_mode_map[search_mode_type]));

        //Network Settings search mode Manual
#if defined (FEATURE_ROUTER)
        ril_error_type res = scan_network();
        if(res != RIL_SUCCESS) {
            //Error handling
            if(res == RIL_ERROR_NETWORK_BUSY){
                ril_error_popup(ERR_NETWORK_BUSY);
            } else {
                ril_error_popup(ERR_MANUAL);
            }
            log_e(" NS search mode Manual scan network failed:%d", res);
        } else {
            log_d(" NS search mode Manual scan network success");
            search_for_network_task_animation();
        }
#else
        search_for_network_task_animation();
#endif
    }
    if (search_mode_type == ID_AUTO) {
        ds_set_int(DS_KEY_SEARCH_MODE, search_mode_type);
        search_mode_label = lv_obj_get_child(liste_search_mode, NULL);
        lv_label_set_text(search_mode_label, get_string(search_mode_map[search_mode_type]));
        lv_liste_w_arrow_align_scroll(liste_search_mode, ID_NW_SEARCH_MODE, get_string(search_mode_map[search_mode_type]));

        //Network Settings search mode Auto
#if defined (FEATURE_ROUTER)
        ril_error_type res = set_nw_selection_auto();
        if(res != RIL_SUCCESS) {
            //Error handling
            ril_error_popup(ERR_AUTO);
            log_e("NS search mode Auto failed :%d", res);
        } else {
            log_d("NS search mode Auto success");
        }
#endif
    }
}

//for "NS-> Search mode-> Choose network" page action
void search_mode_btn_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    int i;
    int item_id = lv_obj_get_user_data(btn);
    log_d(" search_mode_btn_action item_id:%d\n", item_id);

    for (i = 0; i < SEARCH_MODE_MAX_LISTE; i++){
        lv_img_set_src(search_mode_liste_img[i], &btn_list_radio_n);
    }
    lv_img_set_src(search_mode_liste_img[item_id], &btn_list_radio_p);

    if (item_id == ID_AUTO) {
        search_mode_type = ID_AUTO;
    }
    if (item_id == ID_MANUAL) {
        search_mode_type = ID_MANUAL;
    }
    log_d(" search_mode_btn_action search_mode_type:%d\n", search_mode_type);
}

/*
 * create "NS-> Search mode-> Choose network" page
 * will display 2 liste Auto and Manual, default set to Auto
 */
void search_mode_create(void) {
    int i;
    liste_style_create();

    lv_obj_t * win = modify_list_header(lv_scr_act(), get_string(ID_NW_CHOOSE_NW),
        search_for_network_action, lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    //add list element in order
    lv_obj_t * liste;
    int search_mode = ds_get_int(DS_KEY_SEARCH_MODE);
    for (i = 0; i < SEARCH_MODE_MAX_LISTE; i++) {
        liste = lv_liste_w_cbox(list, get_string(search_mode_map[i]), search_mode==i, search_mode_btn_action, i);
        search_mode_liste_img[i] = lv_obj_get_child(liste, NULL);
    }
}

//for "NS-> Search mode" action
void search_mode_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    search_mode_create();
}

//for "NS-> Preference network" action
void preference_network_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    pref_network_create();
}

//for "NS-> Airplane mode" action
void airplane_mode_action(lv_obj_t * sw, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;
    //for device
#if defined (FEATURE_ROUTER)
    bool airplane_mode = ds_get_bool(DS_KEY_AIRPLANE_MODE);
    log_d("airplane_mode_action !airplane_mode:%d",(!airplane_mode));
    set_airplane_mode((!airplane_mode) ? AIRPLANE_MODE_ON : AIRPLANE_MODE_OFF);
    bool modem_airplane_mode = get_airplane_mode_status();
    log_d("airplane_mode_action modem_airplane_mode:%d",modem_airplane_mode);
    //compare airplane mode value between modem and data_storage if switch changes
    if (modem_airplane_mode == (!airplane_mode)) {
        //set airplane mode success
#ifdef CUST_SWITCH
        set_cust_switch_state(liste_airplane_mode, DS_KEY_AIRPLANE_MODE);
#else
        if (lv_sw_get_state(sw)) {
            ds_set_bool(DS_KEY_AIRPLANE_MODE, true);
        } else {
            ds_set_bool(DS_KEY_AIRPLANE_MODE, false);
        }
#endif
        set_airplane_mode_result_popup(true);
    } else {
        //set airplane mode fail
        //1.CUST_SWITCH case:no need call set_cust_switch_state
#ifndef CUST_SWITCH
        //2.Other SWITCH case:switch turn on or off by modem current status.
        if (modem_airplane_mode == true) {
            lv_sw_on(sw, LV_ANIM_OFF);
            ds_set_bool(DS_KEY_AIRPLANE_MODE, true);
        } else {
            lv_sw_off(sw, LV_ANIM_OFF);
            ds_set_bool(DS_KEY_AIRPLANE_MODE, false);
        }
#endif
        set_airplane_mode_result_popup(false);
    }
#else
    //for emulator
#ifdef CUST_SWITCH
    set_cust_switch_state(liste_airplane_mode, DS_KEY_AIRPLANE_MODE);
#else
    if (lv_sw_get_state(sw)) {
        ds_set_bool(DS_KEY_AIRPLANE_MODE, true);
    } else {
        ds_set_bool(DS_KEY_AIRPLANE_MODE, false);
    }
#endif
    set_airplane_mode_result_popup(true);
#endif
}

//call RIL api to enable or disable NS-> Support 4G networks
void enable_support_4g_networks(bool enable){
    ril_error_type res;
    if(enable){
        res = support_4G_network(1);
        if(res != RIL_SUCCESS) {
            //Error handling
            ril_error_popup(ERR_4G);
            log_e("NS support_4G_network() error switch on :%d", res);
        } else {
            regist_the_network_task_animation();
            log_d("NS support_4G_network() switch on success");
        }
    }else{
        res = support_4G_network(0);
        if(res != RIL_SUCCESS) {
            //Error handling
            ril_error_popup(ERR_4G);
            log_e("NS support_4G_network() error switch off :%d", res);
        } else {
            regist_the_network_task_animation();
            log_d("NS support_4G_network() switch off success");
        }
    }
}

void ril_popup_close_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    close_popup();

    if(err_popup == ERR_4G){
        log_d("Network settings ril ERR_4G");

        if (lv_sw_get_state(support_4g_networks_sw)) {
            log_d("encounter ril error, revert sw state to off");
            lv_sw_off(support_4g_networks_sw, LV_ANIM_OFF);
        } else {
            log_d("encounter ril error, revert sw state to on");
            lv_sw_on(support_4g_networks_sw, LV_ANIM_OFF);
        }
    }else if(err_popup == ERR_MANUAL){
        log_d("Network settings ril ERR_MANUAL");
    }else if(err_popup == ERR_AUTO){
        log_d("Network settings ril ERR_AUTO");
    }else if(err_popup == ERR_SET_MANUAL){
        log_d("Network settings ril ERR_SET_MANUAL");
    }else if(err_popup == ERR_NETWORK_BUSY){
        log_d("Network settings ril ERROR_NETWORK_BUSY");
    }
}

void ril_error_popup(int id){
    err_popup = id;

    int err_id = ID_RIL_ERR;
    if(id == ERR_NETWORK_BUSY){
        err_id = ID_RIL_ERR_NETWORK_BUSY;
    }
    //show digit incorrect popup
    static const char *btns[2];
    btns[1] = "";
    btns[0] = get_string(ID_OK);
    popup_anim_not_create(get_string(err_id), btns, ril_popup_close_action, NULL);

}
/*
 * for "NS-> Support 4G networks" action
 * we checked sw state and call support_4G_network() accordingly
 */
void support_4g_action(lv_obj_t * sw, lv_event_t event){

    if (event != LV_EVENT_CLICKED) return;

    log_d(" support_4g_action");

#if defined (FEATURE_ROUTER)
#ifdef CUST_SWITCH
    lv_obj_t * img = lv_obj_get_child(liste_support_4g_networks, NULL);
    if (is_4G_network_supported()){
        //disable
        lv_img_set_src(img, &ic_list_checkbox);
        enable_support_4g_networks(false);
    } else {
        //enable
        lv_img_set_src(img, &ic_list_checkbox_selected);
        enable_support_4g_networks(true);
    }
#else
    if (lv_sw_get_state(sw)) {
        //do support 4g
        enable_support_4g_networks(true);
    } else {
        //do not support 4g
        enable_support_4g_networks(false);
    }
#endif
#else
#ifndef CUST_SWITCH
    if (lv_sw_get_state(sw)) {
        //do support 4g
        regist_the_network_task_animation();
    } else {
        //do not support 4g
    }
#endif
#endif

}

//Enter Network Settings with sim
void network_settings_create(void) {
    liste_style_create();

    lv_obj_t * win = default_list_header (lv_scr_act(), get_string(ID_NW_SETTINGS), lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);
    int sim_state = get_sim_state();
    if (is_mccmnc_available(sim_state) == true) {
#ifdef NETWORK_ENABLE_CONTROLLER
    //add list element in order
    bool enable = is_4G_network_supported();
#ifdef CUST_SWITCH
    liste_support_4g_networks = lv_liste_cust_switch(list,
            get_string(ID_NW_SUPPORT_4G_NW),
            support_4g_action, enable);
#else
    liste_support_4g_networks = lv_liste_w_switch(list, get_string(ID_NW_SUPPORT_4G_NW), support_4g_action);
    support_4g_networks_sw = lv_obj_get_child(liste_support_4g_networks, NULL);
#if defined (FEATURE_ROUTER)
    if (enable) {
        log_d("NS support 4G_network on");
        lv_sw_on(support_4g_networks_sw, LV_ANIM_OFF);
    } else {
        log_d("NS support 4G_network off");
        lv_sw_off(support_4g_networks_sw, LV_ANIM_OFF);
    }
#endif
#endif

#ifndef FEATURE_ROUTER
    //default set to enable "support 4g network"
#ifdef CUST_SWITCH
    lv_obj_t * img = lv_obj_get_child(liste_support_4g_networks, NULL);
    lv_img_set_src(img, &ic_list_checkbox_selected);
#else
    lv_sw_on(support_4g_networks_sw, LV_ANIM_OFF);
#endif
#endif
#endif /* 4G_NETWORK_ENABLE */

        search_mode_type = ds_get_int(DS_KEY_SEARCH_MODE);
        liste_search_mode = lv_liste_w_arrow(list, get_string(ID_NW_SEARCH_MODE), get_string(search_mode_map[search_mode_type]), search_mode_action);

        //add preference network list element
        char pn_title[MAX_PN_TITLE_LEN];
        get_default_pref_network(&pn_title);
        liste_pref_network = lv_liste_w_arrow(list,
                get_string(ID_NW_PREFERENCE_NETWORK), pn_title, preference_network_action);
    }

    //add airplane mode list element
    bool airplane_mode_ina = false;
    if (is_mccmnc_available(sim_state) == true) {
        check_airplane_mode();
    } else {
        airplane_mode_ina = true;
    }
    bool airplane_mode_enable = ds_get_bool(DS_KEY_AIRPLANE_MODE);
    log_d("network_settings_create airplane_mode_enable:%d", airplane_mode_enable);
#ifdef CUST_SWITCH
    liste_airplane_mode = lv_liste_cust_switch(list,
            get_string(ID_NW_AIRPLANE_MODE), airplane_mode_action, airplane_mode_enable);
#else
    liste_airplane_mode = lv_liste_w_switch(list, get_string(ID_NW_AIRPLANE_MODE), airplane_mode_action);
    lv_obj_t * airplane_mode_sw = lv_obj_get_child(liste_airplane_mode, NULL);
    if (airplane_mode_enable) {
        lv_sw_on(airplane_mode_sw, LV_ANIM_OFF);
    } else {
        lv_sw_off(airplane_mode_sw, LV_ANIM_OFF);
    }
#endif
    if (is_blacklist_sim() == true) {
        lv_obj_set_hidden(liste_airplane_mode, true);
    }
    //set airplane liste to inactive state if sim not ready
    if (airplane_mode_ina) {
        lv_liste_w_sw_ina(liste_airplane_mode);
    }
}

void update_pref_network_lable(){
    lv_obj_t * pn_lable = lv_get_child_by_index(liste_pref_network, 3);
    char pn_title[MAX_PN_TITLE_LEN];
    memset(pn_title, '\0', sizeof(pn_title));
    get_default_pref_network(&pn_title);
    lv_label_set_text(pn_lable, pn_title);
    lv_liste_w_arrow_align_scroll(liste_pref_network, ID_NW_PREFERENCE_NETWORK, pn_title);
}

//Enter Network Settings with no sim
void network_settings_create_with_no_sim(void) {
    info_page_create_label_align_center(lv_scr_act(), get_string(ID_NW_SETTINGS),
            get_string(ID_NW_SIM_NOT_READY));
}

//Entry point of Network Settings
void init_network_settings(void) {
    network_settings_create();
}

void airplane_mode_result_popup_close_action(lv_obj_t * mbox, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;
    close_popup();
}

void set_airplane_mode_result_popup(bool res) {
    static const char *btns[2];
    btns[0] = get_string(ID_OK);
    btns[1] = "";
    popup_anim_not_create((res ?
            get_string(ID_NW_SET_SUCCESS) : get_string(ID_NW_SET_FAIL)),
            btns, airplane_mode_result_popup_close_action, NULL);
}
