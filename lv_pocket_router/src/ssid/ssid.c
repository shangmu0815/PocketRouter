#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lvgl/lvgl.h"
#include "lv_pocket_router/src/conn_guide/display_qr_code.h"
#include "lv_pocket_router/src/keyboard/en_kb.h"
#include "lv_pocket_router/src/ssid/ssid.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/page_anim.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/res/values/styles.h"
#include "lv_pocket_router/src/wlan/hostapd_conf.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/status_bar_view.h"

#define MIN_PASSWORD_ASCII  0x21
#define MAX_PASSWORD_ASCII  0x7e

#define SSID_SECURITY_MAX_LISTE 5

enum SSID_SECURITY_IDS {
    ID_SUB_SECURITY_NONE,
    ID_SUB_SECURITY_WEP,
    ID_SUB_SECURITY_WPA_PSK,
    ID_SUB_SECURITY_WPA_WPA2_PSK,
    ID_SUB_SECURITY_WPA3_WPA2_MIXED_MODE
};

int ssid_security_map[SSID_SECURITY_MAX_LISTE] = {
    ID_WIFI_SECURITY_NONE , ID_WIFI_SECURITY_AUTO, ID_WIFI_SECURITY_WPA_PSK,
    ID_WIFI_SECURITY_WPA2_PSK, ID_WIFI_SECURITY_WPA3_WPA2_MIXED_MODE
};

lv_obj_t * ssid_security_liste_img[SSID_SECURITY_MAX_LISTE];
lv_obj_t * ssid_security_liste[SSID_SECURITY_MAX_LISTE];
int ssid_security_type_24g = ID_SUB_SECURITY_WPA_WPA2_PSK;
int ssid_security_type_5g = ID_SUB_SECURITY_WPA_WPA2_PSK;
int ssid_select_security_type;

enum SSID_IDS {
    ID_SUB_SSID_24G,
    ID_SUB_PASSWORD_24G,
    ID_SUB_SECURITY_24G,
    ID_SUB_SSID_5G,
    ID_SUB_PASSWORD_5G,
    ID_SUB_SECURITY_5G,
    ID_SUB_SSID_24G_QR_CODE,
    ID_SUB_SSID_5G_QR_CODE,
};

lv_obj_t * liste_ssid_pwd_visible;
lv_obj_t * liste_ssid_24g;
lv_obj_t * liste_password_24g;
lv_obj_t * liste_security_24g;
lv_obj_t * liste_ssid_5g;
lv_obj_t * liste_password_5g;
lv_obj_t * liste_security_5g;
lv_obj_t * liste_pwd_visible_sw;
lv_obj_t * liste_24g_ssid_qrcode;
lv_obj_t * liste_5g_ssid_qrcode;

char* ssid_label_24g;
char* ssid_label_24g_val;
char* ssid_label_5g;
char* ssid_label_5g_val;
char* pw_label_24g;
char* pw_label_24g_val;
char* pw_label_5g;
char* pw_label_5g_val;
char* security_label_24g;
char* security_label_24g_val;
char* security_label_5g;
char* security_label_5g_val;
bool ssid_pwd_visible;
char pwd_hide_str[MAX_PASSWORD_LEN + 1];

//to get security type and string
char* get_security_info(int band){
    char* str;
    char* security;
    int type;

    if(band == WIFI_BAND_24G){
        security = get_wlan_security(WIFI_BAND_24G);
    } else if(band == WIFI_BAND_5G){
        security = get_wlan_security(WIFI_BAND_5G);
    }
    if (strcmp(SECURITY_WEP , security) == 0) {
        type = ID_SUB_SECURITY_WEP;
        str = get_string(ID_WIFI_SECURITY_AUTO);
    } else if (strcmp(SECURITY_WPA_PSK, security) == 0) {
        type = ID_SUB_SECURITY_WPA_PSK;
        str = get_string(ID_WIFI_SECURITY_WPA_PSK);
    } else if (strcmp(SECURITY_WPA2_PSK, security) == 0) {
        type = ID_SUB_SECURITY_WPA_WPA2_PSK;
        str = get_string(ID_WIFI_SECURITY_WPA2_PSK);
    } else if (strcmp(SECURITY_WPA3_WPA2, security) == 0) {
        type = ID_SUB_SECURITY_WPA3_WPA2_MIXED_MODE;
        str = get_string(ID_WIFI_SECURITY_WPA3_WPA2_MIXED_MODE);
    } else {
        type = ID_SUB_SECURITY_NONE;
        str = get_string(ID_WIFI_SECURITY_NONE);
    }

    if(band == WIFI_BAND_24G){
        ssid_security_type_24g = type;
    } else if(band == WIFI_BAND_5G){
        ssid_security_type_5g = type;
    }
    return str;
}

