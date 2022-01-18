/*
 * restore_default.c
 *
 *  Created on: Mar 28, 2019
 *      Author: joseph
 */
#include <stdio.h>
#include <stdbool.h>
#include "lv_pocket_router/src/display/display.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/power_menu.h"
#include "lv_pocket_router/src/wlan/hostapd_conf.h"
#include "lv_pocket_router/src/settings/restore_default.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/sms/sms_store.h"
#include "lv_pocket_router/src/settings/preference_network.h"

#define REBOOT_AFTER_RESET      1

lv_task_t * restore_factory_settings_task;
lv_obj_t * mbox;
int loading;

void restore_default_action(lv_obj_t * btnm, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    restore_factory_settings_animation();
}

void init_restore_default(void) {
    static const char * btns[2];
    btns[0] = get_string(ID_RESTORE_RESTORE_BTN);
    btns[1] = "";
    info_page_create_btmn(lv_scr_act(), get_string(ID_RESTORE_DEFAULT),
            get_string(ID_RESTORE_CONFIRM_PROMPT), btns, restore_default_action);
}

void restart_ui() {
    // set cb back to null so it won't keep cb when we call ui_cleanup()
    set_popup_cb(NULL);

#ifdef FEATURE_ROUTER
#if !defined (REBOOT_AFTER_RESET)
    // restart pocket router ui
    //res = systemCmd("systemctl restart pocketrouter.service");
    //log_d("restart pocketrouter.service command status: %d", res);

    // TODO below is workaround for not restart pocketrouter
    // hostapd pid got kill after pocketrouter restart
    init_default_settings();
    sms_create_xml(SMS_XML);
    init_default_apn_profile();
    init_data_usage_info();
    init_pref_network();
    // do this after default wlan enable has been load either from init default or
    // oem default xml file
    hostapd_restore_default();
    notify_screen_timeout_changed(get_screen_timeout());
    ui_cleanup();
    dashboard_create();
#else /* REBOOT_AFTER_RESET */
    // Zyxel #61
    init_wlan_data();
    load_nv_item();
    sync();

    backlight_off();
    int res = systemCmd("reboot");
    log_d("factory reset reboot result: %d", res);
#endif /* REBOOT_AFTER_RESET */
#else
    // for simulator testing
    systemCmd("rm -r Data_Store");
    init_default_settings();
    sms_create_xml(SMS_XML);
    init_default_apn_profile();
    init_data_usage_info();
    init_wlan_data();
    init_wlan();
    close_all_lists(0);
    close_all_pages();
    launcher_destroy();
    dashboard_create();
#endif
}

void restore_factory_settings() {
    int res = -1;

    reset_screen_timeout();

    if (loading <= 100) {
        update_loading_bar(mbox, loading);
        loading++;
    } else {
        //close task and popup when finish
        close_popup();
    }
}

void restore_factory_settings_animation(void) {
    int res = -1;
    reset_data_usage(false);
#ifdef FEATURE_ROUTER
    res = systemCmd("rm -r /data/misc/wifi/");
    log_d("rm wifi conf files command status: %d", res);
    res = systemCmd("rm -r /data/misc/pocketrouter/");
    log_d("rm pocketrouter files command status: %d", res);
    res = systemCmd("rm -r /data/misc/fota_client/");
    log_d("rm fota_client files command status: %d", res);
    res = systemCmd("rm /data/misc/fotaclient.timer");
    log_d("rm fotaclient.timer command status: %d", res);
#ifdef CUST_DLINK
    res = systemCmd("cp /oem/data/dfota.timer /data/misc/dfota.timer");
    res = systemCmd("systemctl daemon-reload");
#endif
    // use /oem/mifi_factory flag file to trigger reset
    // cei_customization.service will copy relate from /oem/data to /systemrw/data
    res = systemCmd("touch /oem/mifi_factory");
    log_d("touch /oem/mifi_factory status: %d", res);

    res = systemCmd("rm /data/www/lighttpd.user");
    log_d("rm /data/www/lighttpd.user status: %d", res);

#ifdef CUST_GEMTEKS
    res = systemCmd("rm /data/www/wizard_check");
    log_d("rm /data/www/wizard_check status: %d", res);
#endif
    systemCmd("atcli 'at+ceifacreset=fullreset'");

    // restart hostapd with default config
    res = systemCmd("systemctl start pocketrouter-restore.service");
    log_d("start pocketrouter-restore.service command status: %d", res);
#endif

    loading = 0;
    mbox = popup_anim_loading_create(get_string(ID_RESTORE_RESTORE_FACTORY), get_string(ID_RESTORE_RESTORING));
    set_popup_cb(restart_ui);
    restore_factory_settings_task = popup_loading_task_create(restore_factory_settings, 50, LV_TASK_PRIO_MID, NULL);
}
