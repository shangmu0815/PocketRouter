#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/settings/connected_users.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/wlan/hostapd_conf.h"
#include "lvgl/src/lv_objx/lv_list.h"
#include "lvgl/lvgl.h"

lv_obj_t * no_blocked_user_label;

char * blacklist_name;

#define MAX_CU_LISTE           MAX_CONNECTED_USERS
#define MAX_BLK_LISTE          HOSTAPD_DENY_CONFIG_MAX

lv_obj_t * no_conn_user_label;
lv_obj_t * cu_list;
lv_obj_t * del_list_btn;
char * conn_usr_name[MAX_CU_LISTE];
char * blk_usr_name[MAX_BLK_LISTE];
int blk_cnt;
int cu_cnt;
int blk_id;

// For simulator testing, create file Data_Store/connected_usr.txt with below data
// 28:3f:69:f3:a1:01
// connected_time=3160
// 28:3f:69:f3:a1:02
// connected_time=3160
// 28:3f:69:f3:a1:03
// connected_time=3160
// 28:3f:69:f3:a1:04
// connected_time=3160
// 28:3f:69:f3:a1:05
// connected_time=3160
// 28:3f:69:f3:a1:06
// connected_time=3160

void blacklist_list_action(lv_obj_t * list_btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    char* mac = lv_list_get_btn_text(list_btn);
    log_d("removing %s from blacklist", mac);

    remove_from_deny_list(mac);
    append_user(mac);
    lv_obj_del(list_btn);
    blk_cnt--;
    if (blk_cnt == 0) {
        lv_obj_set_hidden(no_blocked_user_label, false);
    }
}

void max_reached_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    close_popup();
}

void connected_users_select_action(lv_obj_t * list_btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    if (blk_cnt >= MAX_BLK_LISTE) {
        static const char *btns[2];
        btns[1] = "";
        btns[0] = get_string(ID_OK);
        popup_anim_not_create(get_string(ID_CONN_USR_BLOCKED_MAX_REACHED), btns, max_reached_action, NULL);
    } else {
        int id = lv_obj_get_user_data(list_btn);
        blk_id = id;
        del_list_btn = list_btn;
        add_to_blacklist_popup_warning(conn_usr_name[id]);
    }
}

void connected_users_with_bt(void) {
    info_page_create_label_align_center(lv_scr_act(), get_string(ID_CONN_USR),
            get_string(ID_WIFI_ERR_DUE_TO_BT_ON));
}

void init_connected_users(void) {
#ifdef BT_SUPPORT
    connected_users_with_bt();
#else
    connected_users_with_no_bt();
#endif
}

void connected_users_win_close_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    int i;
    for (i = 0; i < MAX_CU_LISTE; i++){
        if (conn_usr_name[i] != NULL) {
            lv_mem_free(conn_usr_name[i]);
            conn_usr_name[i] = NULL;
        }
    }
    for (i = 0; i < MAX_BLK_LISTE; i++){
        if (blk_usr_name[i] != NULL) {
            lv_mem_free(blk_usr_name[i]);
            blk_usr_name[i] = NULL;
        }
    }
}

