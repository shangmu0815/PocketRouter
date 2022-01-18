/*
 * wifi.c
 *
 *  Created on: Mar 22, 2019
 *      Author: joseph
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/settings/wifi.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/wlan/hostapd_conf.h"
#include "lv_pocket_router/res/values/styles.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/res/values/string_value.h"

#define BT_TURNS_ON false
#define WITH_SIM_CARD true
lv_obj_t * liste_wifi_band;
lv_obj_t * liste_wifi_pmf;
lv_obj_t * liste_wifi_enable;
lv_obj_t * liste_hide_ssid;
lv_obj_t * liste_wifi_bandwidth;
lv_obj_t * wifi_pmf_sw;
lv_obj_t * wifi_enable_sw;
lv_obj_t * wifi_band_label;
lv_obj_t * wifi_bandwidth_label;

////////////////////////////////////////////////////////////////////////
#define WIFI_BAND_MAX_LISTE 3

/*#if defined (FEATURE_ROUTER)
lv_obj_t * wifi_band_liste;
lv_obj_t * wifi_band_select_win;
int select_wifi_band = -1;
#else*/
lv_obj_t * wifi_band_liste[WIFI_BAND_MAX_LISTE];
//#endif

enum WIFI_BAND_IDS {
    ID_WIFI_BAND_2_4_G,
    ID_WIFI_BAND_5_G,
    ID_WIFI_BAND_2_4_AND_5_G,
};

static int wifi_band_type = ID_WIFI_BAND_2_4_G;

int wifi_band_selected = -1;

int wifi_band_map[3] = { ID_WIFI_BAND_2_4GHZ,
        ID_WIFI_BAND_5GHZ, ID_WIFI_BAND_2_4GHZ_5GHZ };
lv_obj_t * wifi_band_liste_img[WIFI_BAND_MAX_LISTE];
////////////////////////////////////////////////////////////////////////

int wifi_bandwidth_map[BANDWIDTH_COUNT] = { ID_WIFI_BANDWIDTH_20MHZ,
        ID_WIFI_BANDWIDTH_20MHZ_40MHZ, ID_WIFI_BANDWIDTH_80MHZ };

lv_obj_t * wifi_bandwidth_liste_img[BANDWIDTH_COUNT];
lv_obj_t * wifi_bandwidth_liste[BANDWIDTH_COUNT];
static int wifi_bandwidth_type = WIFI_BANDWIDTH_20MHZ;
////////////////////////////////////////////////////////////////////////
void wifi_bandwidth_confirm_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    wifi_bandwidth_label = lv_obj_get_child(liste_wifi_bandwidth, NULL);
    //lv_label_set_text(wifi_bandwidth_label, get_string(wifi_bandwidth_map[wifi_bandwidth_type]));
    lv_label_set_text(wifi_bandwidth_label, "");
    lv_liste_w_arrow_align(wifi_bandwidth_label);
    write_wifi_bandwidth(wifi_band_selected, wifi_bandwidth_type);
    /*if (btn != NULL) {
        lv_obj_t * win = lv_win_get_from_btn(btn);
        lv_obj_del(win);
    }*/
}

void wifi_bandwidth_btn_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    int i;
    int item_id = lv_obj_get_user_data(btn);
    printf("wifi_bandwith_btn_action item_id:%d\n", item_id);

    int bandwidth_count = BANDWIDTH_COUNT;
    if (wifi_band_selected == WIFI_BAND_24G)
        bandwidth_count--;
    for (i = 0; i < bandwidth_count; i++){
        lv_img_set_src(wifi_bandwidth_liste_img[i], &btn_list_radio_n);

    }
    lv_img_set_src(wifi_bandwidth_liste_img[item_id], &btn_list_radio_p);
    wifi_bandwidth_type = item_id;
    printf("wifi_bandwidth_btn_action wifi_bandwidth_type:%d\n",wifi_bandwidth_type);
}

