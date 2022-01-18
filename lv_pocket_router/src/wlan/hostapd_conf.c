#include "hostapd_conf.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/display/display.h"
#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/power_menu.h"
#include "../../../lvgl/lvgl.h"
#ifdef FEATURE_ROUTER
//For wlan_mac, get mac from nv item
#include "device_management_service_v01.h"
//For cc_regdb, get mcc to wlan_country
#include "network_access_service_v01.h"
#endif
#include "country.h"

#if ANDROID_BUILD
#define HOSTAPD_CONFIG_PATH "/data/misc/wifi/hostapd/hostapd.conf"
#define HOSTAPD_CONFIG_TEMP_PATH "/data/misc/pocketrouter/hostapd_tmp.conf"
#elif FEATURE_ROUTER
#define HOSTAPD_CONFIG_PATH "/data/misc/wifi/hostapd.conf"
#define HOSTAPD_CONFIG_TEMP_PATH "/data/misc/pocketrouter/hostapd_tmp.conf"
#define WLAN1_CONFIG_PATH "/data/misc/wifi/hostapd-wlan1.conf"
#define WLAN1_CONFIG_TEMP_PATH "/data/misc/pocketrouter/hostapd-wlan1_tmp.conf"
#define HOSTAPD_DENY_CONFIG_PATH "/data/misc/wifi/hostapd.deny"
#define HOSTAPD_CONNECTED_FILE "/data/misc/usr_connected_log.txt"
//For ssid customized
#define ETC_HOSTAPD_CONFIG_FILE "/etc/misc/wifi/hostapd.conf"
#define ETC_HOSTAPD_CONFIG_FILE_TEMP "/etc/misc/wifi/hostapd_tmp.conf"
#define ETC_WLAN1_CONFIG_FILE "/etc/misc/wifi/hostapd-wlan1.conf"
#define ETC_WLAN1_CONFIG_FILE_TEMP "/etc/misc/wifi/hostapd-wlan1_tmp.conf"
#else
#define HOSTAPD_CONFIG_PATH "Data_Store/hostapd.conf"
#define HOSTAPD_CONFIG_TEMP_PATH "Data_Store/hostapd_tmp.conf"
#define WLAN1_CONFIG_PATH "Data_Store/hostapd-wlan1.conf"
#define WLAN1_CONFIG_TEMP_PATH "Data_Store/hostapd-wlan1_tmp.conf"
#define HOSTAPD_DENY_CONFIG_PATH "Data_Store/hostapd.deny"
#define HOSTAPD_CONNECTED_FILE "Data_Store/usr_connected_log.txt"
//For ssid customized
#define ETC_HOSTAPD_CONFIG_FILE "Data_Store/etc/hostapd.conf"
#define ETC_HOSTAPD_CONFIG_FILE_TEMP "Data_Store/etc/hostapd_tmp.conf"
#define ETC_WLAN1_CONFIG_FILE "Data_Store/etc/hostapd-wlan1.conf"
#define ETC_WLAN1_CONFIG_FILE_TEMP "Data_Store/etc/hostapd-wlan1_tmp.conf"
#endif
#define HOSTAPD_CONFIG_FILE_COUNT   2

#define WIFI_SECURITY_NONE_WEP  "0"
#define WIFI_SECURITY_WPA_PSK   "1"
#define WIFI_SECURITY_WPA2_PSK  "2"

#define WPA_KEY_MGMT_WPA_PSK        "WPA-PSK"
#define WPA_KEY_MGMT_SAE_WPA_PSK    "SAE WPA-PSK"

#define WIFI_BAND_5G_VALUE      "a"
#define WIFI_BAND_24G_VALUE     "g"

#define MAX_CONFIG_LENGTH   100
#define MAX_CMD_LENGTH      MAX_CONFIG_LENGTH * 3

//For wlan_mac, get mac from nv item
#define WLAN_MAC_BIN_FILE "/data/misc/wifi/wlan_mac.bin"
#define WLAM_MAC_BIN_TEMP_FILE "/data/misc/wifi/wlan_mac_tmp.bin"
//For cc_regdb, get mcc to wlan_country
#define WLAN_MCC_COUNTRY_FILE "/data/misc/wifi/wlan.progconf"
#define WLAN_MCC_COUNTRY_TEMP_FILE "/data/misc/wifi/wlan_tmp.progconf"

#define REBOOT_AFTER_CONFIG_LOAD
#define DISABLE_WLAN_AT_CLOSE

enum {
    SSID,
    WPA_PASSPHRASE, //password
    WPA_PSK, //password if len is 64 and all hex
    WPA,
    WPA_KEY_MGMT,
    SAE_REQUIRE_MFP,
    WEP_KEY0,
    WEP_DEFAULT_KEY,
    AUTH_ALGS,
    HW_MODE, //wifi band
    IEEE80211W, //wifi pmf
    IEEE80211N,
    HT_CAPAB,
    IEEE80211AC,
    VHT_OPER_CHWIDTH,
    IEEE80211AX,
    IGNORE_BROADCAST_SSID, //hide ssid
    EAP_SERVER,
    WPS_STATE,
    CHANNEL,
    VHT_OPER_CENTR_FREQ,
    MAX_CONFIG_CNT
};

char * config_name[MAX_CONFIG_CNT] = {
            "ssid", "wpa_passphrase", "wpa_psk", "wpa", "wpa_key_mgmt", "sae_require_mfp", "wep_key0",
            "wep_default_key", "auth_algs", "hw_mode", "ieee80211w", "ieee80211n",
            "ht_capab", "ieee80211ac", "vht_oper_chwidth", "ieee80211ax", "ignore_broadcast_ssid",
            "eap_server", "wps_state", "channel", "vht_oper_centr_freq_seg0_idx"};

typedef struct {
    //char *name;
    char *value;
} HostapdConfig;

HostapdConfig hostapd_config[MAX_CONFIG_CNT];
HostapdConfig wlan1_config[MAX_CONFIG_CNT];

HostapdConfig hostapd_deny_config[HOSTAPD_DENY_CONFIG_MAX];

static bool wlan_init = false;

char* active_ssid_name = NULL;
#define SLASH_BTW_SSID      " / "

#define DEFAULT_WIFI_BAND   WIFI_BAND_24G
#ifdef SUPPORT_2.4PLUS5GHZ
int current_wifi_band = WIFI_BAND_NONE; // set to none to prevent showing wrong icons at boot time
#else
int current_wifi_band = WIFI_BAND_24G;
#endif

enum {
    DENY_ADD,
    DENY_REMOVE
};

bool auto_close_enabled = false;
bool restart_sleep_flag = false;
bool wifi_sleep = false;

int total_connected_number = 0;
bool connected_users_dump = false;

static lv_task_t * lv_task;
static lv_task_t * lv_auto_close_task;

static pthread_t cmd_exec_thread;
static pthread_t restart_failed_band_thread;
static pthread_t enable_wlan_thread;
static pthread_t disable_wlan_thread;

//For cc_regdb, get mcc to wlan_country
char mccGlobal[3] = "US";

HostapdConfig * get_config(int band) {
    HostapdConfig* config = hostapd_config;
    if (band == WIFI_BAND_5G) {
        config = wlan1_config;
    }
    return config;
}

bool get_wifi_band_enabled(int band) {
    return current_wifi_band & band;
}

void dump_wifi_band_state() {
    FILE *fp = popen("ifconfig | grep wlan", "r");
    if (fp == NULL) {
        log_e("error ifconfig grep for checking wifi band");
        return;
    }

    int band = WIFI_BAND_NONE;
    char buffer[MAX_CONFIG_LENGTH];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        char* found = strstr(buffer, BAND_24G);
        if (found != NULL)
            band |= WIFI_BAND_24G;

        found = strstr(buffer, BAND_5G);
        if (found != NULL) {
#ifdef SUPPORT_2.4PLUS5GHZ
            band |= WIFI_BAND_5G;
#else
            systemCmd("ifconfig wlan1 down"); //For Demo only
#endif
        }
    }

    if ((band & WIFI_BAND_24G) != (current_wifi_band & WIFI_BAND_24G)) {
        log_d("2.4GHz %s but config value not sync, re-config", (band & WIFI_BAND_24G) ? "detectd" : "not detectd");
        write_wifi_band_enabled(WIFI_BAND_24G, current_wifi_band & WIFI_BAND_24G);
    }
    if ((band & WIFI_BAND_5G) != (current_wifi_band & WIFI_BAND_5G)) {
        log_d("5GHz %s but config value not sync, re-config", (band & WIFI_BAND_5G) ? "detectd" : "not detectd");
        write_wifi_band_enabled(WIFI_BAND_5G, current_wifi_band & WIFI_BAND_5G);
    }

    /* we will now store enable state, so disable below as
       we don't follow actual wifi status anymore
    if (current_wifi_band != band) {
        current_wifi_band = band;
        update_active_wlan_ssid();
        update_wifi_band();
    } */

    pclose(fp);
}

int get_wifi_band() {
    return current_wifi_band;

/* obsolete code
    if (hostapd_config[HW_MODE].value == NULL || hostapd_config[CHANNEL].value == NULL ) {
        return WIFI_BAND_24G;
    }

    if (strcmp(hostapd_config[HW_MODE].value, WIFI_BAND_24G_VALUE) == 0 &&
               strcmp(hostapd_config[CHANNEL].value, "6") == 0 &&
               strcmp(hostapd_config[WLAN1_HW_MODE].value, WIFI_BAND_24G_VALUE) == 0 &&
               strcmp(hostapd_config[WLAN1_CHANNEL].value, "6") == 0) {
        return WIFI_BAND_24G;
    } else if (strcmp(hostapd_config[HW_MODE].value, WIFI_BAND_5G_VALUE) == 0 &&
               strcmp(hostapd_config[CHANNEL].value, "36") == 0 &&
               strcmp(hostapd_config[WLAN1_HW_MODE].value, WIFI_BAND_5G_VALUE) == 0 &&
               strcmp(hostapd_config[WLAN1_CHANNEL].value, "36") == 0) {
        return WIFI_BAND_5G;
    } else if (strcmp(hostapd_config[HW_MODE].value, WIFI_BAND_24G_VALUE) == 0 &&
               strcmp(hostapd_config[CHANNEL].value, "6") == 0 &&
               strcmp(hostapd_config[WLAN1_HW_MODE].value, WIFI_BAND_5G_VALUE) == 0 &&
               strcmp(hostapd_config[WLAN1_CHANNEL].value, "36") == 0) {
        return WIFI_BAND_ALL;
    }
    return WIFI_BAND_24G;
*/
}

//
void get_centr_channel(int band, char* channel, int bandwidth, char* return_channel) {
    //reference: 80-Y6630-2,
    //           Regulatory Configuration for QCA WLAN

    //need cause bandwidth decide centr_freq..
    //HostapdConfig* config = get_config(band);

    int select_bw;
    //select_bw = get_wifi_bandwidth(band);
    select_bw = bandwidth;

    int select_ch;
    select_ch = atoi(channel);
    log_d("[ch][bw] get_centr_channel, select_bw:%d, band:%d, channel:%s\n", select_bw, band, channel);

    //if bw == 80
    if ((select_bw == WIFI_BANDWIDTH_80MHZ)) {
    log_d("[ch][bw] get_centr_channel, select_bw:%d, band:%d, channel:%s\n", select_bw, band, channel);
        if ((select_ch >= 36) && (select_ch <= 48)) {
           strcpy(return_channel, "42");
        //} else if ((select_ch >= 52) && (select_ch <= 64)) {
        //    strcpy(return_channel, "58");
        //} else if ((select_ch >= 100) && (select_ch <= 112)) {
        //    strcpy(return_channel, "106");
        //} else if ((select_ch >= 116) && (select_ch <= 128)) {
        //    strcpy(return_channel, "122");
        //} else if ((select_ch >= 132) && (select_ch <= 144)) {
        //    strcpy(return_channel, "138");
        } else if ((select_ch >= 149) && (select_ch <= 161)) {
           strcpy(return_channel, "155");
        }

    //if bw == 40
    } else if ((select_bw == WIFI_BANDWIDTH_20OR40MHZ)) {
    log_d("[ch][bw] get_centr_channel, select_bw:%d, band:%d, channel:%s\n", select_bw, band, channel);
        if ((select_ch >= 36) && (select_ch <= 40)) {
           strcpy(return_channel, "38");
        } else if ((select_ch >= 44) && (select_ch <= 48)) {
           strcpy(return_channel, "46");
        //} else if ((select_ch >= 52) && (select_ch <= 56)) {
        //    strcpy(return_channel, "54");
        //} else if ((select_ch >= 60) && (select_ch <= 64)) {
        //    strcpy(return_channel, "62");
        //} else if ((select_ch >= 100) && (select_ch <= 104)) {
        //    strcpy(return_channel, "102");
        //} else if ((select_ch >= 108) && (select_ch <= 112)) {
        //    strcpy(return_channel, "110");
        //} else if ((select_ch >= 116) && (select_ch <= 120)) {
        //    strcpy(return_channel, "118");
        //} else if ((select_ch >= 124) && (select_ch <= 128)) {
        //    strcpy(return_channel, "126");
        //} else if ((select_ch >= 132) && (select_ch <= 136)) {
        //    strcpy(return_channel, "134");
        //} else if ((select_ch >= 140) && (select_ch <= 140)) {
        //    strcpy(return_channel, "142");
        } else if ((select_ch >= 149) && (select_ch <= 153)) {
            strcpy(return_channel, "151");
        } else if ((select_ch >= 157) && (select_ch <= 161)) {
            strcpy(return_channel, "159");
        }

    //if bw == 20
    } else if ((select_bw == WIFI_BANDWIDTH_20MHZ)) {
    //no need for centr_freq
    }
    log_d("[ch][bw] return_channel:%s\n",return_channel);
    //return return_channel;
}
//
bool find_HT40_minus(int channel) {
     
     //reference: SR#04621642
     int HT40_minus_chan[] = {40, 48, 56, 64, 104, 112, 120, 128, 136, 153, 161};
     int HT40_minus_count = sizeof(HT40_minus_chan)/sizeof(HT40_minus_chan[0]);
     int i;     

     for (i=0; i<HT40_minus_count; i++) {
         if (channel == HT40_minus_chan[i]) {
             log_e("[ch][bw] match in find_HT40_minus.\n");
             return true;
             //break;
         }
     }
     return false;
}
//
int get_wifi_bandwidth(int band) {
    HostapdConfig* config = get_config(band);

    if (config[HT_CAPAB].value == NULL || config[VHT_OPER_CHWIDTH].value == NULL) {
        return WIFI_BANDWIDTH_20MHZ;
    }

    // 20MHZ and 20 or 40MHZ
    if (strcmp(config[VHT_OPER_CHWIDTH].value, "0") == 0) {
        if (strcmp(config[HT_CAPAB].value, "[SHORT-GI-20]") == 0) {
            return WIFI_BANDWIDTH_20MHZ;
        } else if (strcmp(config[HT_CAPAB].value, "[HT40+]") == 0) {
            return WIFI_BANDWIDTH_20OR40MHZ;
        } else if (strcmp(config[HT_CAPAB].value, "[HT40-]") == 0) {
            return WIFI_BANDWIDTH_20OR40MHZ;
        }
    }

    // 80MHZ
    if (strcmp(config[VHT_OPER_CHWIDTH].value, "1") == 0 &&
           ((strcmp(config[HT_CAPAB].value, "[HT40+]") == 0) ||
           (strcmp(config[HT_CAPAB].value, "[HT40-]") == 0))) {
        return WIFI_BANDWIDTH_80MHZ;
    }

    return WIFI_BANDWIDTH_20MHZ;
}
//
//for hostapd.conf, [USAGE] get_wlan_channel
char* get_wlan_channel(int band) {
    HostapdConfig* config = get_config(band);

    if (config[CHANNEL].value == NULL) {
        return "0";
    }

    log_d("[ch][bw] config[CHANNEL].value=%s\n",config[CHANNEL].value);
    return config[CHANNEL].value;
}