void parse_users(char* cmd) {
    char* mac_addr = NULL;

    int i;
    char buffer[128];

    FILE *fp;
    fp = popen(cmd, "r");

    if (fp != NULL) {
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            char* value = strstr(buffer, "=");
            if (value) {
                char* time;
                int len = strlen(value);
                time = lv_mem_alloc(len);
                memcpy(time, value+1, len);
                size_t index = strcspn(time, "\n");
                if(index < strlen(time)){
                    time[index] = 0;
                }
                int t = atoi(time);

                char* h[10];
                int hour = (t / 60) / 60;
                sprintf(h, ((hour < 10)? "0%d" : "%d"), hour);
                char* m[2];
                int min = (t / 60) % 60;
                snprintf(m, sizeof(m), ((min < 10)? "0%d" : "%d"), min);
                char* s[2];
                int sec = t  % 60;
                snprintf(s, sizeof(s), ((sec < 10)? "0%d" : "%d"), sec);

                //lv_list_selector_t selector_info;
                //selector_info.type = SELECTOR_TYPE_COMBO_TYPE1;
                char time_msg[50];
                memset(time_msg, '\0', sizeof(time_msg));
                sprintf(time_msg, "%s:%s:%s:%s", get_string(ID_CONN_USR_DURATION), h, m, s);
                //selector_info.metadata = time_msg;
                if (mac_addr != NULL && !in_deny_list(mac_addr)) {
                    conn_usr_name[cu_cnt] = lv_liste_conn_usr(cu_list, mac_addr, 
                         time_msg, connected_users_select_action, cu_cnt);
                    cu_cnt++;
                }

                lv_mem_free(time);
                time = NULL;
            } else {
                //-1: delete '\n'
                int len = strlen(buffer);
                mac_addr = lv_mem_alloc(len);
                memcpy(mac_addr, buffer, len);
                size_t index = strcspn(mac_addr, "\n");
                if(index < strlen(mac_addr)){
                    mac_addr[index] = 0;
                }
            }
        }
        pclose(fp);
    }
}

void append_user(char * mac) {
    int i;
    for (i = 0; i < MAX_CU_LISTE; i++) {
        if (conn_usr_name[i] == NULL) break;
    }
#ifdef FEATURE_ROUTER
    for (i = WIFI_BAND_24G; i <= WIFI_BAND_5G; i++) {
        if (get_wifi_band_enabled(i)) {
            char cmd[150];
            sprintf(cmd, "hostapd_cli -i %s -p /var/run/hostapd/ all_sta |"
               " grep -e ^%s -e connected_time",
               (i == WIFI_BAND_24G) ? BAND_24G:BAND_5G, mac);
            parse_users(cmd);
        }
    }
#else
    char cmd[150];
    sprintf(cmd, "grep %s -A 1 Data_Store/connected_usr.txt", mac);
    parse_users(cmd);
#endif
}

void append_users() {
#ifdef FEATURE_ROUTER
    int i, count = 0;
    char buffer[128];
    for (i = WIFI_BAND_24G; i <= WIFI_BAND_5G; i++) {
        if (get_wifi_band_enabled(i)) {
            FILE *fp;
#if ANDROID_BUILD
            fp = popen("hostapd_cli -i wlan0 -p /data/vendor/wifi/hostapd/ctrl all_sta |"
                " grep -e ^[a-z0-9]*:[a-z0-9]*:[a-z0-9]*:[a-z0-9]*:[a-z0-9]*:[a-z0-9]* -e connected_time", "r");
#else
            char cmd[150];
            sprintf(cmd, "hostapd_cli -i %s -p /var/run/hostapd/ all_sta |"
               " grep -e ^[a-z0-9]*:[a-z0-9]*:[a-z0-9]*:[a-z0-9]*:[a-z0-9]*:[a-z0-9]* -e connected_time",
               (i == WIFI_BAND_24G) ? BAND_24G:BAND_5G);
            //fp = popen(cmd, "r");
#endif
            parse_users(cmd);
        }
    }
#else
    char cmd[150];
    sprintf(cmd, "cat Data_Store/connected_usr.txt");
    parse_users(cmd);
#endif
}