void wifi_bandwidth_create(void) {
    int i;

    liste_style_create();

    lv_obj_t * win = modify_list_header(lv_scr_act(), get_string(ID_WIFI_BANDWIDTH),
        wifi_bandwidth_confirm_action, lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    //add list element in order
    int bandwidth_count = BANDWIDTH_COUNT;
    if (wifi_band_selected == WIFI_BAND_24G)
        bandwidth_count--;
    int wifi_bandwidth = get_wifi_bandwidth(wifi_band_selected);
    for (i = 0; i < bandwidth_count; i++) {
        wifi_bandwidth_liste[i] = lv_liste_w_cbox(list, get_string(wifi_bandwidth_map[i]), wifi_bandwidth==i, wifi_bandwidth_btn_action, i);
        wifi_bandwidth_liste_img[i] = lv_obj_get_child(wifi_bandwidth_liste[i], NULL);
    }
}

void wifi_popup_comfirm_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(mbox);

    printf("wifi_popup_comfirm_action pressed: %s\n", txt);
    //lv_obj_t * mbox = lv_mbox_get_from_btn(btn);
    if (strcmp(txt, get_string(ID_CANCEL)) == 0) {
        close_popup();
    }
    if (strcmp(txt, get_string(ID_OK)) == 0) {
        //do something
        close_popup();
        wifi_band_label = lv_obj_get_child(liste_wifi_band, NULL);
        lv_label_set_text(wifi_band_label, get_string(wifi_band_map[wifi_band_type]));
        lv_liste_w_arrow_align(wifi_band_label);
        write_wifi_band(wifi_band_type);

        wifi_bandwidth_type = get_wifi_bandwidth(wifi_band_selected);
        wifi_bandwidth_label = lv_obj_get_child(liste_wifi_bandwidth, NULL);
        //lv_label_set_text(wifi_bandwidth_label, get_string(wifi_bandwidth_map[wifi_bandwidth_type]));
        lv_label_set_text(wifi_bandwidth_label, "");
        lv_liste_w_arrow_align(wifi_bandwidth_label);
    }
}

void popup_wifi_band_comfirm_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

/* NOT IN USE
    lv_obj_t * mbox = lv_mbox_get_from_btn(btn);
    if (strcmp(txt, get_string(ID_CANCEL)) == 0) { //cancel
        popup_auto_close(mbox);
        return LV_RES_OK;
    }

#if defined (FEATURE_ROUTER)
    write_wifi_band(select_wifi_band);
    lv_obj_t * label = lv_get_child_by_index(wifi_band_liste, 3);
    lv_label_set_text(label, get_string(wifi_band_map[select_wifi_band]));
    popup_auto_close(mbox);
    lv_obj_del(wifi_band_select_win);
#endif*/
}

void wifi_band_confirm_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

/*#if defined (FEATURE_ROUTER)
    int current_band = get_wifi_band();
    wifi_band_select_win = lv_win_get_from_btn(btn);
    if (current_band != select_wifi_band) {
        static const char * btns[3];
        btns[0] = get_string(ID_CANCEL);
        btns[1] = get_string(ID_OK);
        btns[2] = "";
        popup_scrl_create_impl(get_string(ID_WIFI_BAND_5GHZ_PROMPT_HEADER),
            get_string(ID_WIFI_BAND_5GHZ_PROMPT),
            btns, NULL, popup_wifi_band_comfirm_action);
    } else {
        lv_obj_del(wifi_band_select_win);
    }
#else*/
    if (get_wifi_band() != wifi_band_type) {
        log_d("wifi_band_confirm_action wifi_band_type:%d\n",wifi_band_type);
        if (wifi_band_type == ID_WIFI_BAND_2_4_G) {
            //do something
            wifi_band_label = lv_obj_get_child(liste_wifi_band, NULL);
            lv_label_set_text(wifi_band_label, get_string(wifi_band_map[wifi_band_type]));
            lv_liste_w_arrow_align(wifi_band_label);
            write_wifi_band(wifi_band_type);
            /*if (btn != NULL) {
                lv_obj_t * win = lv_win_get_from_btn(btn);
                lv_obj_del(win);
            }*/
        }
        if (wifi_band_type == ID_WIFI_BAND_5_G) {
            static const char * btns[3];
            btns[0] = get_string(ID_CANCEL);
            btns[1] = get_string(ID_OK);
            btns[2] = "";
            popup_scrl_create_impl(get_string(ID_WIFI_BAND_5GHZ_PROMPT_HEADER),
                    get_string(ID_WIFI_BAND_5GHZ_PROMPT),
                    btns, wifi_popup_comfirm_action, NULL);
        }
        if (wifi_band_type == ID_WIFI_BAND_2_4_AND_5_G) {
            //do something
            wifi_band_label = lv_obj_get_child(liste_wifi_band, NULL);
            lv_label_set_text(wifi_band_label, get_string(wifi_band_map[wifi_band_type]));
            lv_liste_w_arrow_align(wifi_band_label);
            write_wifi_band(wifi_band_type);
            /*if (btn != NULL) {
                lv_obj_t * win = lv_win_get_from_btn(btn);
                lv_obj_del(win);
            }*/
        }
    } else {
        /*if (btn != NULL) {
            lv_obj_t * win = lv_win_get_from_btn(btn);
            lv_obj_del(win);
        }*/
    }
