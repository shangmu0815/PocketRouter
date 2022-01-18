/*
 * bluetooth_settings.c
 *
 *  Created on: Apr 9, 2019
 *      Author: joseph
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/settings/bluetooth_settings.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/res/values/string_value.h"

static lv_obj_t * liste1 = NULL;
static lv_obj_t * liste2 = NULL;
static lv_obj_t * bluetooth_btnm1 = NULL;


static char bluetooth_name_info[50];
static char bluetooth_mac_address_info[50];
static lv_list_selector_t device_selector_info;

static lv_style_t style_btn_bg;
static lv_style_t style_btn_rel;
static lv_style_t style_btn_pr;

lv_obj_t * bluetooth_settings_sw;

enum SSID_IDS {
    ITEMS_ID_SUB_BLUETOOTH_SETTINGS,
    ITEMS_ID_SUB_BLUETOOTH_NAME,
    ITEMS_ID_SUB_BLUETOOTH_MAC_ADDRESS,
};

void bluetooth_settings_sw_release_action(lv_obj_t * sw, lv_event_cb_t event_cb)
{
    if (event_cb != LV_EVENT_CLICKED) return;

    if (lv_sw_get_state(sw) == false) {
        if (liste1 != NULL && liste2 != NULL
                && bluetooth_btnm1 != NULL) {
            ds_set_bool(DS_KEY_BLUETOOTH_SETTINGS, false);
            lv_sw_off(bluetooth_settings_sw, LV_ANIM_OFF);
            lv_obj_set_hidden(liste1, true);
            lv_obj_set_hidden(liste2, true);
            lv_obj_set_hidden(bluetooth_btnm1, true);
        }
    } else {
        //bluetooth_settings_switch == true
        if (liste1 != NULL && liste2 != NULL
                && bluetooth_btnm1 != NULL) {
            ds_set_bool(DS_KEY_BLUETOOTH_SETTINGS, true);
            lv_sw_on(bluetooth_settings_sw, LV_ANIM_OFF);
            lv_obj_set_hidden(liste1, false);
            lv_obj_set_hidden(liste2, false);
            lv_obj_set_hidden(bluetooth_btnm1, false);
        }
    }
}

void btnm_action(lv_obj_t * btnm, lv_event_cb_t event_cb)
{
    if (event_cb != LV_EVENT_CLICKED) return;

    lv_obj_t * label = lv_obj_get_child(btnm, NULL);
    const char * txt = lv_label_get_text(label);

    printf("Button: %s released\n", txt);
}

void bt_list_action(lv_obj_t * list_btn, lv_event_cb_t event_cb)
{
    if (event_cb != LV_EVENT_CLICKED) return;

    printf("List element click:%s\n", lv_list_get_btn_text(list_btn));
    int idx = lv_obj_get_user_data(list_btn);
    printf("bt_list_action idx:%d\n", idx);

    //return LV_RES_OK;
}

void init_bluetooth_settings(void) {
    //get some router bluetooth info
    init_router_bluetooth_info();

    liste_style_create();

    lv_obj_t * win = default_list_header (lv_scr_act(), get_string(ID_BLUETOOTH_SETTINGS), lv_win_close_event_cb);
    lv_obj_t * bluetooth_settings_list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(bluetooth_settings_list, LV_SB_MODE_OFF);
    lv_list_set_style(bluetooth_settings_list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(bluetooth_settings_list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);
    lv_obj_align(bluetooth_settings_list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    //Set list object size
    //lv_obj_set_size(bluetooth_settings_list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_set_size(bluetooth_settings_list, 320 * LV_RES_OFFSET, 140 * LV_RES_OFFSET);
    lv_obj_align(bluetooth_settings_list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(bluetooth_settings_list, LV_LAYOUT_OFF);

    lv_obj_t * liste = lv_liste_w_switch(bluetooth_settings_list, get_string(ID_BLUETOOTH_SETTINGS), bluetooth_settings_sw_release_action);
    bluetooth_settings_sw = lv_obj_get_child(liste, NULL);

    if (ds_get_bool(DS_KEY_BLUETOOTH_SETTINGS) == true) {
        lv_sw_on(bluetooth_settings_sw, LV_ANIM_OFF);
    } else {
        lv_sw_off(bluetooth_settings_sw, LV_ANIM_OFF);
    }

    //Bluetooth name//bluetooth_settings_list
    liste1 = lv_list_add_for_PR(bluetooth_settings_list, NULL,
            NULL, bluetooth_name_info, bt_list_action,
            device_selector_info, ITEMS_ID_SUB_BLUETOOTH_NAME);
    //Bluetooth MAC Address
    liste2 = lv_list_add_for_PR(bluetooth_settings_list, NULL,
            NULL, bluetooth_mac_address_info, bt_list_action,
            device_selector_info, ITEMS_ID_SUB_BLUETOOTH_MAC_ADDRESS);

    //Bluetooth pairing , Connected device button
    lv_style_copy(&style_btn_bg, &lv_style_plain);
    style_btn_bg.body.main_color = LV_COLOR_WHITE;
    style_btn_bg.body.grad_color = LV_COLOR_WHITE;
    style_btn_bg.body.padding.left = 0;
    style_btn_bg.body.padding.right = 0;
    style_btn_bg.body.padding.top = 0;
    style_btn_bg.body.padding.bottom = 0;
    style_btn_bg.body.padding.inner = 0;

    lv_style_copy(&style_btn_rel, &lv_style_btn_rel);
    style_btn_rel.body.main_color = LV_COLOR_WHITE;
    style_btn_rel.body.grad_color = LV_COLOR_WHITE;
    style_btn_rel.body.border.color = LV_COLOR_SILVER;
    style_btn_rel.body.border.width = 2;
    style_btn_rel.body.border.opa = LV_OPA_COVER;
    style_btn_rel.body.border.part = LV_BORDER_TOP;
    style_btn_rel.body.radius = 0;
    style_btn_rel.text.color = LV_COLOR_GREYISH_BROWN;
    style_btn_rel.text.font = get_font(font_w_bold, font_h_20);
    style_btn_rel.text.letter_space = 0;

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.body.main_color = LV_COLOR_WHITE;
    style_btn_pr.body.grad_color = LV_COLOR_WHITE;
    style_btn_pr.body.border.color = LV_COLOR_BASE;
    style_btn_pr.body.border.width = 3;
    style_btn_pr.text.color = LV_COLOR_BASE;

    bluetooth_btnm1 = lv_btnm_create(win, NULL);
    static const char * bluetooth_btnm_map[3];
    bluetooth_btnm_map[0] = get_string(ID_BLUETOOTH_PAIRING);
    bluetooth_btnm_map[1] = get_string(ID_BLUETOOTH_CONNECTED_DEVICE);
    bluetooth_btnm_map[2] = "";
    lv_btnm_set_map(bluetooth_btnm1, bluetooth_btnm_map);
    lv_obj_set_event_cb(bluetooth_btnm1, btnm_action);
    lv_obj_set_size(bluetooth_btnm1, 320 * LV_RES_OFFSET, 50 * LV_RES_OFFSET);

    lv_btnm_set_style(bluetooth_btnm1, LV_BTNM_STYLE_BG, &style_btn_bg);
    lv_btnm_set_style(bluetooth_btnm1, LV_BTNM_STYLE_BTN_REL, &style_btn_rel);
    lv_btnm_set_style(bluetooth_btnm1, LV_BTNM_STYLE_BTN_PR, &style_btn_pr);
    lv_obj_align(bluetooth_btnm1, win, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    static lv_style_t style_line;
    lv_style_copy(&style_line, &lv_style_plain);
    style_line.line.color = LV_COLOR_GRAY;

    static lv_point_t p[] = {{LV_HOR_RES_MAX/2, 10}, {LV_HOR_RES_MAX/2, 40}};
    lv_obj_t * line = lv_line_create(bluetooth_btnm1, NULL);
    lv_line_set_points(line, p, 2);
    lv_line_set_style(line, LV_LABEL_STYLE_MAIN, &style_line);

    //hide the bluetooth name , MAC Address and button when enable bluetooth settings
    if (ds_get_bool(DS_KEY_BLUETOOTH_SETTINGS) == true) {
        lv_obj_set_hidden(liste1, false);
        lv_obj_set_hidden(liste2, false);
        lv_obj_set_hidden(bluetooth_btnm1, false);
    } else {
        lv_obj_set_hidden(liste1, true);
        lv_obj_set_hidden(liste2, true);
        lv_obj_set_hidden(bluetooth_btnm1, true);
    }
}


void init_router_bluetooth_info(void) {
    char *bluetooth_name_info1 = get_string(ID_BLUETOOTH_NAME);
    char *bluetooth_name_info2 = getBluetoothName();
    snprintf(bluetooth_name_info,sizeof(bluetooth_name_info),"%s%s%s%s",bluetooth_name_info1,":","\n",bluetooth_name_info2);

    char *bluetooth_mac_address_info1 = get_string(ID_BLUETOOTH_MAC_ADDRESS);
    char *bluetooth_mac_address_info2 = getBluetoothMacAddress();
    snprintf(bluetooth_mac_address_info,sizeof(bluetooth_mac_address_info),"%s%s%s%s",bluetooth_mac_address_info1,":","\n",bluetooth_mac_address_info2);

    device_selector_info.type = SELECTOR_TYPE_DEFAULT;
    device_selector_info.pos = NONE;
    device_selector_info.enabled = false;
}

char* getBluetoothName() {
    return "COMPAL-168-BT";
}

char* getBluetoothMacAddress() {
    return "XX:XX:XX:XX:XX:XX";
}
