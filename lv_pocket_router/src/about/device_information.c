/*
 * device_information.c
 *
 *  Created on: Mar 18, 2019
 *      Author: joseph
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/about/device_information.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/wlan/hostapd_conf.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/ril/ril.h"

#define MAX_BUFFER_LEN              100

#define DUMP_WANIP_WITH_API

static const char addrPattern[] = "HWaddr ";
static const char ipStartPattern[] = "inet addr:";
static const char ipEndPattern[] = "  Bcast";
static const char rmnetipEndPattern[] = "  Mask";

typedef struct {
    bool init;
    char moduleSwVer[MAX_INFO_LENGTH];
    char swVer[MAX_INFO_LENGTH];
    char phoneNum[MAX_INFO_LENGTH];
    char imei[MAX_INFO_LENGTH];
    char wlanMac[MAX_INFO_LENGTH + MAX_DATA_LENGTH];
    char wanIP[MAX_INFO_LENGTH + MAX_DATA_LENGTH];
    char lanIP[MAX_INFO_LENGTH];
#if defined (CUST_ZYXEL)
    char zyxelFwVer[MAX_INFO_LENGTH];
#endif
    char dlinkFwVer[MAX_INFO_LENGTH];
} DeviceInfo;

DeviceInfo deviceInfo;

void init_device_info() {
    memset(&deviceInfo, 0, sizeof(deviceInfo));

    readSwVersion();
    readRilInfo();
    //readWlanInfo(); //parse it each time at query so no need do init
    readLanInfo();

    deviceInfo.init = true;
}

void show_device_information(void) {
    init_device_info();

    //Draw about page header 320x50
    lv_obj_t * win = default_list_header (lv_scr_act(), get_string(ID_ABOUT_DEVICE_INFO), lv_win_close_event_cb);
    lv_obj_t * device_information_list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(device_information_list, LV_SB_MODE_OFF);
    lv_list_set_style(device_information_list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(device_information_list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(device_information_list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(device_information_list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(device_information_list, LV_LAYOUT_OFF);

    const char *titleStr[] = {
            get_string(ID_ABOUT_IMEI),
            get_string(ID_ABOUT_LAN_MAC_ADDRESS),
#ifdef BT_SUPPORT
            get_string(ID_ABOUT_BLE_MAC_ADDRESS),
            get_string(ID_ABOUT_BLE_NAME),
#endif
#if defined (CUST_ZYXEL)
            get_string(ID_ABOUT_SFT_VERSION_ZYXEL),
#endif
            get_string(ID_ABOUT_MODULE_VERSION),
            get_string(ID_ABOUT_SFT_VERSION),
#ifdef CUST_DLINK
            get_string(ID_ABOUT_SFT_VERSION_DLINK),
#endif
            get_string(ID_ABOUT_PHONE_NUMBER),
            get_string(ID_ABOUT_LAN_IP_ADDRESS),
            get_string(ID_ABOUT_WAN_IP_ADDRESS)
    };

    char *dataStr[] = {
            getImei(),
            getWlanMacAddress(),
#ifdef BT_SUPPORT
            getBtnMacAddress(),
            getBtName(),
#endif
#if defined (CUST_ZYXEL)
            getZyxelFwVersion(),
#endif
            getModuleSwVersion(),
            getSwVersion(),
#ifdef CUST_DLINK
            getDLinkFwVersion(),
#endif
            getPhoneNum(),
            getLanIpAddress(),
            getWanIpAddress()
    };

    int elementNum = sizeof(titleStr) / sizeof(char *);
    int i;
    for (i = 0; i < elementNum; i++) {
        lv_liste_setting_double_info(device_information_list, titleStr[i], dataStr[i]);
    }
}

char* getPhoneNum() {
    if (!deviceInfo.init) init_device_info();
    if (strlen(deviceInfo.phoneNum) == 0) {
        return get_string(ID_UNKNOWN);
    } else {
        return deviceInfo.phoneNum;
    }
}

char* getImei() {
    if (!deviceInfo.init) init_device_info();
    if (strlen(deviceInfo.imei) == 0) {
        return get_string(ID_UNKNOWN);
    } else {
        return deviceInfo.imei;
    }
}

char* getWlanMacAddress() {
    //if (!deviceInfo.init) init_device_info();
    //wlan can be disable/enable at runtime so need to re-parse it each time
    readWlanInfo();
    return deviceInfo.wlanMac;
}

char* getBtnMacAddress() {
    return get_string(ID_UNKNOWN);
}

char* getModuleSwVersion() {
    if (!deviceInfo.init) init_device_info();
    return deviceInfo.moduleSwVer;
}

char* getSwVersion() {
    if (!deviceInfo.init) init_device_info();
    return deviceInfo.swVer;
}

char* getBtName() {
    return get_string(ID_UNKNOWN);
}

char* getLanIpAddress() {
    if (!deviceInfo.init) init_device_info();
    return deviceInfo.lanIP;
}

char* getWanIpAddress() {
    if (!deviceInfo.init) init_device_info();
    return deviceInfo.wanIP;
}

void readVersion(char* file, char* ver) {
    FILE *fp;
    char cmd[50];
    sprintf(cmd, "cat %s", file);
#if FEATURE_ROUTER
    fp = popen(cmd, "r");
#else
    fp = popen("cat /proc/version", "r");
#endif
    if (fp == NULL) {
        return;
    }

    char buffer[MAX_DATA_LENGTH];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    if (strlen(buffer) == 0 || strncmp(buffer, "NULL", sizeof("NULL") - 1) == 0) {
        strncpy(ver, get_string(ID_UNKNOWN), strlen(get_string(ID_UNKNOWN)));
    } else {
        strncpy(ver, buffer, strlen(buffer));
        size_t index = strcspn(ver, "\n");
        if (index < strlen(ver)){
            ver[index] = 0;
        }

        // replace cust_id from last two char
        fp = popen("cat /oem/cust_id", "r");
        if (fp != NULL) {
            char cust_id[MAX_DATA_LENGTH];
            memset(cust_id, 0, sizeof(cust_id));
            fgets(cust_id, sizeof(cust_id), fp);
            if (strlen(cust_id) > 0 && strncmp(cust_id, "NULL", sizeof("NULL") - 1) != 0) {
                strncpy(ver + strlen(ver) - 2, cust_id, strlen(cust_id));
                size_t index = strcspn(ver, "\n");
                if (index < strlen(ver)){
                    ver[strcspn(ver, "\n")] = 0;
                }
            }
        }
    }
    pclose(fp);
}

void readSwVersion() {
    readVersion("/proc/cei_module_ver_id", deviceInfo.moduleSwVer);
    readVersion("/proc/cei_mifi_ver_id", deviceInfo.swVer);
}

void readRilInfo() {
    get_phone_num(deviceInfo.phoneNum, MAX_INFO_LENGTH);
    get_imei(deviceInfo.imei, MAX_INFO_LENGTH);
}

void readWlanInfo() {
    int i;
    for (i = WIFI_BAND_24G; i <= WIFI_BAND_5G; i++) {
        if (get_wifi_band_enabled(i)) {
            FILE *fp;
#if FEATURE_ROUTER
            char cmd[20];
            sprintf(cmd, "ifconfig %s", (i == WIFI_BAND_24G) ? BAND_24G:BAND_5G);
            fp = popen(cmd, "r");
#else
            fp = popen("ifconfig eth0", "r"); //set to eth0 for simulator testing
#endif
            if (fp == NULL) {
                return;
            }

            int pos = strlen(deviceInfo.wlanMac);
            char buffer[MAX_BUFFER_LEN];
            memset(buffer, 0, sizeof(buffer));
            fgets(buffer, sizeof(buffer), fp);
            char *addr = NULL;
            addr = strstr(buffer, addrPattern);
            if (addr != NULL) {
                addr += (sizeof(addrPattern) - 1);
                if (pos > 1) {
                    size_t index = strcspn(deviceInfo.wlanMac, " ") + 1;
                    if (index < strlen(deviceInfo.wlanMac)){
                        deviceInfo.wlanMac[index] = 0;
                    }
                    pos = strlen(deviceInfo.wlanMac);
                }
                strncpy(deviceInfo.wlanMac + pos, addr, strlen(addr));
                size_t index = strcspn(deviceInfo.wlanMac, "\n");
                if (index < strlen(deviceInfo.wlanMac)){
                    deviceInfo.wlanMac[index] = 0;
                }
            }

#ifdef DUMP_WANIP_WITH_IFCONFIG
            pos = strlen(deviceInfo.wanIP);
            fgets(buffer, sizeof(buffer), fp);
            char *ip = NULL;
            ip = strstr(buffer, ipStartPattern);
            if (ip != NULL) {
                ip += (sizeof(ipStartPattern) - 1);
                if (pos > 1) {
                    strncpy(deviceInfo.wanIP + pos, " ", 1);
                    pos++;
                }
                int len = strstr(buffer, ipEndPattern) - ip;
                strncpy(deviceInfo.wanIP + pos, ip, len);
                deviceInfo.wanIP[strcspn(deviceInfo.wanIP, "\n")] = 0;
            }
#endif
            pclose(fp);
        }
    }

#ifdef DUMP_WANIP_WITH_RMNET_DATA
    FILE *fp;
#if FEATURE_ROUTER
    fp = popen("ifconfig rmnet_data0", "r");
#else
    fp = popen("ifconfig docker0", "r"); //set to docker0 for simulator testing
#endif
    if (fp == NULL) {
        return;
    }

    char buffer[MAX_BUFFER_LEN];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    fgets(buffer, sizeof(buffer), fp); //read 2nd line
    char *ip = NULL;
    ip = strstr(buffer, ipStartPattern);
    if (ip != NULL) {
        ip += (sizeof(ipStartPattern) - 1);
        int len = strstr(buffer, rmnetipEndPattern) - ip;
        strncpy(deviceInfo.wanIP, ip, len);
        if(strcspn(deviceInfo.wanIP, "\n") < sizeof(deviceInfo.wanIP)) {
            deviceInfo.wanIP[strcspn(deviceInfo.wanIP, "\n")] = 0;
        }
    }

    pclose(fp);
#endif

#ifdef DUMP_WANIP_WITH_API
    ril_get_network_config_resp resp = {0};
    if(get_network_config(&resp) == RIL_SUCCESS){
        if( resp.error_v4 == RIL_SUCCESS ){
            if( resp.error_v6 == RIL_SUCCESS ){
                sprintf(deviceInfo.wanIP, "%s  %s", resp.v4_public_ip, resp.v6_public_ip);
            }else{
                sprintf(deviceInfo.wanIP, "%s", resp.v4_public_ip);
            }
        } else {
            if( resp.error_v6 == RIL_SUCCESS ){
                sprintf(deviceInfo.wanIP, "%s", resp.v6_public_ip);
            }else{
                log_d("failed to get network config");
            }
        }
    } else {
        log_d("failed to get network config");
    }
#endif
}

void readLanInfo() {
    FILE *fp;
#if FEATURE_ROUTER
    fp = popen("ifconfig bridge0", "r");
#else
    fp = popen("ifconfig eth0", "r"); //set to eth0 for simulator testing
#endif
    if (fp == NULL) {
        return;
    }

    char buffer[MAX_BUFFER_LEN];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    fgets(buffer, sizeof(buffer), fp); //read 2nd line
    char *ip = NULL;
    ip = strstr(buffer, ipStartPattern);
    if (ip != NULL) {
        ip += (sizeof(ipStartPattern) - 1);
        int len = strstr(buffer, ipEndPattern) - ip;
        strncpy(deviceInfo.lanIP, ip, len);
        size_t index = strcspn(deviceInfo.lanIP, "\n");
        if (index < strlen(deviceInfo.lanIP)){
            deviceInfo.lanIP[index] = 0;
        }
    }

    pclose(fp);
}

#if defined (CUST_ZYXEL)
#define ZYXEL_FW_FORMAT               "V1.00(%s.1)b"
void readZyxelFwVersion() {
    FILE *fp;
    char cmd[50];
#if FEATURE_ROUTER
    sprintf(cmd, "cat %s", "/proc/cei_mifi_ver_id");
#else
    sprintf(cmd, "cat %s", "Data_Store/cei_mifi_ver_id");
#endif
    fp = popen(cmd, "r");
    if (fp == NULL) {
        return;
    }

    char buffer[MAX_DATA_LENGTH];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    if (strlen(buffer) == 0 || strncmp(buffer, "NULL", sizeof("NULL") - 1) == 0) {
        strncpy(deviceInfo.zyxelFwVer, get_string(ID_UNKNOWN), strlen(get_string(ID_UNKNOWN)));
    } else {
        char zyxelFw[40];
        memset(zyxelFw, '\0', sizeof(zyxelFw));
        char infobuffer[MAX_DATA_LENGTH];
        memset(infobuffer, '\0', sizeof(infobuffer));
        FILE *fptr;
#if FEATURE_ROUTER
        fptr = popen("cat /oem/Zyxel_cust_id", "r");
#else
        fptr = popen("cat Data_Store/Zyxel_cust_id", "r");
#endif
        if (fptr != NULL) {
            fgets(infobuffer, sizeof(infobuffer), fptr);
            if (strlen(infobuffer) > 0) {
                infobuffer[strcspn(infobuffer, "\n")] = 0;
            } else {
                strcpy(infobuffer,"ABUS");
            }
            sprintf(zyxelFw, ZYXEL_FW_FORMAT, infobuffer);
            pclose(fptr);
        }

        char *version;
        int num = 0;
        version = strtok(buffer, ".");
        while (version != NULL) {
            num++;
            version = strtok(NULL, ".");
            if (num == 3) break;//ex:version = 0XX_0R00 from split "AXDG1.00.00.0XX_0R00"
        }
        char *mapping_version;
        mapping_version = strtok(version, "_"); //ex:get 0XX from 0XX_0R00
        if (mapping_version != NULL) {
            char str_mapping_version[4];
            memset(str_mapping_version, 0, sizeof(str_mapping_version));
            sprintf(str_mapping_version, "%d", atoi(mapping_version));
            strcat(zyxelFw, str_mapping_version); //ex:V1.00(ABUS.0)bXX or V1.00(ABUS.0)bXXX
        }
        strncpy(deviceInfo.zyxelFwVer, zyxelFw, strlen(zyxelFw));
        deviceInfo.zyxelFwVer[strcspn(deviceInfo.zyxelFwVer, "\n")] = 0;
    }
    pclose(fp);
}

char* getZyxelFwVersion() {
    readZyxelFwVersion();
    return deviceInfo.zyxelFwVer;
}
#endif

char* getDLinkFwVersion() {
    FILE *fp;
#if FEATURE_ROUTER
    fp = popen("cat /oem/cei_mifi_dlink_ver_id", "r");
#else
    fp = popen("cat Data_Store/cei_mifi_dlink_ver_id", "r");
#endif
    if (fp == NULL) {
        return "";
    }

    char buffer[MAX_DATA_LENGTH];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    if (strlen(buffer) == 0 || strncmp(buffer, "NULL", sizeof("NULL") - 1) == 0) {
        strncpy(deviceInfo.dlinkFwVer, get_string(ID_UNKNOWN), strlen(get_string(ID_UNKNOWN)));
    } else {
        strncpy(deviceInfo.dlinkFwVer, buffer, strlen(buffer));
        size_t index = strcspn(deviceInfo.dlinkFwVer, "\n");
        if (index < strlen(deviceInfo.dlinkFwVer)){
            deviceInfo.dlinkFwVer[index] = 0;
        }
    }
    pclose(fp);

    return deviceInfo.dlinkFwVer;
}