void update_ssid_label(int band) {
    char* txt = get_wlan_ssid(band);
    if (txt == NULL) {
        txt = "";
    }
    int len = strlen(txt);
    if (txt[len-1] == '\n') {
        txt[len-1] = '\0';
    }
    if(band == WIFI_BAND_24G){
#ifdef SUPPORT_2.4PLUS5GHZ
        int string_id = ID_SSID_24G;
#else
        int string_id = ID_LAUNCHER_SSID;
#endif
        if (ssid_label_24g != NULL) {
            lv_mem_free(ssid_label_24g);
            ssid_label_24g = NULL;
        }
        if (is_ltr()) {
            ssid_label_24g = lv_mem_alloc(strlen(get_string(string_id)) +
                    strlen(": ") + strlen(txt) + 1);
            memset(ssid_label_24g, 0, strlen(get_string(string_id)) + strlen(txt) + 1);
            sprintf(ssid_label_24g, "%s: %s", get_string(string_id), txt);
        } else {
            if (ssid_label_24g_val != NULL) {
                lv_mem_free(ssid_label_24g_val);
                ssid_label_24g_val = NULL;
            }
            ssid_label_24g = lv_mem_alloc(strlen(get_string(string_id)) + 1);
            memset(ssid_label_24g, 0, strlen(get_string(string_id)) + 1);
            sprintf(ssid_label_24g, "%s", get_string(string_id));
            ssid_label_24g_val = lv_mem_alloc(strlen(txt) + 1);
            memset(ssid_label_24g_val, 0, strlen(txt) + 1);
            sprintf(ssid_label_24g_val, "%s", txt);
        }
    } else if(band == WIFI_BAND_5G){
        if (ssid_label_5g != NULL) {
            lv_mem_free(ssid_label_5g);
            ssid_label_5g = NULL;
        }
        if (is_ltr()) {
            ssid_label_5g = lv_mem_alloc(strlen(get_string(ID_SSID_5G)) +
                    strlen(": ") + strlen(txt) + 1);
            memset(ssid_label_5g, 0, strlen(get_string(ID_SSID_5G)) + strlen(txt) + 1);
            sprintf(ssid_label_5g, "%s: %s", get_string(ID_SSID_5G), txt);
        } else {
            if (ssid_label_5g_val != NULL) {
                lv_mem_free(ssid_label_5g_val);
                ssid_label_5g_val = NULL;
            }
            ssid_label_5g = lv_mem_alloc(strlen(get_string(ID_SSID_5G)) + 1);
            memset(ssid_label_5g, 0, strlen(get_string(ID_SSID_5G)) + 1);
            sprintf(ssid_label_5g, "%s", get_string(ID_SSID_5G));
            ssid_label_5g_val = lv_mem_alloc(strlen(txt) + 1);
            memset(ssid_label_5g_val, 0, strlen(txt) + 1);
            sprintf(ssid_label_5g_val, "%s", txt);
        }
    }
}

void set_ssid_pwd_lable_impl(int band, bool empty) {
    if (band == WIFI_BAND_24G) {
        update_pw_label(WIFI_BAND_24G);
        if(empty){
            char pwd24g[strlen(get_string(ID_SSID_PWD_24G)) + 2];
            sprintf(pwd24g, "%s:", get_string(ID_SSID_PWD_24G));
            lv_label_set_text(lv_get_child_by_index(liste_password_24g, 1), pwd24g);
        }else{
            lv_label_set_text(lv_get_child_by_index(liste_password_24g, 1), pw_label_24g);
        }
        if (is_ltr()) {
            lv_liste_w_arrow_align_liste(liste_password_24g);
        } else {
            lv_label_set_text(lv_get_child_by_index(liste_password_24g, 3),
                    (empty ? "" : pw_label_24g_val));
            lv_liste_w_arrow_align(liste_password_24g);
        }
    } else if (band == WIFI_BAND_5G) {
        update_pw_label(WIFI_BAND_5G);
        if(empty){
            char pwd5g[strlen(get_string(ID_SSID_PWD_5G)) + 2];
            sprintf(pwd5g, "%s:", get_string(ID_SSID_PWD_5G));
            lv_label_set_text(lv_get_child_by_index(liste_password_5g, 1), pwd5g);
        }else{
            lv_label_set_text(lv_get_child_by_index(liste_password_5g, 1), pw_label_5g);
        }
        if (is_ltr()) {
            lv_liste_w_arrow_align_liste(liste_password_5g);
        } else {
            lv_label_set_text(lv_get_child_by_index(liste_password_5g, 3),
                    (empty ? "" : pw_label_5g_val));
            lv_liste_w_arrow_align(liste_password_5g);
        }
    }
}

//update ssid password label on UI
void set_ssid_pwd_lable(int band) {
    set_ssid_pwd_lable_impl(band, false);
}

//get ssid password str
char* get_ssid_pwd(int band){
    char* txt;
    ssid_pwd_visible = ds_get_bool(DS_KEY_SSID_PWD_VISIBLE);
    txt = get_wlan_password(band);
    //return a *** string instead of pwd
    if (!ssid_pwd_visible) {
        uint16_t i;
        uint16_t len = strlen(txt);
        memset(pwd_hide_str, 0, sizeof(pwd_hide_str));
        for(i = 0; i < len; i++) {
            pwd_hide_str[i] = '*';
        }
        return pwd_hide_str;
    }
    return txt;
}

