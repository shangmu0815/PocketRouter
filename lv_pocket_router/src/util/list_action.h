/*
 * list_action.h
 *
 *  Created on: Mar 13, 2019
 *      Author: joseph
 */

#ifndef LV_POCKET_ROUTER_SRC_UTILITY_LIST_ACTION_H_
#define LV_POCKET_ROUTER_SRC_UTILITY_LIST_ACTION_H_

#include <stdbool.h>
#include "../../../lvgl/lvgl.h"

//Items check
enum ITEM_IDS {
    ITEM_ID_SUB_CONNECTED_USER_BLACKLIST = -2000,
    ITEM_ID_SUB_CONNECTED_USER = -1000,
    ITEM_ID_SUB_SMS1 = -4,           //for test data
    ITEM_ID_SUB_SMS2 = -3,           //for test data
    ITEM_ID_SUB_SMS3 = -2,           //for test data
    ITEM_ID_NONE = -1,
    ITEM_ID_WIFI_EXTENDER = 10,
    ITEM_ID_WPS,
    ITEM_ID_WIFI,
    ITEM_ID_CONNECTED_USERS,
    ITEM_ID_BLUETOOTH_SETTINGS,
    ITEM_ID_PROFILE_MANAGEMENT,
    ITEM_ID_DATA_USAGE,
    ITEM_ID_DATA_ROAMING,
    ITEM_ID_NETWORK_SETTINGS,
    ITEM_ID_PIN_MANAGEMENT,
    ITEM_ID_PASSWORD_LOCK,
    ITEM_ID_LANGUAGE,
    ITEM_ID_REMOTE_WAKEUP,          //22
    ITEM_ID_UPDATE,
    ITEM_ID_RESTORE_DEFAULT,        //24
    ITEM_ID_DEVICE_INFORMATION,
    ITEM_ID_CAUTION_FOR_USING_5G,
    ITEM_ID_HELP,
    ITEM_ID_OPEN_SOURCE_NOTICE,
    ITEM_ID_SUB_DATA_ROAMING,
    ITEM_ID_SUPPORT_4G_NETWORKS,
    ITEM_ID_SEARCH_MODE,       //31
    ITEM_ID_SEARCH_MODE_AUTO,  //32
    ITEM_ID_SEARCH_MODE_MANUAL,//33
    ITEM_ID_TWM_4G,            //34
    ITEM_ID_TWM_3G,            //35
    ITEM_ID_T_STAR_3G,         //36
    ITEM_ID_SUB_WIFI_EXTENDER,
    ITEM_ID_SUB_WIFI_BAND_WITH_NO_SIM,
    ITEM_ID_SUB_WIFI_BANDWITH_WITH_NO_SIM,
    ITEM_ID_SUB_WIFI_PMF_WITH_NO_SIM,
    ITEM_ID_SUB_WIFI_BAND_WITH_SIM,
    ITEM_ID_SUB_WIFI_BANDWITH_WITH_SIM,
    ITEM_ID_SUB_WIFI_PMF_WITH_SIM,
    ITEM_ID_SUB_WIFI_BAND_INFO_2_4_GHZ,
    ITEM_ID_SUB_WIFI_BAND_INFO_5_GHZ,
    ITEM_ID_SUB_WIFI_BAND_INFO_2_4_AND_5_GHZ,
    ITEM_ID_SUB_WIFI_BANDWITH_INFO_20_MHZ,
    ITEM_ID_SUB_WIFI_BANDWITH_INFO_20_OR_40_MHZ,
    ITEM_ID_SUB_EN,
    ITEM_ID_SUB_FR,
    ITEM_ID_SUB_DE,
    ITEM_ID_SUB_ENABLE_PASSWORD_LOCK,
    ITEM_ID_SUB_REMOTE_WAKEUP_SWITCH,
    ITEM_ID_SUB_DISPLAY_DATA_USAGE_ON_HOME_SCREEN_SWITCH,
    ITEM_ID_SUB_MAX_DATA_USAGE,
    ITEM_ID_SUB_UNIT,
    ITEM_ID_SUB_START_DATE,
    ITEM_ID_SUB_REMIND_WHEN_DATA_USAGE_REACHES_REMINDER_THRESHOLD_SWITCH,
    ITEM_ID_SUB_REMIND_THRESHOLD,
    ITEM_ID_SUB_ENABLE_PIN,
    ITEM_ID_SUB_AUTO_CHECK_FOR_UPDATED,
    ITEM_ID_SUB_BLUETOOTH_SETTINGS,
    ITEM_ID_SUB_BLUETOOTH_NAME,
    ITEM_ID_SUB_BLUETOOTH_MAC_ADDRESS,
};


lv_obj_t * list_header_create (lv_obj_t * par, const void * headline, const void * r_img, const void * l_img, lv_event_cb_t r_action, lv_event_cb_t l_action);
lv_obj_t * del_list_header (lv_obj_t * par, const void * headline, lv_event_cb_t r_action, lv_event_cb_t l_action);
lv_obj_t * modify_list_header (lv_obj_t * par, const void * headline, lv_event_cb_t r_action, lv_event_cb_t l_action);
lv_obj_t * default_list_header (lv_obj_t * par, const void * headline, lv_event_cb_t l_action);
lv_obj_t * block_list_header (lv_obj_t * par, const void * headline, lv_event_cb_t r_action, lv_event_cb_t l_action);
lv_obj_t * sms_list_header_create (lv_obj_t * par, const void * headline, lv_event_cb_t l_action, lv_event_cb_t del_action);
lv_res_t close_all_lists(int start);
void list_action_add_back_cache(lv_obj_t * btn, lv_event_cb_t action);
void win_title_adjust(lv_obj_t * win, const char * headline);
void style_create_(void);
void list_content(lv_obj_t * list, int element_num, char *element_png1[],
                  char *element_png2[], char *element_str[], 
                  lv_list_selector_t selector_info[], int item_id[]);
lv_obj_t * delete_profile_list_header_create (lv_obj_t * par, const void * headline, lv_event_cb_t action, lv_event_cb_t close_action);
#endif /* LV_POCKET_ROUTER_SRC_UTILITY_LIST_ACTION_H_ */

