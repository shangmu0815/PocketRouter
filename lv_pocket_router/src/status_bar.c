#include "status_bar.h"
#include "status_bar_view.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lv_pocket_router/src/battery/battery_info.h"
#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/launcher.h"
#include "lv_pocket_router/src/sms/sms_store.h"
#include "lv_pocket_router/src/wlan/hostapd_conf.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"

#if defined(HIGH_RESOLUTION)
#else
#endif

void update_ssid() {
    if (statusbar_second_right[INDEX_SSID].slot.con != NULL) {
        lv_label_set_text(statusbar_second_right[INDEX_SSID].slot.con, get_active_wlan_ssid());
        update_ssid_address();
    }
}

void update_sim_signal() {
    StatusBarInfo bar = statusbar_second_left[INDEX_CARRIER];

    if (statusbar_left[INDEX_SIM_ABSENT].slot.con == NULL ||
            statusbar_left[INDEX_SIGNAL_STRENGTH].slot.con == NULL ||
            statusbar_left[INDEX_SIGNAL_STRENGTH].image.img == NULL) {
        return ;
    }

    int state = get_sim_state();
    bool org_hidden = statusbar_left[INDEX_SIM_ABSENT].hidden;
    if (state == SIM_ABSENT || state == CARD_IO_ERROR) {
        statusbar_left[INDEX_SIM_ABSENT].hidden = false;
        statusbar_left[INDEX_SIGNAL_STRENGTH].hidden = true;
        if (bar.slot.con != NULL) {
            lv_label_set_text(bar.slot.con, "");
        }
    } else {
        statusbar_left[INDEX_SIM_ABSENT].hidden = true;
        statusbar_left[INDEX_SIGNAL_STRENGTH].hidden = false;

        //For cc_regdb, get mcc to wlan_country
        check_current_country();

        char operator[OPERATOR_NAME_MAX_LENGTH+1];
        memset(operator, 0, OPERATOR_NAME_MAX_LENGTH+1);
        get_operator_name(operator, OPERATOR_NAME_MAX_LENGTH);
        if (bar.slot.con != NULL && strlen(operator) > 0) {
            lv_label_set_text(bar.slot.con, operator);
        }

        ril_nw_signal_rat_t src = RIL_NW_SIG_UNKNOWN;
        int strength = get_strength_state(&src);
        switch (state) {
            case SIM_READY:
                if (get_roaming_status() == RIL_NW_ROAMING_STATE_ROAMING_ON && !is_wwan_connected()) {
                    lv_img_set_src(statusbar_left[INDEX_SIGNAL_STRENGTH].image.img, &ic_status_signal_no);
                } else if (strength == 0) {
#if defined(HIGH_RESOLUTION)
                    lv_img_set_src(statusbar_left[INDEX_SIGNAL_STRENGTH].image.img, &ic_status_signal_no_hd);
#else
                    lv_img_set_src(statusbar_left[INDEX_SIGNAL_STRENGTH].image.img, &ic_status_signal_no);
#endif
                } else if (strength == 1) {
#if defined(HIGH_RESOLUTION)
                    lv_img_set_src(statusbar_left[INDEX_SIGNAL_STRENGTH].image.img,
                        &ic_status_signal_level01_hd);
#else
                    lv_img_set_src(statusbar_left[INDEX_SIGNAL_STRENGTH].image.img,
                        &ic_status_signal_level01);
#endif
                } else if (strength == 2) {
#if defined(HIGH_RESOLUTION)
                    lv_img_set_src(statusbar_left[INDEX_SIGNAL_STRENGTH].image.img,
                        &ic_status_signal_level02_hd);
#else
                    lv_img_set_src(statusbar_left[INDEX_SIGNAL_STRENGTH].image.img,
                        &ic_status_signal_level02);
#endif
                } else if (strength == 3) {
#if defined(HIGH_RESOLUTION)
                    lv_img_set_src(statusbar_left[INDEX_SIGNAL_STRENGTH].image.img,
                        &ic_status_signal_level03_hd);
#else
                    lv_img_set_src(statusbar_left[INDEX_SIGNAL_STRENGTH].image.img,
                        &ic_status_signal_level03);
#endif
                } else if (strength == 4) {
#if defined(HIGH_RESOLUTION)
                    lv_img_set_src(statusbar_left[INDEX_SIGNAL_STRENGTH].image.img,
                        &ic_status_signal_level04_hd);
#else
                    lv_img_set_src(statusbar_left[INDEX_SIGNAL_STRENGTH].image.img,
                        &ic_status_signal_level04);
#endif
                } else if (strength >= 5) {
                    lv_img_set_src(statusbar_left[INDEX_SIGNAL_STRENGTH].image.img,
                        &ic_status_signal_level05);
                }
                break;
            case SIM_PIN_REQUEST:
            case SIM_PUK_REQUEST:
            case SIM_NETWORK_LOCKED:
            default:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_SIGNAL_STRENGTH].image.img, &ic_status_signal_no_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_SIGNAL_STRENGTH].image.img, &ic_status_signal_no);