void update_pw_label(int band) {
    char* txt = get_ssid_pwd(band);
    if (txt == NULL) {
        txt = "";
    }
    int len = strlen(txt);
    if (txt[len-1] == '\n') {
        txt[len-1] = '\0';
    }
    if(band == WIFI_BAND_24G){
#ifdef SUPPORT_2.4PLUS5GHZ
        int string_id = ID_SSID_PWD_24G;
#else
        int string_id = ID_PASSWORD;
#endif
        if (pw_label_24g != NULL) {
            lv_mem_free(pw_label_24g);
            pw_label_24g = NULL;
        }
        if (is_ltr()) {
            int len = strlen(get_string(string_id)) + strlen(": ") + strlen(txt) + 1;
            pw_label_24g = lv_mem_alloc(len);
            memset(pw_label_24g, 0, len);
            sprintf(pw_label_24g, "%s: %s", get_string(string_id), txt);
        } else {
            if (pw_label_24g_val != NULL) {
                lv_mem_free(pw_label_24g_val);
                pw_label_24g_val = NULL;
            }
            pw_label_24g = lv_mem_alloc(strlen(get_string(string_id)) + 1);
            memset(pw_label_24g, 0, strlen(get_string(string_id)) + 1);
            sprintf(pw_label_24g, "%s", get_string(string_id));
            pw_label_24g_val = lv_mem_alloc(strlen(txt) + 1);
            memset(pw_label_24g_val, 0, strlen(txt) + 1);
            sprintf(pw_label_24g_val, "%s", txt);
        }
    } else if(band == WIFI_BAND_5G){
        if (pw_label_5g != NULL) {
            lv_mem_free(pw_label_5g);
            pw_label_5g = NULL;
        }
        if (is_ltr()) {
            int len = strlen(get_string(ID_SSID_PWD_5G)) + strlen(": ") + strlen(txt) + 1;
            pw_label_5g = lv_mem_alloc(len);
            memset(pw_label_5g, 0, len);
            sprintf(pw_label_5g, "%s: %s", get_string(ID_SSID_PWD_5G), txt);
        } else {
            if (pw_label_5g_val != NULL) {
                lv_mem_free(pw_label_5g_val);
                pw_label_5g_val = NULL;
            }
            pw_label_5g = lv_mem_alloc(strlen(get_string(ID_SSID_PWD_5G)) + 1);
            memset(pw_label_5g, 0, strlen(get_string(ID_SSID_PWD_5G)) + 1);
            sprintf(pw_label_5g, "%s", get_string(ID_SSID_PWD_5G));
            pw_label_5g_val = lv_mem_alloc(strlen(txt) + 1);
            memset(pw_label_5g_val, 0, strlen(txt) + 1);
            sprintf(pw_label_5g_val, "%s", txt);
        }
    }
}

void update_security_label(int band) {
    char* txt = get_security_info(band);
    char* end = "";
    if(strcmp(txt, get_string(ID_WIFI_SECURITY_WPA2_PSK)) == 0){
        int len = strlen(get_string(ID_WIFI_SECURITY_ENCRYPTION_AES));
        end = lv_mem_alloc(len + 2);
        memset(end, 0, len + 2);
        sprintf(end, "%s", get_string(ID_WIFI_SECURITY_ENCRYPTION_AES));
    }

    if(band == WIFI_BAND_24G){
#ifdef SUPPORT_2.4PLUS5GHZ
        int string_id = ID_SSID_SECURITY_24G;
#else
        int string_id = ID_WIFI_SECURITY;
#endif
        if (security_label_24g != NULL) {
            lv_mem_free(security_label_24g);
            security_label_24g = NULL;
        }
        if (is_ltr()) {
            int len = strlen(get_string(string_id)) + strlen(": ") + strlen(txt) + 1 + strlen(end) + 1;
            security_label_24g = lv_mem_alloc(len);
            memset(security_label_24g, 0, len);
            sprintf(security_label_24g, "%s: %s%s", get_string(string_id), txt, end);
        } else {
            if (security_label_24g_val != NULL) {
                lv_mem_free(security_label_24g_val);
                security_label_24g_val = NULL;
            }
            security_label_24g = lv_mem_alloc(strlen(get_string(string_id)) + 1);
            memset(security_label_24g, 0, strlen(get_string(string_id)) + 1);
            sprintf(security_label_24g, "%s", get_string(string_id));
            security_label_24g_val = lv_mem_alloc(strlen(txt) + 1 + strlen(end) + 1);
            memset(security_label_24g_val, 0, strlen(txt) + 1 + strlen(end) + 1);
            sprintf(security_label_24g_val, "%s%s", txt, end);
        }
    } else if(band == WIFI_BAND_5G){
        if (security_label_5g != NULL) {
            lv_mem_free(security_label_5g);
            security_label_5g = NULL;
        }
        if (is_ltr()) {
            int len = strlen(get_string(ID_SSID_SECURITY_5G)) + strlen(": ") + strlen(txt) + 1 + strlen(end) + 1;
            security_label_5g = lv_mem_alloc(len);
            memset(security_label_5g, 0, len);
            sprintf(security_label_5g, "%s: %s%s", get_string(ID_SSID_SECURITY_5G), txt, end);
        } else {
            if (security_label_5g_val != NULL) {
                lv_mem_free(security_label_5g_val);
                security_label_5g_val = NULL;
            }
            security_label_5g = lv_mem_alloc(strlen(get_string(ID_SSID_SECURITY_5G)) + 1);
            memset(security_label_5g, 0, strlen(get_string(ID_SSID_SECURITY_5G)) + 1);
            sprintf(security_label_5g, "%s", get_string(ID_SSID_SECURITY_5G));
            security_label_5g_val = lv_mem_alloc(strlen(txt) + 1 + strlen(end) + 1);
            memset(security_label_5g_val, 0, strlen(txt) + 1 + strlen(end) + 1);
            sprintf(security_label_5g_val, "%s%s", txt, end);
        }
    }

    if (end != "") {
        lv_mem_free(end);
        end = NULL;
    }
}