int get_connected_number() {
    return total_connected_number;
}

void user_connected_update(CONNECT_STATE state) {
// might be missing some state change event, so remove accumulation, and
// dump each time state changes
#if (0)
    int count = total_connected_number;
    if (state == USER_CONNECTED) {
        count++;
    } else {
        count = (count > 0) ? (count - 1) : 0;
    }
#else
    int count = dump_connected_users();
#endif

    if (auto_close_enabled) {
        // Check if need to reset wifi auto close timer
        if ((count == 0 && total_connected_number > count) ||
             (total_connected_number == 0 && count > 0)) {
            total_connected_number = count;
            wifi_close_task_refresh();
        }
    }

    total_connected_number = count;
    //update_hotspot_user_counter(); // let main thread do it
    log_d("total wifi connected users: %d", total_connected_number);
}

void dump_usr_connect_log(int count) {
    char datetime[17];
    char cmd[200];
    memset(datetime, 0, sizeof(datetime));
    memset(cmd, 0, sizeof(cmd));

    // dump date info
    time_t t = time(NULL);
    struct tm *now = localtime(&t);
    strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M", now);
    sprintf(cmd, "echo %s connected users %d >> %s", datetime, count, HOSTAPD_CONNECTED_FILE);
    systemCmd(cmd);

    if (!count) return; // no need to parse if count becomes zero

    int i;
    for (i = WIFI_BAND_24G; i <= WIFI_BAND_5G; i++) {
        if (get_wifi_band_enabled(i)) {
#ifdef FEATURE_ROUTER
            sprintf(cmd, "hostapd_cli -i %s -p /var/run/hostapd/ all_sta | grep -e ^[a-z0-9]*:[a-z0-9]*:[a-z0-9]*:[a-z0-9]*:[a-z0-9]*:[a-z0-9]* >> %s", (i == WIFI_BAND_24G) ? BAND_24G:BAND_5G, HOSTAPD_CONNECTED_FILE);
#else
            sprintf(cmd, "cat Data_Store/connected.txt >> %s", HOSTAPD_CONNECTED_FILE);
#endif
            systemCmd(cmd);
        }
    }
}

int dump_connected_users() {
    int i, count = 0;
    for (i = WIFI_BAND_24G; i <= WIFI_BAND_5G; i++) {
        if (get_wifi_band_enabled(i)) {
            FILE *fp = NULL;
#ifdef FEATURE_ROUTER
            char cmd[150];
            sprintf(cmd, "hostapd_cli -i %s -p /var/run/hostapd/ all_sta | grep AUTHORIZED",
                         (i == WIFI_BAND_24G) ? BAND_24G:BAND_5G);
            fp = popen(cmd, "r");
#endif
            if (fp == NULL) {
                log_e("error dump for connected users info");
                return count;
            }

            char buffer[MAX_CONFIG_LENGTH];
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                char* error = strstr(buffer, "Failed");
                if (error == NULL) { //command return without fail
                    count++;
                }
            }
            pclose(fp);
        }
    }
    log_d("hostapd connected users %d", count);

    if (connected_users_dump && total_connected_number != count) {
        dump_usr_connect_log(count);
    }

    return count;
}

void monitor_wlan() {
    dump_wifi_band_state();
}

void startMonitorWlan() {
    //Stop monitor wlan state which will casue wlan driver unstable
    //if (lv_task == NULL) {
    //    lv_task = lv_task_create(monitor_wlan, 5000, LV_TASK_PRIO_LOW, NULL);
    //}
}

char* get_active_wlan_ssid() {
#ifdef HIGH_SPEED_WIFI_DOWN
    if (ds_get_bool(DS_KEY_HIGH_SPEED)) {
        return "";
    }
#endif
    return (active_ssid_name == NULL) ? "":active_ssid_name;
}

void update_active_wlan_ssid() {
    if (active_ssid_name != NULL) {
        lv_mem_free(active_ssid_name);
        active_ssid_name = NULL;
    }

    if (hostapd_config[SSID].value == NULL || wlan1_config[SSID].value == NULL) {
        return;
    }

    int len = 0;
    if (current_wifi_band == WIFI_BAND_ALL &&
                strcmp(hostapd_config[SSID].value, wlan1_config[SSID].value) != 0) {
        len = strlen(hostapd_config[SSID].value) + 1 + strlen(wlan1_config[SSID].value) + 1 + sizeof(SLASH_BTW_SSID);
        active_ssid_name = lv_mem_alloc(len + 1);
        memset(active_ssid_name, 0, len + 1);
        sprintf(active_ssid_name, "%s%s%s", hostapd_config[SSID].value, SLASH_BTW_SSID, wlan1_config[SSID].value);
    } else if (current_wifi_band != WIFI_BAND_NONE) {
        HostapdConfig* config = get_config(current_wifi_band);
        len = strlen(config[SSID].value) + 1;
        active_ssid_name = lv_mem_alloc(len + 1);
        memset(active_ssid_name, 0, len + 1);
        sprintf(active_ssid_name, "%s", config[SSID].value);
    }

    update_ssid();
}

char* get_wlan_ssid(int band) {
    HostapdConfig* config = get_config(band);

    if (config[SSID].value == NULL) {
        return "COMPAL-168";
    }

    return config[SSID].value;
}

char* get_wlan_password(int band) {
    HostapdConfig* config = get_config(band);

    if (get_wlan_security(band) == SECURITY_WEP) {
        if (config[WEP_KEY0].value != NULL)
            return config[WEP_KEY0].value;
    } else {
        if (config[WPA_PASSPHRASE].value != NULL) {
            return config[WPA_PASSPHRASE].value;
        } else if (config[WPA_PSK].value != NULL) {
            return config[WPA_PSK].value;
        }
    }
    return "12345678";
}

char* get_wlan_security(int band) {
    HostapdConfig* config = get_config(band);

    if (config[WPA].value == NULL) {
        return SECURITY_NONE;
    }

    if (strcmp(config[WPA].value, WIFI_SECURITY_NONE_WEP) == 0) {
        if (config[WEP_KEY0].value != NULL) {
            return SECURITY_WEP;
        } else {
            return SECURITY_NONE;
        }
    } else if (strcmp(config[WPA].value, WIFI_SECURITY_WPA_PSK) == 0) {
        return SECURITY_WPA_PSK;
    } else if (strcmp(config[WPA].value, WIFI_SECURITY_WPA2_PSK) == 0) {
        if (strcmp(config[WPA_KEY_MGMT].value, WPA_KEY_MGMT_WPA_PSK) == 0) {
            return SECURITY_WPA2_PSK;
        } else {
            return SECURITY_WPA3_WPA2;
        }
    }
    return SECURITY_NONE;
}

char* get_wlan_security_string(int band) {
    HostapdConfig* config = get_config(band);

    if (config[WPA].value == NULL) {
        return get_string(ID_WIFI_SECURITY_NONE);
    }

    if (strcmp(config[WPA].value, WIFI_SECURITY_NONE_WEP) == 0) {
        if (config[WEP_KEY0].value != NULL) {
            return get_string(ID_WIFI_SECURITY_AUTO);
        } else {
            return get_string(ID_WIFI_SECURITY_NONE);
        }
    } else if (strcmp(config[WPA].value, WIFI_SECURITY_WPA_PSK) == 0) {
        return get_string(ID_WIFI_SECURITY_WPA_PSK);
    } else if (strcmp(config[WPA].value, WIFI_SECURITY_WPA2_PSK) == 0) {
        if (strcmp(config[WPA_KEY_MGMT].value, WPA_KEY_MGMT_WPA_PSK) == 0) {
            return get_string(ID_WIFI_SECURITY_WPA2_PSK);
        } else {
            return get_string(ID_WIFI_SECURITY_WPA3_WPA2_MIXED_MODE);
        }
    }
    return get_string(ID_WIFI_SECURITY_NONE);
}

bool get_wlan_pmf(int band) {
    HostapdConfig* config = get_config(band);

    if (config[IEEE80211W].value == NULL) {
        return false;
    }

    if (strcmp(config[IEEE80211W].value, "1") == 0) {
        return true;
    } else {
        return false;
    }
}

bool get_wlan_hide_ssid(int band) {
    HostapdConfig* config = get_config(band);

    if (config[IGNORE_BROADCAST_SSID].value == NULL) {
        return false;
    }

    if (strcmp(config[IGNORE_BROADCAST_SSID].value, "2") == 0) {
        return true;
    } else {
        return false;
    }
}

bool get_wlan_wps_state(int band) {
    HostapdConfig* config = get_config(band);

    if (config[EAP_SERVER].value == NULL || config[WPS_STATE].value == NULL) {
        return false;
    }

    if (strcmp(config[EAP_SERVER].value, "1") == 0 &&
               strcmp(config[WPS_STATE].value, "2") == 0) {
        return true;
    } else {
        return false;
    }
}

void load_config(char * filepath, char * startwith, char ** config_value) {
    char cmd[MAX_CMD_LENGTH];
    sprintf(cmd, "grep -a \"^%s=\" %s", startwith, filepath);

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        log_e("%s popen failed", filepath);
        assert(false);
        return;
    }

    char buffer[MAX_CONFIG_LENGTH];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    if (buffer != NULL && strlen(buffer) != 0) {
        char *found = buffer;
        found += strlen(startwith) + 1;
        char * value = NULL;
        size_t length = strlen(found);
        value = lv_mem_alloc(length);
        strncpy(value, found , length);
        size_t index = strcspn(value, "\n");
        if(index < length){
            value[index] = 0;
        }
        *config_value = value;
    }
    pclose(fp);
}

void load_hostapd_config() {
    memset(hostapd_config, 0, sizeof(hostapd_config));
    memset(wlan1_config, 0, sizeof(wlan1_config));

    int i;
    for (i = 0; i < MAX_CONFIG_CNT; i++) {
        //hostapd_config[i].name = config_name[i];
        load_config(HOSTAPD_CONFIG_PATH, config_name[i], &hostapd_config[i].value);
        load_config(WLAN1_CONFIG_PATH, config_name[i], &wlan1_config[i].value);
        //log_d("hostapd_config[%d] name: %s, value: %s", i, config_name[i], hostapd_config[i].value);
        //log_d("wlan1_config[%d] name: %s, value: %s", i, config_name[i], wlan1_config[i].value);
    }
}

void enableWLANinterface() {
#ifdef FEATURE_ROUTER
    if(EnableWLAN()) {
        log_d("EnableWLAN interface success");
    }
#endif
    // disable the disabled band
    if (!ds_get_bool(DS_KEY_WIFI_24G_ENABLED)) {
        write_wifi_band_enabled(WIFI_BAND_24G, false);
    }
    if (!ds_get_bool(DS_KEY_WIFI_5G_ENABLED)) {
        write_wifi_band_enabled(WIFI_BAND_5G, false);
    }
}

int start_hostapd_daemon(int band) {
    int res_24g = -1, res_5g = -1;
    if (band & WIFI_BAND_24G) {
        res_24g = systemCmd("hostapd -i wlan0 /data/misc/wifi/hostapd.conf -B");
        log_d("Start 2.4GHz hostapd daemon command status: %d", res_24g);
        if (res_24g == 0) {
            res_24g = systemCmd("hostapd_cli -i wlan0 -p /var/run/hostapd/ -a /usr/bin/QCMAP_StaInterface -B");
            log_d("Start 2.4GHz hostapd_cli command status: %d", res_24g);
        }
    }
    if (band & WIFI_BAND_5G) {
        res_5g = systemCmd("hostapd -i wlan1 /data/misc/wifi/hostapd-wlan1.conf -B");
        log_d("Start 5GHz hostapd daemon command status: %d", res_5g);
        if (res_5g == 0) {
            res_5g = systemCmd("hostapd_cli -i wlan1 -p /var/run/hostapd/ -a /usr/bin/QCMAP_StaInterface -B");
            log_d("Start 5GHz hostapd_cli command status: %d", res_5g);
        }
    }
    int res = -1;
    if (band == WIFI_BAND_24G) res = res_24g;
    if (band == WIFI_BAND_5G) res = res_5g;
    if (band == WIFI_BAND_ALL) res = res_24g | res_5g;
    log_d("start_hostapd_daemon for band: %d, res: %d", band, res);
    return res;
}