//#endif
}

void wifi_band_btn_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    int i;
/*#if defined (FEATURE_ROUTER)
    select_wifi_band = lv_obj_get_user_data(btn);
    for (i = 0; i < WIFI_BAND_MAX_LISTE; i++) {
        if (i == select_wifi_band) {
            lv_img_set_src(wifi_band_liste_img[i], &btn_list_radio_p);
        } else {
            lv_img_set_src(wifi_band_liste_img[i], &btn_list_radio_n);
        }
    }
#else*/
    int item_id = lv_obj_get_user_data(btn);
    printf("wifi_band_btn_action item_id:%d\n", item_id);

    for (i = 0; i < WIFI_BAND_MAX_LISTE; i++){
        lv_img_set_src(wifi_band_liste_img[i], &btn_list_radio_n);
    }
    lv_img_set_src(wifi_band_liste_img[item_id], &btn_list_radio_p);
    if (item_id == ID_WIFI_BAND_2_4_G) {
        wifi_band_type = ID_WIFI_BAND_2_4_G;
    }
    if (item_id == ID_WIFI_BAND_5_G) {
        wifi_band_type = ID_WIFI_BAND_5_G;
    }
    if (item_id == ID_WIFI_BAND_2_4_AND_5_G) {
        wifi_band_type = ID_WIFI_BAND_2_4_AND_5_G;
    }
    log_d("wifi_band_btn_action wifi_band_type:%d\n",wifi_band_type);
//#endif
}

void wifi_band_close_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    //do something
    //lv_win_close_event_cb(btn, LV_EVENT_RELEASED);

    wifi_bandwidth_type = get_wifi_bandwidth(wifi_band_selected);
    wifi_bandwidth_label = lv_obj_get_child(liste_wifi_bandwidth, NULL);
    //lv_label_set_text(wifi_bandwidth_label, get_string(wifi_bandwidth_map[wifi_bandwidth_type]));
    lv_label_set_text(wifi_bandwidth_label, "");
    lv_liste_w_arrow_align(wifi_bandwidth_label);
}

void wifi_band_create(void) {
    int i;

    liste_style_create();

    lv_obj_t * win = modify_list_header(lv_scr_act(), get_string(ID_WIFI_BAND),
        wifi_band_confirm_action, wifi_band_close_action);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    //add list element in order
/*#if defined (FEATURE_ROUTER)
    int band = get_wifi_band();
    for (i = 0; i < WIFI_BAND_MAX_LISTE; i++) {
        //mapping WIFI_BAND_24G, WIFI_BAND_5G, WIFI_BAND_ALL
        lv_obj_t * liste = lv_liste_w_cbox(list, get_string(wifi_band_map[i]),
            (band == i), wifi_band_btn_action, i);
        wifi_band_liste_img[i] = lv_obj_get_child(liste, NULL);
    }
#else*/
    int wifi_band = get_wifi_band();
    for (i = 0; i < WIFI_BAND_MAX_LISTE; i++) {
        wifi_band_liste[i] = lv_liste_w_cbox(list, get_string(wifi_band_map[i]), wifi_band==i, wifi_band_btn_action, i);
        wifi_band_liste_img[i] = lv_obj_get_child(wifi_band_liste[i], NULL);
    }
//#endif
}