void ssid_len_err_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    close_popup();
}

//prompt popup for invalid ssid name/passowrd
void ssid_len_err_popup(int id){
    static const char *btns[2];
    btns[1] = "";
    btns[0] = get_string(ID_OK);
    popup_anim_not_create(get_string(id), btns, ssid_len_err_action, NULL);
}

void ssid_keyboard_action(int id, const char* txt) {
    if (id == ID_SUB_SSID_24G) {
        if (check_input_is_all_space(txt) == false) {
            write_wlan_ssid(WIFI_BAND_24G, txt);
            update_ssid_label(WIFI_BAND_24G);
            lv_label_set_text(lv_get_child_by_index(liste_ssid_24g,1), ssid_label_24g);
            if (is_ltr()) {
                lv_liste_w_arrow_align_liste(liste_ssid_24g);
            } else {
                lv_label_set_text(lv_get_child_by_index(liste_ssid_24g,3), ssid_label_24g_val);
                lv_liste_w_arrow_align(liste_ssid_24g);
            }
            update_ssid_address();
            en_kb_cleanup();
            close_kb();
            if (strcmp(get_wlan_ssid(WIFI_BAND_24G), get_wlan_ssid(WIFI_BAND_5G)) == 0) {
                write_wlan_password(WIFI_BAND_24G, get_wlan_password(WIFI_BAND_5G));
                set_ssid_pwd_lable(WIFI_BAND_24G);
                ssid_len_err_popup(ID_SSID_SSID_PWD_SAME_PROMPT);
            }
        } else {
            ssid_len_err_popup(ID_SSID_ALL_SPACE_ERR);
        }
    } else if (id == ID_SUB_PASSWORD_24G) {
        if (check_input_contain_space(txt) == false) {
            if (strlen(txt) == 64) {
                if (check_input_fit_in_64_len(txt)
                    && write_wlan_password(WIFI_BAND_24G, txt)) {
                    //length 64 case
                    set_ssid_pwd_lable(WIFI_BAND_24G);
                    close_kb();
                    if (strcmp(get_wlan_ssid(WIFI_BAND_24G), get_wlan_ssid(WIFI_BAND_5G)) == 0) {
                        write_wlan_password(WIFI_BAND_5G, get_wlan_password(WIFI_BAND_24G));
                        set_ssid_pwd_lable(WIFI_BAND_5G);
                        ssid_len_err_popup(ID_SSID_SSID_PWD_SAME_PROMPT);
                    }
                } else {
                    ssid_len_err_popup(ID_SSID_PWD_HEX_ERR);
                }
            } else if (check_input_fit_len(txt, PASSWORD_LEN_8, PASSWORD_LEN_63)
                    && check_input_fit_ascii(txt)
                    && write_wlan_password(WIFI_BAND_24G, txt)) {
                //length 8~63 case
                set_ssid_pwd_lable(WIFI_BAND_24G);
                close_kb();
                if (strcmp(get_wlan_ssid(WIFI_BAND_24G), get_wlan_ssid(WIFI_BAND_5G)) == 0) {
                    write_wlan_password(WIFI_BAND_5G, get_wlan_password(WIFI_BAND_24G));
                    set_ssid_pwd_lable(WIFI_BAND_5G);
                    ssid_len_err_popup(ID_SSID_SSID_PWD_SAME_PROMPT);
                }
            } else {
                ssid_len_err_popup(ID_SSID_PWD_LEN_ERR);
            }
        } else {
            ssid_len_err_popup(ID_SSID_PWD_CONTAIN_SPACE_ERR);
        }
    }  else if (id == ID_SUB_SSID_5G) {
        if (check_input_is_all_space(txt) == false) {
            write_wlan_ssid(WIFI_BAND_5G, txt);
            update_ssid_label(WIFI_BAND_5G);
            lv_label_set_text(lv_get_child_by_index(liste_ssid_5g,1), ssid_label_5g);
            if (is_ltr()) {
                lv_liste_w_arrow_align_liste(liste_ssid_5g);
            } else {
                lv_label_set_text(lv_get_child_by_index(liste_ssid_5g,3), ssid_label_5g_val);
                lv_liste_w_arrow_align(liste_ssid_5g);
            }
            update_ssid_address();
            en_kb_cleanup();
            close_kb();
            if (strcmp(get_wlan_ssid(WIFI_BAND_5G), get_wlan_ssid(WIFI_BAND_24G)) == 0) {
                write_wlan_password(WIFI_BAND_5G, get_wlan_password(WIFI_BAND_24G));
                set_ssid_pwd_lable(WIFI_BAND_5G);
                ssid_len_err_popup(ID_SSID_SSID_PWD_SAME_PROMPT);
            }
        } else {
            ssid_len_err_popup(ID_SSID_ALL_SPACE_ERR);
        }
    } else if (id == ID_SUB_PASSWORD_5G) {
        if (check_input_contain_space(txt) == false) {
            if (strlen(txt) == 64) {
                if (check_input_fit_in_64_len(txt)
                    && write_wlan_password(WIFI_BAND_5G, txt)) {
                    //length 64 case
                    set_ssid_pwd_lable(WIFI_BAND_5G);
                    close_kb();
                    if (strcmp(get_wlan_ssid(WIFI_BAND_24G), get_wlan_ssid(WIFI_BAND_5G)) == 0) {
                        write_wlan_password(WIFI_BAND_24G, get_wlan_password(WIFI_BAND_5G));
                        set_ssid_pwd_lable(WIFI_BAND_24G);
                        ssid_len_err_popup(ID_SSID_SSID_PWD_SAME_PROMPT);
                    }
                } else {
                    ssid_len_err_popup(ID_SSID_PWD_HEX_ERR);
                }
            } else if (check_input_fit_len(txt, PASSWORD_LEN_8, PASSWORD_LEN_63)
                    && check_input_fit_ascii(txt)
                    && write_wlan_password(WIFI_BAND_5G, txt)) {
                //length 8~63 case
                set_ssid_pwd_lable(WIFI_BAND_5G);
                close_kb();
                if (strcmp(get_wlan_ssid(WIFI_BAND_24G), get_wlan_ssid(WIFI_BAND_5G)) == 0) {
                    write_wlan_password(WIFI_BAND_24G, get_wlan_password(WIFI_BAND_5G));
                    set_ssid_pwd_lable(WIFI_BAND_24G);
                    ssid_len_err_popup(ID_SSID_SSID_PWD_SAME_PROMPT);
                }
            } else {
                ssid_len_err_popup(ID_SSID_PWD_LEN_ERR);
            }
        } else {
            ssid_len_err_popup(ID_SSID_PWD_CONTAIN_SPACE_ERR);
        }
    }
}