#endif
                break;
        }
    }

    //re-align operator name
    if (bar.slot.con != NULL) {
        if(is_ltr()){
            lv_label_set_align(bar.slot.con, LV_LABEL_ALIGN_LEFT);
            lv_obj_align(bar.slot.con, NULL,
                    LV_ALIGN_IN_LEFT_MID, 8, bar.slot.shift.y);
        }else{
            lv_label_set_align(bar.slot.con, LV_LABEL_ALIGN_RIGHT);
            lv_obj_align(bar.slot.con, NULL,
                    LV_ALIGN_IN_RIGHT_MID, -8, bar.slot.shift.y);
        }
    }
    if (org_hidden != statusbar_left[INDEX_SIM_ABSENT].hidden) {
        refresh_status_bar_list(STATUSBAR_LEFT);
    }
}

void update_data_flow() {
    if (statusbar_left[INDEX_DATA_FLOW].slot.con == NULL ||
        statusbar_left[INDEX_DATA_FLOW].image.img == NULL) {
        return ;
    }

    bool downloading= is_data_downloading();
    bool uploading = is_data_upgrading();

    bool org_hidden = statusbar_left[INDEX_DATA_FLOW].hidden;
    if (downloading || uploading) {
        statusbar_left[INDEX_DATA_FLOW].hidden = false;
    } else {
        statusbar_left[INDEX_DATA_FLOW].hidden = true;
    }
    
    if (downloading && uploading) {
#if defined(HIGH_RESOLUTION)
        lv_img_set_src(statusbar_left[INDEX_DATA_FLOW].image.img, &ic_status_data_ud_hd);
#else
        lv_img_set_src(statusbar_left[INDEX_DATA_FLOW].image.img, &ic_status_data_ud);
#endif
    } else if (uploading) {
#if defined(HIGH_RESOLUTION)
        lv_img_set_src(statusbar_left[INDEX_DATA_FLOW].image.img, &ic_status_data_up_hd);
#else
        lv_img_set_src(statusbar_left[INDEX_DATA_FLOW].image.img, &ic_status_data_up);
#endif
    } else if (downloading) {
#if defined(HIGH_RESOLUTION)
        lv_img_set_src(statusbar_left[INDEX_DATA_FLOW].image.img, &ic_status_data_down_hd);
#else
        lv_img_set_src(statusbar_left[INDEX_DATA_FLOW].image.img, &ic_status_data_down);
#endif
    }

    if (org_hidden != statusbar_left[INDEX_DATA_FLOW].hidden) {
        refresh_status_bar_list(STATUSBAR_LEFT);
    }
}

