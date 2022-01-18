AM_CFLAGS = \
    -DDEBUG \
    -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/libxml2 \
    -Wall -Wshadow -Wundef -Wmaybe-uninitialized -fbounds-check  \
    -DUSE_EVDEV=1 -DBGR_16BIT=1 -DFEATURE_ROUTER=1 -std=gnu99 \
    -DANDROID_LOG=1 \
    -DDEBUG_MEM \
    -O3 -g3 -funwind-tables \
    -I$(WORKSPACE)/qmi-framework/inc \
     $(QMIF_CFLAGS) \
     $(QMI_CFLAGS)\
     -I$(WORKSPACE)/qmi/services \
     -I$(WORKSPACE)/qmi/inc \
     -I$(WORKSPACE)/qmi/platform

requiredlibs = -lm -lpthread -lxml2 $(QMIF_LIBS) $(QMI_LIBS)

if IS_DEFAULT
AM_CFLAGS += -DCUST_DEFAULT
endif
if IS_DLINK
AM_CFLAGS += -DCUST_DLINK -DOOKLA_SPEED_TEST -DCUST_SWITCH
endif
if IS_ZYXEL
AM_CFLAGS += -DCUST_ZYXEL
endif
if IS_ONDA
AM_CFLAGS += -DCUST_ONDA
endif
if IS_GEMTEKS
AM_CFLAGS += -DCUST_GEMTEKS -DLV_USE_ARABIC_PERSIAN_CHARS
#AM_CFLAGS += -DCUST_GEMTEKS -DUSE_FRIBIDI
#requiredlibs += ./libs/libfribidi.so
endif
if IS_ZX73_DEFAULT
AM_CFLAGS += -DZX73_DEFAULT
endif
if IS_ZX73_NEC
AM_CFLAGS += -DCUST_ZX73_NEC
endif

if IS_ZX73_APAL
AM_CFLAGS += -DCUST_ZX73_APAL
endif
if IS_UPURPLE
AM_CFLAGS += -DCUST_LUXSHARE -DCUST_LUXSHARE_UPURPLE
endif
if IS_JARVISH
AM_CFLAGS += -DCUST_LUXSHARE -DCUST_LUXSHARE_JARVISH
endif

