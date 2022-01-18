#include "status_bar.h"
#include "status_bar_view.h"
#include "launcher.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "lv_pocket_router/src/battery/battery_info.h"
#include <lv_pocket_router/src/ril/ril.h>
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"
#include <lv_pocket_router/src/wlan/hostapd_conf.h>

#define SB_BATT_SHIFT_HOR 35

pthread_mutex_t mutex;
lv_style_t style_s_bar_bg;
bool create_status_bar_done = false;
lv_task_t * lv_timer_task;

void storage_full_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    set_storage_full_promp_flag();
    close_popup();
}

void storage_full_popup(void){
    static const char *btns[2];
    btns[1] = "";
    btns[0] = get_string(ID_OK);
    popup_anim_not_create(get_string(ID_STORAGE_FULL), btns, storage_full_action, NULL);
}

void update_time_label() {
    if (statusbar_left[INDEX_TIME].slot.con != NULL) {
        time_t t = time(NULL);
        struct tm *timeinfo = localtime(&t);

        char current_time[9];
        memset(current_time, '\0', sizeof(current_time));
#ifdef CUST_GEMTEKS
        strftime(current_time, sizeof(current_time), "%I:%M%p", timeinfo);
#else
        strftime(current_time, sizeof(current_time), "%H:%M", timeinfo);
#endif
        statusbar_left[INDEX_TIME].slot.w = strlen(current_time) * LV_RES_OFFSET;
        lv_obj_set_size(statusbar_left[INDEX_TIME].slot.con,
            statusbar_left[INDEX_TIME].slot.w, statusbar_left[INDEX_TIME].slot.h);
        lv_label_set_text(statusbar_left[INDEX_TIME].slot.con, current_time);
    }
}

void update_battery_label() {
    update_battery_level();
}

void update_statusbar_label() {
    update_time_label();
    update_battery_label();
    //update sms unread msg icon in status bar
    update_unread_message();
    update_sw_update();
    update_hotspot_user_counter();
    update_wifi_band();
    if (storage_full_check()) {
        storage_full_popup();
    }
}

//re-align for ssid
void update_ssid_address() {
    StatusBarInfo bar =  statusbar_second_right[INDEX_SSID];
    if (bar.slot.con == NULL) return;
    if(is_ltr()){
        lv_label_set_align(bar.slot.con, LV_LABEL_ALIGN_RIGHT);
        lv_obj_align(bar.slot.con, NULL,
                LV_ALIGN_IN_RIGHT_MID, -8, bar.slot.shift.y);
    }else{
        lv_label_set_align(bar.slot.con, LV_LABEL_ALIGN_LEFT);
        lv_obj_align(bar.slot.con, NULL,
                LV_ALIGN_IN_LEFT_MID, 8, bar.slot.shift.y);
    }
}

void update_icon_address(int index, StatusBarInfo bar) {
    if (bar.slot.con == NULL) return;
    if (!bar.hidden) {
        if (lv_obj_get_hidden(bar.slot.con)) {
            lv_obj_set_hidden(bar.slot.con, 0);
            if (bar.image.img != NULL) {
                lv_obj_set_hidden(bar.image.img, 0);
            }
        }

        if (index == 0) {
            // The clock/battery label is a base, if call lv_obj_align() again,
            // all icons will be shift to bottom right/bottom left.
            // Because lv_obj_align(obj, base pointer,..)'s base pointer is itself.
        } else {
            StatusBarInfo last;
            int i;
            for (i = (index-1); i >= 0; i--) {
                if (bar.id == STATUSBAR_LEFT && statusbar_left[i].slot.con != NULL && !statusbar_left[i].hidden) {
                    last = statusbar_left[i];
                    break;
                } else if (bar.id == STATUSBAR_RIGHT && !statusbar_right[i].hidden) {
                    last = statusbar_right[i];
                    break;
                } else if (bar.id == STATUSBAR_SECOND_LEFT && !statusbar_second_left[i].hidden) {
                    last = statusbar_second_left[i];
                    break;
                } else if (bar.id == STATUSBAR_SECOND_RIGHT && !statusbar_second_right[i].hidden) {
                    last = statusbar_second_right[i];
                    break;
                }
            }

            //re-align statusbar icon depend on is_ltr result
            if(is_ltr()){
                lv_obj_align(bar.slot.con, last.slot.con,
                        bar.slot.align, bar.slot.shift.x, bar.slot.shift.y);
                if (bar.image.img != NULL) {
                    lv_obj_align(bar.image.img,
                        (last.image.img != NULL)? last.image.img:last.slot.con,
                        bar.image.align, bar.image.shift.x, bar.image.shift.y);
                }
            }else{
                if(bar.slot.align == LV_ALIGN_IN_TOP_RIGHT){
                    lv_obj_align(bar.slot.con, last.slot.con, LV_ALIGN_IN_TOP_LEFT,
                            -bar.slot.shift.x, bar.slot.shift.y);
                    if (bar.image.img != NULL) {
                        lv_obj_align(bar.image.img,
                            (last.image.img != NULL)? last.image.img:last.slot.con,
                                    LV_ALIGN_IN_TOP_LEFT, -bar.image.shift.x, bar.image.shift.y);
                    }
                }

                if(bar.slot.align == LV_ALIGN_IN_TOP_LEFT){
                    lv_obj_align(bar.slot.con, last.slot.con, LV_ALIGN_IN_TOP_RIGHT, -bar.slot.shift.x, 0);
                    if (bar.image.img != NULL) {
                        lv_obj_align(bar.image.img,
                            (last.image.img != NULL)? last.image.img:last.slot.con,
                                    LV_ALIGN_IN_TOP_RIGHT, -bar.image.shift.x, 0);
                    }
                }
            }
            //re-align statusbar icon depend on is_ltr result end
        }
    } else {
        lv_obj_set_hidden(bar.slot.con, 1);
        if (bar.image.img != NULL) {
            lv_obj_set_hidden(bar.image.img, 1);
        }
    }
}

