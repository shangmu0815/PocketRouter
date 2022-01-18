#include <stdlib.h>

#include "manual_conn.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/ssid/ssid.h"
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/wlan/hostapd_conf.h"
#include "lv_pocket_router/src/util/util.h"

void manual_conn_create(void)
{
    char* ssid_24_val = get_wlan_ssid(WIFI_BAND_24G);
    char* ssid_5_val = get_wlan_ssid(WIFI_BAND_5G);
    char* security_24_val = get_wlan_security_string(WIFI_BAND_24G);
    char* security_5_val = get_wlan_security_string(WIFI_BAND_5G);
    char* pwd_24_val = "";
    char* pwd_5_val = "";
    if (strcmp(get_string(ID_WIFI_SECURITY_NONE), security_24_val) != 0) {
        pwd_24_val = get_ssid_pwd(WIFI_BAND_24G);
    }
    if (strcmp(get_string(ID_WIFI_SECURITY_NONE), security_5_val) != 0) {
        pwd_5_val = get_ssid_pwd(WIFI_BAND_5G);
    }
    const char* info = get_string(ID_CONN_GUIDE_MANUAL_INFO);
    const char* ssid_24g_title = get_string(ID_SSID_24G);
    const char* ssid_5g_title = get_string(ID_SSID_5G);
    const char* ssid_title = get_string(ID_SSID);
    const char* password_title = get_string(ID_PASSWORD);
    const char* security_title = get_string(ID_WIFI_SECURITY);
    const char* encryp24G = "";
    const char* encryp5G = "";

    if (strcmp(SECURITY_WPA2_PSK, get_wlan_security(WIFI_BAND_24G)) == 0) {
        encryp24G = get_string(ID_WIFI_SECURITY_ENCRYPTION_AES);
    }
    if (strcmp(SECURITY_WPA2_PSK, get_wlan_security(WIFI_BAND_5G)) == 0) {
        encryp5G = get_string(ID_WIFI_SECURITY_ENCRYPTION_AES);
    }

    //add strlen of '\n' ':' and space in the end
    int len = strlen(info) + strlen(ssid_24g_title) + (strlen(ssid_title) * 2) +
            strlen(ssid_24_val) + (strlen(password_title) * 2) + strlen(pwd_24_val) +
            (strlen(security_title) * 2) + strlen(security_24_val) + strlen(encryp24G) +
            strlen(ssid_5g_title) + strlen(ssid_5_val) + strlen(pwd_5_val) +
            strlen(security_5_val) + strlen(encryp5G) + 25;

    char manual_info[len];
    char* format;
    if (is_ltr()) {
        format = "%s\n\n%s\n%s: %s\n%s: %s\n%s: %s %s\n\n%s\n%s: %s\n%s: %s\n%s: %s %s";
    } else {
#ifdef LV_USE_ARABIC_PERSIAN_CHARS
        format = "%s\n\n%s\n%s: %s\n%s: %s\n%s: %s %s\n\n%s\n%s: %s\n%s: %s\n%s: %s %s";
#else
        //workaround for AR
        bool security_none_24g = strlen(pwd_24_val) > 0 ? false : true;
        bool security_none_5g = strlen(pwd_5_val) > 0 ? false : true;
        if(security_none_24g && !security_none_5g){
            format = "%s\n\n%s\n%s:\n%s\n%s:\n%s%s: %s %s\n\n%s\n%s:\n%s\n%s:\n%s\n%s: %s %s";
        }else if(!security_none_24g && security_none_5g){
            format = "%s\n\n%s\n%s:\n%s\n%s:\n%s\n%s: %s %s\n\n%s\n%s:\n%s\n%s:\n%s%s: %s %s";
        }else if(security_none_24g && security_none_5g){
            format = "%s\n\n%s\n%s:\n%s\n%s:\n%s%s: %s %s\n\n%s\n%s:\n%s\n%s:\n%s%s: %s %s";
        }else{
            format = "%s\n\n%s\n%s:\n%s\n%s:\n%s\n%s: %s %s\n\n%s\n%s:\n%s\n%s:\n%s\n%s: %s %s";
        }
#endif
    }
    sprintf(manual_info, format,
            info,
            ssid_24g_title,
            ssid_title, ssid_24_val,
            password_title , pwd_24_val,
            security_title, security_24_val, encryp24G,
            ssid_5g_title,
            ssid_title, ssid_5_val,
            password_title , pwd_5_val,
            security_title, security_5_val, encryp5G);

    info_page_create(lv_scr_act(), get_string(ID_CONN_GUIDE_MANUAL_CONN), manual_info);
}
