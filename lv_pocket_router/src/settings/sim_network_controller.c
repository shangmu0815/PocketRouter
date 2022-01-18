#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <libxml/tree.h>
#include <libxml/xmlsave.h>
#include <libxml/xmlwriter.h>
#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/settings/sim_network_controller.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"

static int prev_sim_state = UNKNOWN;
static int blacklist_sim_file_state = -2;

//to get airplane mode status with AT command "atcli at+cfun?"
//airplane mode off:1
//airplane mode on:4 or not 1
bool get_airplane_mode_status() {
    bool airplaneMode = false;
    int airplaneModeStatus = -1;
    char expectResStr[8] = "+CFUN: ";
    char cmd[20];
    char buffer[60];
    char tmpAirplaneModeStatus[2];
    memset(cmd, '\0', sizeof(cmd));
    memset(buffer, '\0', sizeof(buffer));
    memset(tmpAirplaneModeStatus, '\0', sizeof(tmpAirplaneModeStatus));
    sprintf(cmd, "atcli at+cfun?");
    log_d("get_airplane_mode cmd:%s",cmd);
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        log_e("popen failed cmd:%s", cmd);
        assert(false);
        return airplaneMode;
    }
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if(buffer != NULL && strlen(buffer) > 0){
            char *found = strstr(buffer, expectResStr);
            if (found != NULL) {
                strncpy(tmpAirplaneModeStatus, found + 7, 1);
                airplaneModeStatus = atoi(tmpAirplaneModeStatus);
            }
        }
    }
    pclose(fp);
    fp = NULL;
    log_d("get_airplane_mode airplaneModeStatus:%d",airplaneModeStatus);
    airplaneMode = ((airplaneModeStatus == AIRPLANE_MODE_OFF) ? false : true);
    return airplaneMode;
}

bool set_airplane_mode(int val) {
    bool res = false;
    char cmd[20];
    char buffer[60];
    memset(cmd, '\0', sizeof(cmd));
    memset(buffer, '\0', sizeof(buffer));
    sprintf(cmd, "atcli at+cfun=%d", val);
    log_d("set_airplane_mode cmd:%s",cmd);
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        log_e("popen failed cmd:%s", cmd);
        assert(false);
        return res;
    }
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if(buffer != NULL && strlen(buffer) > 0){
            log_d("set airplane mode buffer:%s", buffer);
            char *found = strstr(buffer, "OK");
            if (found != NULL) {
                //set airplane mode OK
                res = true;
            }
        }
    }
    pclose(fp);
    fp = NULL;
    log_d("set_airplane_mode res:%d",res);
    return res;
}

void check_airplane_mode() {
    bool modem_airplane_mode = get_airplane_mode_status();
    log_d("check_airplane_mode modem_airplane_mode:%d", modem_airplane_mode);
    log_d("check_airplane_mode blacklist_sim_file_state:%d", blacklist_sim_file_state);
    if (is_blacklist_sim() == true) {
        if (modem_airplane_mode != true) {
            set_airplane_mode(AIRPLANE_MODE_ON);
        }
        return;
    }
    if (modem_airplane_mode != false) {
        set_airplane_mode(AIRPLANE_MODE_ON);
    }
    ds_set_bool(DS_KEY_AIRPLANE_MODE, modem_airplane_mode);
    log_d("check_airplane_mode ds_get_bool(DS_KEY_AIRPLANE_MODE):%d",ds_get_bool(DS_KEY_AIRPLANE_MODE));
}

void init_airplane_mode() {
    bool airplane_mode = ds_get_bool(DS_KEY_AIRPLANE_MODE);
    log_d("init_airplane_mode airplane_mode:%d", airplane_mode);
    set_airplane_mode(airplane_mode ? AIRPLANE_MODE_ON : AIRPLANE_MODE_OFF);
}

void check_blacklist_sim() {
    if (blacklist_sim_file_state == -2) {
        blacklist_sim_file_state = access(SIM_BLACKLIST_STORE_FILE, F_OK);
        log_d("check_blacklist_sim blacklist_sim_file_state:%d", blacklist_sim_file_state);
    }

    int state = get_sim_state();
    //sim insert
    if (prev_sim_state != READY && state == READY) {
        if (is_blacklist_sim() == true) {
            log_d("check_blacklist_sim sim READY set_airplane_mode");
            set_airplane_mode(AIRPLANE_MODE_ON);
        }
    }
    //sim remove
    if (prev_sim_state != ABSENT && state == ABSENT) {
        bool airplane_mode = ds_get_bool(DS_KEY_AIRPLANE_MODE);
        log_d("check_blacklist_sim sim ABSENT airplane_mode:%d", airplane_mode);
        set_airplane_mode(airplane_mode ? AIRPLANE_MODE_ON : AIRPLANE_MODE_OFF);
    }
    prev_sim_state = state;
}

//check mcc mnc whether in /oem/data/sim_blacklist.xml or not
bool is_blacklist_sim() {
    bool isBlacklistSim = false;
    int state = get_sim_state();
    log_d("is_blacklist_sim blacklist_sim_file_state:%d", blacklist_sim_file_state);
    if (state == READY && blacklist_sim_file_state == 0) {
        char mcc[RIL_NW_MCC_MAX_LENGTH + 1];
        char mnc[RIL_NW_MNC_MAX_LENGTH + 1];
        memset(mcc, '\0', sizeof(mcc));
        memset(mnc, '\0', sizeof(mnc));
        #if defined (FEATURE_ROUTER)
            get_sim_mcc_mnc(mcc, RIL_NW_MCC_MAX_LENGTH+1, mnc, RIL_NW_MNC_MAX_LENGTH+1);
        #else
            strcpy(mcc,"466");
            strcpy(mnc,"92");
        #endif

        xmlDocPtr doc = xmlReadFile(SIM_BLACKLIST_STORE_FILE, NULL, XML_PARSE_RECOVER);
        if (doc == NULL) {
            log_e("Failed to read %s", SIM_BLACKLIST_STORE_FILE);
            return isBlacklistSim;
        }
        xmlNodePtr  curNode;
        curNode = xmlDocGetRootElement(doc);
        curNode = curNode->xmlChildrenNode;
        while (curNode != NULL) {
            if (!xmlStrcmp(curNode->name, BAD_CAST (const xmlChar *)SIM_BLACKLIST_HEADER)) {
                char* blacklist_mcc = xmlGetProp(curNode,BAD_CAST SIM_BLACKLIST_MCC_HEADER);
                char* blacklist_mnc = xmlGetProp(curNode,BAD_CAST SIM_BLACKLIST_MNC_HEADER);
                if (strcmp(((char *)blacklist_mcc), mcc) == 0 &&
                            strcmp(((char *)blacklist_mnc), mnc) == 0) {
                    isBlacklistSim = true;
                    xmlFree(blacklist_mcc);
                    xmlFree(blacklist_mnc);
                    break;
                }
                xmlFree(blacklist_mcc);
                xmlFree(blacklist_mnc);
            }
            curNode = curNode->next;
        }
        xmlFreeDoc(doc);
    }
    log_d("is_blacklist_sim isBlacklistSim:%d", isBlacklistSim);
    return isBlacklistSim;
}