void wifi_enable_action(lv_obj_t * sw, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    char * key = (wifi_band_selected == WIFI_BAND_24G) ?
           DS_KEY_WIFI_24G_ENABLED : DS_KEY_WIFI_5G_ENABLED;

    bool enable_another = false;
#ifdef CUST_SWITCH
    lv_obj_t * img = lv_obj_get_child(liste_wifi_enable, NULL);
    if (ds_get_bool(key)){
        //disable
        lv_img_set_src(img, &ic_list_checkbox);
        ds_set_bool(key, false);
        write_wifi_band_enabled(wifi_band_selected, false);
        if (!ds_get_bool((key == DS_KEY_WIFI_24G_ENABLED) ? DS_KEY_WIFI_5G_ENABLED : DS_KEY_WIFI_24G_ENABLED)) {
            enable_another = true;
        }
    } else {
        //enable
        lv_img_set_src(img, &ic_list_checkbox_selected);
        ds_set_bool(key, true);
        write_wifi_band_enabled(wifi_band_selected, true);
    }
#else
    if (lv_sw_get_state(sw)) {
        ds_set_bool(key, true);
        if (!write_wifi_band_enabled(wifi_band_selected, true)) {
            lv_sw_off(wifi_enable_sw, LV_ANIM_OFF);
        }
    } else {
        ds_set_bool(key, false);
        if (!write_wifi_band_enabled(wifi_band_selected, false)) {
            lv_sw_on(wifi_enable_sw, LV_ANIM_OFF);
        }
        if (!ds_get_bool((key == DS_KEY_WIFI_24G_ENABLED) ? DS_KEY_WIFI_5G_ENABLED : DS_KEY_WIFI_24G_ENABLED)) {
            enable_another = true;
        }
    }
#endif
#if (0)
    // wifi 2.4 or 5 G must at least one enabled, so if user try to disable one
    // update another to enabled
    if (enable_another) {
        ds_set_bool((key == DS_KEY_WIFI_24G_ENABLED) ? DS_KEY_WIFI_5G_ENABLED : DS_KEY_WIFI_24G_ENABLED, true);
        write_wifi_band_enabled((wifi_band_selected == WIFI_BAND_24G) ? WIFI_BAND_5G : WIFI_BAND_24G, true);
    }
#endif
    popup_anim_not_plain_create(get_string(ID_LOADING), 3000);
}

void wifi_pmf_action(lv_obj_t * sw, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

#ifdef CUST_SWITCH
    lv_obj_t * img = lv_obj_get_child(liste_wifi_pmf, NULL);
    if (get_wlan_pmf(wifi_band_selected)){
        //disable
        if (write_wlan_pmf(wifi_band_selected, false)) {
            lv_img_set_src(img, &ic_list_checkbox);
        }
    } else {
        //enable
        if (write_wlan_pmf(wifi_band_selected, true)) {
            lv_img_set_src(img, &ic_list_checkbox_selected);
        }
    }
#else
    if (lv_sw_get_state(sw)) {
        //wifi_pmf = true;
        //do something
        if (!write_wlan_pmf(wifi_band_selected, true)) {
            lv_sw_off(wifi_pmf_sw, LV_ANIM_OFF);
        }
    } else {
        //wifi_pmf = false;
        //do something
        if (!write_wlan_pmf(wifi_band_selected, false)) {
            lv_sw_on(wifi_pmf_sw, LV_ANIM_OFF);
        }
    }
#endif
    printf("wifi_pmf_action lv_sw_get_state(sw):%d\n",lv_sw_get_state(sw));
}

void wifi_bandwidth_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    wifi_bandwidth_create();
}

void wifi_band_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    wifi_band_create();
}

void hide_ssid_action(lv_obj_t * sw, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

#ifdef CUST_SWITCH
    lv_obj_t * img = lv_obj_get_child(liste_hide_ssid, NULL);
    if (get_wlan_hide_ssid(wifi_band_selected)){
        //disable
        lv_img_set_src(img, &ic_list_checkbox);
        write_wlan_hide_ssid(wifi_band_selected, false);
    } else {
        //enable
        lv_img_set_src(img, &ic_list_checkbox_selected);
        write_wlan_hide_ssid(wifi_band_selected, true);
    }
#else
    if (lv_sw_get_state(sw)) {
        write_wlan_hide_ssid(wifi_band_selected, true);
    } else {
        write_wlan_hide_ssid(wifi_band_selected, false);
    }
#endif
}