void ssid_en_kb_create(int id, int tip_res_id, const void * lable, int max_len, bool pwd){
    en_kb_create(get_string(ID_MODIFY), id, ssid_keyboard_action);
    en_kb_set_tip(get_string(tip_res_id));
    en_kb_set_lable_length(max_len);
    if(pwd && !ssid_pwd_visible){
        //set pwd mode
        en_kb_set_pwd_lable(lable);
    } else{
        en_kb_set_lable(lable);
    }
}

void ssid_list_action(lv_obj_t * liste, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    //do nothing is liste state set to inactive
    if(lv_btn_get_state(liste) == LV_BTN_STATE_INA) return;

    int item_id = lv_obj_get_user_data(liste);
    if (item_id == ID_SUB_SSID_24G) {
        ssid_en_kb_create(ID_SUB_SSID_24G, ID_KB_SSID_TIP,
                get_wlan_ssid(WIFI_BAND_24G), MAX_SSID_LEN, false);

    } else if (item_id == ID_SUB_PASSWORD_24G) {
        ssid_en_kb_create(ID_SUB_PASSWORD_24G, ID_KB_PWD_TIP,
                get_wlan_password(WIFI_BAND_24G), MAX_PASSWORD_LEN, true);

    } else if (item_id == ID_SUB_SECURITY_24G) {
        ssid_security_create(WIFI_BAND_24G);

    } else if (item_id == ID_SUB_SSID_5G) {
        ssid_en_kb_create(ID_SUB_SSID_5G, ID_KB_SSID_TIP,
                get_wlan_ssid(WIFI_BAND_5G), MAX_SSID_LEN, false);

    } else if (item_id == ID_SUB_PASSWORD_5G) {
        ssid_en_kb_create(ID_SUB_PASSWORD_5G, ID_KB_PWD_TIP,
                get_wlan_password(WIFI_BAND_5G), MAX_PASSWORD_LEN, true);

    } else if (item_id == ID_SUB_SECURITY_5G) {
        ssid_security_create(WIFI_BAND_5G);
    } else if (item_id == ID_SUB_SSID_24G_QR_CODE) {
        ssid_qrcode_action(WIFI_BAND_24G);
    } else if (item_id == ID_SUB_SSID_5G_QR_CODE) {
        ssid_qrcode_action(WIFI_BAND_5G);
    }
}

void ssid_security_update() {
    int band = ssid_select_security_type;
    int type = (band == WIFI_BAND_24G) ? ssid_security_type_24g : ssid_security_type_5g;

    if (type == ID_SUB_SECURITY_NONE) {
        write_wlan_security(band, SECURITY_NONE);
    } else if (type == ID_SUB_SECURITY_WEP) {
        write_wlan_security(band, SECURITY_WEP);
    } else if (type == ID_SUB_SECURITY_WPA_PSK) {
        write_wlan_security(band, SECURITY_WPA_PSK);
    } else if (type == ID_SUB_SECURITY_WPA_WPA2_PSK) {
        write_wlan_security(band, SECURITY_WPA2_PSK);
    } else if (type == ID_SUB_SECURITY_WPA3_WPA2_MIXED_MODE) {
        write_wlan_security(band, SECURITY_WPA3_WPA2);
    }

    if(band == WIFI_BAND_24G){
        update_security_label(band);
        lv_label_set_text(lv_get_child_by_index(liste_security_24g,1), security_label_24g);
        if (is_ltr()) {
            lv_liste_w_arrow_align_liste(liste_security_24g);
        } else {
            lv_label_set_text(lv_get_child_by_index(liste_security_24g,3), security_label_24g_val);
            lv_liste_w_arrow_align(lv_get_child_by_index(liste_security_24g,3));
        }
    } else if(band == WIFI_BAND_5G){
        update_security_label(band);
        lv_label_set_text(lv_get_child_by_index(liste_security_5g,1), security_label_5g);
        if (is_ltr()) {
            lv_liste_w_arrow_align_liste(liste_security_5g);
        } else {
            lv_label_set_text(lv_get_child_by_index(liste_security_5g,3), security_label_5g_val);
            lv_liste_w_arrow_align(lv_get_child_by_index(liste_security_5g,3));
        }
    }

    set_ssid_pwd_lable(WIFI_BAND_24G);
#ifdef SUPPORT_2.4PLUS5GHZ
    set_ssid_pwd_lable(WIFI_BAND_5G);
#endif

    //update password liste state accordingly
    if (type == ID_SUB_SECURITY_NONE) {
        lv_liste_w_arrow_ina(((band == WIFI_BAND_24G) ?
                liste_password_24g : liste_password_5g), false);
        //empty password column if user set security to none
        set_ssid_pwd_lable_impl(band, true);
    } else{
        lv_liste_w_arrow_ina(((band == WIFI_BAND_24G) ?
                liste_password_24g : liste_password_5g), true);
    }
}