void update_radio_tech() {
    if (statusbar_left[INDEX_RADIO_TECH].slot.con == NULL ||
        statusbar_left[INDEX_RADIO_TECH].image.img == NULL) {
        return ;
    }

    ril_nw_signal_rat_t src = RIL_NW_SIG_UNKNOWN;
    int radioTech = get_radio_tech();
    bool org_hidden = statusbar_left[INDEX_RADIO_TECH].hidden;

    if (radioTech == DATA_BEARER_TECH_TYPE_UNKNOWN) {
        statusbar_left[INDEX_RADIO_TECH].hidden = true;
    } else {
        statusbar_left[INDEX_RADIO_TECH].hidden = false;
        switch (radioTech) {
            case DATA_BEARER_TECH_TYPE_5G:
                get_strength_state(&src);
                if (src == RIL_NW_SIG_NR5G) {
                    lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_5g_c);
                } else {
                    //log_e("5G radio tech but signal rat: %d", src);
#if defined(HIGH_RESOLUTION)
                    lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_5g_hd);
#else
                    lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_5g);
#endif
                }
                break;
            case DATA_BEARER_TECH_TYPE_LTE:
                if (is_nr5g_icon_supported()) {
                    lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_5g);
                    //log_e("LTE radio tech but is nr5g icon supported");
                } else {
                    if (get_lte_ca_activated()) {
                        lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_4gp);
                    } else {
#if defined(HIGH_RESOLUTION)
                        lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_4g_hd);
#else
                        lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_4g);
#endif
                    }
                }
                break;
            case DATA_BEARER_TECH_TYPE_HSDPA_PLUS:
            case DATA_BEARER_TECH_TYPE_DC_HSDPA_PLUS:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_hp_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_hp);
#endif
                break;
            case DATA_BEARER_TECH_TYPE_HSPA:
            case DATA_BEARER_TECH_TYPE_HSDPA:
            case DATA_BEARER_TECH_TYPE_HSUPA:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_h_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_h);
#endif
                break;
            case DATA_BEARER_TECH_TYPE_WCDMA:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_3g_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_3g);
#endif
                break;
            case DATA_BEARER_TECH_TYPE_EDGE:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_e_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_e);
#endif
                break;
            case DATA_BEARER_TECH_TYPE_GPRS:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_g_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_g);
#endif
                break;
            // TODO roaming
            /*case xxx:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_r_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_r);
#endif
                break;*/
            default:
                statusbar_left[INDEX_RADIO_TECH].hidden = true;
                break;
        }
        //update to roaming icon if get_roaming_status() == RIL_NW_ROAMING_STATE_ROAMING_ON
        if (get_roaming_status() == RIL_NW_ROAMING_STATE_ROAMING_ON) {
#if defined(HIGH_RESOLUTION)
            lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_r_hd);
#else
            lv_img_set_src(statusbar_left[INDEX_RADIO_TECH].image.img, &ic_status_signal_r);
#endif
        }
    }

    if (org_hidden != statusbar_left[INDEX_RADIO_TECH].hidden) {
        refresh_status_bar_list(STATUSBAR_LEFT);
    }
}

#ifdef BT_SUPPORT
void update_bluetooth_user_counter() {
    if (statusbar_left[INDEX_USER_BLUETTOTH].slot.con == NULL ||
        statusbar_left[INDEX_USER_BLUETTOTH].image.img == NULL) {
        return ;
    }

    int num = 0;
    bool org_hidden = statusbar_left[INDEX_USER_BLUETTOTH].hidden;
    if (num == 0) {
        statusbar_left[INDEX_USER_BLUETTOTH].hidden = true;
    } else {
        statusbar_left[INDEX_USER_BLUETTOTH].hidden = false;
        switch (num) {
            case 1:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level01_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level01);
#endif
                break;
            case 2:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level02_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level02);
#endif
                break;
            case 3:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level03_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level03);
#endif
                break;
            case 4:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level04_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level04);
#endif
                break;
            case 5:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_y5device_level05_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_y5device_level05);
#endif
                break;
            case 6:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level06_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level06);
