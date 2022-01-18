#include <stdio.h>
#include <stdbool.h>
#include "display_qr_code.h"
#include "../util/info_page.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/wlan/hostapd_conf.h"
#include "lv_lib_qrcode/lv_qrcode.h"

char output_str[200];
char qrcode_info[300];
lv_obj_t * qrcode_24g;
lv_obj_t * qrcode_5g;
SSID_INFO ssid_24g_info;
char* qrcode_24g_data;
SSID_INFO ssid_5g_info;
char* qrcode_5g_data;

void ssid_qrcode_close_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    if (qrcode_24g != NULL) {
        lv_qrcode_delete(qrcode_24g);
        qrcode_24g = NULL;
    }
    if (qrcode_5g != NULL) {
        lv_qrcode_delete(qrcode_5g);
        qrcode_5g = NULL;
    }
    if (qrcode_24g_data != NULL) {
        lv_mem_free(qrcode_24g_data);
        qrcode_24g_data = NULL;
    }
    if (qrcode_5g_data != NULL) {
        lv_mem_free(qrcode_5g_data);
        qrcode_5g_data = NULL;
    }
}

//for connection guide QR code showing page
void display_qr_code_create(int band)
{
    log_d("display_qr_code_create band:%d",band);
    static lv_style_t title_style;
    lv_style_copy(&title_style, &lv_style_plain);
    title_style.text.font = get_font(font_w_bold, font_h_18);
    title_style.text.color = LV_COLOR_GREYISH_BROWN;
    title_style.text.letter_space = 1;
    liste_style_create();
    const char* title;
    if (band == WIFI_BAND_24G) {
        title = get_string(ID_SSID_24G_QR_CODE);
    } else if (band == WIFI_BAND_5G) {
        title = get_string(ID_SSID_5G_QR_CODE);
    } else {
        title = get_string(ID_CONN_GUIDE_QR_CODE);
    }

    lv_obj_t * win = default_list_header(lv_scr_act(), title,
            ssid_qrcode_close_action);

    bool wifi_24g_enabled = ds_get_bool(DS_KEY_WIFI_24G_ENABLED);
    bool wifi_5g_enabled = ds_get_bool(DS_KEY_WIFI_5G_ENABLED);
    log_d("display_qr_code_create wifi_24g_enabled:%d wifi_5g_enabled:%d",
            wifi_24g_enabled, wifi_5g_enabled);

    int qrcode_size;
    if (band == WIFI_BAND_24G || band == WIFI_BAND_5G) {
        //zyxel ssid item qrcode
        qrcode_size = 120;
    } else {
        //Connection guide qrcode
        if (wifi_24g_enabled == true && wifi_5g_enabled == true) {
            qrcode_size = 100;
        } else {
            qrcode_size = 120;
        }
    }

    if (wifi_24g_enabled == true) {
        memset(&ssid_24g_info, '\0', sizeof(SSID_INFO));
        ssid_24g_info.wlan_ssid = get_wlan_ssid(WIFI_BAND_24G);
        ssid_24g_info.wlan_security = get_wlan_security(WIFI_BAND_24G);
        ssid_24g_info.wlan_password = get_wlan_password(WIFI_BAND_24G);
        ssid_24g_info.wlan_hide_ssid = get_wlan_hide_ssid(WIFI_BAND_24G);

        char* qrcode_24g_info = qr_code_info(ssid_24g_info);
        qrcode_24g_data = lv_mem_alloc(strlen(qrcode_24g_info) + 1);
        memset(qrcode_24g_data, 0, strlen(qrcode_24g_info) + 1);
        sprintf(qrcode_24g_data, "%s", qrcode_24g_info);
    }

    if (wifi_5g_enabled == true) {
        memset(&ssid_5g_info, '\0', sizeof(SSID_INFO));
        ssid_5g_info.wlan_ssid = get_wlan_ssid(WIFI_BAND_5G);
        ssid_5g_info.wlan_security = get_wlan_security(WIFI_BAND_5G);
        ssid_5g_info.wlan_password = get_wlan_password(WIFI_BAND_5G);
        ssid_5g_info.wlan_hide_ssid = get_wlan_hide_ssid(WIFI_BAND_5G);

        char* qrcode_5g_info = qr_code_info(ssid_5g_info);
        qrcode_5g_data = lv_mem_alloc(strlen(qrcode_5g_info) + 1);
        memset(qrcode_5g_data, 0, strlen(qrcode_5g_info) + 1);
        sprintf(qrcode_5g_data, "%s", qrcode_5g_info);
    }
    if (band == WIFI_BAND_24G) {
        //zyxel ssid item 2.4g qrcode
        qrcode_24g = lv_qrcode_create(win, qrcode_size, LV_COLOR_BLACK,
                LV_COLOR_WHITE);
        lv_qrcode_update(qrcode_24g, qrcode_24g_data, strlen(qrcode_24g_data));
        lv_obj_align(qrcode_24g, NULL, LV_ALIGN_CENTER, 0, 0);
    }
    else if (band == WIFI_BAND_5G) {
        //zyxel ssid item 5g qrcode
        qrcode_5g = lv_qrcode_create(win, qrcode_size, LV_COLOR_BLACK,
                LV_COLOR_WHITE);
        lv_qrcode_update(qrcode_5g, qrcode_5g_data, strlen(qrcode_5g_data));
        lv_obj_align(qrcode_5g, NULL, LV_ALIGN_CENTER, 0, 0);
    }
    else {
        //connection guide qrcode
        if (wifi_24g_enabled == true && wifi_5g_enabled == false) {
            //2.4g qrcode
            qrcode_24g = lv_qrcode_create(win, qrcode_size, LV_COLOR_BLACK,
                    LV_COLOR_WHITE);
            lv_qrcode_update(qrcode_24g, qrcode_24g_data, strlen(qrcode_24g_data));
            lv_obj_align(qrcode_24g, NULL, LV_ALIGN_CENTER, 0, 0);

            //2.4g ssid title
            lv_obj_t *  ssid_24g_label =  lv_label_create(win, NULL);
            lv_label_set_text(ssid_24g_label, get_string(ID_SSID_24G));
            lv_obj_set_style(ssid_24g_label, &title_style);
            lv_obj_align(ssid_24g_label, qrcode_24g, LV_ALIGN_OUT_TOP_MID, 0, -10);
        }
        else if (wifi_24g_enabled == false && wifi_5g_enabled == true) {
            //5g qrcode
            qrcode_5g = lv_qrcode_create(win, qrcode_size, LV_COLOR_BLACK,
                    LV_COLOR_WHITE);
            lv_qrcode_update(qrcode_5g, qrcode_5g_data, strlen(qrcode_5g_data));
            lv_obj_align(qrcode_5g, NULL, LV_ALIGN_CENTER, 0, 0);

            //5g ssid title
            lv_obj_t *  ssid_5g_label =  lv_label_create(win, NULL);
            lv_label_set_text(ssid_5g_label, get_string(ID_SSID_5G));
            lv_obj_set_style(ssid_5g_label, &title_style);
            lv_obj_align(ssid_5g_label, qrcode_5g, LV_ALIGN_OUT_TOP_MID, 0, -10);
        }
        else if (wifi_24g_enabled == true && wifi_5g_enabled == true) {
            //2.4g rcode
            qrcode_24g = lv_qrcode_create(win, qrcode_size, LV_COLOR_BLACK,
                    LV_COLOR_WHITE);
            lv_qrcode_update(qrcode_24g, qrcode_24g_data, strlen(qrcode_24g_data));
            lv_obj_align(qrcode_24g, NULL, LV_ALIGN_IN_LEFT_MID, 40, 0);

            //5g qrcode
            qrcode_5g = lv_qrcode_create(win, qrcode_size, LV_COLOR_BLACK,
                    LV_COLOR_WHITE);
            lv_qrcode_update(qrcode_5g, qrcode_5g_data, strlen(qrcode_5g_data));
            lv_obj_align(qrcode_5g, NULL, LV_ALIGN_IN_RIGHT_MID, -40, 0);

            //2.4g ssid title
            lv_obj_t *  ssid_24g_label =  lv_label_create(win, NULL);
            lv_label_set_text(ssid_24g_label, get_string(ID_SSID_24G));
            lv_obj_set_style(ssid_24g_label, &title_style);
            lv_obj_align(ssid_24g_label, qrcode_24g, LV_ALIGN_OUT_TOP_MID, 0, -10);

            //5g ssid title
            lv_obj_t *  ssid_5g_label =  lv_label_create(win, NULL);
            lv_label_set_text(ssid_5g_label, get_string(ID_SSID_5G));
            lv_obj_set_style(ssid_5g_label, &title_style);
            lv_obj_align(ssid_5g_label, qrcode_5g, LV_ALIGN_OUT_TOP_MID, 0, -10);
        }
    }
}