SRCS = \
    main.c \
    lv_pocket_router/src/manager.c \
    lv_pocket_router/src/uevent.c \
    lv_pocket_router/src/battery/battery_info.c \
    lv_pocket_router/src/battery/battery_charging.c \
    lv_pocket_router/src/ipc/socket_server.c \
    lv_pocket_router/src/ipc/socket_client.c \
    lv_pocket_router/src/dashboard.c \
    lv_pocket_router/src/launcher.c \
    lv_pocket_router/src/status_bar.c \
    lv_pocket_router/src/status_bar_view.c \
    lv_pocket_router/src/ssid/ssid.c \
    lv_pocket_router/src/wlan/hostapd_conf.c \
    lv_pocket_router/src/ril/ril.c \
    lv_pocket_router/src/ril/sms_ril.c \
    lv_pocket_router/src/ril/ril_sim.c \
    lv_pocket_router/src/ril/ril_nw.c \
    lv_pocket_router/src/ril/ril_qcmap.c \
    lv_pocket_router/src/ril/ril_utils.c \
    lv_pocket_router/src/util/battery_log.c \
    lv_pocket_router/src/util/page_anim.c \
    lv_pocket_router/src/util/data_store.c \
    lv_pocket_router/src/util/default_settings.c \
    lv_pocket_router/src/util/popup_box.c \
    lv_pocket_router/src/util/convert_string.c \
    lv_pocket_router/src/util/ethernet_controller.c \
    lv_pocket_router/src/util/led.c \
    lv_pocket_router/src/util/list_action.c \
    lv_pocket_router/src/util/power_manager.c \
    lv_pocket_router/src/util/liste_style.c \
    lv_pocket_router/src/util/info_page.c \
    lv_pocket_router/src/util/power_menu.c \
    lv_pocket_router/src/util/admin_settings.c \
    lv_pocket_router/src/util/usb_compositions.c \
    lv_pocket_router/src/util/high_speed.c \
    lv_pocket_router/src/util/util.c \
    lv_pocket_router/src/conn_guide/display_qr_code.c \
    lv_pocket_router/src/conn_guide/guide_wps.c \
    lv_pocket_router/src/conn_guide/guide.c \
    lv_pocket_router/src/conn_guide/wps.c \
    lv_pocket_router/src/conn_guide/manual_conn.c \
    lv_pocket_router/src/settings/connected_users.c \
    lv_pocket_router/src/settings/remote_wakeup.c \
    lv_pocket_router/src/settings/restore_default.c \
    lv_pocket_router/src/settings/data_usage.c \
    lv_pocket_router/src/settings/update.c \
    lv_pocket_router/src/settings/data_roaming.c \
    lv_pocket_router/src/settings/settings.c \
    lv_pocket_router/src/settings/pin_management.c \
    lv_pocket_router/src/settings/network_settings.c \
    lv_pocket_router/src/settings/wifi.c \
    lv_pocket_router/src/settings/language.c \
    lv_pocket_router/src/settings/profile_management.c \
    lv_pocket_router/src/settings/profile_management_data_store.c \
    lv_pocket_router/src/settings/password_lock.c \
    lv_pocket_router/src/settings/wifi_extender.c \
    lv_pocket_router/src/settings/time_setting.c \
    lv_pocket_router/src/settings/preference_network.c \
    lv_pocket_router/src/settings/sim_network_controller.c \
    lv_pocket_router/src/sms/sms_store.c \
    lv_pocket_router/src/sms/sms.c \
    lv_pocket_router/src/about/open_source_notice.c \
    lv_pocket_router/src/about/caution_for_using_5g.c \
    lv_pocket_router/src/about/about_help.c \
    lv_pocket_router/src/about/about.c \
    lv_pocket_router/src/about/device_information.c \
    lv_pocket_router/src/power_saving/battery_optimize.c \
    lv_pocket_router/src/power_saving/power_saving.c \
    lv_pocket_router/src/display/wake_lock.c \
    lv_pocket_router/src/display/display.c \
    lv_pocket_router/src/keyboard/en_kb.c \
    lv_pocket_router/src/keyboard/num_kb_box.c \
    lv_pocket_router/src/keyboard/num_kb_col.c \
    lv_pocket_router/src/keyboard/basic_kb.c \
    lv_pocket_router/res/values/styles.c \
    lv_pocket_router/res/values/string_value.c \
    lv_pocket_router/res/images/launcher_icon/ic_launcher_btn_pressed_01.c \
    lv_pocket_router/res/images/launcher_icon/ic_launcher_btn_pressed_02.c \
    lv_pocket_router/res/images/launcher_icon/ic_launcher_return.c \
    lv_pocket_router/res/images/launcher_icon/ic_launcher_ssid.c \
    lv_pocket_router/res/images/launcher_icon/ic_launcher_power.c \
    lv_pocket_router/res/images/launcher_icon/ic_page_indicator_focus.c \
    lv_pocket_router/res/images/launcher_icon/ic_launcher_guide.c \
    lv_pocket_router/res/images/launcher_icon/ic_launcher_about.c \
    lv_pocket_router/res/images/launcher_icon/ic_launcher_setting.c \
    lv_pocket_router/res/images/launcher_icon/ic_launcher_sms.c \
    lv_pocket_router/res/images/launcher_icon/ic_indicator_unfocus.c \
    lv_pocket_router/res/images/launcher_icon/ic_launcher_speedtest.c \
    lv_pocket_router/res/images/launcher_icon/ic_notifi_badge.c \
    lv_pocket_router/res/images/launcher_icon/image_page_to_page.c \
    lv_pocket_router/res/images/launcher_icon/Zyxel_logo_2016.c \
    lv_pocket_router/res/images/dashboard_anim/ic_launcher_info_device.c \
    lv_pocket_router/res/images/dashboard_anim/ic_launcher_info_charger.c \
    lv_pocket_router/res/images/sms_icon/ic_list_sms_read.c \
    lv_pocket_router/res/images/sms_icon/ic_list_sms.c \
    lv_pocket_router/res/images/popup_icon/ic_pop_restart.c \
    lv_pocket_router/res/images/popup_icon/ic_pop_power.c \
    lv_pocket_router/res/images/popup_icon/ic_pop_cancel.c \
    lv_pocket_router/res/images/list_icon/btn_list_radio_p.c \
    lv_pocket_router/res/images/list_icon/btn_list_radio_n.c \
    lv_pocket_router/res/images/list_icon/ic_arrow.c \
    lv_pocket_router/res/images/list_icon/ic_arrow_rtl.c \
    lv_pocket_router/res/images/list_icon/ic_list_users.c \
    lv_pocket_router/res/images/list_icon/ic_list_remove_n.c \
    lv_pocket_router/res/images/list_icon/ic_list_language.c \
    lv_pocket_router/res/images/list_icon/ic_list_remote.c \
    lv_pocket_router/res/images/list_icon/ic_list_data.c \
    lv_pocket_router/res/images/list_icon/ic_list_update.c \
    lv_pocket_router/res/images/list_icon/ic_list_networking.c \
    lv_pocket_router/res/images/list_icon/ic_list_password.c \
    lv_pocket_router/res/images/list_icon/ic_list_profile.c \
    lv_pocket_router/res/images/list_icon/ic_list_wifiex.c \
    lv_pocket_router/res/images/list_icon/ic_list_checkbox_selected.c \
    lv_pocket_router/res/images/list_icon/ic_list_usage_h.c \
    lv_pocket_router/res/images/list_icon/ic_list_usage_l.c \
    lv_pocket_router/res/images/list_icon/ic_list_roaming.c \
    lv_pocket_router/res/images/list_icon/ic_list_checkbox.c \
    lv_pocket_router/res/images/list_icon/ic_list_block_n.c \
    lv_pocket_router/res/images/list_icon/ic_list_wifi.c \
    lv_pocket_router/res/images/list_icon/ic_list_pin.c \
    lv_pocket_router/res/images/list_icon/ic_list_wps.c \
    lv_pocket_router/res/images/list_icon/ic_list_restore.c \
    lv_pocket_router/res/images/list_icon/ic_list_time.c \
    lv_pocket_router/res/images/list_icon/ic_help_wifi.c \
    lv_pocket_router/res/images/list_icon/ic_help_noservice.c \
    lv_pocket_router/res/images/list_icon/ic_help_roaming.c \
    lv_pocket_router/res/images/list_icon/ic_help_sim.c \
    lv_pocket_router/res/images/list_icon/ic_help_network.c \
    lv_pocket_router/res/images/list_icon/ic_help_hotspot.c \
    lv_pocket_router/res/images/list_icon/ic_help_sms.c \
    lv_pocket_router/res/images/list_icon/ic_help_battery.c \
    lv_pocket_router/res/images/list_icon/ic_help_charging.c \
    lv_pocket_router/res/images/list_icon/ic_help_wifidevice.c \
    lv_pocket_router/res/images/list_icon/ic_help_dfs.c \
    lv_pocket_router/res/images/list_icon/ic_help_indoor.c \
    lv_pocket_router/res/images/list_icon/ic_help_data.c \
    lv_pocket_router/res/images/list_icon/ic_help_home.c \
    lv_pocket_router/res/images/list_icon/ic_help_back.c \
    lv_pocket_router/res/images/list_icon/ic_help_cancel.c \
    lv_pocket_router/res/images/list_icon/ic_help_ok.c \
    lv_pocket_router/res/images/list_icon/ic_help_blacklist.c \
    lv_pocket_router/res/images/list_icon/ic_help_sw_update.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_battery_charging.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_battery_level01.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_battery_level02.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_battery_level03.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_battery_level04.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_battery_low.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_data_down.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_data_up.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_data_ud.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_message.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_sim.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_sw_update.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_wifi_s1.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_wifi_s2.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_wifi_s3.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_signal_3g.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_signal_4g.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_signal_4gp.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_signal_5g.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_signal_5g_c.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_signal_e.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_signal_g.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_signal_h.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_signal_hp.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_signal_no.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_signal_r.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_signal_level01.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_signal_level02.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_signal_level03.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_signal_level04.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_signal_level05.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level01.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level02.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level03.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level04.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level05.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level06.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level07.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level08.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level09.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level10.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level11.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level12.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level13.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level14.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level15.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level16.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level17.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level18.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level19.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level20.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level21.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level22.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level23.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level24.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level25.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level26.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level27.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level28.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level29.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level30.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level31.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_y5device_level32.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_interruptible_1.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_interruptible_2.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_interruptible_3.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_interruptible_4.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_interruptible_5.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_interruptible_6.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_interruptible_7.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_interruptible_8.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_interruptible_9.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_interruptible_10.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_notice_1.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_notice_2.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_notice_3.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_notice_4.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_notice_5.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_notice_6.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_notice_7.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_notice_8.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_notice_9.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_notice_10.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_question_1.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_question_2.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_question_3.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_question_4.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_question_5.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_question_6.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_question_7.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_question_8.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_question_9.c \
    lv_pocket_router/res/images/popup_animation/ic_pop_question_10.c \
    lv_pocket_router/res/images/popup_animation/icon_process_1.c \
    lv_pocket_router/res/images/popup_animation/icon_process_2.c \
    lv_pocket_router/res/images/popup_animation/icon_process_3.c \
    lv_pocket_router/res/images/popup_animation/icon_process_4.c \
    lv_pocket_router/res/images/popup_animation/icon_process_5.c \
    lv_pocket_router/res/images/popup_animation/icon_process_6.c \
    lv_pocket_router/res/images/popup_animation/icon_process_7.c \
    lv_pocket_router/res/images/popup_animation/icon_process_8.c \
    lv_pocket_router/res/images/power_saving/ic_list_brightness_h.c \
    lv_pocket_router/res/images/power_saving/ic_list_brightness_l.c \
    lv_pocket_router/res/images/keyboard/btn_ime_delete_n.c \
    lv_pocket_router/res/images/keyboard/btn_ime_delete_n_rtl.c \
    lv_pocket_router/res/images/keyboard/btn_tab_space_n.c \
    lv_pocket_router/res/images/keyboard/ic_keyboard_delete.c \
    lv_pocket_router/res/images/keyboard/slash_date.c \
    lv_pocket_router/res/images/keyboard/slash_time.c \
    lv_pocket_router/res/images/headline/ic_headline_home.c \
    lv_pocket_router/res/images/headline/ic_headline_back.c \
    lv_pocket_router/res/images/headline/ic_headline_back_rtl.c \
    lv_pocket_router/res/images/headline/ic_headline_delete.c \
    lv_pocket_router/res/images/headline/ic_headline_block.c \
    lv_pocket_router/res/images/headline/ic_headline_tick.c \
    lv_pocket_router/res/images/headline/ic_headline_delete_dis.c \
    lv_pocket_router/res/images/headline/ic_headline_tick_dis.c \
    lv_pocket_router/res/images/headline/ic_headline_close.c \
    lv_pocket_router/res/images/headline/ic_headline_edit.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dashboard_slide.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_count_1.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_count_2.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_count_3.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_count_4.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_count_5.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_count_6.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_count_7.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_count_8.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_count_9.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_count_10.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_count_11.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_count_12.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_user_number_1.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_user_number_2.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_user_number_3.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_user_number_4.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_user_number_5.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_user_number_6.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_user_number_7.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_level_1.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_level_2.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_level_3.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_level_4.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_level_5.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_level_6.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_level_7.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_level_8.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_level_9.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_level_10.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_level_11.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_data_level_12.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_battery.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_battery_low.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_battery_remain_1.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_battery_remain_2.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_battery_remain_3.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_battery_remain_4.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_battery_remain_5.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_battery_remain_6.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_battery_remain_7.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_battery_remain_alert.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_battery_remain_charging.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_battery_remain_normal.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dashboard_data.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dashboard_data_remain.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dashboard_data_5.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dashboard_data_25.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dashboard_data_50.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dashboard_data_75.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dashboard_data_100.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_data_bg.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_data_level01.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_data_level02.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_data_level03.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_data_level04.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_data_level05.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_data_status_alert.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_data_status_normal.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_data_status_warning.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_data_unlimited.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_device_connected.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_bettary_low.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_bettary_level04.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_bettary_level03.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_bettary_level02.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_bettary_level01.c \
    lv_pocket_router/res/images/dashboard_zyxel/ic_dash_bettary_charging.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_battery_alert.c \
    lv_pocket_router/res/images/dashboard_anim/ic_dash_battery_charging.c \
    lv_pocket_router/res/images/dashboard_anim/ic_launcher_info_charging.c \
    lv_pocket_router/res/images/dashboard_anim/ic_launcher_info_low.c \
    lv_pocket_router/res/images/charge_mode/icon_offscreen_charging_7.c \
    lv_pocket_router/res/images/charge_mode/icon_offscreen_charging_6.c \
    lv_pocket_router/res/images/charge_mode/icon_offscreen_charging_5.c \
    lv_pocket_router/res/images/charge_mode/icon_offscreen_charging_4.c \
    lv_pocket_router/res/images/charge_mode/icon_offscreen_charging_3.c \
    lv_pocket_router/res/images/charge_mode/icon_offscreen_charging_2.c \
    lv_pocket_router/res/images/charge_mode/icon_offscreen_charging_1.c \
    lvgl/src/lv_font/lv_font.c \
    lvgl/src/lv_font/lv_font_fmt_txt.c \
    lvgl/src/lv_font/lv_font_roboto_12.c \
    lvgl/src/lv_font/lv_font_roboto_16.c \
    lvgl/src/lv_font/lv_font_roboto_22.c \
    lvgl/src/lv_font/lv_font_roboto_28.c \
    lvgl/src/lv_font/lv_font_unscii_8.c \
    lvgl/src/lv_misc/lv_circ.c \
    lvgl/src/lv_misc/lv_area.c \
    lvgl/src/lv_misc/lv_task.c \
    lvgl/src/lv_misc/lv_fs.c \
    lvgl/src/lv_misc/lv_anim.c \
    lvgl/src/lv_misc/lv_mem.c \
    lvgl/src/lv_misc/lv_ll.c \
    lvgl/src/lv_misc/lv_color.c \
    lvgl/src/lv_misc/lv_txt.c \
    lvgl/src/lv_misc/lv_log.c \
    lvgl/src/lv_misc/lv_async.c \
    lvgl/src/lv_misc/lv_utils.c \
    lvgl/src/lv_misc/lv_math.c \
    lvgl/src/lv_misc/lv_gc.c \
    lvgl/src/lv_misc/lv_txt_ap.c \
    lvgl/src/lv_core/lv_group.c \
    lvgl/src/lv_core/lv_indev.c \
    lvgl/src/lv_core/lv_obj.c \
    lvgl/src/lv_core/lv_refr.c \
    lvgl/src/lv_core/lv_style.c \
    lvgl/src/lv_core/lv_disp.c \
    lvgl/src/lv_themes/lv_theme_alien.c \
    lvgl/src/lv_themes/lv_theme.c \
    lvgl/src/lv_themes/lv_theme_default.c \
    lvgl/src/lv_themes/lv_theme_night.c \
    lvgl/src/lv_themes/lv_theme_templ.c \
    lvgl/src/lv_themes/lv_theme_zen.c \
    lvgl/src/lv_themes/lv_theme_material.c \
    lvgl/src/lv_themes/lv_theme_nemo.c \
    lvgl/src/lv_themes/lv_theme_mono.c \
    lvgl/src/lv_hal/lv_hal_disp.c \
    lvgl/src/lv_hal/lv_hal_indev.c \
    lvgl/src/lv_hal/lv_hal_tick.c \
    lvgl/src/lv_objx/lv_arc.c \
    lvgl/src/lv_objx/lv_bar.c \
    lvgl/src/lv_objx/lv_cb.c \
    lvgl/src/lv_objx/lv_tileview.c \
    lvgl/src/lv_objx/lv_ddlist.c \
    lvgl/src/lv_objx/lv_spinbox.c \
    lvgl/src/lv_objx/lv_kb.c \
    lvgl/src/lv_objx/lv_line.c \
    lvgl/src/lv_objx/lv_mbox.c \
    lvgl/src/lv_objx/lv_preload.c \
    lvgl/src/lv_objx/lv_canvas.c \
    lvgl/src/lv_objx/lv_table.c \
    lvgl/src/lv_objx/lv_roller.c \
    lvgl/src/lv_objx/lv_tabview.c \
    lvgl/src/lv_objx/lv_btn.c \
    lvgl/src/lv_objx/lv_calendar.c \
    lvgl/src/lv_objx/lv_chart.c \
    lvgl/src/lv_objx/lv_gauge.c \
    lvgl/src/lv_objx/lv_label.c \
    lvgl/src/lv_objx/lv_list.c \
    lvgl/src/lv_objx/lv_slider.c \
    lvgl/src/lv_objx/lv_ta.c \
    lvgl/src/lv_objx/lv_btnm.c \
    lvgl/src/lv_objx/lv_cont.c \
    lvgl/src/lv_objx/lv_img.c \
    lvgl/src/lv_objx/lv_led.c \
    lvgl/src/lv_objx/lv_lmeter.c \
    lvgl/src/lv_objx/lv_page.c \
    lvgl/src/lv_objx/lv_sw.c \
    lvgl/src/lv_objx/lv_win.c \
    lvgl/src/lv_draw/lv_img_decoder.c \
    lvgl/src/lv_draw/lv_draw_basic.c \
    lvgl/src/lv_draw/lv_draw.c \
    lvgl/src/lv_draw/lv_img_cache.c \
    lvgl/src/lv_draw/lv_draw_rect.c \
    lvgl/src/lv_draw/lv_draw_label.c \
    lvgl/src/lv_draw/lv_draw_line.c \
    lvgl/src/lv_draw/lv_draw_img.c \
    lvgl/src/lv_draw/lv_draw_arc.c \
    lvgl/src/lv_draw/lv_draw_triangle.c \
    lv_drivers/display/android_fbdev.c \
    lv_drivers/indev/evdev.c \
    lv_drivers/indev/hw_events.c \
    lv_lib_qrcode/qrcodegen.h \
    lv_lib_qrcode/qrcodegen.c \
    lv_lib_qrcode/lv_qrcode.h \
    lv_lib_qrcode/lv_qrcode.c

