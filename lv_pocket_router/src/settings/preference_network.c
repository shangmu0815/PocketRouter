#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/settings/preference_network.h"
#include "lv_pocket_router/src/settings/network_settings.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"

//TODO need add implementation for AT timeout case
#define AT_CMD_TIMEOUT     1000

int pn_value_map[]  = {
#ifndef CUST_GEMTEKS
                        ID_3G_ONLY,
#endif
                        ID_3G_4G, ID_4G_ONLY,
#ifndef CUST_GEMTEKS
                        ID_5G_ONLY,
#endif
                        ID_4G_5G, ID_3G_4G_5G };
int pn_str_id_map[] = {
#ifndef CUST_GEMTEKS
                        ID_NW_3G_ONLY,
#endif
                        ID_NW_3G_4G, ID_NW_4G_ONLY,
#ifndef CUST_GEMTEKS
                        ID_NW_5G_ONLY,
#endif
                        ID_NW_4G_5G, ID_NW_3G_4G_5G };

#define MAX_LISTE          sizeof(pn_value_map) / (sizeof(int))

lv_obj_t * pnListImg[MAX_LISTE];
int selectId = -1;

bool cmp_pref_network();
bool set_pref_network(int id);
int get_pref_network();

//init preference network default value
void init_pref_network(){
    if(!ds_get_bool(DS_KEY_INIT_PREF_NETWORK)){
#ifdef CUST_ZX73_NEC
        int res = set_pref_network(ID_5G_ONLY);
        log_d("preference network set 5G only as default, res: %d", res);
#else
        int res = set_pref_network(ID_3G_4G_5G);
        log_d("preference network set 3G+4G_5G as default, res: %d", res);
#endif
        ds_set_bool(DS_KEY_INIT_PREF_NETWORK, true);
    }
}

//close action for preference network result popup
void result_popup_close_action(lv_obj_t * mbox, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;
    close_popup();
}

//preference network result popup, shows up after preference network list selected -> ok action
void pref_network_result_popup(bool res) {
    static const char *btns[2];
    btns[0] = get_string(ID_OK);
    btns[1] = "";
    popup_anim_not_create((res ?
            get_string(ID_NW_SET_SUCCESS) : get_string(ID_NW_SET_FAIL)),
            btns, result_popup_close_action, NULL);
    log_d("preference network set value to: %d, %s",
            pn_value_map[selectId], res ? "Success":"Failed");
}

//preference network list selected -> ok action
void set_pref_network_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    set_pref_network(pn_value_map[selectId]);

    //update preference network liste value
    update_pref_network_lable();

    //compare preference network between current/set value
    pref_network_result_popup(cmp_pref_network());
}

void uncheck_all_img(){
    int i;
    for (i = 0; i < MAX_LISTE; i++) {
        lv_img_set_src(pnListImg[i], &btn_list_radio_n);
    }
}

//action for user choosing preference network list
void pref_network_list_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    int id = lv_obj_get_user_data(btn);
    if(selectId < 0){
        uncheck_all_img();
        log_e("fail to get preference network default value");
    } else {
        lv_img_set_src(pnListImg[selectId], &btn_list_radio_n);
    }
    lv_img_set_src(pnListImg[id], &btn_list_radio_p);
    selectId = id;
}

//to get preference network with AT command “atcli at\$ceinsp?"
int get_pref_network(){
    //TODO in current design, preference network did not set to a default value if fail to get valid selectId
    int nwId = -1;

    char expectResStr[10] = "$CEINSP: ";
    char cmd[20];
    char buffer[60];
    char tmpNwId[2];
    memset(cmd, '\0', sizeof(cmd));
    memset(buffer, '\0', sizeof(buffer));
    memset(tmpNwId, '\0', sizeof(tmpNwId));

    sprintf(cmd, "atcli at\\\$ceinsp\?");
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        log_e("popen failed cmd %s", cmd);
        assert(false);
        return nwId;
    }
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if(buffer != NULL && strlen(buffer) > 0){
            char *found = strstr(buffer, expectResStr);
            if (found != NULL) {
                strncpy(tmpNwId, found + 9, 1);
                nwId = atoi(tmpNwId);
            }
        }
    }
    pclose(fp);
    fp = NULL;

    return nwId;
}

//to check preference network been set successfully
bool cmp_pref_network(){
    int currId = get_pref_network();
    return (currId == pn_value_map[selectId]) ? true : false;
}

//to set preference network with AT command “atcli at\$ceinsp=n", the "n" in the end is a variable
//return true if set successfully, false otherwise
bool set_pref_network(int val){
    int res = false;
    char cmd[20];
    char buffer[60];
    memset(cmd, '\0', sizeof(cmd));
    memset(buffer, '\0', sizeof(buffer));
    sprintf(cmd, "atcli at\\\$ceinsp=%d", val);

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        log_e("popen failed cmd %s", cmd);
        assert(false);
        return res;
    }
    //TODO error handling?
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if(buffer != NULL && strlen(buffer) > 0){
            log_d("set preference network buffer: %s", buffer);
            char *found = strstr(buffer, "OK");
            if (found != NULL) {
                //set preference network OK
                res = true;
            }
        }
    }
    pclose(fp);
    fp = NULL;

    return res;
}

//get the current preference network choice string for Settings-> Network settings-> Preference network liste
void get_default_pref_network(char* title){
    int id = get_pref_network();
    log_d("get default preference network id: %d", id);
    int i;
    for (i = 0; i < MAX_LISTE; i++) {
        if (id == pn_value_map[i]) {
            sprintf(title, "%s", get_string(pn_str_id_map[i]));
            return;
        }
    }
}

//to create Settings-> Network settings-> Preference network page
void pref_network_create(void) {
    log_d("preference network create");

    liste_style_create();
    lv_obj_t * win = modify_list_header(
            lv_scr_act(), get_string(ID_NW_PREFERENCE_NETWORK),
            set_pref_network_action, lv_win_close_event_cb);

    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    int i;
    int currId = get_pref_network();
    selectId = -1;
    for (i = 0; i < MAX_LISTE; i++) {
        if (currId == pn_value_map[i]) {
            selectId = i;
        }
        lv_obj_t * l = lv_liste_w_cbox(list, get_string(pn_str_id_map[i]),
                (selectId == i), pref_network_list_action, i);
        pnListImg[i] = lv_obj_get_child(l, NULL);
    }
}