void ssid_popup_comfirm_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(mbox);
    log_d("ssid_popup_comfirm_action txt:%s",txt);

    if (strcmp(txt, get_string(ID_OK)) == 0) {
        ssid_security_update();
    }
    close_popup();
}

void ssid_security_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    int type;
    int band;

    if(ssid_select_security_type == WIFI_BAND_24G){
        type = ssid_security_type_24g;
        band = WIFI_BAND_24G;
    } else if(ssid_select_security_type == WIFI_BAND_5G){
        type = ssid_security_type_5g;
        band = WIFI_BAND_5G;
    }

    if (type != ID_SUB_SECURITY_WPA_WPA2_PSK && type != ID_SUB_SECURITY_WPA3_WPA2_MIXED_MODE) {
        static const char * btns[3];
        btns[0] = get_string(ID_CANCEL);
        btns[1] = get_string(ID_OK);
        btns[2] = "";

        char * prompt_header = get_string(ID_SSID_SECURITY_PROMPT_HEADER);
        char * prompt_contents;
        if (type == ID_SUB_SECURITY_NONE) {
            prompt_contents = get_string(ID_SSID_SECURITY_NONE_PROMPT);
        } else if (type == ID_SUB_SECURITY_WEP) {
            prompt_contents = get_string(ID_SSID_SECURITY_AUTO_PROMPT);
        } else if (type == ID_SUB_SECURITY_WPA_PSK) {
            prompt_contents = get_string(ID_SSID_SECURITY_WPAPSK_PROMPT);
        }

        set_security_type(type);
        popup_scrl_create_impl(prompt_header, prompt_contents, btns,
                   ssid_popup_comfirm_action, NULL);
    } else {
        set_security_type(type);
        ssid_security_update();
    }
}

//cleanup
void ssid_win_close_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    if (ssid_label_24g != NULL) {
        lv_mem_free(ssid_label_24g);
        ssid_label_24g = NULL;
    }
    if (pw_label_24g != NULL) {
        lv_mem_free(pw_label_24g);
        pw_label_24g = NULL;
    }
    if (security_label_24g != NULL) {
        lv_mem_free(security_label_24g);
        security_label_24g = NULL;
    }
    if (ssid_label_24g_val != NULL) {
        lv_mem_free(ssid_label_24g_val);
        ssid_label_24g_val = NULL;
    }
    if (pw_label_24g_val != NULL) {
        lv_mem_free(pw_label_24g_val);
        pw_label_24g_val = NULL;
    }
    if (security_label_24g_val != NULL) {
        lv_mem_free(security_label_24g_val);
        security_label_24g_val = NULL;
    }
    if (ssid_label_5g != NULL) {
        lv_mem_free(ssid_label_5g);
        ssid_label_5g = NULL;
    }
    if (pw_label_5g != NULL) {
        lv_mem_free(pw_label_5g);
        pw_label_5g = NULL;
    }
    if (security_label_5g != NULL) {
        lv_mem_free(security_label_5g);
        security_label_5g = NULL;
    }
    if (ssid_label_5g_val != NULL) {
        lv_mem_free(ssid_label_5g_val);
        ssid_label_5g_val = NULL;
    }
    if (pw_label_5g_val != NULL) {
        lv_mem_free(pw_label_5g_val);
        pw_label_5g_val = NULL;
    }
    if (security_label_5g_val != NULL) {
        lv_mem_free(security_label_5g_val);
        security_label_5g_val = NULL;
    }
}

//SSID password visible action
void ssid_pwd_visible_action(lv_obj_t * sw, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

#ifdef CUST_SWITCH
    lv_obj_t * img = lv_obj_get_child(liste_ssid_pwd_visible, NULL);
    if (ds_get_bool(DS_KEY_SSID_PWD_VISIBLE)){
        //disable
        lv_img_set_src(img, &ic_list_checkbox);
        ds_set_bool(DS_KEY_SSID_PWD_VISIBLE, false);
        ssid_pwd_visible = 0;
    } else {
        //enable
        lv_img_set_src(img, &ic_list_checkbox_selected);
        ds_set_bool(DS_KEY_SSID_PWD_VISIBLE, true);
        ssid_pwd_visible = 1;
    }
#else
    if (lv_sw_get_state(sw)) {
        ds_set_bool(DS_KEY_SSID_PWD_VISIBLE, true);
        ssid_pwd_visible = 1;
    } else {
        ds_set_bool(DS_KEY_SSID_PWD_VISIBLE, false);
        ssid_pwd_visible = 0;
    }
#endif
    //refresh password UI if security not set to none
    if (ssid_security_type_24g != ID_SUB_SECURITY_NONE) {
        set_ssid_pwd_lable(WIFI_BAND_24G);
    }
    if (ssid_security_type_5g != ID_SUB_SECURITY_NONE) {
        set_ssid_pwd_lable(WIFI_BAND_5G);
    }
}