if IS_BT_SUPPORT
AM_CFLAGS += -DBT_SUPPORT
SRCS += \
    lv_pocket_router/src/settings/bluetooth_settings.c \
    lv_pocket_router/res/images/list_icon/ic_list_bt.c \
    lv_pocket_router/res/images/list_icon/ic_help_btdevice.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_btdevice_level01.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_btdevice_level02.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_btdevice_level03.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_btdevice_level04.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_btdevice_level05.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_btdevice_level06.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_btdevice_level07.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_btdevice_level08.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_btdevice_level09.c \
    lv_pocket_router/res/images/status_bar_icon/ic_status_btdevice_level10.c
endif

if IS_BOOT_ANIMATION
AM_CFLAGS += -DBOOT_ANIMATION
SRCS += \
    lv_pocket_router/res/images/boot_anim/bootup_3.c \
    lv_pocket_router/res/images/boot_anim/bootup_dlink.c \
    lv_pocket_router/res/images/boot_anim/bootup_zyxel.c
endif

if IS_SPEEDTEST
AM_CFLAGS += -DSPEED_TEST
SPEED_TEST_SOURCES = \
    lv_pocket_router/src/speedtest/speedtest.c \
    lv_pocket_router/src/speedtest/downloadtest.c \
    lv_pocket_router/res/images/speed_test/ic_speedtest_upload_wb.c \
    lv_pocket_router/res/images/speed_test/ic_speedtest_pinspeed_wb.c \
    lv_pocket_router/res/images/speed_test/ic_speedtest_download_wb.c \
    lv_pocket_router/res/images/speed_test/icon_chart_1.c \
    lv_pocket_router/res/images/speed_test/icon_chart_2.c \
    lv_pocket_router/res/images/speed_test/icon_chart_3.c \
    lv_pocket_router/res/images/speed_test/icon_chart_4.c \
    lv_pocket_router/res/images/speed_test/icon_chart_5.c \
    lv_pocket_router/res/images/speed_test/icon_chart_6.c \
    lv_pocket_router/res/images/speed_test/icon_chart_7.c \
    lv_pocket_router/res/images/speed_test/icon_chart_8.c \
    lv_pocket_router/res/images/speed_test/icon_chart_9.c \
    lv_pocket_router/res/images/speed_test/icon_chart_10.c \
    lv_pocket_router/res/images/speed_test/icon_chart_11.c \
    lv_pocket_router/res/images/speed_test/icon_chart_12.c \
    lv_pocket_router/res/images/speed_test/icon_chart_13.c \
    lv_pocket_router/res/images/speed_test/icon_chart_14.c \
    lv_pocket_router/res/images/speed_test/icon_chart_15.c \
    lv_pocket_router/res/images/speed_test/icon_chart_16.c \
    lv_pocket_router/res/images/speed_test/icon_chart_17.c \
    lv_pocket_router/res/images/speed_test/icon_chart_18.c \
    lv_pocket_router/res/images/speed_test/icon_chart_19.c \
    lv_pocket_router/res/images/speed_test/icon_chart_20.c \
    lv_pocket_router/res/images/speed_test/icon_chart_21.c \
    lv_pocket_router/res/images/speed_test/icon_chart_22.c