void start_hostapd_daemon_with_check(int band) {
    char cmd[MAX_CMD_LENGTH];
    if (band == WIFI_BAND_24G) {
        sprintf(cmd, "ps | grep [w]lan0");
    } else {
        sprintf(cmd, "ps | grep [w]lan1");
    }

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        log_e("grep hostapd process failed");
        return;
    }

    char buffer[MAX_CONFIG_LENGTH];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    if (strlen(buffer) == 0) {
        if (start_hostapd_daemon(band) != 0) {
            enableWLANinterface();
        }
    }
    pclose(fp);
}

int kill_hostapd(int band) {
    int res = 0;
#ifdef FEATURE_ROUTER
    FILE *fp = NULL;

    char cmd[150];
    if (band == WIFI_BAND_24G) {
        sprintf(cmd, "pgrep -f hostapd.conf");
    } else {
        sprintf(cmd, "pgrep -f hostapd-wlan1.conf");
    }
    fp = popen(cmd, "r");

    if (fp == NULL) {
        log_e("error get hostapd pid");
        return res;
    }

    char buffer[MAX_CONFIG_LENGTH];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        sprintf(cmd, "kill -9 %s", buffer);
        res = systemCmd(cmd);
    }
    pclose(fp);
#endif
    return res;
}

void stop_hostapd_daemon(int band) {
    int res = -1;
    if (band & WIFI_BAND_24G) {
        //res = systemCmd("kill -9 `pgrep -f hostapd.conf`");
        res = kill_hostapd(WIFI_BAND_24G);
        log_d("Stop 2.4GHz hostapd daemon command status: %d", res);
    }
    if (band & WIFI_BAND_5G) {
        //res = systemCmd("kill -9 `pgrep -f hostapd-wlan1.conf`");
        res = kill_hostapd(WIFI_BAND_5G);
        log_d("Stop 5GHz hostapd daemon command status: %d", res);
    }

    // dump again since we kill hostapd so any disconnect we won't receive notify
    total_connected_number = dump_connected_users();
    //update_hotspot_user_counter(); // let main thread do it
}

int restart(int band) {
    int res = -1;
    int failed_band = WIFI_BAND_NONE;
    if (get_wifi_band_enabled(band)) {
        if (band & WIFI_BAND_24G) {
            stop_hostapd_daemon(WIFI_BAND_24G);
            res = systemCmd("ifconfig wlan0 down");
            log_d("wlan0 down command status: %d", res);
            if (ds_get_bool(DS_KEY_WIFI_24G_ENABLED)) {
                res = systemCmd("ifconfig wlan0 up");
                log_d("wlan0 up command status: %d", res);
                if (restart_sleep_flag) {
                    log_d("wait 3 sec afer wlan0 interface up before we start hostapd daemon");
                    usleep(3000000);
                }
                if (start_hostapd_daemon(WIFI_BAND_24G) != 0) {
                    failed_band |= WIFI_BAND_24G;
                }
            }
        }
        if (band & WIFI_BAND_5G) {
            stop_hostapd_daemon(WIFI_BAND_5G);
            res = systemCmd("ifconfig wlan1 down");
            log_d("wlan1 down command status: %d", res);
            if (ds_get_bool(DS_KEY_WIFI_5G_ENABLED)) {
                res = systemCmd("ifconfig wlan1 up");
                log_d("wlan1 up command status: %d", res);
                if (restart_sleep_flag) {
                    log_d("wait 3 sec afer wlan1 interface up before we start hostapd daemon");
                    usleep(3000000);
                }
                if (start_hostapd_daemon(WIFI_BAND_5G) != 0) {
                    failed_band |= WIFI_BAND_5G;
                }
            }
        }
    } else {
        stop_hostapd_daemon(band);
    }
    return failed_band;
}

void exec_commands_thread(void *arg)
{
    int band = *(int *)arg;
    int res = -1;
    int failed_band = WIFI_BAND_NONE;

    failed_band = restart(band);

    restart_sleep_flag = false;

    if (failed_band != WIFI_BAND_NONE) {
        log_d("start_hostapd_daemon error, wait 5 sec to restart wlan for failed band: %d", failed_band);
        restart_failed_band(failed_band);
    }

    pthread_exit(NULL);
}

void exec_commands(int band) {
#ifdef FEATURE_ROUTER
    static int band_conf = WIFI_BAND_ALL;
    band_conf = band;
    memset(&cmd_exec_thread, 0, sizeof(pthread_t));
    int rc = pthread_create(&cmd_exec_thread, NULL, exec_commands_thread, (void*)&band_conf);
    if (rc) log_e("create cmd_exec_thread failed");
#endif
}

bool wps_support() {
    if ((get_wifi_band_enabled(WIFI_BAND_24G) && get_wlan_security(WIFI_BAND_24G) == SECURITY_WEP) ||
            (get_wifi_band_enabled(WIFI_BAND_5G) && get_wlan_security(WIFI_BAND_5G) == SECURITY_WEP)) {
        return false;
    } else {
        return true;
    }
}

void start_wps(char * pin) {
#ifdef FEATURE_ROUTER
    char cmd[MAX_CMD_LENGTH];
    memset(cmd, 0, sizeof(cmd));
    int res = -1;

    int band;
    for (band = WIFI_BAND_5G; band >= WIFI_BAND_24G; band--) {
        if (get_wifi_band_enabled(band)) {
            char* interface = (band == WIFI_BAND_24G) ? BAND_24G : BAND_5G;
            if (pin == NULL) {
                sprintf(cmd, "hostapd_cli -i %s -p /var/run/hostapd/ wps_pbc", interface);
                res = systemCmd(cmd);
            } else {
                sprintf(cmd, "hostapd_cli -i %s -p /var/run/hostapd/ wps_pin any %s", interface, pin);
                res = systemCmd(cmd);
            }
            log_d("start wps cmd: %s, status: %d", cmd, res);
        }
    }
#endif
}

void restart_wlan(int band) {
    exec_commands(band);
}

void restart_commands_thread(void *arg)
{
    int band = *(int *)arg;
    usleep(5000000);
    log_d("restart_commands_thread for band: %d", band);
    restart_wlan(band);
}

void restart_failed_band(int failed_band) {
#ifdef FEATURE_ROUTER
    static int band_conf = WIFI_BAND_ALL;
    band_conf = failed_band;
    memset(&restart_failed_band_thread, 0, sizeof(pthread_t));
    int rc = pthread_create(&restart_failed_band_thread, NULL, restart_commands_thread, (void*)&band_conf);
    if (rc) log_e("create restart_failed_band_thread failed");
#endif
}

void update_config_file(char * old, char * replace, char * dest, char * temp_dest) {
    char cmd[MAX_CMD_LENGTH];
    sprintf(cmd, "sed \"s/%s/%s/g\" %s > %s", old, replace, dest, temp_dest);
    log_d("Update hostapd value: %s", cmd);

    if (systemCmd(cmd) == 0) {
        sprintf(cmd, "cp %s %s", temp_dest, dest);
        if (systemCmd(cmd) != 0) {
            log_e("Copy %s file failed", dest);
            hostapd_conf_reload();
        }
    } else {
        log_e("Update %s failed", dest);
        hostapd_conf_reload();
    }
    sync();
}

bool special_char(char character) {
    static const char SPEC_CHAR_LIST[] = "[]$*/&\"\\";

    int i;
    for (i = 0; i < strlen(SPEC_CHAR_LIST); i++) {
        if (character == SPEC_CHAR_LIST[i]) {
            return true;
        }
    }
    return false;
}

void special_char_check(char * string) {
    int i, offset = 0, len = strlen(string);
    for (i = 0; i < len; i++) {
        int j = i + offset;
        if (special_char(string[j])) {
            char tmp[MAX_CONFIG_LENGTH];
            memset(tmp, 0, MAX_CONFIG_LENGTH);
            if (string[j] == '\\') {
                // special handle for backslash by replacing each \ with four backslashes
                strncpy(tmp, string + j + 1, strlen(string + j + 1));
                strncpy(string + j + 4, tmp, strlen(tmp));
                strncpy(string + j, "\\\\\\\\", 4);
                offset+=3; // no need to include the backslash user input
            } else {
                strncpy(tmp, string + j, strlen(string + j));
                strncpy(string + j + 1, tmp, strlen(tmp));
                strncpy(string + j, "\\", 1);
                offset++;
            }
        }
    }
#if (0)
    int pos = strcspn(string, special_char);
    char tmp[MAX_CONFIG_LENGTH];
    memset(tmp, 0, MAX_CONFIG_LENGTH);
    strncpy(tmp, string + pos, strlen(string + pos));

    strncpy(string + pos + 1, tmp, strlen(tmp));
    strncpy(string + pos, "\\", 1);
#endif
}

void disable_wep(int band, bool disable) {
    //Disable wep_default_key, wep_key0, auth_algs, ie add # in conf file
    if (disable) {
        disable_config(band, WEP_KEY0);
        disable_config(band, WEP_DEFAULT_KEY);
        disable_config(band, AUTH_ALGS);
    } else {
        enable_config(band, WEP_KEY0);
        enable_config(band, WEP_DEFAULT_KEY);
        enable_config(band, AUTH_ALGS);
    }
}

void change_config_state(int band, int index, bool disable) {
    char *dest, *temp_dest;
    if (band == WIFI_BAND_24G) {
        dest = HOSTAPD_CONFIG_PATH;
        temp_dest = HOSTAPD_CONFIG_TEMP_PATH;
    } else {
        dest = WLAN1_CONFIG_PATH;
        temp_dest = WLAN1_CONFIG_TEMP_PATH;
    }

    char cmd[MAX_CMD_LENGTH];
    if (disable) {
        sprintf(cmd, "sed 's/^%s=/#%s=/g' %s > %s", config_name[index], config_name[index], dest, temp_dest);
    } else {
        sprintf(cmd, "sed 's/^#%s=/%s=/g' %s > %s", config_name[index], config_name[index], dest, temp_dest);
    }
    log_d("Enable/Disable config cmd: %s", cmd);

    if (systemCmd(cmd) == 0) {
        sprintf(cmd, "cp %s %s", temp_dest, dest);
        if (systemCmd(cmd) != 0) {
            log_e("Copy %s file failed", dest);
            hostapd_conf_reload();
        }
    } else {
        log_e("Update %s failed", dest);
        hostapd_conf_reload();
    }
    sync();

    HostapdConfig* config = get_config(band);

    if (disable) {
        lv_mem_free(config[index].value);
        config[index].value = NULL;
    }
    load_config(dest, config_name[index], &config[index].value);
}

void disable_config(int band, int index) {
    HostapdConfig* config = get_config(band);
    if (config[index].value != NULL) {
        change_config_state(band, index, true);
    }
}

void enable_config(int band, int index) {
    HostapdConfig* config = get_config(band);
    if (config[index].value == NULL) {
        change_config_state(band, index, false);
    }
}

void update_value(int band, int index, char* value) {
    struct stat buffer;
    char *dest, *temp_dest;
    int exist;
    if (band == WIFI_BAND_24G) {
        dest = HOSTAPD_CONFIG_PATH;
        temp_dest = HOSTAPD_CONFIG_TEMP_PATH;
        exist = stat(HOSTAPD_CONFIG_PATH, &buffer);
    } else {
        dest = WLAN1_CONFIG_PATH;
        temp_dest = WLAN1_CONFIG_TEMP_PATH;
        exist = stat(WLAN1_CONFIG_PATH, &buffer);
    }
    if (exist != 0) {
        log_e("%s not exist", dest);
        assert(false);
        return;
    }

    if(index < MAX_CONFIG_CNT){
        HostapdConfig* config = get_config(band);

        char old[MAX_CONFIG_LENGTH];
        char replace[MAX_CONFIG_LENGTH];
        memset(&old, 0, MAX_CONFIG_LENGTH);
        memset(&replace, 0, MAX_CONFIG_LENGTH);

        sprintf(old, "^%s=%s", config_name[index], config[index].value);

        lv_mem_free(config[index].value);
        config[index].value = lv_mem_alloc(strlen(value) + 1);
        memset(config[index].value, 0, strlen(value) + 1);
        strncpy(config[index].value, value , strlen(value));

        if (index == HT_CAPAB || index == SSID || index == WPA_PASSPHRASE) {
            special_char_check(old);

            // fix ST1 issue where can not modify ssid if ends with $ char
            // sed uses basic regular expressions where $ will be used to
            // matches the ending position of the string
            int len = strlen(old) - 1;
            if(!strncmp(old + len, "$", 1)) {
                strncpy(old + len - 1, "." , 1);
            }
        }

        sprintf(replace, "%s=%s", config_name[index], config[index].value);

        if (index == HT_CAPAB || index == SSID || index == WPA_PASSPHRASE) {
            special_char_check(replace);
        }

        update_config_file(old, replace, dest, temp_dest);
    }
}

bool value_valid_check(int band, int index) {
    HostapdConfig* config = get_config(band);
    if (config[index].value != NULL) {
        return true;
    } else {
        return false;
    }
}

void write_wlan_ssid(int band, char* value) {
    if (value_valid_check(band, SSID) && value != NULL && value[0] != '\0') {
        update_value(band, SSID, value);
        restart_wlan(band);
    }
    update_active_wlan_ssid();
}

