#include "../../res/values/string_value.h"
#include "../launcher.h"
#include "lv_pocket_router/src/settings/settings.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lvgl/lvgl.h"
#ifdef WIFI_EXTENDER
#include "lv_pocket_router/src/settings/wifi_extender.h"
#endif
#include "lv_pocket_router/src/settings/wifi.h"
#ifdef BT_SUPPORT
#include "lv_pocket_router/src/settings/bluetooth_settings.h"
#endif
#include "lv_pocket_router/src/settings/profile_management.h"
#include "lv_pocket_router/src/settings/data_usage.h"
#include "lv_pocket_router/src/settings/data_roaming.h"
#include "lv_pocket_router/src/settings/network_settings.h"
#include "lv_pocket_router/src/settings/pin_management.h"
#include "lv_pocket_router/src/settings/password_lock.h"
#include "lv_pocket_router/src/settings/language.h"
#include "lv_pocket_router/src/settings/time_setting.h"
#ifdef REMOTE_WAKEUP
#include "lv_pocket_router/src/settings/remote_wakeup.h"
#endif
#include "lv_pocket_router/src/settings/update.h"
#include "lv_pocket_router/src/settings/restore_default.h"
#include "lv_pocket_router/src/settings/connected_users.h"
#include "lv_pocket_router/src/util/page_anim.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/display/display.h"
#include "lv_pocket_router/src/conn_guide/wps.h"
#include "lv_pocket_router/src/wlan/hostapd_conf.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"
/*Will be called on click of a button of a list*/
#define MAX_LISTE   15
lv_obj_t * btn_list[MAX_LISTE];
lv_obj_t * win;

const int INDEX_WIFI = 0;
const int INDEX_PROFILE_MANAGEMENT = 1;
const int INDEX_LANGUAGE = 2;

enum SETTINGS_IDS {
#ifdef WIFI_EXTENDER
    ITEMS_ID_WIFI_EXTENDER,
#endif
    ITEMS_ID_WPS,
    ITEMS_ID_WIFI_2_4g,
#ifdef SUPPORT_2.4PLUS5GHZ
    ITEMS_ID_WIFI_5g,
#endif
    ITEMS_ID_CONNECTED_USERS,
#ifdef BT_SUPPORT
    ITEMS_ID_BLUETOOTH_SETTINGS,
#endif
    ITEMS_ID_PROFILE_MANAGEMENT,
    ITEMS_ID_DATA_USAGE,
    ITEMS_ID_DATA_ROAMING,
    ITEMS_ID_NETWORK_SETTINGS,
    ITEMS_ID_PIN_MANAGEMENT,
    ITEMS_ID_PASSWORD_LOCK,
    ITEMS_ID_LANGUAGE,
    ITEMS_ID_TIME_SETTING,
#ifdef REMOTE_WAKEUP
    ITEMS_ID_REMOTE_WAKEUP,
#endif
    ITEMS_ID_UPDATE,
    ITEMS_ID_RESTORE_DEFAULT,
};

void setting_list_release_action(lv_obj_t * list_btn, lv_event_cb_t event_cb)
{
    if (event_cb != LV_EVENT_CLICKED) return;

    int item_id = lv_obj_get_user_data(list_btn);
    log_d("setting_list_release_action item_id:%d", item_id);
#ifdef WIFI_EXTENDER
    if (item_id == ITEMS_ID_WIFI_EXTENDER) {
        init_wifi_extender();
    } else 
#endif
    if (item_id == ITEMS_ID_WPS) {
        wps_create();
    } else if (item_id == ITEMS_ID_WIFI_2_4g) {
        wifi_init(WIFI_BAND_24G);
#ifdef SUPPORT_2.4PLUS5GHZ
    } else if (item_id == ITEMS_ID_WIFI_5g) {
        wifi_init(WIFI_BAND_5G);
#endif
    } else if (item_id == ITEMS_ID_CONNECTED_USERS) {
        init_connected_users();
#ifdef BT_SUPPORT
    } else if (item_id == ITEMS_ID_BLUETOOTH_SETTINGS) {
        init_bluetooth_settings();
#endif
    } else if (item_id == ITEMS_ID_PROFILE_MANAGEMENT) {
        init_profile_management_loading_task_animation();
    } else if (item_id == ITEMS_ID_DATA_USAGE) {
        init_data_usage();
    } else if (item_id == ITEMS_ID_DATA_ROAMING) {
        init_data_roaming();
    } else if (item_id == ITEMS_ID_NETWORK_SETTINGS) {
        init_network_settings();
    } else if (item_id == ITEMS_ID_PIN_MANAGEMENT) {
        init_pin_management();
    } else if (item_id == ITEMS_ID_PASSWORD_LOCK) {
        password_lock_create();
    } else if (item_id == ITEMS_ID_LANGUAGE) {
        init_language();
    } else if (item_id == ITEMS_ID_TIME_SETTING) {
        time_setting_create();
#ifdef REMOTE_WAKEUP
    } else if (item_id == ITEMS_ID_REMOTE_WAKEUP) {
        init_remote_wakeup();
#endif
    } else if (item_id == ITEMS_ID_UPDATE) {
        init_update();
    } else if (item_id == ITEMS_ID_RESTORE_DEFAULT) {
        init_restore_default();
    }
}