endif

FONT_SRCS = \
    lv_pocket_router/res/fonts/NotoSansArabic_ExtraCondensedBold_18.c \
    lv_pocket_router/res/fonts/NotoSansArabic_ExtraCondensedBold_20.c \
    lv_pocket_router/res/fonts/NotoSansArabic_ExtraCondensedBold_22.c \
    lv_pocket_router/res/fonts/NotoSansArabic_ExtraCondensedBold_24.c \
    lv_pocket_router/res/fonts/NotoSansArabic_ExtraCondensedBold_26.c \
    lv_pocket_router/res/fonts/NotoSansArabic_ExtraCondensedBold_30.c \
    lv_pocket_router/res/fonts/NotoSansArabic_ExtraCondensedBold_32.c \
    lv_pocket_router/res/fonts/NotoSansArabic_ExtraCondensedBold_40.c \
    lv_pocket_router/res/fonts/NotoSansArabic_ExtraCondensedBold_50.c \
    lv_pocket_router/res/fonts/NotoSansArabic_ExtraCondensedMedium_14.c \
    lv_pocket_router/res/fonts/NotoSansArabic_ExtraCondensedMedium_22.c \
    lv_pocket_router/res/fonts/NotoSansArabic_ExtraCondensedMedium_26.c \
    lv_pocket_router/res/fonts/Oswald_SemiBold_12.c \
    lv_pocket_router/res/fonts/Oswald_SemiBold_16.c \
    lv_pocket_router/res/fonts/Oswald_SemiBold_18.c \
    lv_pocket_router/res/fonts/Oswald_SemiBold_20.c \
    lv_pocket_router/res/fonts/Oswald_SemiBold_22.c \
    lv_pocket_router/res/fonts/Oswald_SemiBold_24.c \
    lv_pocket_router/res/fonts/Oswald_SemiBold_26.c \
    lv_pocket_router/res/fonts/Oswald_SemiBold_30.c \
    lv_pocket_router/res/fonts/Oswald_SemiBold_32.c \
    lv_pocket_router/res/fonts/Oswald_SemiBold_40.c \
    lv_pocket_router/res/fonts/Oswald_SemiBold_50.c \
    lv_pocket_router/res/fonts/Oswald_Medium_14.c \
    lv_pocket_router/res/fonts/Oswald_Medium_22.c