#endif
                break;
            case 7:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level07_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level07);
#endif
                break;
            case 8:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level08_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level08);
#endif
                break;
            case 9:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level09_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level09);
#endif
                break;
            default:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level10_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_BLUETTOTH].image.img, &ic_status_btdevice_level10);
#endif
                break;
        }
    }

    if (org_hidden != statusbar_left[INDEX_USER_BLUETTOTH].hidden) {
        refresh_status_bar_list(STATUSBAR_LEFT);
    }
}
#endif

void update_hotspot_user_counter() {
    if (statusbar_left[INDEX_USER_HOTSPOT].slot.con == NULL ||
        statusbar_left[INDEX_USER_HOTSPOT].image.img == NULL) {
        return ;
    }

    int num = get_connected_number();
    bool org_hidden = statusbar_left[INDEX_USER_HOTSPOT].hidden;
    if (num == 0) {
        statusbar_left[INDEX_USER_HOTSPOT].hidden = true;
    } else {
        statusbar_left[INDEX_USER_HOTSPOT].hidden = false;
        switch (num) {
            case 1:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level01_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level01);
#endif
                break;
            case 2:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level02_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level02);
#endif
                break;
            case 3:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level03_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level03);
#endif
                break;
            case 4:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level04_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level04);
#endif
                break;
            case 5:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level05_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level05);
#endif
                break;
            case 6:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level06_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level06);
#endif
                break;
            case 7:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level07_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level07);
#endif
                break;
            case 8:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level08_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level08);
#endif
                break;
            case 9:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level09_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level09);
#endif
                break;
            case 10:
#if defined(HIGH_RESOLUTION)
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level10_hd);
#else
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level10);
#endif
                break;
            case 11:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level11);
                break;
            case 12:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level12);
                break;
            case 13:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level13);
                break;
            case 14:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level14);
                break;
            case 15:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level15);
                break;
#ifdef CUST_DLINK
            case 16:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level16);
                break;
            case 17:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level17);
                break;
            case 18:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level18);
                break;
            case 19:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level19);
                break;
            case 20:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level20);
                break;
            case 21:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level21);
                break;
            case 22:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level22);
                break;
            case 23:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level23);
                break;
            case 24:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level24);
                break;
            case 25:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level25);
                break;
            case 26:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level26);
                break;
            case 27:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level27);
                break;
            case 28:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level28);
                break;
            case 29:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level29);
                break;
            case 30:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level30);
                break;
            case 31:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level31);
                break;
            default:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level32);
                break;
#else
            default:
                lv_img_set_src(statusbar_left[INDEX_USER_HOTSPOT].image.img, &ic_status_y5device_level16);
                break;
#endif
        }
    }

    if (org_hidden != statusbar_left[INDEX_USER_HOTSPOT].hidden) {
        refresh_status_bar_list(STATUSBAR_LEFT);
    }
}

void update_unread_message() {
    if (statusbar_left[INDEX_UNREAD_MESSAGE].slot.con == NULL ||
        statusbar_left[INDEX_UNREAD_MESSAGE].image.img == NULL) {
        return ;
    }

    bool org_hidden = statusbar_left[INDEX_UNREAD_MESSAGE].hidden;
    if (get_sms_unread() > 0) {
        statusbar_left[INDEX_UNREAD_MESSAGE].hidden = false;
#if defined(HIGH_RESOLUTION)
        lv_img_set_src(statusbar_left[INDEX_UNREAD_MESSAGE].image.img, &ic_status_message_hd);
#else
        lv_img_set_src(statusbar_left[INDEX_UNREAD_MESSAGE].image.img, &ic_status_message);
#endif
    } else {
        statusbar_left[INDEX_UNREAD_MESSAGE].hidden = true;
    }

    if (org_hidden != statusbar_left[INDEX_UNREAD_MESSAGE].hidden) {
        refresh_status_bar_list(STATUSBAR_LEFT);
    }
}

