/*
 * about_help.c
 *
 *  Created on: Mar 18, 2019
 *      Author: joseph
 */

#include <stdio.h>

#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/about/about_help.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/util/usb_compositions.h"
#include "lv_pocket_router/src/util/util.h"
#define MAX_LISTE  20
lv_obj_t* btn_list[MAX_LISTE];
enum ABOUT_HELP_IDS {
    ITEMS_ID_NONE,
    ITEMS_ID_USB_DEBUG
};

#define USB_DEBUG_ENABLE_PRESS          6
static int debug_press_cnt = 0;

void reset_debug_press() {
    if (debug_press_cnt < USB_DEBUG_ENABLE_PRESS) {
        debug_press_cnt = 0;
    }
}

void help_list_action(lv_obj_t * list_btn, lv_event_cb_t event_cb)
{
    if (event_cb != LV_EVENT_CLICKED) return;

    int item_id = lv_obj_get_user_data(list_btn);

    if (item_id == ITEMS_ID_NONE) reset_debug_press();

    if (item_id == ITEMS_ID_USB_DEBUG && debug_press_cnt <= USB_DEBUG_ENABLE_PRESS) {
        debug_press_cnt++;
        if (debug_press_cnt == USB_DEBUG_ENABLE_PRESS) {
            enable_usb_debug();
            popup_anim_not_plain_create(get_string(ID_LOADING), 1000);
        }
    }
}

void show_Help(void) {
    //Draw about page header 320x50
    lv_obj_t * win = default_list_header (lv_scr_act(), get_string(ID_HELP), lv_win_close_event_cb);
    lv_obj_t * about_help_list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(about_help_list, LV_SB_MODE_OFF);
    lv_list_set_style(about_help_list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(about_help_list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(about_help_list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(about_help_list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(about_help_list, LV_LAYOUT_OFF);

    /*Style of the list element images on buttons*/
    char *elementPng1[] = { &ic_help_wifi, &ic_help_noservice,
            &ic_help_roaming, &ic_help_sim, &ic_help_network,
#ifdef HOTSPOT_SUPPORT
            &ic_help_hotspot,
#endif
            &ic_help_sms, &ic_help_battery,
            &ic_help_charging, &ic_help_wifidevice,
#ifdef BT_SUPPORT
            &ic_help_btdevice,
#endif
#ifdef DFS_SUPPORT
            &ic_help_dfs,
#endif
#ifdef INDOOR_SUPPORT
            &ic_help_indoor,
#endif
#ifdef CUST_ZYXEL
            &ic_status_data_ud,
#else
            &ic_help_data,
#endif
            &ic_help_home, &ic_headline_back,
            &ic_help_cancel, &ic_help_ok, &ic_help_blacklist,
#ifdef CUST_DLINK
            &ic_help_sw_update
#endif
            };

    /*Style of the list element text on buttons*/
    const char *elementStr[] = {
        get_string(ID_ABOUT_DEVICE_SIGNAL),
        get_string(ID_ABOUT_NO_SERVICE),
        get_string(ID_ABOUT_ROAMING),
        get_string(ID_ABOUT_SIM_NOT_FOUND),
        get_string(ID_ABOUT_NW_MODE),
#ifdef HOTSPOT_SUPPORT
        get_string(ID_ABOUT_WF_HOTSPOT),
#endif
        get_string(ID_LAUNCHER_SMS),
        get_string(ID_ABOUT_BATTERY_LEVEL),
        get_string(ID_ABOUT_CHARGING),
        get_string(ID_ABOUT_NUM_OF_CONN_WF_DEVICE),
#ifdef BT_SUPPORT
        get_string(ID_ABOUT_NUM_OF_CONN_BLE_DEVICE),
#endif
#ifdef DFS_SUPPORT
        get_string(ID_ABOUT_DFS),
#endif
#ifdef INDOOR_SUPPORT
        get_string(ID_ABOUT_INDOOR_ICON),
#endif
        get_string(ID_ABOUT_DATA_DELIVERY),
        get_string(ID_ABOUT_RETURN_TO_HME_SCRN),
        get_string(ID_ABOUT_RETURN_TO_PRE_SCRN),
        get_string(ID_CANCEL),
        get_string(ID_OK),
        get_string(ID_ABOUT_ADD_TO_BLACKLIST),
#ifdef CUST_DLINK
        get_string(ID_ABOUT_UPDATES_AVAILABLE)
#endif
    };
    /*Style of the list element size on buttons*/
    int elementNum = sizeof(elementStr) / sizeof(char *);

    /*item_id for check which item select or press*/
    int item_id[] = { ITEMS_ID_NONE, ITEMS_ID_NONE, ITEMS_ID_NONE, ITEMS_ID_NONE, ITEMS_ID_NONE,
#ifdef HOTSPOT_SUPPORT
            ITEMS_ID_NONE,
#endif
            ITEMS_ID_NONE, ITEMS_ID_NONE, ITEMS_ID_NONE, ITEMS_ID_NONE,
#ifdef BT_SUPPORT
            ITEMS_ID_NONE,
#endif
#ifdef DFS_SUPPORT
            ITEMS_ID_NONE,
#endif
#ifdef INDOOR_SUPPORT
            ITEMS_ID_NONE,
#endif
            ITEMS_ID_NONE, ITEMS_ID_NONE, ITEMS_ID_NONE, ITEMS_ID_NONE, ITEMS_ID_NONE, ITEMS_ID_USB_DEBUG,
#ifdef CUST_DLINK
            ITEMS_ID_NONE
#endif
    };

    /*list content*/
    int is_debug_b = usb_debug_enabled();
    int i;
    for (i = 0; i < elementNum; i++) {
        btn_list[i] = lv_liste_setting_info(about_help_list,
                      elementPng1[i], elementStr[i],
#ifdef USB_DEBUG_ENABLING
                      (is_debug_b ? NULL : help_list_action),
#else
                      NULL,
#endif
                      item_id[i]);
    }

    reset_debug_press();
}