bool write_wlan_password(int band, char* value) {
    if (value != NULL && value[0] != '\0'/* && strlen(value) == 8*/) {

        if (get_wlan_security(band) == SECURITY_WEP && value_valid_check(band, WEP_KEY0)) {
            update_value(band, WEP_KEY0, value);
        } else {
            if (strlen(value) < MAX_PASSWORD_LEN) {
                if (!value_valid_check(band, WPA_PASSPHRASE)) {
                    enable_config(band, WPA_PASSPHRASE);
                    disable_config(band, WPA_PSK);
                }
                update_value(band, WPA_PASSPHRASE, value);
            } else if (strlen(value) == MAX_PASSWORD_LEN) {
                if (!value_valid_check(band, WPA_PSK)) {
                    enable_config(band, WPA_PSK);
                    disable_config(band, WPA_PASSPHRASE);
                }
                update_value(band, WPA_PSK, value);
            }
        }

        restart_wlan(band);
        return true;
    }
    return false;
}

void write_wlan_security(int band, char* value) {

    if (value_valid_check(band, WPA) && value != NULL && value[0] != '\0') {
        if (strcmp(value, SECURITY_WPA_PSK) == 0) {
            update_value(band, WPA, WIFI_SECURITY_WPA_PSK);
            update_value(band, WPA_KEY_MGMT, WPA_KEY_MGMT_WPA_PSK);
            update_value(band, SAE_REQUIRE_MFP, "0");
            disable_wep(band, true);
            write_wlan_pmf(band, false);
        } else if (strcmp(value, SECURITY_WPA2_PSK) == 0) {
            update_value(band, WPA, WIFI_SECURITY_WPA2_PSK);
            update_value(band, WPA_KEY_MGMT, WPA_KEY_MGMT_WPA_PSK);
            update_value(band, SAE_REQUIRE_MFP, "0");
            disable_wep(band, true);
            write_wlan_pmf(band, false);
        } else if (strcmp(value, SECURITY_WPA3_WPA2) == 0) {
            update_value(band, WPA, WIFI_SECURITY_WPA2_PSK);
            update_value(band, WPA_KEY_MGMT, WPA_KEY_MGMT_SAE_WPA_PSK);
            update_value(band, SAE_REQUIRE_MFP, "1");
            disable_wep(band, true);
            write_wlan_pmf(band, true);
        } else if (strcmp(value, SECURITY_WEP) == 0) {
            update_value(band, WPA, WIFI_SECURITY_NONE_WEP);
            update_value(band, WPA_KEY_MGMT, WPA_KEY_MGMT_WPA_PSK);
            update_value(band, SAE_REQUIRE_MFP, "0");
            disable_wep(band, false);
            write_wlan_pmf(band, false);
        } else if (strcmp(value, SECURITY_NONE) == 0) {
            update_value(band, WPA, WIFI_SECURITY_NONE_WEP);
            update_value(band, WPA_KEY_MGMT, WPA_KEY_MGMT_WPA_PSK);
            update_value(band, SAE_REQUIRE_MFP, "0");
            disable_wep(band, true);
            write_wlan_pmf(band, false);
        }
        restart_wlan(band);
    }
}

bool write_wlan_pmf(int band, bool enable) {
    if (value_valid_check(band, IEEE80211W)) {
        if (enable) {
            update_value(band, IEEE80211W, "1");
        } else {
            update_value(band, IEEE80211W, "0");
        }
        restart_wlan(band);
        return true;
    }
    return false;
}

// Disable 802.1 ac, 802.1 ax and 802.1 n in both WiFi 2.4G and 5G if set to Fixed Mode
void wlan_fixed_mode(bool enable) {
    int band;
    for (band = WIFI_BAND_24G; band <= WIFI_BAND_5G; band++) {
        if (value_valid_check(band, IEEE80211AC) && value_valid_check(band, IEEE80211AX) &&
                  value_valid_check(band, IEEE80211N)) {
            if (enable) {
                update_value(band, IEEE80211AC, "1");
                update_value(band, IEEE80211AX, "1");
                update_value(band, IEEE80211N, "1");
            } else {
                update_value(band, IEEE80211AC, "0");
                update_value(band, IEEE80211AX, "0");
                update_value(band, IEEE80211N, "0");
            }
            restart_wlan(band);
        }
    }
}

bool write_wlan_hide_ssid(int band, bool enable) {
    if (value_valid_check(band, IGNORE_BROADCAST_SSID)) {
        if (enable) {
            update_value(band, IGNORE_BROADCAST_SSID, "2");
        } else {
            update_value(band, IGNORE_BROADCAST_SSID, "0");
        }
        restart_wlan(band);
        return true;
    }
    return false;
}

bool write_wlan_wps_state(bool enable) {
    if (value_valid_check(WIFI_BAND_24G, EAP_SERVER) && value_valid_check(WIFI_BAND_24G, WPS_STATE)) {
        if (enable) {
            update_value(WIFI_BAND_24G, EAP_SERVER, "1");
            update_value(WIFI_BAND_24G, WPS_STATE, "2");
            update_value(WIFI_BAND_5G, EAP_SERVER, "1");
            update_value(WIFI_BAND_5G, WPS_STATE, "2");
        } else {
            update_value(WIFI_BAND_24G, EAP_SERVER, "0");
            update_value(WIFI_BAND_24G, WPS_STATE, "0");
            update_value(WIFI_BAND_5G, EAP_SERVER, "0");
            update_value(WIFI_BAND_5G, WPS_STATE, "0");
        }
        restart_wlan(WIFI_BAND_ALL);
        return true;
    }
    return false;
}

bool write_wifi_band_enabled(int band, bool enable) {
    int res = -1;
    if (band == WIFI_BAND_24G) {
        if (enable) {
#ifdef FEATURE_ROUTER
            res = systemCmd("ifconfig wlan0 up");
            log_d("band enable wlan0 up command status: %d", res);
            if (res != 0) enableWLANinterface();
            start_hostapd_daemon_with_check(WIFI_BAND_24G);
#endif
            current_wifi_band |= WIFI_BAND_24G;
        } else {
#ifdef FEATURE_ROUTER
            stop_hostapd_daemon(band);
            res = systemCmd("ifconfig wlan0 down");
            log_d("band disable wlan0 down command status: %d", res);
#endif
            current_wifi_band &= ~WIFI_BAND_24G;
        }
    } else {
        if (enable) {
#ifdef FEATURE_ROUTER
            res = systemCmd("ifconfig wlan1 up");
            log_d("band enable wlan1 up command status: %d", res);
            if (res != 0) enableWLANinterface();
            start_hostapd_daemon_with_check(WIFI_BAND_5G);
#endif
            current_wifi_band |= WIFI_BAND_5G;
        } else {
#ifdef FEATURE_ROUTER
            stop_hostapd_daemon(band);
            res = systemCmd("ifconfig wlan1 down");
            log_d("band disable wlan1 down command status: %d", res);
#endif
            current_wifi_band &= ~WIFI_BAND_5G;
        }
    }
    update_active_wlan_ssid();
    //update_wifi_band(); // let main thread do it

    return true;
}

void write_wifi_band(int band) {
/* obsolete code
    if (hostapd_config[HW_MODE].value != NULL && hostapd_config[CHANNEL].value != NULL &&
           hostapd_config[WLAN1_HW_MODE].value != NULL && hostapd_config[WLAN1_CHANNEL].value != NULL) {
        int bandwidth = get_wifi_bandwidth();
        if (band == WIFI_BAND_24G) {
            update_value(HW_MODE, WIFI_BAND_24G_VALUE);
            update_value(CHANNEL, "6");
            update_value(WLAN1_HW_MODE, WIFI_BAND_24G_VALUE);
            update_value(WLAN1_CHANNEL, "6");
            if (bandwidth == WIFI_BANDWIDTH_80MHZ) {
                write_wifi_bandwidth(WIFI_BANDWIDTH_20OR40MHZ);
            }
        } else if (band == WIFI_BAND_5G) {
            update_value(HW_MODE, WIFI_BAND_5G_VALUE);
            update_value(CHANNEL, "36");
            update_value(WLAN1_HW_MODE, WIFI_BAND_5G_VALUE);
            update_value(WLAN1_CHANNEL, "36");
            if (bandwidth != WIFI_BANDWIDTH_80MHZ) {
                write_wifi_bandwidth(WIFI_BANDWIDTH_80MHZ);
            }
        } else {
            update_value(HW_MODE, WIFI_BAND_24G_VALUE);
            update_value(CHANNEL, "6");
            update_value(WLAN1_HW_MODE, WIFI_BAND_5G_VALUE);
            update_value(WLAN1_CHANNEL, "36");
            //TODO, should update the 2 conf file with different bandwidth values?
        }
        restart_wlan(band);
    }
*/
}

void write_wifi_bandwidth(int band, int bandwidth) {

    //need to calrify which channel selection first...
    char *channel;
    int get_channel;
    channel = get_wlan_channel(band);
    get_channel = atoi(channel);
    log_d("[ch][bw] band:%d, bandwidth:%d\n", band, bandwidth);

    if (value_valid_check(band, HT_CAPAB) && value_valid_check(band, VHT_OPER_CHWIDTH)) {

        char vht_oper_centr[MAX_CONFIG_LENGTH];
        log_d("[ch][bw] channel:%s, get_chanel:%d\n", channel, get_channel);

        if (bandwidth == WIFI_BANDWIDTH_20MHZ) {
            update_value(band, VHT_OPER_CHWIDTH, "0");
            update_value(band, HT_CAPAB, "[SHORT-GI-20]");

        } else if (bandwidth == WIFI_BANDWIDTH_20OR40MHZ) {
            //deal with bandwidth first..
            update_value(band, VHT_OPER_CHWIDTH, "0");

            //for 2.4g freq channel select above '8'
            //for 5g freq channel select 'find_HT40_minus'
            //setup with 'HT40-'
            if ((get_channel >= 8) && (band == WIFI_BAND_24G)) {
                log_d("[ch][bw] write_wlan_bandwidth, there will be 2.4g freq select higher channel, need to adjust to [HT40-] cases.\n");
                update_value(band, HT_CAPAB, "[HT40-]");
            } else if ( find_HT40_minus(get_channel) && (band == WIFI_BAND_5G)) {
                log_d("[ch][bw] write_wlan_bandwidth:%d, there will be 5g freq select option:%d, need to adjust to [HT40-] cases.\n"
                       ,bandwidth
                       ,get_channel);
                update_value(band, HT_CAPAB, "[HT40-]");

            //else, setup with 'HT40+'
            } else {
                log_d("[ch][bw] default [HT40+], suit for 2.4g freq lower channel, or 5g freq.\n");
                update_value(band, HT_CAPAB, "[HT40+]");
            }

        } else if (bandwidth == WIFI_BANDWIDTH_80MHZ && band != WIFI_BAND_24G) {

            if ( find_HT40_minus(get_channel) && (band == WIFI_BAND_5G)) {
                log_d("[ch][bw] write_wlan_bandwidth:%d, there will be 5g freq select option:%d, need to adjust to [HT40-] cases.\n"
                       ,bandwidth
                       ,get_channel);
                update_value(band, VHT_OPER_CHWIDTH, "1");
                update_value(band, HT_CAPAB, "[HT40-]");
            } else {
                update_value(band, VHT_OPER_CHWIDTH, "1");
                update_value(band, HT_CAPAB, "[HT40+]");
            }

        }

        //deal with vht_oper_centr then...
        //modified if the vht_oper_centr been changed (even when not channel changed, bandwidth will also affect centr_freq..)
        log_d("[ch][bw] AGAIN, bandwidth:%d, get_channel:%d\n",bandwidth ,get_channel);
        if (bandwidth == WIFI_BANDWIDTH_20MHZ) {
                disable_config(band, VHT_OPER_CENTR_FREQ);
                log_d("[ch][bw] disable VHT_OPER_CENTR_FREQ while 20MHz choice\n");
        } else { 
            if (get_channel == 0) {
                disable_config(band, VHT_OPER_CENTR_FREQ);
                log_d("[ch][bw] write_wlan_channel, select channel:%s without correspond centr freq\n", channel);
                log_d("[ch][bw] write_wlan_channel, disable VHT_OPER_CENTR_FREQ\n");
            } else {
                enable_config(band, VHT_OPER_CENTR_FREQ);
                get_centr_channel(band, channel, bandwidth, vht_oper_centr);
                log_d("[ch][bw] write_wlan_channel, using channel:%s, update vht_oper_centr:%s\n", channel, vht_oper_centr);
                update_value(band, VHT_OPER_CENTR_FREQ, vht_oper_centr);
            }
        }

        restart_wlan(band);
    }//if value_valid_check
}
//
void update_deny_value(int change_type, char* mac) {
    int i;
    if (change_type == DENY_ADD) {
        for (i = 0; i < HOSTAPD_DENY_CONFIG_MAX; i++) {
            if (hostapd_deny_config[i].value == NULL) {
                log_d("add to deny list index:%d, mac: %s", i, mac);
                hostapd_deny_config[i].value = lv_mem_alloc(strlen(mac) + 1);
                memset(hostapd_deny_config[i].value, 0, strlen(mac) + 1);
                strncpy(hostapd_deny_config[i].value, mac , strlen(mac));
                return;
            }
        }
    } else {
        for (i = 0; i < HOSTAPD_DENY_CONFIG_MAX; i++) {
            if (hostapd_deny_config[i].value != NULL &&
                    strcmp(hostapd_deny_config[i].value, mac) == 0) {
                log_d("remove from deny list index:%d, mac: %s", i, mac);
                lv_mem_free(hostapd_deny_config[i].value);
                hostapd_deny_config[i].value = NULL;
                int j;
                for (j = i; j < HOSTAPD_DENY_CONFIG_MAX; j++) {
                    if (hostapd_deny_config[j + 1].value != NULL) {
                        hostapd_deny_config[j].value = hostapd_deny_config[j + 1].value;
                        hostapd_deny_config[j + 1].value = NULL;
                    } else {
                        return;
                    }
                }
            }
        }
    }
}

int deny_list_count() {
    int i, count = 0;
    for (i = 0; i < HOSTAPD_DENY_CONFIG_MAX; i++) {
        if (hostapd_deny_config[i].value != NULL) {
            count++;
        }
    }
    return count;
}