void refresh_status_bar_list(int id) {
    pthread_mutex_lock(&mutex);
    if (create_status_bar_done) {
        int i;
        if (id == STATUSBAR_LEFT) {
            for (i = 1; i < MAX_STATUS_BAR_LEFT_ICON; i++) {
                update_icon_address(i, statusbar_left[i]);
            }
        } else if (id == STATUSBAR_RIGHT) { 
            for (i = 1; i < MAX_STATUS_BAR_RIGHT_ICON; i++) {
                update_icon_address(i, statusbar_right[i]);
            }
        } else if (id == STATUSBAR_SECOND_LEFT) {
            for (i = 1; i < MAX_STATUS_BAR_SECOND_LEFT_ICON; i++) {
                if(i < MAX_STATUS_BAR_SECOND_LEFT_ICON) {
                    update_icon_address(i, statusbar_second_left[i]);
                }
            }
        } else if (id == STATUSBAR_SECOND_RIGHT) {
            for (i = 1; i < MAX_STATUS_BAR_SECOND_RIGHT_ICON; i++) {
                if(i < MAX_STATUS_BAR_SECOND_RIGHT_ICON) {
                    update_icon_address(i, statusbar_second_right[i]);
                }
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

void create_icon(lv_obj_t * root, int id, int index, char* name, bool hidden, void* func,
        int slot_w, int slot_h, int slot_align, int slot_shift_x, int slot_shift_y,
        int img_w, int img_h, int img_align, int img_shift_x, int img_shift_y) {

    log_d("Create status bar icon: %s", name);

    StatusBarInfo bar;
    bar.func = func;
    bar.hidden = hidden;
    bar.id = id;
    bar.slot.w = slot_w;
    bar.slot.h = slot_h;
    bar.slot.align = slot_align;
    bar.slot.shift.x = slot_shift_x;
    bar.slot.shift.y = slot_shift_y;
    bar.slot.con = lv_cont_create(root, NULL);
    lv_obj_set_size(bar.slot.con, bar.slot.w, bar.slot.h);
    lv_obj_set_style(bar.slot.con, &style_s_bar_bg);

    bar.image.w = img_w;
    bar.image.h = img_h;
    bar.image.align = img_align;
    bar.image.shift.x = img_shift_x;
    bar.image.shift.y = img_shift_y;
    bar.image.img = lv_img_create(bar.slot.con, NULL);
    lv_obj_set_size(bar.image.img, bar.image.w, bar.image.h);

    update_icon_address(index, bar);

    if (id == STATUSBAR_LEFT) {
        statusbar_left[index] = bar;
    } else if (id == STATUSBAR_RIGHT) {
        statusbar_right[index] = bar;
    } else if (id == STATUSBAR_SECOND_LEFT) {
        statusbar_second_left[index] = bar;
    } else if (id == STATUSBAR_SECOND_RIGHT) {
        statusbar_second_right[index] = bar;
    }

    //update image
    if (bar.func != NULL && !bar.hidden) {
        (*bar.func)();
    }
}

void create_statusbar(lv_obj_t * root, lv_color_t font_color, lv_color_t bg_color, status_bg_view bg_view) {
    create_status_bar_done = false;

    lv_style_copy(&style_s_bar_bg, &lv_style_plain);
    style_s_bar_bg.body.main_color = bg_color;
    style_s_bar_bg.body.grad_color = bg_color;
    style_s_bar_bg.body.padding.top = 0;

    static lv_style_t font_style;
#if defined(HIGH_RESOLUTION)
    lv_style_copy(&font_style, &lv_style_plain);
    font_style.text.font = get_font(font_w_bold, font_h_50);
    font_style.text.color = font_color;
#else
    lv_style_copy(&font_style, &lv_style_plain);
    font_style.text.font = get_font(font_w_bold, font_h_16);
    font_style.text.color = font_color;
    font_style.text.letter_space = 0.8;
#endif

    // clock slot
    StatusBarInfo clock;
    clock.hidden = false;
    clock.id = STATUSBAR_LEFT;
    clock.slot.w = 40 * LV_RES_OFFSET;
    clock.slot.h = 18 * LV_RES_OFFSET;
    clock.slot.align = LV_ALIGN_CENTER;
#ifdef CUST_DLINK // shift clock out of UI for DLink as workaround to hide it
    clock.slot.shift.x = 8 * LV_RES_OFFSET - clock.slot.w;
#else
    clock.slot.shift.x = 8 * LV_RES_OFFSET;
#endif
    clock.slot.shift.y = 6 *LV_RES_OFFSET + 3;
    clock.slot.con = lv_label_create(root, NULL);
    clock.func = NULL;
    clock.image.img = NULL;
    lv_obj_set_size(clock.slot.con, clock.slot.w, clock.slot.h);
    lv_obj_set_style(clock.slot.con, &font_style);

    //re-align statusbar icon depend on is_ltr result
    if(is_ltr()){
        lv_obj_align(clock.slot.con, clock.slot.con, clock.slot.align,
            clock.slot.shift.x, clock.slot.shift.y);
    }else{
        lv_obj_align(clock.slot.con, NULL, LV_ALIGN_IN_TOP_RIGHT, -12, clock.slot.shift.y);
    }

    statusbar_left[INDEX_TIME] = clock;
    update_time_label();

    //SIM absent indicator
#if defined(HIGH_RESOLUTION)
    create_icon(root, STATUSBAR_LEFT, INDEX_SIM_ABSENT, "sim_absent", true, NULL,
        80, 60, LV_ALIGN_IN_TOP_RIGHT, 82, -3,
        78, 60, LV_ALIGN_IN_TOP_RIGHT, 82, -3);
    lv_img_set_src(statusbar_left[INDEX_SIM_ABSENT].image.img, &ic_status_sim_hd);
#else
    create_icon(root, STATUSBAR_LEFT, INDEX_SIM_ABSENT, "sim_absent", true, NULL,
        28, 20, LV_ALIGN_IN_TOP_RIGHT, 30, -3,
        26, 20, LV_ALIGN_IN_TOP_RIGHT, 30, -3);
    lv_img_set_src(statusbar_left[INDEX_SIM_ABSENT].image.img, &ic_status_sim);
#endif

    //Signal strength indicator
#if defined(HIGH_RESOLUTION)
    create_icon(root, STATUSBAR_LEFT, INDEX_SIGNAL_STRENGTH, "signal", true, update_sim_signal,
        62, 60, LV_ALIGN_IN_TOP_RIGHT, 64, -3,
        60, 60, LV_ALIGN_IN_TOP_RIGHT, 64, -3);
#else
    create_icon(root, STATUSBAR_LEFT, INDEX_SIGNAL_STRENGTH, "signal", true, update_sim_signal,
        28, 20, LV_ALIGN_IN_TOP_RIGHT, 30, -3,
        20, 20, LV_ALIGN_IN_TOP_RIGHT, 30, -3);
#endif

    // Data flow indicator
#if defined(HIGH_RESOLUTION)
    create_icon(root, STATUSBAR_LEFT, INDEX_DATA_FLOW, "data_flow", true, update_data_flow,
        62, 60, LV_ALIGN_IN_TOP_RIGHT, 62, 0,
        60, 60, LV_ALIGN_IN_TOP_RIGHT, 62, 0);
#else
    create_icon(root, STATUSBAR_LEFT, INDEX_DATA_FLOW, "data_flow", true, update_data_flow,
        22, 20, LV_ALIGN_IN_TOP_RIGHT, 20, 0,
        20, 20, LV_ALIGN_IN_TOP_RIGHT, 20, 0);
#endif

    // Radio tech
#if defined(HIGH_RESOLUTION)
    create_icon(root, STATUSBAR_LEFT, INDEX_RADIO_TECH, "radio_tech", true, update_radio_tech,
        68, 60, LV_ALIGN_IN_TOP_RIGHT, 68, 0,
        66, 60, LV_ALIGN_IN_TOP_RIGHT, 68, 0);
#else
    create_icon(root, STATUSBAR_LEFT, INDEX_RADIO_TECH, "radio_tech", true, update_radio_tech,
        24, 20, LV_ALIGN_IN_TOP_RIGHT, 24, 0,
        22, 20, LV_ALIGN_IN_TOP_RIGHT, 24, 0);
#endif

    // Connected user counter for hotspot
#if defined(HIGH_RESOLUTION)
    create_icon(root, STATUSBAR_LEFT, INDEX_USER_HOTSPOT, "user_counter", false, update_hotspot_user_counter,
        86, 60, LV_ALIGN_IN_TOP_RIGHT, 86, 0,
        84, 60, LV_ALIGN_IN_TOP_RIGHT, 86, 0);
#else
    create_icon(root, STATUSBAR_LEFT, INDEX_USER_HOTSPOT, "user_counter", false, update_hotspot_user_counter,
        30, 20, LV_ALIGN_IN_TOP_RIGHT, 30, 0,
        28, 20, LV_ALIGN_IN_TOP_RIGHT, 30, 0);
#endif

    // Connected user counter for bluetooth
#ifdef BT_SUPPORT
#if defined(HIGH_RESOLUTION)
    create_icon(root, STATUSBAR_LEFT, INDEX_USER_BLUETTOTH, "bluetooth", true, update_bluetooth_user_counter,
        74, 60, LV_ALIGN_IN_TOP_RIGHT, 72, 0,
        72, 60, LV_ALIGN_IN_TOP_RIGHT, 72, 0);
#else
    create_icon(root, STATUSBAR_LEFT, INDEX_USER_BLUETTOTH, "bluetooth", true, update_bluetooth_user_counter,
        26, 20, LV_ALIGN_IN_TOP_RIGHT, 26, 0,
        24, 20, LV_ALIGN_IN_TOP_RIGHT, 26, 0);
#endif
#endif

    // Unread message
#if defined(HIGH_RESOLUTION)
    create_icon(root, STATUSBAR_LEFT, INDEX_UNREAD_MESSAGE, "unread_message", true, update_unread_message,
        74, 60, LV_ALIGN_IN_TOP_RIGHT, 74, 0,
        72, 60, LV_ALIGN_IN_TOP_RIGHT, 74, 0);
#else
    create_icon(root, STATUSBAR_LEFT, INDEX_UNREAD_MESSAGE, "unread_message", true, update_unread_message,
        26, 20, LV_ALIGN_IN_TOP_RIGHT, 26, 0,
        24, 20, LV_ALIGN_IN_TOP_RIGHT, 26, 0);
#endif

    // Wifi band
#if defined(HIGH_RESOLUTION)
    create_icon(root, STATUSBAR_LEFT, INDEX_WIFI_BAND, "wifi_band", false, update_wifi_band,
        62, 60, LV_ALIGN_IN_TOP_RIGHT, 62, 0,
        60, 60, LV_ALIGN_IN_TOP_RIGHT, 62, 0);
#else
    create_icon(root, STATUSBAR_LEFT, INDEX_WIFI_BAND, "wifi_band", false, update_wifi_band,
        26, 26, LV_ALIGN_IN_TOP_RIGHT, 26, 0,
        24, 20, LV_ALIGN_IN_TOP_RIGHT, 26, 0);
#ifdef CUST_DLINK // Show 2.4 & 5GHz icon separately for DLink
    create_icon(root, STATUSBAR_LEFT, INDEX_WIFI_BAND_5G, "wifi_band_5g", false, update_wifi_band,
        26, 26, LV_ALIGN_IN_TOP_RIGHT, 26, 0,
        24, 20, LV_ALIGN_IN_TOP_RIGHT, 26, 0);
#endif
#endif

    // SW Update Available
    create_icon(root, STATUSBAR_LEFT, INDEX_SW_UPDATE, "sw_update", false, update_sw_update,
        22, 20, LV_ALIGN_IN_TOP_RIGHT, 22, 0,
        20, 20, LV_ALIGN_IN_TOP_RIGHT, 22, 0);

    //Battery level
    StatusBarInfo battery_level;
    battery_level.hidden = false;
    battery_level.id = STATUSBAR_RIGHT;
    battery_level.slot.w = 40 * LV_RES_OFFSET;
    battery_level.slot.h = 18 * LV_RES_OFFSET;
    battery_level.slot.align = LV_ALIGN_CENTER;
#ifdef CUST_DLINK // shift battery level out of UI for DLink as workaround to hide it
    // hide status bar battery icon by shift out in dashboard, but keep in launcher
    //if (bg_view == DASHBOARD_BG) {
        battery_level.slot.shift.x = LV_HOR_RES_MAX + (SB_BATT_SHIFT_HOR * LV_RES_OFFSET);
    //} else {
    //    battery_level.slot.shift.x = LV_HOR_RES_MAX;
    //}
#else
    battery_level.slot.shift.x = LV_HOR_RES_MAX - (SB_BATT_SHIFT_HOR * LV_RES_OFFSET);
#endif
    battery_level.slot.shift.y = 6 * LV_RES_OFFSET;
    battery_level.slot.con = lv_label_create(root, NULL);
    battery_level.image.img = NULL;
    lv_obj_set_size(battery_level.slot.con, battery_level.slot.w, battery_level.slot.h);
    lv_obj_set_style(battery_level.slot.con, &font_style);

    //re-align statusbar icon depend on is_ltr result
    if(is_ltr()){
        lv_obj_align(battery_level.slot.con, battery_level.slot.con, battery_level.slot.align,
                battery_level.slot.shift.x, battery_level.slot.shift.y);
    }else{
        lv_obj_align(battery_level.slot.con, NULL, LV_ALIGN_IN_TOP_LEFT, 12, battery_level.slot.shift.y);
    }

    statusbar_right[INDEX_BATTERY_LEVEL] = battery_level;

    //Charging icon
    bool show_charing = is_charging();

#if defined(HIGH_RESOLUTION)
    create_icon(root, STATUSBAR_RIGHT, INDEX_CHARING_ICON, "charing", !show_charing, NULL,
        74, 60, LV_ALIGN_IN_TOP_LEFT, -74, 0,
        72, 60, LV_ALIGN_IN_TOP_LEFT, -72, 0);

    lv_img_set_src(statusbar_right[INDEX_CHARING_ICON].image.img, &ic_status_battery_charging_hd);
#else
    create_icon(root, STATUSBAR_RIGHT, INDEX_CHARING_ICON, "charing", !show_charing, NULL,
        26, 20, LV_ALIGN_IN_TOP_LEFT, -26, 0,
        24, 20, LV_ALIGN_IN_TOP_LEFT, -24, 0);

    lv_img_set_src(statusbar_right[INDEX_CHARING_ICON].image.img, &ic_status_battery_charging);
#endif

    //Battery icon
#if defined(HIGH_RESOLUTION)
    create_icon(root, STATUSBAR_RIGHT, INDEX_BATTERY_ICON, "battery_icon", show_charing, update_battery_level,
        44, 60, LV_ALIGN_IN_TOP_LEFT, -44, 0,
        42, 60, LV_ALIGN_IN_TOP_LEFT, -42, 0);
#else
    create_icon(root, STATUSBAR_RIGHT, INDEX_BATTERY_ICON, "battery_icon", show_charing, update_battery_level,
        16, 20, LV_ALIGN_IN_TOP_LEFT, -16, 0,
        14, 20, LV_ALIGN_IN_TOP_LEFT, -14, 0);
#endif
    update_battery_label();

    // Carrier
    StatusBarInfo carrier;
    carrier.hidden = false;
    carrier.id = STATUSBAR_SECOND_LEFT;
    carrier.slot.w = 152;
    carrier.slot.h = 25;
    carrier.slot.align = LV_ALIGN_OUT_BOTTOM_MID;
    carrier.slot.shift.x = 0 * LV_RES_OFFSET;
    carrier.slot.shift.y = 8 * LV_RES_OFFSET;
    carrier.slot.con = lv_label_create(root, NULL);
    carrier.image.img = NULL;
    lv_obj_set_size(carrier.slot.con, carrier.slot.w, carrier.slot.h);
    lv_obj_set_style(carrier.slot.con, &font_style);
    lv_obj_align(carrier.slot.con, statusbar_left[INDEX_TIME].slot.con,
            carrier.slot.align, carrier.slot.shift.x, carrier.slot.shift.y);
    statusbar_second_left[INDEX_CARRIER] = carrier;

    char oper_name[OPERATOR_NAME_MAX_LENGTH+1];
    memset(oper_name, 0, OPERATOR_NAME_MAX_LENGTH+1);
    get_operator_name(oper_name, OPERATOR_NAME_MAX_LENGTH);
    lv_label_set_text(carrier.slot.con, oper_name);

    //SSID
    StatusBarInfo ssid;
    ssid.hidden = false;
    ssid.id = STATUSBAR_SECOND_RIGHT;
    ssid.slot.w = 152;
    ssid.slot.h = 25;
    ssid.slot.align = LV_ALIGN_OUT_BOTTOM_MID;
    ssid.slot.shift.x = 0 * LV_RES_OFFSET;
    ssid.slot.shift.y = 8 * LV_RES_OFFSET;
    ssid.slot.con = lv_label_create(root, NULL);
    ssid.image.img = NULL;
    lv_obj_set_size(ssid.slot.con, ssid.slot.w, ssid.slot.h);
    lv_obj_set_style(ssid.slot.con, &font_style);

    ssid.func = update_ssid;
    statusbar_second_right[INDEX_SSID] = ssid;
    lv_label_set_text(statusbar_second_right[INDEX_SSID].slot.con, get_active_wlan_ssid());
    //set ssid display to circle mode in case of UI issue
    lv_label_set_long_mode(statusbar_second_right[INDEX_SSID].slot.con, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_width(statusbar_second_right[INDEX_SSID].slot.con, 150);
    update_ssid_address();
    create_status_bar_done = true;

    if (lv_timer_task == NULL) {
        lv_timer_task = lv_task_create(update_statusbar_label, 1000, LV_TASK_PRIO_LOW, NULL);
    }

    monitor_task(); //refresh status icon status after create
    refresh_status_bar_list(STATUSBAR_LEFT);
    refresh_status_bar_list(STATUSBAR_RIGHT);
    refresh_status_bar_list(STATUSBAR_SECOND_LEFT);
    refresh_status_bar_list(STATUSBAR_SECOND_RIGHT);
}


void reset_statusbar_info() {
    memset(statusbar_left, 0, sizeof(statusbar_left));
    memset(statusbar_right, 0, sizeof(statusbar_right));
    memset(statusbar_second_left, 0, sizeof(statusbar_second_left));
    memset(statusbar_second_right, 0, sizeof(statusbar_second_right));
}

void statusbar_reset() {
    /* mark out to allow task keep running for led light state monitor
    if (lv_timer_task != NULL) {
        lv_task_set_period(lv_timer_task, LV_TASK_PRIO_OFF);
        lv_task_del(lv_timer_task);
        lv_timer_task = NULL;
    }*/

    int i;
    for (i = 0; i < MAX_STATUS_BAR_LEFT_ICON; i++) {
        if (statusbar_left[i].image.img != NULL) {
            lv_obj_del(statusbar_left[i].image.img);
        }
        if (statusbar_left[i].slot.con != NULL) {
            lv_obj_del(statusbar_left[i].slot.con);
        }
    }

    for (i = 0; i < MAX_STATUS_BAR_RIGHT_ICON; i++) {
        if (statusbar_right[i].image.img != NULL) {
            lv_obj_del(statusbar_right[i].image.img);
        }
        if (statusbar_right[i].slot.con != NULL) {
            lv_obj_del(statusbar_right[i].slot.con);
        }
    }

    for (i = 0; i < MAX_STATUS_BAR_SECOND_LEFT_ICON; i++) {
        if (statusbar_second_left[i].image.img != NULL) {
            lv_obj_del(statusbar_second_left[i].image.img);
        }
        if (statusbar_second_left[i].slot.con != NULL) {
            lv_obj_del(statusbar_second_left[i].slot.con);
        }
    }

    for (i = 0; i < MAX_STATUS_BAR_SECOND_RIGHT_ICON; i++) {
        if (statusbar_second_right[i].image.img != NULL) {
            lv_obj_del(statusbar_second_right[i].image.img);
        }
        if (statusbar_second_right[i].slot.con != NULL) {
            lv_obj_del(statusbar_second_right[i].slot.con);
        }
    }

    reset_statusbar_info();
}

void statusbar_init() {
    pthread_mutex_init(&mutex, 0);

    reset_statusbar_info();
}
