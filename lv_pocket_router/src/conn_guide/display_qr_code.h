#ifndef LV_POCKET_ROUTER_SRC_CONN_GUIDE_DISPLAY_QR_CODE_H_
#define LV_POCKET_ROUTER_SRC_CONN_GUIDE_DISPLAY_QR_CODE_H_

#include "../../../lvgl/lvgl.h"
typedef struct {
    char* wlan_ssid;
    char* wlan_password;
    char* wlan_security;
    bool wlan_hide_ssid;
} SSID_INFO;

void ssid_qrcode_close_action(lv_obj_t * btn, lv_event_t event);
void display_qr_code_create(int band);
char* special_character_check(char* input_str);
char* qr_code_info(SSID_INFO ssid_info);

#endif /* LV_POCKET_ROUTER_SRC_CONN_GUIDE_DISPLAY_QR_CODE_H_ */