bool in_deny_list(char * mac) {
    int i;
    for (i = 0; i < HOSTAPD_DENY_CONFIG_MAX; i++) {
        if (hostapd_deny_config[i].value != NULL &&
            strcmp(hostapd_deny_config[i].value, mac) == 0) {
            return true;
        }
    }
    return false;
}

char* read_deny_list(int index) {
    return hostapd_deny_config[index].value;
}

void add_to_deny_list(char * mac) {
    update_deny_value(DENY_ADD, mac);

    FILE* fp = fopen(HOSTAPD_DENY_CONFIG_PATH, "a");
    if (fp != NULL)
    {
        fprintf(fp, "%s\n", mac);
        fclose(fp);
        sync();
        restart_wlan(WIFI_BAND_ALL);
    }
}

void remove_from_deny_list(char * mac) {
    update_deny_value(DENY_REMOVE, mac);

    char cmd[MAX_CMD_LENGTH];
    sprintf(cmd, "sed -i '/%s/d' %s", mac, HOSTAPD_DENY_CONFIG_PATH);
    log_d("rm deny config cmd: %s", cmd);

    if (systemCmd(cmd) != 0) {
        log_e("Remove deny config %s failed", mac);
    }

    sync();
    restart_wlan(WIFI_BAND_ALL);
}

void clean_deny_config() {
    int i, cnt = deny_list_count();
    for (i = 0; i < cnt; i++) {
        update_deny_value(DENY_REMOVE, read_deny_list(0));
    }
}

void load_deny_config() {
    memset(hostapd_deny_config, 0, sizeof(hostapd_deny_config));

    char buffer[MAX_CONFIG_LENGTH];
    FILE* fp = fopen(HOSTAPD_DENY_CONFIG_PATH, "r");
    if (fp) {
        while(!feof(fp)) {
            if (fgets(buffer, sizeof(buffer), fp) != NULL) {
                size_t index = strcspn(buffer, "\n");
                if(index < sizeof(buffer)){
                    buffer[index] = 0;
                }
                if (strlen(buffer) > 0) {
                    char* comments = strstr(buffer, "#");
                    if (comments == NULL) {
                        update_deny_value(DENY_ADD, buffer);
                    }
                }
            }
        }
        fclose(fp);
    }
}

//For wlan_mac, get mac from nv item
void get_wlan_mac(char *wlan0_mac_buff, char *wlan1_mac_buff, int buff_len){
#ifdef FEATURE_ROUTER
    qmi_client_error_type rc;
    qmi_idl_service_object_type dms_service_obj;
    int time_out = 4;
    qmi_client_os_params  os_params;
    qmi_client_type dms_user_handle;
    qmi_client_error_type qmi_err;
    dms_get_mac_address_req_msg_v01 get_mac_address_req_msg;
    dms_get_mac_address_resp_msg_v01 get_mac_address_resp_msg;
    uint8_t tempNvMac[QMI_DMS_MAC_ADDR_MAX_V01];
    uint8_t tempUint;

    log_e("[wlan_mac] entering get_wlan_mac\n");

    if (NULL == wlan0_mac_buff || NULL == wlan1_mac_buff || 0 == buff_len){
        log_e("[wlan_mac] Buffer is empty.");
        return;
    }

    dms_service_obj = dms_get_service_object_v01();
    rc = qmi_client_init_instance( dms_service_obj,
                                   QMI_CLIENT_INSTANCE_ANY,
                                   NULL,
                                   NULL,
                                   &os_params,
                                   time_out,
                                   &dms_user_handle );
    if (rc != QMI_NO_ERR )
    {
         log_e("[wlan_mac] Error: connection not Initialized...Error Code:%d\n",rc);
         rc = qmi_client_release(dms_user_handle);
         if (rc < 0 )
         {
            log_e("[wlan_mac] Release not successful \n");
         }
         return ;
    }
    else
    {
        log_e("[wlan_mac] Connection Initialized....User Handle:%p\n", dms_user_handle);
    }

    memset(&get_mac_address_req_msg, 0, sizeof(dms_get_mac_address_req_msg_v01));
    memset(&get_mac_address_resp_msg, 0, sizeof(dms_get_mac_address_resp_msg_v01));

    qmi_err = qmi_client_send_msg_sync(
                   dms_user_handle,
                   QMI_DMS_GET_MAC_ADDRESS_REQ_V01,
                   (void*) &get_mac_address_req_msg,
                   sizeof(dms_get_mac_address_req_msg_v01),
                   (void*) &get_mac_address_resp_msg,
                   sizeof(dms_get_mac_address_resp_msg_v01),
                   5000);

    if(QMI_NO_ERR == qmi_err)
    {
        if(get_mac_address_resp_msg.resp.result == 0)
        {
            if(get_mac_address_resp_msg.mac_address_valid == 1){

                memcpy(tempNvMac, get_mac_address_resp_msg.mac_address, QMI_DMS_MAC_ADDR_MAX_V01);
                log_e("[wlan_mac] tempNvMac: %u %u %u %u %u %u %u %u\n",tempNvMac[0],tempNvMac[1],tempNvMac[2],tempNvMac[3],tempNvMac[4],tempNvMac[5],tempNvMac[6],tempNvMac[7]);

                snprintf(wlan0_mac_buff, 13, "%02X%02X%02x%02X%02X%02X", tempNvMac[0],tempNvMac[1],tempNvMac[2],tempNvMac[3],tempNvMac[4],tempNvMac[5]);
                log_e("[wlan_mac] getting wlan0 mac from nv as:%s\n",wlan0_mac_buff);

                //check the NV+1 action causing overflow or not...
                int overcount=5;
                int overflow;
                while(overcount>0) {
                    log_d("[wlan_mac] check for overflow\n");

                    overflow = tempNvMac[overcount];
                    overflow += 1;

                    if(overflow <= 255) {
                        tempNvMac[overcount] = overflow;
                        break;
                    } else {
                        if((overcount-1)>0)
                            tempNvMac[overcount] = 0;
                        else
                            log_d("[wlan_mac] you reach overflow MAX...\n");
                    }
                    overcount--;
                }
                snprintf(wlan1_mac_buff, 13, "%02X%02X%02x%02X%02X%02X", tempNvMac[0],tempNvMac[1],tempNvMac[2],tempNvMac[3],tempNvMac[4],tempNvMac[5]);
                log_e("[wlan_mac] transfer wlan1 mac from nv as:%s\n",wlan1_mac_buff);

            }else{
                log_e("[wlan_mac] device wlan_mac is invalid. imei_valid=%d\n",  get_mac_address_resp_msg.mac_address_valid);
                log_e("[wlan_mac] will use wlan_mac.bin instead of NV...\n\n");
            }//if(get_mac_address_resp_msg.mac_address_valid == 1)
        }else{
            log_e("[wlan_mac] QMI get data settings resp error. error code = 0x%x\n", get_mac_address_resp_msg.resp.error);
        }//if(get_mac_address_resp_msg.resp.result == 0)
    }else{
        log_e("[wlan_mac] qmi_err = %d\n", qmi_err);
    }//if(QMI_NO_ERR == qmi_err)

    rc = qmi_client_release(dms_user_handle);
    if (rc < 0 )
    {
        log_e("[wlan_mac] After get_value, Release not successful \n");
    }
    else
    {
        printf("[wlan_mac] QMI client release successful \n");
    }
#endif
}
//
void replace_wlan_mac(char *expectStr, char *readMac, char *NvMac)
{
    char old[MAX_CONFIG_LENGTH];
    char replace[MAX_CONFIG_LENGTH];
    memset(&old, 0, MAX_CONFIG_LENGTH);
    memset(&replace, 0, MAX_CONFIG_LENGTH);
    char *dest, *temp_dest;

    sprintf(old, "^%s%s", expectStr, readMac);
    sprintf(replace, "%s%s", expectStr, NvMac);
    dest = WLAN_MAC_BIN_FILE;
    temp_dest = WLAM_MAC_BIN_TEMP_FILE;

    //can't modified /etc cause Read-Only filesystem, remount for modified..
    //char cmd[MAX_CMD_LENGTH];
    //sprintf(cmd, "mount -o remount,rw /");
    //systemCmd(cmd);

    update_config_file(old, replace, dest, temp_dest);
}
//
void random_wlan_mac(char *wlan0_mac_buff, char *wlan1_mac_buff)
{
    //char wlan0Mac_nv[MAX_INFO_LENGTH + MAX_DATA_LENGTH];
    //char wlan1Mac_nv[MAX_INFO_LENGTH + MAX_DATA_LENGTH];
    int i;
    uint8_t tempRandMac[3];
    uint8_t tempUint;

    //
    srand(time(0)+getpid());
    for (i=0; i<3; i++) {
        tempRandMac[i] = rand() % 255;
    }
    snprintf(wlan0_mac_buff, 13, "00037F%02X%02X%02X",tempRandMac[0],tempRandMac[1],tempRandMac[2]);
    log_e("[wlan_mac] random wlan0 mac:%s\n",wlan0_mac_buff);

    tempUint = tempRandMac[2];
    tempUint += 1;
    log_e("[wlan_mac] tempUint as %d\n",tempUint);
    snprintf(wlan1_mac_buff, 13, "02037F%02X%02X%02X",tempRandMac[0],tempRandMac[1],tempUint);
    
    log_e("[wlan_mac] exit remove_etc_wlan_mac\n");
}