void connected_users_with_no_bt() {
    lv_obj_t * win = block_list_header(lv_scr_act(), get_string(ID_CONN_USR),
            any_existed_blacklist_action, connected_users_win_close_action);

    cu_cnt = 0;
    blk_cnt = deny_list_count();
    blk_id = 0;
    cu_list = lv_list_create(win, NULL);
    lv_obj_set_size(cu_list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_list_set_sb_mode(cu_list, LV_SB_MODE_OFF);
    lv_list_set_style(cu_list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(cu_list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(cu_list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(cu_list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(cu_list, LV_LAYOUT_OFF);

    append_users();

    static lv_style_t label_style;
    lv_style_copy(&label_style, &lv_style_plain);
    label_style.text.color = LV_COLOR_GREYISH_BROWN;
    label_style.text.font = get_font(font_w_bold, font_h_16);
    label_style.text.letter_space = 1;
    no_conn_user_label = lv_label_create(win, NULL);
    lv_label_set_text(no_conn_user_label, get_string(ID_CONN_USR_NO_USERS));
    lv_label_set_style(no_conn_user_label, LV_LABEL_STYLE_MAIN, &label_style);
    lv_obj_align(no_conn_user_label, win, LV_ALIGN_CENTER, 0, 15);
    lv_obj_set_hidden(no_conn_user_label, (cu_cnt == 0) ? false : true);
}

void blacklist_close_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    if (del_list_btn != NULL) {
        lv_obj_del(del_list_btn);
        del_list_btn = NULL;
    }

    lv_obj_set_hidden(no_conn_user_label, (cu_cnt == 0) ? false : true);
}

void blacklist() {

    lv_obj_t * win = default_list_header(lv_scr_act(), get_string(ID_CONN_USR_BLACKLIST), blacklist_close_action);

    lv_obj_t * blk_list = lv_list_create(win, NULL);
    lv_obj_set_size(blk_list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_list_set_sb_mode(blk_list, LV_SB_MODE_OFF);
    lv_list_set_style(blk_list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(blk_list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(blk_list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(blk_list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(blk_list, LV_LAYOUT_OFF);

    blk_cnt = deny_list_count();

    int i;
    for (i = 0; i < blk_cnt; i++){
        lv_liste_blk_usr(blk_list, read_deny_list(i)/*blk_usr_name[i]*/, blacklist_list_action, i);
    }
    static lv_style_t style_blacklist_label;
    lv_style_copy(&style_blacklist_label, &lv_style_plain);
    style_blacklist_label.text.color = LV_COLOR_GREYISH_BROWN;
    style_blacklist_label.text.font = get_font(font_w_bold, font_h_16);
    style_blacklist_label.text.letter_space = 1;
    no_blocked_user_label = lv_label_create(win, NULL);
    lv_label_set_text(no_blocked_user_label, get_string(ID_CONN_USR_NO_BLOCKED_USER));
    lv_label_set_style(no_blocked_user_label, LV_LABEL_STYLE_MAIN, &style_blacklist_label);
    lv_obj_align(no_blocked_user_label, win, LV_ALIGN_CENTER, 0, 15);
    if (blk_cnt > 0) {
        lv_obj_set_hidden(no_blocked_user_label, true);
    }
}

void add_to_blacklist_popup_warning_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(mbox);

    //lv_obj_t * mbox = lv_mbox_get_from_btn(btn);

    if(strcmp(txt, get_string(ID_CANCEL)) == 0) {
        del_list_btn = NULL;
        close_popup();
    } else if (strcmp(txt, get_string(ID_OK)) == 0) {
        log_d("adding %s to blacklist", blacklist_name);

        lv_obj_del(del_list_btn);
        del_list_btn = NULL;
        cu_cnt--;

        add_to_deny_list(blacklist_name);
        blk_usr_name[blk_cnt] = conn_usr_name[blk_id];
        conn_usr_name[blk_id] = NULL;
        blk_cnt++;
        blacklist();
        close_popup();
    }
}

void any_existed_blacklist_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    blacklist();
}

void add_to_blacklist_popup_warning(char * text) {
    char* data_info;
    data_info = lv_mem_alloc(strlen(get_string(ID_CONN_USR_ADD_TO_BLACKLIST)) + strlen(text) + 1);
    sprintf(data_info, get_string(ID_CONN_USR_ADD_TO_BLACKLIST), text);

    static const char * btns[3];
    btns[0] = get_string(ID_CANCEL);
    btns[1] = get_string(ID_OK);
    btns[2] = "";
    popup_anim_que_create(data_info, btns, add_to_blacklist_popup_warning_action, NULL);
    blacklist_name = text;

    if (data_info != NULL) {
        lv_mem_free(data_info);
        data_info = NULL;
    }
}
