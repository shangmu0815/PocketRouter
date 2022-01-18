#ifndef LV_POCKET_ROUTER_SRC_UTIL_DATA_STORES_H_
#define LV_POCKET_ROUTER_SRC_UTIL_DATA_STORES_H_

#include <stdbool.h>

#if FEATURE_ROUTER
#define DEFAULT_DATA_STORE_PATH         "/data/misc/pocketrouter"
#else
#define DEFAULT_DATA_STORE_PATH         "Data_Store"
#endif
#define DEFAULT_DATA_STORE_FILE         DEFAULT_DATA_STORE_PATH "/data_storage.xml"

#if FEATURE_ROUTER
#define DEFAULT_APN_STORE_PATH          "/oem/data"
#else
#define DEFAULT_APN_STORE_PATH          "Data_Store"
#endif
#define DEFAULT_APN_STORE_FILE          DEFAULT_APN_STORE_PATH "/apn.xml"

#define SIM_BLACKLIST_STORE_FILE        DEFAULT_APN_STORE_PATH "/sim_blacklist.xml"

#define DEFAULT_ROOT_ELEMENT            "settings"

/* Data Store Keys */
//power
#define DS_KEY_SCREEN_OFF_TIME          "screen_off_time"
#define DS_KEY_BRIGHTNESS               "brightness_value"
#define DS_KEY_BATTERY_OPTIMIZE         "battery_optimize"
#define DS_KEY_HIGH_SPEED               "high_speed"
//lock screen
#define DS_KEY_PASSWORD_LOCK_ENABLE     "password_lock_enable"
#define DS_KEY_PASSWORD_LOCK_VALUE      "password_lock_value"
#define DS_KEY_PASSWORD_LOCK            "password_lock_value"
#define DS_KEY_LAUNCHER_CLEANUP         "launcher_cleanup"
//data
#define DS_KEY_DATA_USAGE_MONITOR       "data_usage_monitor"
#define DS_KEY_MAX_DATA_USAGE           "max_data_usage"
#define DS_KEY_DATA_USAGE_ACCUMULATED   "data_usage_accumulated"
#define DS_KEY_DATA_USAGE_UNIT          "data_usage_unit"
#define DS_KEY_DATA_USAGE_REMIND        "data_usage_remind"
#define DS_KEY_DATA_USAGE_REMIND_VALUE  "data_usage_remind_value"
#define DS_KEY_SHOW_DATA_USAGE_ON_HOME  "show_data_usage_on_home"
#define DS_KEY_DATA_USAGE_START_DATE    "data_usage_start_date"
#define DS_KEY_DATA_USAGE_RESET_TIME    "data_usage_reset_time"
#define DS_KEY_DATA_USAGE_NEXT_RESET_TIME "data_usage_next_reset_time"
#define DS_KEY_DATA_USAGE_RESET_ACCUMULATED "data_usage_reset_accumulated"
#define DS_KEY_WIFI_EXTENDER            "wifi_extender"
#define DS_KEY_AUTO_CHECK_FOR_UPDATE    "auto_check_for_update"
#define DS_KEY_BLUETOOTH_SETTINGS       "bluetooth_settings"
#define DS_KEY_WIFI_AUTO_CLOSE_DURA     "wifi_auto_close_dura"
#define DS_KEY_WIFI_AUTO_CLOSE          "wifi_auto_close"
#define DS_KEY_SSID_PWD_VISIBLE         "ssid_pwd_visible"
#define DS_KEY_WIFI_24G_ENABLED         "wifi_24g_enabled"
#define DS_KEY_WIFI_5G_ENABLED          "wifi_5g_enabled"
#define DS_KEY_UPDATE_LAST_CHECK_TIME   "update_last_check_time"
#define DS_KEY_USB_DEBUG                "usb_debug"

#define DS_KEY_DEMO_MODE             	"demo_mode"
#define DS_KEY_SUPPORT_4G_NETWORKS      "support_4g_networks"
#define DS_KEY_SEARCH_MODE              "search_mode"
#define DS_KEY_MANUAL_CHOOSE_NETWORK    "manual_choose_network"
#define DS_KEY_LANGUAGE                 "language"

#define DS_KEY_ENABLE_SIM_PIN           "enable_sim_pin"
#define DS_KEY_SIM_PIN_VALUE            "sim_pin_value"
#define DS_KEY_SIM_PUK_VALUE            "sim_puk_value"
//language list to control enable or not
#define DS_KEY_LANG_EN                  "lang_en"
#define DS_KEY_LANG_JP                  "lang_jp"
#define DS_KEY_LANG_AR                  "lang_ar"
#define DS_KEY_LANG_FR                  "lang_fr"
#define DS_KEY_LANG_DE                  "lang_de"
#define DS_KEY_LANG_NL                  "lang_nl"
#define DS_KEY_LANG_PT                  "lang_pt"
#define DS_KEY_LANG_SL                  "lang_sl"
#define DS_KEY_LANG_IT                  "lang_it"
#define DS_KEY_LANG_ES                  "lang_es"
#define DS_KEY_LANG_RU                  "lang_ru"
#define DS_KEY_LANG_ZH_CN               "lang_zh_cn"
#define DS_KEY_LANG_ZH_TW               "lang_zh_tw"
#define DS_KEY_LANG_PL                  "lang_pl"
#define DS_KEY_LANG_HU                  "lang_hu"

//cust
#define DS_KEY_CUST_SSID_CHANGED        "cust_ssid_changed"
#define DS_KEY_CUST_PASSWD_CHANGED      "cust_passwd_changed"
#define DS_KEY_WIFI_REBOOT_FLAG         "wifi_reboot_flag"
#define DS_KEY_INIT_PREF_NETWORK        "init_pref_network"
//airplane mode
#define DS_KEY_AIRPLANE_MODE            "airplane_mode"
#define DS_KEY_BOOT_APN_LOADDING_DONE   "boot_apn_loading_done"

#ifdef CUST_LUXSHARE
#define DS_KEY_CHARGE_ENABLE            "charge_enable"
#define DS_KEY_TETHERING_ENABLE         "tethering_enable"
#endif

typedef enum {
    DS_OK,
    DS_ERROR
} DS_RES;

int ds_get_int(char* key);
long int ds_get_long_int(char* key);
long long int ds_get_long_long_int(char* key);
bool ds_get_bool(char* key);
char* ds_get_value(char* key);

void ds_create_file(const char *filepath);
DS_RES ds_set_int(char* key, int value);
DS_RES ds_set_long_int(char* key, long int value);
DS_RES ds_set_long_long_int(char* key, long long int value);
DS_RES ds_set_bool(char* key, bool value);
DS_RES ds_set_value(char* key, char* value);

DS_RES ds_set_data(const char *filename, const char* xpathExpr, char* value);
DS_RES get_ds_data(const char *filename, const char* xpathExpr, char* value);

#endif /* LV_POCKET_ROUTER_SRC_UTIL_DATA_STORES_H_ */