//For wlan_mac, get mac from nv item
void update_wlan_mac_from_nv()
{
    log_e("[wlan_mac] Entering update_wlan_mac_from_nv...\n");
    //get wlan_mac from nv...
    char wlan0Mac_nv[MAX_INFO_LENGTH + MAX_DATA_LENGTH];
    char wlan1Mac_nv[MAX_INFO_LENGTH + MAX_DATA_LENGTH];
    get_wlan_mac(wlan0Mac_nv, wlan1Mac_nv, MAX_INFO_LENGTH + MAX_DATA_LENGTH);

    FILE *fp;
    fp = fopen(WLAN_MAC_BIN_FILE, "r+");
    char searchStr[30];
    char tmpWlan0Mac[29];
    char tmpWlan1Mac[29];
    memset(searchStr, '\0', sizeof(searchStr));
    memset(tmpWlan0Mac, '\0', sizeof(tmpWlan0Mac));
    memset(tmpWlan1Mac, '\0', sizeof(tmpWlan1Mac));
    char expectWlan0Str[17] = "Intf0MacAddress=";
    char expectWlan1Str[17] = "Intf1MacAddress=";
    char cmd[MAX_CMD_LENGTH];

    if(NULL == fp) {
        log_e("[wifi mac] bin file not exist! used random mac \n");
        return;
    } else {
        while(fgets(searchStr, 30, fp) != NULL) {
            //log.e("[wlan_mac] readWifiMacFile, check wifi mac address \n");

            if(strstr(searchStr, expectWlan0Str) != NULL){
                strncpy(tmpWlan0Mac, searchStr+16, 12);
                log_e("[wlan mac] tmpWlan0Mac:%s\n",tmpWlan0Mac);
                if(strlen(tmpWlan0Mac) == 12) {
                    memset(searchStr, '\0', sizeof(searchStr));
                    //continue;
                } else {
                    printf("[wlan_mac] mac length wrong in bin? \n");
                    break;
                }
            }//end of strstr wlan0_mac

            if(strstr(searchStr, expectWlan1Str) != NULL){
                strncpy(tmpWlan1Mac, searchStr+16, 12);
                log_e("[wlan mac] tmpWlan1Mac:%s\n",tmpWlan1Mac);
                if(strlen(tmpWlan1Mac) == 12) {
                    memset(searchStr, '\0', sizeof(searchStr));
                    //continue;
                } else {
                    printf("[wlan_mac] mac length wrong in bin? \n");
                    break;
                }
            }//end of strstr wlan1_mac

        }//end of fget
    }//end of fp
    //close file first? since sed do later?
    fclose(fp);

    if(strlen(wlan0Mac_nv) == 12) {
    //If NV get length of value, use NV to update wlan_mac
    //200811, check if /data MAC been modified, if it does, requrst additional reboot
    bool wlan0Mac_change = false;
    bool wlan1Mac_change = false; 
        if ((strcmp(tmpWlan0Mac,wlan0Mac_nv) != 0) ) {
            log_e("[wlan_mac] %s replace %s by %s...\n",expectWlan0Str ,tmpWlan0Mac, wlan0Mac_nv);
            replace_wlan_mac(expectWlan0Str, tmpWlan0Mac, wlan0Mac_nv);
            wlan0Mac_change = true;
            sync();
        } else {
            log_e("[wlan_mac] wlan0 mac NV already update, SKIP...\n\n");
        }
        if (strcmp(tmpWlan1Mac,wlan1Mac_nv) != 0) {
            log_e("[wlan_mac] %s replace %s by %s...\n",expectWlan1Str ,tmpWlan1Mac, wlan1Mac_nv);
            replace_wlan_mac(expectWlan1Str, tmpWlan1Mac, wlan1Mac_nv);
            wlan1Mac_change = true;
            sync();
        } else {
            log_e("[wlan_mac] wlan1 mac NV already update, SKIP...\n\n");
        }
        
        //In zyxel case, 2.4G MAC did not modified, it's 5G MAC been changed.
        //[wlan1Mac_change] been setup 'true' also imply 2.4G MAC check already go through.
        //if condition [wlan1Mac_change] for now..
        if (wlan1Mac_change) {
            log_e("[wlan_mac][zyxel] /data MAC been modified, request additional reboot\n");
            ds_set_bool(DS_KEY_WIFI_REBOOT_FLAG, false);
        }
    } else {
    //If NV not valid, for system default wlan_mac platform, it may occurred 2 devices with same mac phenomenon
    //using random create mac update
        log_e("[wlan_mac] wlan0_NV get wrong, remove wlan_mac.bin, trigger use random mac\n");
        random_wlan_mac(wlan0Mac_nv, wlan1Mac_nv);

    //bool wlan0Mac_change = false;
    //bool wlan1Mac_change = false;
        if ((strcmp(tmpWlan0Mac,wlan0Mac_nv) != 0) ) {
            log_e("[wlan_mac] %s replace %s by %s...\n",expectWlan0Str ,tmpWlan0Mac, wlan0Mac_nv);
            replace_wlan_mac(expectWlan0Str, tmpWlan0Mac, wlan0Mac_nv);
            //wlan0Mac_change = true;
            //sync();
        } else {
            log_e("[wlan_mac] random wlan0 beed update, SKIP...\n\n");
        }
        if (strcmp(tmpWlan1Mac,wlan1Mac_nv) != 0) {
            log_e("[wlan_mac] %s replace %s by %s...\n",expectWlan1Str ,tmpWlan1Mac, wlan1Mac_nv);
            replace_wlan_mac(expectWlan1Str, tmpWlan1Mac, wlan1Mac_nv);
            //wlan1Mac_change = true;
            //sync();
        } else {
            log_e("[wlan_mac] random wlan1 been update, SKIP...\n\n");
        }

        //if condition [wlan1Mac_change] for now..
        //if (wlan1Mac_change) {
        //    log_e("[wlan_mac][zyxel] /data MAC been modified, requust additional reboot\n");
        //    ds_set_bool(DS_KEY_WIFI_REBOOT_FLAG, false);
        //}

    }//end of strlen wlan0Mac_nv
}
//for wlan_passwd_cus
//get default pwd from etc conf
void get_wlan_pwd_etc(int band, char* value)
{
    char startwith[16] = "wpa_passphrase=";
    char *filepath;
    if (band == WIFI_BAND_24G) {
        filepath = ETC_HOSTAPD_CONFIG_FILE;
    } else {
        filepath = ETC_WLAN1_CONFIG_FILE;
    }
    char cmd[MAX_CMD_LENGTH];
    sprintf(cmd, "grep ^%s %s", startwith, filepath);

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        log_e("[pwd_cus] %s popen failed", filepath);
        assert(false);
        return;
    }

    char buffer[MAX_CONFIG_LENGTH];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    if (buffer != NULL && strlen(buffer) != 0) {
        char *found = buffer;
        found += strlen(startwith);
        //[pwd_cus] ignore the bot '\n' for copy, sed the content only as well.
        strncpy(value, found , strlen(found)-1);
    }
    pclose(fp);
    //return value;
}
//for wlan_passwd_cus
//get defined pwd from NV item #6856
void get_wlan_pwd_nv(char* value) {

    char cmd[MAX_CMD_LENGTH];
    sprintf(cmd, "atcli \"AT+CEISSID\"");
    log_e("[pwd_cus] get_wlan_pwd_nv cmd:%s\n",cmd);

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        log_e("[pwd_cus] atcli popen failed");
        assert(false);
        return;
    }

    char buffer[MAX_CONFIG_LENGTH];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    if (buffer != NULL && strlen(buffer) != 0) {
        
        //[pwd_cus] atcli may return something elase than CEISSIE expect, strncmp first
        if(strncmp(buffer, "AT+CEISSID", strlen("AT+CEISSID")) == 0) {
        //[pwd_cus] get next line since atcli result at next line.
        // and clean up '\n'
        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, sizeof(buffer), fp);
        if (buffer[strlen(buffer)-1] == '\n') {
            buffer[strlen(buffer)-1] = '\0';
        }

        log_e("[pwd_cus] buffer nextline:%s\n",buffer);
        char *found = buffer;
        //[pwd_cus] ignore the bot '\n' for copy, sed the content only as well.
        //log_e("[pwd_cus] found:%s\n",found);
        //log_e("[pwd_cus] strlen:%d\n",strlen(found));
        strncpy(value, found , strlen(found)-1);
        log_e("[pwd_cus] value:%s\n",value);
        } else {
        log_e("[pwd_cus] not require atcli buffer:%s for pwd_cus\n",buffer);
        }
    }
    pclose(fp);
    //return value;
}
//For wlan_passwd_cus
//update passwd from nv item while cust_id required
void update_wlan_passwd_from_nv(int custId) {
    log_e("[pwd_cus] enter update_pwd_from_nv\n");

    //get_ssid_from_etc_conf
    char pwd_24_val_etc[MAX_CONFIG_LENGTH]={'\0'};
    char pwd_5_val_etc[MAX_CONFIG_LENGTH]={'\0'};
    get_wlan_pwd_etc(WIFI_BAND_24G, pwd_24_val_etc);
    get_wlan_pwd_etc(WIFI_BAND_5G, pwd_5_val_etc);

    //get_ssid_from_data_conf
    char *pwd_24_val = get_wlan_password(WIFI_BAND_24G);
    char *pwd_5_val = get_wlan_password(WIFI_BAND_5G);
    int len = strlen(pwd_24_val);
    if (pwd_24_val[len-1] == '\n') {
        pwd_24_val[len-1] = '\0';
    }
    len = strlen(pwd_5_val);
    if (pwd_5_val[len-1] == '\n') {
        pwd_5_val[len-1] = '\0';
    }

    //get_ssid_from_nv
    char pwd_24g_nv[MAX_INFO_LENGTH]={'\0'};
    //char pwd_5g_nv[MAX_INFO_LENGTH]={'\0'};
    get_wlan_pwd_nv(pwd_24g_nv);
    len = strlen(pwd_24g_nv);
    if (pwd_24g_nv[len-1] == '\n') {
        pwd_24g_nv[len-1] = '\0';
    }
    log_e("[pwd_cus] pwd_24g_nv:%s\n",pwd_24g_nv);
    log_e("[pwd_cus] len:%d\n",len);

    //use ds_get_bool for determined cust_pwd changed.
    bool pwd_cus_changed;
    pwd_cus_changed = ds_get_bool(DS_KEY_CUST_PASSWD_CHANGED);

    //for passwd requirement instead of NV item, using cust_id for separated.
    if (custId == 5) {

        //for @Gemteks, without password_nv written..
        //if(strlen(pwd_24g_nv) != 0 && strlen(pwd_24g_nv) >= 8) {
            if ((pwd_24_val != NULL) && !pwd_cus_changed){
                memset(pwd_24g_nv, '\0', sizeof(pwd_24g_nv));
                strncpy(pwd_24g_nv, "smartbro", 9);
                log_e("[pwd_cus][Gemteks] /data 24g password replace %s by %s...\n",pwd_24_val ,pwd_24g_nv);
                //update 24g, 5g passwd identical..
                update_value(WIFI_BAND_24G, WPA_PASSPHRASE, pwd_24g_nv);
                update_value(WIFI_BAND_5G, WPA_PASSPHRASE, pwd_24g_nv);

                //when cust_pwd been, ds_set_bool to 'true'
                ds_set_bool(DS_KEY_CUST_PASSWD_CHANGED, true);

            } else {
                log_e("[pwd_cus][Gemteks] password had already update, or user had manually modified, SKIP update pwd_cus\n");
            }
        //} else {
        //    log_e("[pwd_cus][Gemteks] get_wlan_pwd_nv return length error, SKIP update password for /data.\n");
        //}//else

    } else {

        //[pwd_cus] passwd length had requirement, only accept length >= 8
        if(strlen(pwd_24g_nv) != 0 && strlen(pwd_24g_nv) >= 8) {

            if ((pwd_24_val != NULL) && !pwd_cus_changed){

                log_e("[pwd_cus] /data 24g password replace %s by %s...\n",pwd_24_val ,pwd_24g_nv);
                //update 24g, 5g passwd identical..
                update_value(WIFI_BAND_24G, WPA_PASSPHRASE, pwd_24g_nv);
                update_value(WIFI_BAND_5G, WPA_PASSPHRASE, pwd_24g_nv);

                //when cust_pwd been, ds_set_bool to 'true'
                ds_set_bool(DS_KEY_CUST_PASSWD_CHANGED, true);
            } else {
                log_e("[pwd_cus] password had already update, or user had manually modified, SKIP update pwd_cus\n");
            }
        } else {
            log_e("[pwd_cus] get_wlan_pwd_nv return length error, SKIP update password for /data.\n");
        }//else

    }//custId or not
}

