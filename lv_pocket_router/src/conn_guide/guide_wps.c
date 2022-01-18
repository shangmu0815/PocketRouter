#include <stdio.h>

#include "guide_wps.h"
#include "wps.h"
#include "manual_conn.h"
#include "display_qr_code.h"
#include "../util/info_page.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/wlan/hostapd_conf.h"
void wps_disabled_prompt_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    close_popup();
}

//conn guide wps page btnm action
void guide_wps_action(lv_obj_t * btnm, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(btnm);
    if(strcmp(txt, get_string(ID_CONN_GUIDE_MANUAL)) == 0){
	//Manual btn action
        manual_conn_create();

    } else if(strcmp(txt, get_string(ID_WPS)) == 0){
        if (!get_wlan_wps_state()) {
            static const char *btns[2];
            btns[1] = "";
            btns[0] = get_string(ID_OK);
            popup_anim_not_create(get_string(ID_CONN_GUIDE_WPS_DISABLED_PROMPT), btns,
                                  wps_disabled_prompt_action, NULL);
        } else {
            //WPS btn action
            wps_connect_create();
        }
    } else {
        display_qr_code_create(WIFI_BAND_ALL);
    }
}

void guide_wps_create(void){
    static const char *btns[4];
    btns[0] = get_string(ID_CONN_GUIDE_MANUAL);
    btns[1] = get_string(ID_WPS);
#if defined(CUST_ZYXEL)
    btns[2] = "";
#else
    btns[2] = get_string(ID_CONN_GUIDE_QR_CODE);
    btns[3] = "";
#endif

    info_page_create_btmn(lv_scr_act(), get_string(ID_CONN_GUIDE), get_string(ID_CONN_GUIDE_OPTIONS), btns, guide_wps_action);
}