void wifi_init(int band) {

    liste_style_create();

#ifdef SUPPORT_2.4PLUS5GHZ
    int string_id = (band == WIFI_BAND_24G) ? ID_WIFI_2_4G : ID_WIFI_5G;
#else
    int string_id = ID_WIFI;
#endif
    lv_obj_t * win = default_list_header (lv_scr_act(),
            get_string(string_id), lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

#ifdef SUPPORT_2.4PLUS5GHZ
    //add list element in order
    bool wifi_enabled = (band == WIFI_BAND_24G) ?
                         ds_get_bool(DS_KEY_WIFI_24G_ENABLED) :
                         ds_get_bool(DS_KEY_WIFI_5G_ENABLED);
#ifdef CUST_SWITCH
    liste_wifi_enable = lv_liste_cust_switch(list,
            get_string(ID_WIFI_ENABLE), wifi_enable_action, wifi_enabled);
#else
    liste_wifi_enable = lv_liste_w_switch(list, get_string(ID_WIFI_ENABLE),
            wifi_enable_action);
    wifi_enable_sw = lv_obj_get_child(liste_wifi_enable, NULL);
    if (wifi_enabled) {
        lv_sw_on(wifi_enable_sw, LV_ANIM_OFF);
    } else {
        lv_sw_off(wifi_enable_sw, LV_ANIM_OFF);
    }
#endif
#ifndef WIFI_SUPPORT
    //disable wifi enable setting
    lv_liste_w_sw_ina(liste_wifi_enable);
#endif
#ifdef HIGH_SPEED_WIFI_DOWN
    if (ds_get_bool(DS_KEY_HIGH_SPEED)) {
        lv_liste_w_sw_ina(liste_wifi_enable);
    }
#endif
#endif

    //wifi bandwidth
    wifi_bandwidth_type = get_wifi_bandwidth(band);
    liste_wifi_bandwidth = lv_liste_w_arrow(list, get_string(ID_WIFI_BANDWIDTH),
            //get_string(wifi_bandwidth_map[wifi_bandwidth_type]), wifi_bandwidth_action);
            "", wifi_bandwidth_action);

    //hide ssid
    bool hide_ssid_enable = get_wlan_hide_ssid(band);
#ifdef CUST_SWITCH
    liste_hide_ssid = lv_liste_cust_switch(list,
            get_string(ID_SSID_HIDE), hide_ssid_action, hide_ssid_enable);
#else
    liste_hide_ssid = lv_liste_w_switch(list, get_string(ID_SSID_HIDE),
                     hide_ssid_action);
    lv_obj_t * hide_ssid_sw = lv_obj_get_child(liste_hide_ssid, NULL);
    if (hide_ssid_enable) {
        lv_sw_on(hide_ssid_sw, LV_ANIM_OFF);
    } else {
        lv_sw_off(hide_ssid_sw, LV_ANIM_OFF);
    }
#endif

    //wifi pmf
    // Hide PMF if security type is WPA3
    // and remove wifi pmf for DLink
#ifndef CUST_DLINK
#ifndef CUST_ZYXEL
    //zyxel case
    //only disable wifi pmf item in wpa/wpa2 mixed mode,
    //no need to hide it
    if (strcmp(get_wlan_security(band), SECURITY_WPA3_WPA2) != 0) {
#endif
        bool pmf_enable = get_wlan_pmf(band);
#ifdef CUST_SWITCH
        liste_wifi_pmf = lv_liste_cust_switch(list,
                get_string(ID_WIFI_PMF), wifi_pmf_action, pmf_enable);
#else
        liste_wifi_pmf = lv_liste_w_switch(list, get_string(ID_WIFI_PMF),
                wifi_pmf_action);
        wifi_pmf_sw = lv_obj_get_child(liste_wifi_pmf, NULL);
        if (pmf_enable) {
            lv_sw_on(wifi_pmf_sw, LV_ANIM_OFF);
        } else {
            lv_sw_off(wifi_pmf_sw, LV_ANIM_OFF);
        }
#endif
#ifdef CUST_ZYXEL
        //zyxel case
        //only disable wifi pmf item in wpa/wpa2 mixed mode,
        //no need to hide it
        if (strcmp(get_wlan_security(band), SECURITY_WPA3_WPA2) == 0) {
            lv_liste_w_sw_ina(liste_wifi_pmf);
            lv_liste_w_scrl_txt_ina(list, get_string(ID_WIFI_INFORMATION), true);
        } else {
            lv_liste_w_scrl_txt(list, get_string(ID_WIFI_INFORMATION));
        }
#else
        //not zyxel,dlink and not wpa/wpa2 mixed mode case
        lv_liste_w_scrl_txt(list, get_string(ID_WIFI_INFORMATION));
    }
#endif
#endif
    wifi_band_selected = band;
}