#ifdef DEMO_MODE
void demo_mode_switcher_action(lv_obj_t * sw, lv_event_cb_t event_cb)
{
    if (event_cb != LV_EVENT_CLICKED) return;

    bool on = lv_sw_get_state(sw);
    ds_set_bool(DS_KEY_DEMO_MODE, on);
    set_keep_screen_on(on);
    return LV_RES_OK;
}
#endif

void show_settings_list(void) {
    /*Show Settings List*/

    //Draw Settings page header 320x50
    lv_obj_t * win = default_list_header (lv_scr_act(), get_string(ID_LAUNCHER_SETTINGS), lv_win_close_event_cb);
    lv_obj_t * settings_list = lv_list_create(win, NULL);

    lv_list_set_sb_mode(settings_list, LV_SB_MODE_OFF);
    lv_list_set_style(settings_list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(settings_list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(settings_list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(settings_list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    /*Style of the list element images on buttons*/
    char *elementPng1[] = {
#ifdef WIFI_EXTENDER
            &ic_list_wifiex,
#endif
            &ic_list_wps,
            &ic_list_wifi,
#ifdef SUPPORT_2.4PLUS5GHZ
            &ic_list_wifi,
#endif
            &ic_list_users,
#ifdef BT_SUPPORT
            &ic_list_bt,
#endif
            &ic_list_profile, &ic_list_data, &ic_list_roaming,
            &ic_list_networking, &ic_list_pin, &ic_list_password,
            &ic_list_language,
            &ic_list_time,
#ifdef REMOTE_WAKEUP
            &ic_list_remote,
#endif
            &ic_list_update, &ic_list_restore };

    /*Style of the list element text on buttons*/
    const char *elementStr[] = {
#ifdef WIFI_EXTENDER
            get_string(ID_WIFI_EXTENDER),
#endif
            get_string(ID_WPS),
#ifdef SUPPORT_2.4PLUS5GHZ
            get_string(ID_WIFI_2_4G),
            get_string(ID_WIFI_5G),
#else
            get_string(ID_WIFI),
#endif
            get_string(ID_CONN_USR),
#ifdef BT_SUPPORT
            get_string(ID_BLUETOOTH_SETTINGS),
#endif
            get_string(ID_PROF_MGMT), get_string(ID_DATA_USAGE),
            get_string(ID_DATA_ROAMING), get_string(ID_NW_SETTINGS),
            get_string(ID_PIN_MNG), get_string(ID_PW_LOCK),
            get_string(ID_LANG),
            get_string(ID_TIME_SETTING),
#ifdef REMOTE_WAKEUP
            get_string(ID_REMOTE_WAKEUP),
#endif
            get_string(ID_UPDATE), get_string(ID_RESTORE_DEFAULT) };
    /*Style of the list element size on buttons*/
    int elementNum = sizeof(elementStr) / sizeof(char *);
    /*item_id for check which item select or press*/
    int item_id[] = {
#ifdef WIFI_EXTENDER
            ITEMS_ID_WIFI_EXTENDER,
#endif
            ITEMS_ID_WPS, ITEMS_ID_WIFI_2_4g,
#ifdef SUPPORT_2.4PLUS5GHZ
            ITEMS_ID_WIFI_5g,
#endif
            ITEMS_ID_CONNECTED_USERS,
#ifdef BT_SUPPORT
            ITEMS_ID_BLUETOOTH_SETTINGS,
#endif
            ITEMS_ID_PROFILE_MANAGEMENT, ITEMS_ID_DATA_USAGE,
            ITEMS_ID_DATA_ROAMING, ITEMS_ID_NETWORK_SETTINGS,
            ITEMS_ID_PIN_MANAGEMENT, ITEMS_ID_PASSWORD_LOCK, ITEMS_ID_LANGUAGE,
            ITEMS_ID_TIME_SETTING,
#ifdef REMOTE_WAKEUP
            ITEMS_ID_REMOTE_WAKEUP,
#endif
            ITEMS_ID_UPDATE, ITEMS_ID_RESTORE_DEFAULT };

    liste_style_create();
#ifdef DEMO_MODE
    lv_obj_t * demo_obj = lv_liste_w_switch(settings_list, get_string(ID_DEMO_MODE), demo_mode_switcher_action);
    lv_obj_t * switcher = lv_obj_get_child(demo_obj, NULL);
    if (ds_get_bool(DS_KEY_DEMO_MODE)) {
        lv_sw_on(switcher, LV_ANIM_OFF);
    } else {
        lv_sw_off(switcher, LV_ANIM_OFF);
    }
#endif

    /*list content*/
    int i;
    for (i = 0; i < elementNum; i++) {
        btn_list[i] = lv_liste_settings(settings_list, elementPng1[i],
                elementStr[i], setting_list_release_action, item_id[i]);
    }

    //show page to page exit part
    page_anim_exit();
}
