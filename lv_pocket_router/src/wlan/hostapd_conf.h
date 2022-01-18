#include <stdbool.h>
#include <stdint.h>
#ifdef FEATURE_ROUTER
#include "wireless_data_service_v01.h"
#include "qmi_client.h"
#include "qmi.h"
#include "qualcomm_mobile_access_point_msgr_v01.h"
#endif

#define BAND_24G        "wlan0"
#define BAND_5G         "wlan1"

#define SECURITY_NONE      "None"
#define SECURITY_WEP       "WEP"
#define SECURITY_WPA_PSK   "WPA-1"
#define SECURITY_WPA2_PSK  "WPA-2"
#define SECURITY_WPA3_WPA2 "WPA-3"

#define WIFI_BAND_NONE  0
#define WIFI_BAND_24G   1
#define WIFI_BAND_5G    2
#define WIFI_BAND_ALL   3

#define HOSTAPD_DENY_CONFIG_MAX     20

#define MAX_CONNECTED_USERS         16

#define MAX_TITLE_LENGTH            18
#define MAX_DATA_LENGTH             32
#define MAX_INFO_LENGTH             (MAX_TITLE_LENGTH + MAX_DATA_LENGTH)

#define MAX_SSID_LEN      32
#define MAX_PASSWORD_LEN  64
#define PASSWORD_LEN_63  63
#define PASSWORD_LEN_8  8

enum WIFI_BANDWIDTH {
    WIFI_BANDWIDTH_20MHZ,
    WIFI_BANDWIDTH_20OR40MHZ,
    WIFI_BANDWIDTH_80MHZ,
    BANDWIDTH_COUNT
};

typedef enum {
    USER_CONNECTED,
    USER_DISCONNECTED
} CONNECT_STATE;

char* get_wlan_ssid(int band);
char* get_wlan_password(int band);
char* get_wlan_security(int band);
char* get_wlan_security_string(int band);
bool get_wlan_pmf(int band);
bool get_wlan_hide_ssid(int band);
int get_connected_number();
bool get_wifi_band_enabled(int band);
int get_wifi_band();
int get_wifi_bandwidth(int band);
void startMonitorWlan();
void write_wlan_ssid(int band, char* value);
bool write_wlan_password(int band, char* value);
void write_wlan_security(int band, char* value);
bool write_wlan_pmf(int band, bool enable);
bool write_wlan_hide_ssid(int band, bool enable);
bool write_wifi_band_enabled(int band, bool enable);
void write_wifi_band(int band);
void write_wifi_bandwidth(int band, int bandwidth);
bool wlan_initialized();
void init_wlan();
bool wps_support();
void start_wps(char* pin);
void wlan_fixed_mode(bool enable);
void wifi_close_task_refresh();
void hostapd_restore_default();
void user_connected_update(CONNECT_STATE state);
bool wifi_close_wait();
void wifi_wakeup();
void restart_failed_band(int failed_band);
bool get_config_popup_enable();
//
void load_nv_item();
void check_current_country();
void wifi_down();
void wifi_up();