//For ssid customized
void get_cust_id(int *custId) {
    log_e("[ssid_cus] enter get_cust_id\n");

    char cmd[MAX_CMD_LENGTH];
    sprintf(cmd, "cat /oem/cust_id");

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        log_e("[ssid_cus] cat cust_id failed, SKIP\n");
        return;
    }

    char buffer[MAX_CONFIG_LENGTH];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    if (strncmp(buffer, "00", 2) == 0) {
        *custId = 0;
    } else if (strncmp(buffer, "01", 2) == 0) {
        *custId = 1;
    } else if (strncmp(buffer, "02", 2) == 0) {
        *custId = 2;
    } else if (strncmp(buffer, "03", 2) == 0) {
        *custId = 3;
    } else if (strncmp(buffer, "04", 2) == 0) {
        *custId = 4;
    } else if (strncmp(buffer, "05", 2) == 0) {
        *custId = 5;
    //for ZX73 internal 'mcc=999'
    } else if (strncmp(buffer, "50", 2) == 0) {
        *custId = 50;
    } else if (strncmp(buffer, "51", 2) == 0) {
        *custId = 51;
    } else if (strncmp(buffer, "52", 2) == 0) {
        *custId = 52;
    } else {
        log_e("[ssid_cus] un-defined cust_id:%s, return as default choice\n",buffer);
    }
    pclose(fp);
}
//For ssid customized
void get_wlan_ssid_etc(int band, char* value)
{
    char startwith[6] = "ssid=";
    char *filepath;
    if (band == WIFI_BAND_24G) {
        filepath = ETC_HOSTAPD_CONFIG_FILE;
    } else {
        filepath = ETC_WLAN1_CONFIG_FILE;
    }
    char cmd[MAX_CMD_LENGTH];
    sprintf(cmd, "grep ^%s %s", startwith, filepath);

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        log_e("[ssid_cus] %s popen failed", filepath);
        assert(false);
        return;
    }

    char buffer[MAX_CONFIG_LENGTH];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    if (buffer != NULL && strlen(buffer) != 0) {
        char *found = buffer;
        found += strlen(startwith);
        //[ssid_cus] ignore the bot '\n' for copy, sed the content only as well.
        strncpy(value, found , strlen(found)-1);
    }
    pclose(fp);
    //return value;
}
//For ssid customized
void ssid_combine_mac(int custId)
{
    log_e("[ssid_cus] enter ssid_combine_mac\n");
    log_e("[ssid_cus] getting custId as :%d\n",custId);
    //get_mac_from_nv
    char wlan0Mac_nv[MAX_INFO_LENGTH + MAX_DATA_LENGTH]={'\0'};
    char wlan1Mac_nv[MAX_INFO_LENGTH + MAX_DATA_LENGTH]={'\0'};
    get_wlan_mac(wlan0Mac_nv, wlan1Mac_nv, MAX_INFO_LENGTH + MAX_DATA_LENGTH);

    //get_ssid_from_etc_conf
    char ssid_24_val_etc[MAX_CONFIG_LENGTH]={'\0'};
    char ssid_5_val_etc[MAX_CONFIG_LENGTH]={'\0'};
    get_wlan_ssid_etc(WIFI_BAND_24G, ssid_24_val_etc);
    get_wlan_ssid_etc(WIFI_BAND_5G, ssid_5_val_etc);

    //get_ssid_from_data_conf
    char *ssid_24_val = get_wlan_ssid(WIFI_BAND_24G);
    char *ssid_5_val = get_wlan_ssid(WIFI_BAND_5G);
    int len = strlen(ssid_24_val);
    if (ssid_24_val[len-1] == '\n') {
        ssid_24_val[len-1] = '\0';
    }
    len = strlen(ssid_5_val);
    if (ssid_5_val[len-1] == '\n') {
        ssid_5_val[len-1] = '\0';
    }
    

    //ssid desire pattern pre-setup
    char *ssid_desire_pre_val;
    char *ssid_desire_post_val;
    if(custId == 3) {
    //Zyxel ssid customized rule: Zyxel_{mac_last_4_word}_5G
        ssid_desire_pre_val = "Zyxel_";
        ssid_desire_post_val = "_5G";
    } else if(custId == 2) {
    //DLink ssid customized rule: dlink_DWR-2101-5G-{mac_last_4_word}
        ssid_desire_pre_val = "dlink_DWR-2101-";
        ssid_desire_post_val = "5G-";
    } else if(custId == 4) {
    //DLink ssid customized rule: Modem Wi-Fi 5G_{mac_last_4_word}
        ssid_desire_pre_val = "Modem Wi-Fi 5G_";
        //ssid_desire_post_val = "5G-";
    } else if(custId == 5) {
    //Gemteks ssid customized rule:
    //2.4GHz SSID - Smart_Bro_5G_XXXXX (last 5 characters of 2.4GHz WiFi MAC address in UPPERCASE)
    //5GHz SSID - Smart_Bro_5G_XXXXX (last 5 characters of 5GHz WiFi MAC address in UPPERCASE)
        ssid_desire_pre_val = "Smart_Bro_5G_";
        //ssid_desire_post_val = "5G-";
    } else {
        log_e("[ssid_cus] customized id:%d did not require yet, abort\n", custId);
        return;
    }

    char buffer[MAX_CONFIG_LENGTH];
    char temp_mac0[MAX_CONFIG_LENGTH];
    char temp_mac1[MAX_CONFIG_LENGTH];

    //use ds_get_bool to determine ssid_cus hanged.
    bool ssid_cus_changed;

    if(!(strlen(wlan0Mac_nv) == 12)) {
        log_e("[ssid_cus] wlan_mac get from NV error, can't pass to SSID, abort\n");
        return;
/*
        //used random mac first..
        random_wlan_mac(wlan0Mac_nv, wlan1Mac_nv);

        if (custId == 3) {//Zyxel
            //2.4G
            memset(temp_mac0, '\0', sizeof(temp_mac0));
            strncpy(temp_mac0, wlan0Mac_nv+8, 4);
            sprintf(buffer, "%s%s", ssid_desire_pre_val, temp_mac0);
            if ((ssid_24_val != NULL) && (strncmp(ssid_24_val, buffer, strlen(ssid_24_val)) != 0)){
                //write both /data & /etc SSID
                log_e("[ssid_cus][no mac] replace zyxel 24g ssid\n");
                write_wlan_ssid_etc(WIFI_BAND_24G, ssid_24_val, buffer);
                write_wlan_ssid(WIFI_BAND_24G, buffer);
            } else {
                log_e("[ssid_cus][zyxel]2.4G ssid get faiure OR already changend default ssid, update ssid_customized SKIP\n");
            }

            //5G
            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer, "%s%s%s", ssid_desire_pre_val, temp_mac0, ssid_desire_post_val);
            if ((ssid_5_val != NULL) && (strncmp(ssid_5_val, buffer, strlen(ssid_5_val)) != 0)){
                //write both /data & /etc
                log_e("[ssid_cus][no mac] replace zyxel 5g ssid\n");
                write_wlan_ssid_etc(WIFI_BAND_5G, ssid_5_val, buffer);
                write_wlan_ssid(WIFI_BAND_5G, buffer);
            } else {
                log_e("[ssid_cus][zyxel]5G ssid get failure OR already changend default ssid, update ssid_customized SKIP\n");
                //return;
            }

        }
*/

    } else if((strlen(wlan0Mac_nv) == 12) && (custId == 3)) {
        //For Zyxel.
        ssid_cus_changed = ds_get_bool(DS_KEY_CUST_SSID_CHANGED);
        //2.4G
        memset(temp_mac0, '\0', sizeof(temp_mac0));
        strncpy(temp_mac0, wlan0Mac_nv+8, 4);
        sprintf(buffer, "%s%s", ssid_desire_pre_val, temp_mac0);

        if ((ssid_24_val != NULL) && !ssid_cus_changed) {
            log_e("[ssid_cus][zyxel] ssid_24_val:%s, buffer:%s\n",ssid_24_val, buffer);
            update_value(WIFI_BAND_24G, SSID, buffer);
        } else {
            log_e("[ssid_cus][zyxel] 2.4G ssid get faiure OR already changend default ssid, update ssid_customized SKIP\n");
        }

        //5G
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%s%s%s", ssid_desire_pre_val, temp_mac0, ssid_desire_post_val);
        if ((ssid_5_val != NULL) && !ssid_cus_changed) {
            log_e("[ssid_cus][zyxel] ssid_5_val:%s, buffer:%s\n",ssid_5_val, buffer);
            update_value(WIFI_BAND_5G, SSID, buffer);
        } else {
            log_e("[ssid_cus][zyxel] 5G ssid get failure OR already changend default ssid, update ssid_customized SKIP\n");
            //when ssid_cus been changed, directly return.
            return;
        }
        //SSID been cust modified, de_set_bool to 'true'.
        ds_set_bool(DS_KEY_CUST_SSID_CHANGED, true);

    } else if ((strlen(wlan0Mac_nv) == 12) && (custId == 2)) {
        //For DLink.
        ssid_cus_changed = ds_get_bool(DS_KEY_CUST_SSID_CHANGED);
        //2.4G
        memset(temp_mac0, '\0', sizeof(temp_mac0));
        strncpy(temp_mac0, wlan0Mac_nv+8, 4);
        sprintf(buffer, "%s%s", ssid_desire_pre_val, temp_mac0);
        if ((ssid_24_val != NULL) && !ssid_cus_changed) {
            log_e("[ssid_cus][dlink] ssid_24_val:%s, buffer:%s\n",ssid_24_val, buffer);
            update_value(WIFI_BAND_24G, SSID, buffer);
        } else {
            log_e("[ssid_cus][dlink] 2.4G ssid get faiure OR already changend default ssid, update ssid_customized SKIP\n");
        }

        //5G
        memset(temp_mac1, '\0', sizeof(temp_mac1));
        strncpy(temp_mac1, wlan1Mac_nv+8, 4);
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%s%s%s", ssid_desire_pre_val, ssid_desire_post_val, temp_mac1);
        if ((ssid_5_val != NULL) && !ssid_cus_changed) {
            log_e("[ssid_cus][dlink] ssid_5_val:%s, buffer:%s\n",ssid_5_val, buffer);
            update_value(WIFI_BAND_5G, SSID, buffer);
        } else {
            log_e("[ssid_cus][dlink] 5G ssid get failure OR already changend default ssid, update ssid_customized SKIP\n");
            //when ssid_cus been changed, directly return.
            return;
        }
        //SSID been cust modified, de_set_bool to 'true'.
        ds_set_bool(DS_KEY_CUST_SSID_CHANGED, true);

    } else if ((strlen(wlan0Mac_nv) == 12) && (custId == 4)) {
        //For Onda.
        ssid_cus_changed = ds_get_bool(DS_KEY_CUST_SSID_CHANGED);
        //2.4G
        memset(temp_mac0, '\0', sizeof(temp_mac0));
        strncpy(temp_mac0, wlan0Mac_nv+8, 4);
        sprintf(buffer, "%s%s", ssid_desire_pre_val, temp_mac0);
        if ((ssid_24_val != NULL) && !ssid_cus_changed) {
            log_e("[ssid_cus][onda] ssid_24_val:%s, buffer:%s\n",ssid_24_val, buffer);
            update_value(WIFI_BAND_24G, SSID, buffer);
        } else {
            log_e("[ssid_cus][onda] 2.4G ssid get faiure OR already changend default ssid, update ssid_customized SKIP\n");
        }

        //5G
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%s%s", ssid_desire_pre_val, temp_mac0);
        if ((ssid_5_val != NULL) && !ssid_cus_changed) {
            log_e("[ssid_cus][onda] ssid_5_val:%s, buffer:%s\n",ssid_5_val, buffer);
            update_value(WIFI_BAND_5G, SSID, buffer);
        } else {
            log_e("[ssid_cus][onda] 5G ssid get failure OR already changend default ssid, update ssid_customized SKIP\n");
            //when ssid_cus been changed, directly return.
            return;
        }
        //SSID been cust modified, de_set_bool to 'true'.
        ds_set_bool(DS_KEY_CUST_SSID_CHANGED, true);

    } else if ((strlen(wlan0Mac_nv) == 12) && (custId == 5)) {
        //For Gemteks.
        ssid_cus_changed = ds_get_bool(DS_KEY_CUST_SSID_CHANGED);

        //2.4G
        memset(temp_mac0, '\0', sizeof(temp_mac0));
        strncpy(temp_mac0, wlan0Mac_nv+7, 5);
        sprintf(buffer, "%s%s", ssid_desire_pre_val, temp_mac0);
        if ((ssid_24_val != NULL) && !ssid_cus_changed) {
            log_e("[ssid_cus][Gemteks] ssid_24_val:%s, buffer:%s\n",ssid_24_val, buffer);
            update_value(WIFI_BAND_24G, SSID, buffer);
        } else {
            log_e("[ssid_cus][Gemteks] 2.4G ssid get faiure OR already changend default ssid, update ssid_customized SKIP\n");
        }

        //5G
        memset(temp_mac1, '\0', sizeof(temp_mac1));
        strncpy(temp_mac1, wlan1Mac_nv+7, 5);
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%s%s", ssid_desire_pre_val, temp_mac1);
        if ((ssid_5_val != NULL) && !ssid_cus_changed) {
            log_e("[ssid_cus][Gemteks] ssid_5_val:%s, buffer:%s\n",ssid_5_val, buffer);
            update_value(WIFI_BAND_5G, SSID, buffer);
        } else {
            log_e("[ssid_cus][Gemteks] 5G ssid get failure OR already changend default ssid, update ssid_customized SKIP\n");
            //when ssid_cus been changed, directly return.
            return;
        }

        //SSID been cust modified, de_set_bool to 'true'.
        ds_set_bool(DS_KEY_CUST_SSID_CHANGED, true);

    } else {
        log_e("[ssid_cus] cust_id:%d did not had requirement yet, abort\n", custId);
        //return;
    }
}
//For ssid customized
void update_wlan_ssid_customized()
{
    log_e("[ssid_cus] enter update_wlan_ssid_customized.\n");

    int custId_choose = 0;
    get_cust_id(&custId_choose);
    
    switch (custId_choose) {
        case 0:
        case 1:
        case 50:
        case 51:
        case 52:
            log_e("[ssid_cus] no SSID customized requirement for cust_Id yet, SKIP.\n");
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            //DLink-02, Zyxel-03, Onda-04, Gemteks-05 chaned SSID, combined with or without MAC.
            ssid_combine_mac(custId_choose);
            break;
        default:
            log_e("[ssid_cus] un-defiend cust_id, SKIP in switch\n");
    }    

}
//For passwd customized
void update_wlan_passwd_customized()
{
    log_e("[pwd_cus] enter update_wlan_passwd_customized.\n");

    int custId_choose = 0;
    get_cust_id(&custId_choose);

    switch (custId_choose) {
        case 0:
        case 1:
        case 50:
        case 51:
        case 52:
            log_e("[pwd_cus] no passwd customized requirement for cust_Id yet, SKIP.\n");
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            //DLink-02, Zyxel-03, Onda-04, Gemteks-05 changed passwd to NV item or other customized.
            update_wlan_passwd_from_nv(custId_choose);
            break;
        default:
            log_e("[pwd_cus] un-defiend cust_id, SKIP in switch\n");
    }
}

//For wlan_mac, get mac from nv item
//For ssid customized
void load_nv_item() {
    update_wlan_mac_from_nv();
    update_wlan_ssid_customized();

    update_wlan_passwd_customized();
    //restart the wlan after passed SSID_cust & pwd_cust, avoid frequently unload reload hostapd
    //restart_wlan(WIFI_BAND_ALL);
    //update_active_wlan_ssid();
}

//For cc_regdb, get mcc to wlan_country
void replace_wlan_country(char *expectStr, char *readMcc, char *replaceMcc)
{
    char old[MAX_CONFIG_LENGTH];
    char replace[MAX_CONFIG_LENGTH];
    memset(&old, 0, MAX_CONFIG_LENGTH);
    memset(&replace, 0, MAX_CONFIG_LENGTH);
    char *dest, *temp_dest;

    sprintf(old, "^%s%s", expectStr, readMcc);
    sprintf(replace, "%s%s", expectStr, replaceMcc);
    dest = WLAN_MCC_COUNTRY_FILE;
    temp_dest = WLAN_MCC_COUNTRY_TEMP_FILE;

    //can't modified /etc cause Read-Only filesystem, remount for modified..
    //char cmd[MAX_CMD_LENGTH];
    //sprintf(cmd, "mount -o remount,rw /");
    //systemCmd(cmd);

    update_config_file(old, replace, dest, temp_dest);
}

//For restart wlan,
void RestartWLAN()
{
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    int retry_count =1;

    if(qmi_qcmap_init() == QMI_ERROR) {
        log_e("[restart] QCMAP unable to initialize, exiting\n");
    } else {
        while(retry_count < 10)//max retry
        {
            if(EnableMobileAP()== true) {

                log_e("[restart] bring down wlan");
                if (DisableWLAN()) {
                    log_e("[restart] bring up wlan start");
                    if(EnableWLAN()) {
                        log_e("[restart] bring up wlan end");
                    }
                }
            } else {
                log_e("[restart] QCMAP restart EnableMobileAP() count :%d\n", retry_count);
            }
            retry_count++;
        }//while retry
    }//qmi_qcmap_init
#endif
}