void init_ssid(void) {
    liste_style_create();
    ssid_select_security_type = 0;

    lv_obj_t * win = default_list_header(lv_scr_act(), get_string(ID_LAUNCHER_SSID), ssid_win_close_action);
    lv_obj_t * ssid_list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(ssid_list, LV_SB_MODE_OFF);
    lv_list_set_style(ssid_list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(ssid_list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(ssid_list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(ssid_list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(ssid_list, LV_LAYOUT_OFF);

    bool demo_mode_enable = ds_get_bool(DS_KEY_DEMO_MODE);

    //SSID password visible
    ssid_pwd_visible = ds_get_bool(DS_KEY_SSID_PWD_VISIBLE);
#ifdef CUST_SWITCH
    liste_ssid_pwd_visible = lv_liste_cust_switch(ssid_list,
            get_string(ID_SSID_PWD_VISIBLE), ssid_pwd_visible_action, ssid_pwd_visible);
#else
    liste_ssid_pwd_visible = lv_liste_w_switch(ssid_list,
            get_string(ID_SSID_PWD_VISIBLE), ssid_pwd_visible_action);
    liste_pwd_visible_sw = lv_obj_get_child(liste_ssid_pwd_visible, NULL);
    if (ssid_pwd_visible) {
        lv_sw_on(liste_pwd_visible_sw, LV_ANIM_OFF);
    } else {
        lv_sw_off(liste_pwd_visible_sw, LV_ANIM_OFF);
    }
#endif

    bool wifi_24g_enabled = ds_get_bool(DS_KEY_WIFI_24G_ENABLED);
    bool wifi_5g_enabled = ds_get_bool(DS_KEY_WIFI_5G_ENABLED);
#if defined(CUST_ZYXEL)
    liste_24g_ssid_qrcode = lv_liste_w_arrow_w_item_id_ssid(ssid_list,
            get_string(ID_SSID_24G_QR_CODE), "", ssid_list_action,
            ID_SUB_SSID_24G_QR_CODE);
    if (!wifi_24g_enabled) {
        lv_liste_w_arrow_ina(liste_24g_ssid_qrcode, false);
    }
    liste_5g_ssid_qrcode = lv_liste_w_arrow_w_item_id_ssid(ssid_list,
            get_string(ID_SSID_5G_QR_CODE), "", ssid_list_action,
            ID_SUB_SSID_5G_QR_CODE);
    if (!wifi_5g_enabled) {
        lv_liste_w_arrow_ina(liste_5g_ssid_qrcode, false);
    }
#endif

    update_ssid_label(WIFI_BAND_24G);
    update_ssid_label(WIFI_BAND_5G);
    update_pw_label(WIFI_BAND_24G);
    update_pw_label(WIFI_BAND_5G);
    update_security_label(WIFI_BAND_24G);
    update_security_label(WIFI_BAND_5G);

    //2.4G
    liste_ssid_24g = lv_liste_w_arrow_w_item_id_ssid(ssid_list, ssid_label_24g, ssid_label_24g_val,
            demo_mode_enable ? NULL : ssid_list_action, ID_SUB_SSID_24G);
    liste_password_24g = lv_liste_w_arrow_w_item_id_ssid(ssid_list, pw_label_24g, pw_label_24g_val,
            demo_mode_enable ? NULL : ssid_list_action, ID_SUB_PASSWORD_24G);
    liste_security_24g = lv_liste_w_arrow_w_item_id_ssid(ssid_list, security_label_24g, security_label_24g_val,
            demo_mode_enable ? NULL : ssid_list_action, ID_SUB_SECURITY_24G);
#ifdef SUPPORT_2.4PLUS5GHZ
    //5G
    liste_ssid_5g = lv_liste_w_arrow_w_item_id_ssid(ssid_list, ssid_label_5g, ssid_label_5g_val,
            demo_mode_enable ? NULL : ssid_list_action, ID_SUB_SSID_5G);
    liste_password_5g = lv_liste_w_arrow_w_item_id_ssid(ssid_list, pw_label_5g, pw_label_5g_val,
            demo_mode_enable ? NULL : ssid_list_action, ID_SUB_PASSWORD_5G);
    liste_security_5g = lv_liste_w_arrow_w_item_id_ssid(ssid_list, security_label_5g, security_label_5g_val,
            demo_mode_enable ? NULL : ssid_list_action, ID_SUB_SECURITY_5G);
#endif
    if (!wifi_24g_enabled) {
        lv_liste_w_arrow_ina(liste_ssid_24g, false);
        lv_liste_w_arrow_ina(liste_password_24g, false);
        lv_liste_w_arrow_ina(liste_security_24g, false);
    }
    if (!wifi_5g_enabled) {
        lv_liste_w_arrow_ina(liste_ssid_5g, false);
        lv_liste_w_arrow_ina(liste_password_5g, false);
        lv_liste_w_arrow_ina(liste_security_5g, false);
    }
    //set password liste state to inactive if security set to none
    if(strcmp(get_security_info(WIFI_BAND_24G),
            get_string(ID_WIFI_SECURITY_NONE)) == 0){
        lv_liste_w_arrow_ina(liste_password_24g, false);
        //empty password column if user set security to none
        set_ssid_pwd_lable_impl(WIFI_BAND_24G, true);
    }
    if(strcmp(get_security_info(WIFI_BAND_5G),
            get_string(ID_WIFI_SECURITY_NONE)) == 0){
        lv_liste_w_arrow_ina(liste_password_5g, false);
        set_ssid_pwd_lable_impl(WIFI_BAND_5G, true);
    }

#ifdef HIGH_SPEED_WIFI_DOWN
    if (ds_get_bool(DS_KEY_HIGH_SPEED)) {
#if defined(CUST_ZYXEL)
        lv_liste_w_arrow_ina(liste_24g_ssid_qrcode, false);
        lv_liste_w_arrow_ina(liste_5g_ssid_qrcode, false);
#endif
        lv_liste_w_arrow_ina(liste_ssid_24g, false);
        lv_liste_w_arrow_ina(liste_password_24g, false);
        lv_liste_w_arrow_ina(liste_security_24g, false);
        lv_liste_w_arrow_ina(liste_ssid_5g, false);
        lv_liste_w_arrow_ina(liste_password_5g, false);
        lv_liste_w_arrow_ina(liste_security_5g, false);
    }
#endif

    //show page to page exit part
    page_anim_exit();
}

void set_security_type(int select){
    if(ssid_select_security_type == WIFI_BAND_24G){
        ssid_security_type_24g = select;
    } else if(ssid_select_security_type == WIFI_BAND_5G){
        ssid_security_type_5g = select;
    }
}

void ssid_security_btn_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    int i;
    int item_id = lv_obj_get_user_data(btn);

    for (i = 0; i < SSID_SECURITY_MAX_LISTE; i++){
        lv_img_set_src(ssid_security_liste_img[i], &btn_list_radio_n);

    }
    lv_img_set_src(ssid_security_liste_img[item_id], &btn_list_radio_p);
    set_security_type(item_id);
}

void ssid_security_create(int band) {
    int type;
    int i;
    get_security_info(band);

    ssid_select_security_type = 0;
    if(band == WIFI_BAND_24G){
        ssid_select_security_type = WIFI_BAND_24G;
        type = ssid_security_type_24g;
    } else if(band == WIFI_BAND_5G){
        ssid_select_security_type = WIFI_BAND_5G;
        type = ssid_security_type_5g;
    }
    liste_style_create();

    lv_obj_t * win = modify_list_header(lv_scr_act(), get_string(ID_SSID_SECURITY),
            ssid_security_action, lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    //add list element in order
    for (i = 0; i < SSID_SECURITY_MAX_LISTE; i++) {
        ssid_security_liste[i] = lv_liste_w_cbox(list, get_string(ssid_security_map[i]),
                ((type==i)?true:false),
                ssid_security_btn_action, i);
        ssid_security_liste_img[i] = lv_obj_get_child(ssid_security_liste[i], NULL);
    }

    lv_obj_set_hidden(ssid_security_liste[1], true); // hide WEP security
}

bool check_input_fit_len(const char * input, int min, int max) {
    bool res = false;
    int length = strlen(input);
    if (length >= min && length <= max) {
        res = true;
    }
    return res;
}

bool check_input_fit_ascii(const char * input) {
    bool ascii_range_fit = true;
    uint32_t length = strlen(input);
    uint32_t i = 0;
    uint32_t letter;
    while (i < length) {
        letter = lv_txt_encoded_next(input, &i);
        if (letter < MIN_PASSWORD_ASCII || letter > MAX_PASSWORD_ASCII) {
            ascii_range_fit = false;
            break;
        }
    }
    return ascii_range_fit;
}

bool check_input_contain_space(const char * input) {
    bool contain_space = false;
    uint32_t length = strlen(input);
    uint32_t i = 0;
    uint32_t letter;
    while (i < length) {
        letter = lv_txt_encoded_next(input, &i);
        if (letter == 0x20) {
            contain_space = true;
            break;
        }
    }
    return contain_space;
}

/**********************
 *  input length when 64 character case
 *  only contain
 *  0 to 9 (hex:0x30~0x39)
 *  or A to F (hex:0x41~0x46)
 *  or a to f (hex:0x61~0x66)
 **********************/
bool check_input_fit_in_64_len(const char * input) {
    bool input_fit_in_64_len = true;
    uint32_t length = strlen(input);
    uint32_t i = 0;
    uint32_t letter;
    while (i < length) {
        letter = lv_txt_encoded_next(input, &i);
        if (!((letter >= 0x30 && letter <= 0x39)
                || (letter >= 0x41 && letter <= 0x46)
                || (letter >= 0x61 && letter <= 0x66))) {
            input_fit_in_64_len = false;
            break;
        }
    }
    return input_fit_in_64_len;
}

bool check_input_is_all_space(const char * input) {
    bool input_all_space = true;
    uint32_t length = strlen(input);
    uint32_t i = 0;
    uint32_t letter;
    while (i < length) {
        letter = lv_txt_encoded_next(input, &i);
        if (letter != 0x20) {
            input_all_space = false;
            break;
        }
    }
    return input_all_space;
}

void ssid_qrcode_action(int band) {
    display_qr_code_create(band);
}