void update_wifi_band() {
    if (statusbar_left[INDEX_WIFI_BAND].slot.con == NULL ||
        statusbar_left[INDEX_WIFI_BAND].image.img == NULL) {
        return ;
    }
#ifdef SUPPORT_2.4PLUS5GHZ
    bool org_hidden = statusbar_left[INDEX_WIFI_BAND].hidden;
    bool org_hidden_5g = statusbar_left[INDEX_WIFI_BAND_5G].hidden;
    int band = get_wifi_band();
    if (band == WIFI_BAND_ALL) {
        statusbar_left[INDEX_WIFI_BAND].hidden = false;
#if defined(HIGH_RESOLUTION)
        lv_img_set_src(statusbar_left[INDEX_WIFI_BAND].image.img, &ic_status_wifi_s3_hd);
#else
        lv_img_set_src(statusbar_left[INDEX_WIFI_BAND].image.img, &ic_status_wifi_s3);
#endif
#ifdef CUST_DLINK // Show 2.4 & 5GHz icon separately for DLink
        statusbar_left[INDEX_WIFI_BAND_5G].hidden = false;
        lv_img_set_src(statusbar_left[INDEX_WIFI_BAND].image.img, &ic_status_wifi_s2);
        if (statusbar_left[INDEX_WIFI_BAND_5G].slot.con != NULL) {
            lv_img_set_src(statusbar_left[INDEX_WIFI_BAND_5G].image.img, &ic_status_wifi_s1);
        }
#endif
    } else if (band == WIFI_BAND_5G) {
        statusbar_left[INDEX_WIFI_BAND].hidden = false;
#if defined(HIGH_RESOLUTION)
        lv_img_set_src(statusbar_left[INDEX_WIFI_BAND].image.img, &ic_status_wifi_s1_hd);
#else
        lv_img_set_src(statusbar_left[INDEX_WIFI_BAND].image.img, &ic_status_wifi_s1);
#endif
#ifdef CUST_DLINK // Show 2.4 & 5GHz icon separately for DLink
        statusbar_left[INDEX_WIFI_BAND_5G].hidden = true;
#endif
    } else if (band == WIFI_BAND_24G) {
        statusbar_left[INDEX_WIFI_BAND].hidden = false;
#if defined(HIGH_RESOLUTION)
        lv_img_set_src(statusbar_left[INDEX_WIFI_BAND].image.img, &ic_status_wifi_s2_hd);
#else
        lv_img_set_src(statusbar_left[INDEX_WIFI_BAND].image.img, &ic_status_wifi_s2);
#endif
#ifdef CUST_DLINK // Show 2.4 & 5GHz icon separately for DLink
        statusbar_left[INDEX_WIFI_BAND_5G].hidden = true;
#endif
    } else {
        statusbar_left[INDEX_WIFI_BAND].hidden = true;
#ifdef CUST_DLINK // Show 2.4 & 5GHz icon separately for DLink
        statusbar_left[INDEX_WIFI_BAND_5G].hidden = true;
#endif
    }

    if (org_hidden != statusbar_left[INDEX_WIFI_BAND].hidden ||
            org_hidden_5g != statusbar_left[INDEX_WIFI_BAND_5G].hidden) {
        refresh_status_bar_list(STATUSBAR_LEFT);
    }
#else
    statusbar_left[INDEX_WIFI_BAND].hidden = true;
#endif
}