//For cc_regdb, get mcc to wlan_country
void check_current_country()
{
    int mccNum;
    int country_max = sizeof(all_country_list)/sizeof(all_country_list[0]);
    int count;
    char country[3];
    memset(country, '\0', sizeof(country));
    //for ZX73 internal 'mcc=999'
    int custId_choose = 0;
    get_cust_id(&custId_choose);
    log_e("[cc_regdb] custId_choose: %d\n",custId_choose);

    get_service_mcc(&mccNum);

    //if (mccNum == 0) {
    //    log_e("[cc_regdb] get mccNum failed as: %d, SKIP...\n",mccNum);
    //    return;
    //} else {
        log_e("[cc_regdb] mccNum as: %d\n", mccNum);

        for(count=0; count < country_max; count++) {
            if (mccNum == all_country_list[count].mcc) {
            break;
            }
        }
        log_e("[cc_regdb] tempCC index: %d\n",count);
        //
        FILE *fp;
        fp = fopen(WLAN_MCC_COUNTRY_FILE, "r");
        char searchStr[30];
        char tmpMccStr[29];
        memset(searchStr, '\0', sizeof(searchStr));
        memset(tmpMccStr, '\0', sizeof(tmpMccStr));
        char expectMccStr[6] = "ARG2=";
        char cmd[MAX_CMD_LENGTH];

        if(NULL == fp) {
            log_e("[cc_regdb] bin file not exist! used default country \n");
            return;
        } else {
            while(fgets(searchStr, 30, fp) != NULL) {

                if(strstr(searchStr, expectMccStr) != NULL){
                    strncpy(tmpMccStr, searchStr+5, 2);
                    log_e("[cc_regdb] tmpMccStr:%s\n",tmpMccStr);
                    if(strlen(tmpMccStr) == 2) {
                        //memset(searchStr, '\0', sizeof(searchStr));
                        //continue;
                    } else {
                        log_e("[cc_regdb] mcc length wrong in bin? \n");
                        break;
                    }
                }//end of strstr country

            }//end of fget
        }//end of fp
        fclose(fp);


        if( (count < country_max) && (strcmp(all_country_list[count].alpha, mccGlobal) != 0)) {
            log_e("[cc_regdb] tempCC country_code: %s\n",all_country_list[count].alpha);
            log_e("[cc_regdb] mccGlobal: %s\n",mccGlobal);

            strncpy(country, all_country_list[count].alpha, strlen(all_country_list[count].alpha)+1);
            log_e("[cc_regdb] strlen(country):%d\n",strlen(country));

            if(strlen(country) == 2) {
            //if country get valid, update with this country
                if ((strcmp(tmpMccStr,country) != 0) ) {
                    //for ZX73 internal 'mcc=999', for log checking
                    //if ((custId_choose == 50) || (custId_choose == 51) || (custId_choose == 52)){
                    //    char zx73_country[3] = "JP";
                    //    strncpy(country, zx73_country, 2);
                    //    log_e("[cc_regdb][ZX73] For ZX73, changed this country_code to:%s\n",country);
                    //    log_e("[cc_regdb][ZX73] %s replace %s by %s...\n",expectMccStr ,tmpMccStr ,country);
                    //} else {
                    log_e("[cc_regdb] %s replace %s by %s...\n",expectMccStr ,tmpMccStr ,country);
                    //}
                    replace_wlan_country(expectMccStr, tmpMccStr, country);
                    //update mccGlobal
                    strcpy(mccGlobal, country);
                    //after update country, restart wlan for this country setup
#ifndef REBOOT_AFTER_CONFIG_LOAD
                    RestartWLAN();
#endif
                } else {
                    log_e("[cc_regdb] wlan MCC country already update, SKIP...\n\n");
                }

            } else {
                log_e("[cc_regdb] set country as :%s, SKIP..\n", country);
            }

        } else if(count < country_max) {
            log_e("[cc_regdb] mccGlobal:%s already update as:%s\n",mccGlobal ,all_country_list[count].alpha);
        } else {
            log_e("[cc_regdb] exceed country_max:%d, SKIP...\n",country_max);
            //for ZX73 internal 'mcc=999'
            if ((custId_choose == 50) || (custId_choose == 51) || (custId_choose == 52)){
                char zx73_country[3] = "JP";
                strncpy(country, zx73_country, 2);
                log_e("[cc_regdb][ZX73] For ZX73, there will be internal USIM usage (MCC=999), changed this country_code to:%s\n",country);
                log_e("[cc_regdb][ZX73] %s replace %s by %s...\n",expectMccStr ,tmpMccStr ,country);
                    replace_wlan_country(expectMccStr, tmpMccStr, country);
                    //update mccGlobal
                    strcpy(mccGlobal, country);
                    //after update country, restart wlan for this country setup
#ifndef REBOOT_AFTER_CONFIG_LOAD
                    RestartWLAN();
#endif

            }
        }//end of count, country_max
    //}//end of mccNum
}

void wifi_enable() {
    log_d("wifi enabling...");

#if (0) //QCT suggest not to off then on wlan at boot so change back to disable disabled band
    // restart the enabled band
    restart(current_wifi_band);

    // disable the disabled band
    if (!get_wifi_band_enabled(WIFI_BAND_24G)) {
        write_wifi_band_enabled(WIFI_BAND_24G, false);
    }
    if (!get_wifi_band_enabled(WIFI_BAND_5G)) {
        write_wifi_band_enabled(WIFI_BAND_5G, false);
    }
#else
    write_wifi_band_enabled(WIFI_BAND_24G, ds_get_bool(DS_KEY_WIFI_24G_ENABLED));
    write_wifi_band_enabled(WIFI_BAND_5G, ds_get_bool(DS_KEY_WIFI_5G_ENABLED));
#endif
}

void enable_wlan(void *arg)
{
    log_d("enable_wlan()");
#ifdef FEATURE_ROUTER
    if (EnableWLAN()) {
        restart_wlan(WIFI_BAND_ALL);
    } else {
        log_d("EnableWLAN failed");
    }
#endif
}

void enable_wlan_task() {
    memset(&enable_wlan_thread, 0, sizeof(pthread_t));
    int rc = pthread_create(&enable_wlan_thread, NULL, enable_wlan, NULL);
    if (rc) log_e("create enable_wlan_thread failed");
}

void disable_wlan() {
    log_d("disable_wlan()");
    stop_hostapd_daemon(WIFI_BAND_ALL); // kill hostapd first before sleep instead of at wake up

#if defined (FEATURE_ROUTER) && defined(DISABLE_WLAN_AT_CLOSE)
    if (DisableWLAN()) {
        log_d("DisableWLAN success");
    } else {
        log_d("DisableWLAN failed");
    }
#else
    if (get_wifi_band_enabled(WIFI_BAND_24G)) {
        write_wifi_band_enabled(WIFI_BAND_24G, false);
    }
    if (get_wifi_band_enabled(WIFI_BAND_5G)) {
        write_wifi_band_enabled(WIFI_BAND_5G, false);
    }
#endif
}

void disable_wlan_thread_func(void *arg) {
    disable_wlan();
}

void disable_wlan_task() {
    memset(&disable_wlan_thread, 0, sizeof(pthread_t));
    int rc = pthread_create(&disable_wlan_thread, NULL, disable_wlan_thread_func, NULL);
    if (rc) log_e("create disable_wlan_thread failed");
}

void wifi_down() {
    log_d("wifi down...");
    disable_wlan_task();
}

void wifi_up() {
    log_d("wifi up...");
#if defined (FEATURE_ROUTER) && defined(DISABLE_WLAN_AT_CLOSE)
    enable_wlan_task();
#else
    restart_wlan(WIFI_BAND_ALL);
#endif
}

void wifi_wakeup() {
    if (wifi_sleep) {
        // turn modem on line
        //systemCmd("cat /dev/smd11 &");
        //systemCmd("echo -e \"AT+CFUN=1\r\n\" > /dev/smd11");

        log_d("wifi waking up...");
        wifi_sleep = false;
        load_wifi_band(); //reload band enabled state from db
        //restart_wlan(WIFI_BAND_ALL);
        restart_sleep_flag = true; // set this flag to sleep a bit before start hostapd process

#if defined (FEATURE_ROUTER) && defined(DISABLE_WLAN_AT_CLOSE)
        log_d("start 30 sec timer before start enable_wlan");
        lv_task = lv_task_create(enable_wlan_task, 30000, LV_TASK_PRIO_LOW, NULL);
        lv_task_once(lv_task);
#else
        restart_wlan(WIFI_BAND_ALL);
#endif
        update_active_wlan_ssid();
        //update_wifi_band(); // let main thread do it
    }

    // Because we disabled 2.4G for lower power consumption at screen off, so
    // re-enable per UI state at screen on
    //if (ds_get_bool(DS_KEY_WIFI_24G_ENABLED)) {
    //    write_wifi_band_enabled(WIFI_BAND_24G, true);
    //}
}

void wifi_close() {
    log_d("wifi_close() timeout!");
    if (get_power_state() == SCREEN_ON)
        return;

    if (lv_auto_close_task != NULL) {
        lv_task_del(lv_auto_close_task);
        lv_auto_close_task = NULL;
    }

    wifi_sleep = true;
    log_d("wifi going to sleep...");

    disable_wlan();

    // turn modem off line
    if (!is_ethernet_connected()) {
        //systemCmd("cat /dev/smd11 &");
        //systemCmd("echo -e \"AT+CFUN=4\r\n\" > /dev/smd11");
    } else {
        log_d("Not entering airplane mode coz LAN connected");
    }

    start_sleep();
}

bool wifi_close_wait() {
    if (lv_auto_close_task != NULL)
        return true;
    else
        return false;
}

void wifi_close_task_refresh() {
    auto_close_enabled = ds_get_bool(DS_KEY_WIFI_AUTO_CLOSE);
    if (lv_auto_close_task != NULL) {
        lv_task_del(lv_auto_close_task);
        lv_auto_close_task = NULL;
    }
    if (auto_close_enabled && total_connected_number == 0 &&
             get_power_state() == SCREEN_OFF
#ifndef CUST_ZYXEL // Zyxel request to do wifi auto close even during charging
             && !is_charging()
#endif
             ) {
        log_d("wifi close task start counting down to do wifi close");
        lv_auto_close_task = lv_task_create(wifi_close, ds_get_int(DS_KEY_WIFI_AUTO_CLOSE_DURA), LV_TASK_PRIO_LOW, NULL);
        lv_task_once(lv_auto_close_task);
    }

    // disable 2.4G for lower power consumption if 2.4G is enable in UI setting
    //if (total_connected_number == 0 && get_power_state() == SCREEN_OFF) {
    //    if (ds_get_bool(DS_KEY_WIFI_24G_ENABLED)) {
    //        write_wifi_band_enabled(WIFI_BAND_24G, false);
    //    }
    //}
}

void hostapd_conf_value_reset() {
    if (active_ssid_name != NULL) {
        lv_mem_free(active_ssid_name);
        active_ssid_name = NULL;
    }

    int i;
    for (i = 0; i < MAX_CONFIG_CNT; i++) {
        if (hostapd_config[i].value != NULL) {
            lv_mem_free(hostapd_config[i].value);
            hostapd_config[i].value = NULL;
        }
        if (wlan1_config[i].value != NULL) {
            lv_mem_free(wlan1_config[i].value);
            wlan1_config[i].value = NULL;
        }
    }
}

bool config_consistency_reload(int band, int config_id) {
    bool consist = true;
    HostapdConfig target_conf;
    HostapdConfig* config = get_config(band);

    char *dest;
    if (band == WIFI_BAND_24G) {
        dest = HOSTAPD_CONFIG_PATH;
    } else {
        dest = WLAN1_CONFIG_PATH;
    }

    memset(&target_conf, 0, sizeof(HostapdConfig));

    load_config(dest, config_name[config_id], &target_conf.value);

    if (target_conf.value == NULL || config[config_id].value == NULL) {
        // wep case
        if (target_conf.value != config[config_id].value) {
            consist = false;
        } else {
            consist = true;
        }
    } else if (strcmp(target_conf.value, config[config_id].value) != 0) {
        consist = false;
    } else {
        consist = true;
    }

    if (consist) {
        lv_mem_free(target_conf.value);
    } else {
        lv_mem_free(config[config_id].value);
        config[config_id].value = target_conf.value;
    }

    if (!consist && config_id == SSID) {
        update_active_wlan_ssid();
    }

    return consist;
}

// do partial reload for those values that might got modify by Web UI
void hostapd_conf_partial_reload() {
    if (!wlan_init) return;

    load_wifi_band();

    // Fix Bug672, dump again in case connected user changes during suspend
    total_connected_number = dump_connected_users();
    //update_hotspot_user_counter(); // let main thread do it

    int band;
    for (band = WIFI_BAND_24G; band <= WIFI_BAND_5G; band++) {
        // SSID & Password
        config_consistency_reload(band, SSID);
        config_consistency_reload(band, WPA_PASSPHRASE);
        config_consistency_reload(band, WPA_PSK);

        // Hide SSID
        config_consistency_reload(band, IGNORE_BROADCAST_SSID);

        // Security type
        config_consistency_reload(band, WPA);
        config_consistency_reload(band, WPA_KEY_MGMT);
        config_consistency_reload(band, SAE_REQUIRE_MFP);
        config_consistency_reload(band, WEP_KEY0);
        config_consistency_reload(band, WEP_DEFAULT_KEY);
        config_consistency_reload(band, AUTH_ALGS);
        config_consistency_reload(band, IEEE80211W);

        // bandwidth
        config_consistency_reload(band, VHT_OPER_CHWIDTH);
        config_consistency_reload(band, HT_CAPAB);

        // WPS
        config_consistency_reload(band, EAP_SERVER);
        config_consistency_reload(band, WPS_STATE);

        // Channel, vht_centr_freq
        config_consistency_reload(band, CHANNEL);
        config_consistency_reload(band, VHT_OPER_CENTR_FREQ);
    }

    // deny list reload
    clean_deny_config();
    load_deny_config();

    update_active_wlan_ssid();
    update_wifi_band();
}

void hostapd_conf_reload() {
    hostapd_conf_value_reset();
    load_hostapd_config();
}

void hostapd_restore_default() {
    //ds_set_bool(DS_KEY_WIFI_24G_ENABLED, true);
    //ds_set_bool(DS_KEY_WIFI_5G_ENABLED, false);
    //current_wifi_band = DEFAULT_WIFI_BAND;
    //restart_wlan(current_wifi_band);

    hostapd_conf_value_reset();
    init_wlan_data();
    init_wlan();
}

void load_wifi_band() {
    current_wifi_band = WIFI_BAND_NONE;
    if (ds_get_bool(DS_KEY_WIFI_24G_ENABLED)) {
        current_wifi_band |= WIFI_BAND_24G;
    }
    if (ds_get_bool(DS_KEY_WIFI_5G_ENABLED)) {
        current_wifi_band |= WIFI_BAND_5G;
    }
}

void init_wlan_data() {
    load_wifi_band();
    load_hostapd_config();
    load_deny_config();
}

bool wlan_initialized() {
    return wlan_init;
}

void init_wlan() {
    //For wlan_mac, get mac from nv item
    load_nv_item();

    // when data file missing, can not read correct band enable state
    // skip calling enabling to prevent bands got disabled
    if (fs_ready()) {
        wifi_enable();
    }

    update_active_wlan_ssid();

    //For cc_regdb, get mcc to wlan_country
    check_current_country();

    wlan_init = true;

    struct stat buffer;
    int exist = stat(HOSTAPD_CONNECTED_FILE, &buffer);
    if (exist == 0) {
        connected_users_dump = true;
    }

    total_connected_number = dump_connected_users();
    //update_hotspot_user_counter(); // let main thread do it

    log_d("init_wlan with band: %d", current_wifi_band);

#if defined (FEATURE_ROUTER) && defined(REBOOT_AFTER_CONFIG_LOAD)
    if (!ds_get_bool(DS_KEY_WIFI_REBOOT_FLAG)) {
        // make sure data storage xml file exist, to stop device from keep on trying to reboot
        if (fs_ready()) {
            set_reboot_popup_enable(ID_SHUTDOWN_CONFIG_REBOOT, 0);
        }
    }
#endif
}