//Special characters '\' ';' ',' and ':' should be escaped
//with a backslash '\'
char* special_character_check(char* input_str)
{
    memset(output_str, '\0', sizeof(output_str));
    int i = 0;
    for (i = 0; i < strlen(input_str); i++) {
        if (!strncmp((input_str + i), "\\", strlen("\\"))) {
            strcat(output_str, "\\\\");
        } else if (!strncmp((input_str + i), ";", strlen(";"))) {
            strcat(output_str, "\\;");
        } else if (!strncmp((input_str + i), ",", strlen(","))) {
            strcat(output_str, "\\,");
        } else if (!strncmp((input_str + i), ":", strlen(":"))) {
            strcat(output_str, "\\:");
        } else {
            strncat(output_str, input_str + i, 1);
        }
    }
    //log_d("special_character_check output_str:%s", output_str);
    return output_str;
}

//qr code ssid format info
//if ssid not hidden
//WIFI:S:<SSID>;T:<wpa|wep|>;P:<password>;;
//if ssid hidden
//WIFI:S:<SSID>;T:<wpa|wep|>;P:<password>;H:true;
char* qr_code_info(SSID_INFO ssid_info)
{
    memset(qrcode_info, '\0', sizeof(qrcode_info));
    strcpy(qrcode_info, "WIFI:S:");
    strcat(qrcode_info, special_character_check(ssid_info.wlan_ssid));
    strcat(qrcode_info, ";");
    if (strcmp(ssid_info.wlan_security, "None") == 0) {
        strcat(qrcode_info, "T:nopass;");
        strcat(qrcode_info, "P:;");
    } else {
        strcat(qrcode_info, "T:WPA;");
        strcat(qrcode_info, "P:");
        strcat(qrcode_info, special_character_check(ssid_info.wlan_password));
        strcat(qrcode_info, ";");
    }
    if (ssid_info.wlan_hide_ssid == true) {
        strcat(qrcode_info, "H:true;");
    } else {
        strcat(qrcode_info, ";");
    }
    return qrcode_info;
}