void update_sw_update() {
#ifdef CUST_DLINK
    if (statusbar_left[INDEX_SW_UPDATE].slot.con == NULL ||
        statusbar_left[INDEX_SW_UPDATE].image.img == NULL) {
        return ;
    }

    bool org_hidden = statusbar_left[INDEX_SW_UPDATE].hidden;
    if (get_available_updates_state()) {
        statusbar_left[INDEX_SW_UPDATE].hidden = false;
        lv_img_set_src(statusbar_left[INDEX_SW_UPDATE].image.img, &ic_status_sw_update);
    } else {
        statusbar_left[INDEX_SW_UPDATE].hidden = true;
    }

    if (org_hidden != statusbar_left[INDEX_SW_UPDATE].hidden) {
        refresh_status_bar_list(STATUSBAR_LEFT);
    }
#else
    statusbar_left[INDEX_SW_UPDATE].hidden = true;

    if (!is_static_popup() && !is_upgrade_notified() && get_available_updates_state()) {
        upgrade_available_check();
    }
#endif
}

void update_battery_level() {
    int level = get_battery_info();
    bool show_charging = is_charging();

    if (statusbar_right[INDEX_BATTERY_LEVEL].slot.con != NULL) {
        char capacity[5];
        memset(capacity, '\0', sizeof(capacity));
        sprintf(capacity, "%d%s", level, "%");
        lv_label_set_text(statusbar_right[INDEX_BATTERY_LEVEL].slot.con, capacity);
    }

    if (statusbar_right[INDEX_CHARING_ICON].slot.con == NULL ||
            statusbar_right[INDEX_BATTERY_ICON].slot.con == NULL ||
            statusbar_right[INDEX_BATTERY_ICON].image.img == NULL) {
        return ;
    }

    if (show_charging) {
        bool org_hidden = statusbar_right[INDEX_CHARING_ICON].hidden;
        statusbar_right[INDEX_CHARING_ICON].hidden = false;
        statusbar_right[INDEX_BATTERY_ICON].hidden = true;
        if (org_hidden != statusbar_right[INDEX_CHARING_ICON].hidden) {
            refresh_status_bar_list(STATUSBAR_RIGHT);
        }
    } else {
        bool org_hidden = statusbar_right[INDEX_BATTERY_ICON].hidden;
        statusbar_right[INDEX_CHARING_ICON].hidden = true;
        statusbar_right[INDEX_BATTERY_ICON].hidden = false;
        if (org_hidden != statusbar_right[INDEX_BATTERY_ICON].hidden) {
            refresh_status_bar_list(STATUSBAR_RIGHT);
        }

        if (level == 100) {
#if defined(HIGH_RESOLUTION)
            lv_img_set_src(statusbar_right[INDEX_BATTERY_ICON].image.img, &ic_status_battery_level04_hd);
#else
            lv_img_set_src(statusbar_right[INDEX_BATTERY_ICON].image.img, &ic_status_battery_level04);
#endif
        } else if (level < 100 && level >= 70) {
#if defined(HIGH_RESOLUTION)
            lv_img_set_src(statusbar_right[INDEX_BATTERY_ICON].image.img, &ic_status_battery_level03_hd);
#else
            lv_img_set_src(statusbar_right[INDEX_BATTERY_ICON].image.img, &ic_status_battery_level03);
#endif
        } else if (level < 70 && level >= 40) {
#if defined(HIGH_RESOLUTION)
            lv_img_set_src(statusbar_right[INDEX_BATTERY_ICON].image.img, &ic_status_battery_level02_hd);
#else
            lv_img_set_src(statusbar_right[INDEX_BATTERY_ICON].image.img, &ic_status_battery_level02);
#endif
        } else if (level < 40 && level >= 10) {
#if defined(HIGH_RESOLUTION)
            lv_img_set_src(statusbar_right[INDEX_BATTERY_ICON].image.img, &ic_status_battery_level01_hd);
#else
            lv_img_set_src(statusbar_right[INDEX_BATTERY_ICON].image.img, &ic_status_battery_level01);
#endif
        } else if (level < 10) {
#if defined(HIGH_RESOLUTION)
            lv_img_set_src(statusbar_right[INDEX_BATTERY_ICON].image.img, &ic_status_battery_low_hd);
#else
            lv_img_set_src(statusbar_right[INDEX_BATTERY_ICON].image.img, &ic_status_battery_low);
#endif
        }
    }
}