# ifndef JP_AR_FONT
FONT_SRCS += \
    lv_pocket_router/res/fonts/NotoSansCJKjp_Bold_16.c \
    lv_pocket_router/res/fonts/NotoSansCJKjp_Bold_24.c \
    lv_pocket_router/res/fonts/NotoSansCJKjp_Medium_14.c \
    lv_pocket_router/res/fonts/NotoSansCJKjp_Medium_26.c
# otherwise enable below
#FONT_SRCS += \
#    lv_pocket_router/res/fonts/MPLUS1p_Bold_16.c \
#    lv_pocket_router/res/fonts/MPLUS1p_Bold_24.c \
#    lv_pocket_router/res/fonts/MPLUS1p_Medium_14.c \
#    lv_pocket_router/res/fonts/MPLUS1p_Medium_26.c

PocketRouter_LDFLAGS = -lqmiservices $(QMIFRAMEWORK_LIBS)

PocketRouter_LDADD = $(requiredlibs) 

AUTOMAKE_OPTIONS = subdir-objects

PocketRouter_SOURCES = $(SRCS) $(FONT_SRCS) $(SPEED_TEST_SOURCES)

PocketRouterDebug_SOURCES = $(SRCS)
PocketRouterDebug_LDFLAGS = $(PocketRouter_LDFLAGS)
PocketRouterDebug_LDADD = $(PocketRouter_LDADD)
FLAGS_FILTERED = $(filter-out -DSPEED_TEST,$(AM_CFLAGS))
PocketRouterDebug_CFLAGS = $(FLAGS_FILTERED) -DCOREDUMP_DEBUG

bin_PROGRAMS = PocketRouter PocketRouterDebug


