#include "default_settings.h"
#include "data_store.h"
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/power_manager.h"

#define SETTINGS_VERSION_KEY    "setting_version"
#define SETTINGS_VERSION        1

void settings_file_init(const char *filename) {
    struct stat buffer;
    int exist = stat(filename, &buffer);

    if (exist != 0 || buffer.st_size == 0 ||
       (buffer.st_size > 0 && DS_ERROR == ds_integrity_check())) {

        log_d("ds create file:%s\n", filename);
        if(mkdir(DEFAULT_DATA_STORE_PATH, 0755) < 0) {
            log_e("mkdir %s failed: %d\n", DEFAULT_DATA_STORE_PATH, errno);
        }

        ds_create_file(filename);

        //Set default value
        ds_set_int(SETTINGS_VERSION_KEY, SETTINGS_VERSION);

        ds_set_int(DS_KEY_SCREEN_OFF_TIME, SCN_OFF_DEFAULT);
#ifdef BATTERY_OPTIMIZE_SUPPORT
        ds_set_bool(DS_KEY_BATTERY_OPTIMIZE, false);
#endif
#ifdef HIGH_SPEED_SUPPORT
        ds_set_bool(DS_KEY_HIGH_SPEED, false);
#endif
        //lock screen
        ds_set_int(DS_KEY_BRIGHTNESS, 128);
        ds_set_bool(DS_KEY_PASSWORD_LOCK_ENABLE, false);
        ds_set_bool(DS_KEY_LAUNCHER_CLEANUP, false);
        //data
        ds_set_bool(DS_KEY_DATA_USAGE_MONITOR, false);
        dump_webui_data_usage(); // dump default value for data usage for webui use
        ds_set_bool(DS_KEY_SHOW_DATA_USAGE_ON_HOME, true);
        ds_set_int(DS_KEY_MAX_DATA_USAGE, 1024); //1024 MB
        ds_set_int(DS_KEY_DATA_USAGE_ACCUMULATED, 0);
        ds_set_int(DS_KEY_DATA_USAGE_UNIT, DATA_USAGE_UNIT_MB);
        ds_set_bool(DS_KEY_DATA_USAGE_REMIND, false);
        ds_set_int(DS_KEY_DATA_USAGE_REMIND_VALUE, 60); //%
        ds_set_int(DS_KEY_DATA_USAGE_START_DATE, 1);
        ds_set_value(DS_KEY_DATA_USAGE_RESET_TIME, "");
        ds_set_int(DS_KEY_WIFI_AUTO_CLOSE_DURA, WIFI_DURA_DEFAULT);
        ds_set_bool(DS_KEY_WIFI_AUTO_CLOSE, false);
        ds_set_bool(DS_KEY_SSID_PWD_VISIBLE, false);
        ds_set_bool(DS_KEY_WIFI_24G_ENABLED, true);
        ds_set_bool(DS_KEY_WIFI_5G_ENABLED, false);
        ds_set_value(DS_KEY_UPDATE_LAST_CHECK_TIME, "");
        ds_set_bool(DS_KEY_USB_DEBUG, false);

        ds_set_bool(DS_KEY_DEMO_MODE, false);
        ds_set_bool(DS_KEY_SUPPORT_4G_NETWORKS, false);//for UI test
        ds_set_int(DS_KEY_SEARCH_MODE, 0);//for UI test
        ds_set_int(DS_KEY_MANUAL_CHOOSE_NETWORK, 0);//for UI test
        ds_set_bool(DS_KEY_ENABLE_SIM_PIN, false);//for UI test
        //language
        ds_set_int(DS_KEY_LANGUAGE, EN);
        ds_set_bool(DS_KEY_LANG_EN, true);
        ds_set_bool(DS_KEY_LANG_JP, true);
#ifdef FEATURE_ROUTER
        ds_set_bool(DS_KEY_LANG_AR, false);
        ds_set_bool(DS_KEY_LANG_RU, false);
        ds_set_bool(DS_KEY_LANG_NL, false); //only for Onda, enabled by oem xml
        ds_set_bool(DS_KEY_LANG_SL, false); //only for Onda, enabled by oem xml
#else
        ds_set_bool(DS_KEY_LANG_AR, true);
        //TODO include RU language in zyxel only
        ds_set_bool(DS_KEY_LANG_RU, true);
        ds_set_bool(DS_KEY_LANG_NL, true);
        ds_set_bool(DS_KEY_LANG_SL, true);
        ds_set_bool(DS_KEY_LANG_PL, true);
        ds_set_bool(DS_KEY_LANG_HU, true);
#endif
        ds_set_bool(DS_KEY_LANG_FR, true);
        ds_set_bool(DS_KEY_LANG_DE, true);
        ds_set_bool(DS_KEY_LANG_PT, true);
        ds_set_bool(DS_KEY_LANG_IT, true);
        ds_set_bool(DS_KEY_LANG_ES, true);
        ds_set_bool(DS_KEY_LANG_ZH_CN, true);
        ds_set_bool(DS_KEY_LANG_ZH_TW, true);
        //cust
        ds_set_bool(DS_KEY_CUST_SSID_CHANGED, false);
        ds_set_bool(DS_KEY_CUST_PASSWD_CHANGED, false);
        ds_set_bool(DS_KEY_INIT_PREF_NETWORK, false);
        //airplane mode
        ds_set_bool(DS_KEY_AIRPLANE_MODE, false);
#ifdef CUST_LUXSHARE
        ds_set_bool(DS_KEY_CHARGE_ENABLE, true);
        ds_set_bool(DS_KEY_TETHERING_ENABLE, true);
#endif
    } else {
        // used for re-config after fota upgrade
#ifdef CUST_DLINK
        if (!get_ds_exist(DEFAULT_DATA_STORE_FILE, DS_KEY_LANG_PL)) {
            ds_set_bool(DS_KEY_LANG_PL, true);
            log_d("enable polish lang");
        }
        if (!get_ds_exist(DEFAULT_DATA_STORE_FILE, DS_KEY_LANG_HU)) {
            ds_set_bool(DS_KEY_LANG_HU, true);
            log_d("enable hungarian lang");
        }
#endif
    }
}

void init_default_settings() {
    settings_file_init(DEFAULT_DATA_STORE_FILE);

    // config current locale
    config_locale();
    //reset boot_apn_loading_done config
    ds_set_bool(DS_KEY_BOOT_APN_LOADDING_DONE,false);

    int version = ds_get_int(SETTINGS_VERSION_KEY);
    if (version != SETTINGS_VERSION) {
        log_e("settings store upgrading, old version: %d, current version: %d", version, SETTINGS_VERSION);
        //TODO, upgrade
    }
}
