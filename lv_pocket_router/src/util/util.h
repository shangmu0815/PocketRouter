#ifndef LV_POCKET_ROUTER_SRC_UTIL_H_
#define LV_POCKET_ROUTER_SRC_UTIL_H_

#include <stdbool.h>
#include <stdio.h>

#include "../../../lvgl/lvgl.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/debug_log.h"

LV_IMG_DECLARE(btn_ime_delete_n);
LV_IMG_DECLARE(btn_ime_delete_n_rtl);
LV_IMG_DECLARE(btn_list_radio_n);
LV_IMG_DECLARE(btn_list_radio_p);
LV_IMG_DECLARE(btn_tab_space_n);
LV_IMG_DECLARE(ic_arrow);
LV_IMG_DECLARE(ic_arrow_rtl);
LV_IMG_DECLARE(ic_dash_data_count_1);
LV_IMG_DECLARE(ic_dash_data_count_2);
LV_IMG_DECLARE(ic_dash_data_count_3);
LV_IMG_DECLARE(ic_dash_data_count_4);
LV_IMG_DECLARE(ic_dash_data_count_5);
LV_IMG_DECLARE(ic_dash_data_count_6);
LV_IMG_DECLARE(ic_dash_data_count_7);
LV_IMG_DECLARE(ic_dash_data_count_8);
LV_IMG_DECLARE(ic_dash_data_count_9);
LV_IMG_DECLARE(ic_dash_data_count_10);
LV_IMG_DECLARE(ic_dash_data_count_11);
LV_IMG_DECLARE(ic_dash_data_count_12);
LV_IMG_DECLARE(ic_dash_user_number_1);
LV_IMG_DECLARE(ic_dash_user_number_2);
LV_IMG_DECLARE(ic_dash_user_number_3);
LV_IMG_DECLARE(ic_dash_user_number_4);
LV_IMG_DECLARE(ic_dash_user_number_5);
LV_IMG_DECLARE(ic_dash_user_number_6);
LV_IMG_DECLARE(ic_dash_user_number_7);
LV_IMG_DECLARE(ic_dashboard_slide);
LV_IMG_DECLARE(ic_dashboard_data_5);
LV_IMG_DECLARE(ic_dashboard_data_25);
LV_IMG_DECLARE(ic_dashboard_data_50);
LV_IMG_DECLARE(ic_dashboard_data_75);
LV_IMG_DECLARE(ic_dashboard_data_100);
LV_IMG_DECLARE(ic_dash_data_level_1);
LV_IMG_DECLARE(ic_dash_data_level_2);
LV_IMG_DECLARE(ic_dash_data_level_3);
LV_IMG_DECLARE(ic_dash_data_level_4);
LV_IMG_DECLARE(ic_dash_data_level_5);
LV_IMG_DECLARE(ic_dash_data_level_6);
LV_IMG_DECLARE(ic_dash_data_level_7);
LV_IMG_DECLARE(ic_dash_data_level_8);
LV_IMG_DECLARE(ic_dash_data_level_9);
LV_IMG_DECLARE(ic_dash_data_level_10);
LV_IMG_DECLARE(ic_dash_data_level_11);
LV_IMG_DECLARE(ic_dash_data_level_12);
LV_IMG_DECLARE(ic_headline_back);
LV_IMG_DECLARE(ic_headline_back_rtl);
LV_IMG_DECLARE(ic_headline_block);
LV_IMG_DECLARE(ic_headline_close);
LV_IMG_DECLARE(ic_headline_delete);
LV_IMG_DECLARE(ic_headline_delete_dis);
LV_IMG_DECLARE(ic_headline_edit);
LV_IMG_DECLARE(ic_headline_home);
LV_IMG_DECLARE(ic_headline_tick);
LV_IMG_DECLARE(ic_headline_tick_dis);
LV_IMG_DECLARE(ic_help_back);
LV_IMG_DECLARE(ic_help_battery);
LV_IMG_DECLARE(ic_help_blacklist);
#ifdef BT_SUPPORT
LV_IMG_DECLARE(ic_help_btdevice);
LV_IMG_DECLARE(ic_list_bt);
#endif
LV_IMG_DECLARE(ic_help_cancel);
LV_IMG_DECLARE(ic_help_charging);
LV_IMG_DECLARE(ic_help_data);
#ifdef DFS_SUPPORT
LV_IMG_DECLARE(ic_help_dfs);
#endif
LV_IMG_DECLARE(ic_help_home);
#ifdef HOTSPOT_SUPPORT
LV_IMG_DECLARE(ic_help_hotspot);
#endif
#ifdef INDOOR_SUPPORT
LV_IMG_DECLARE(ic_help_indoor);
#endif
LV_IMG_DECLARE(ic_help_network);
LV_IMG_DECLARE(ic_help_noservice);
LV_IMG_DECLARE(ic_help_ok);
LV_IMG_DECLARE(ic_help_roaming);
LV_IMG_DECLARE(ic_help_sim);
LV_IMG_DECLARE(ic_help_sms);
LV_IMG_DECLARE(ic_help_sw_update);
LV_IMG_DECLARE(ic_help_wifi);
LV_IMG_DECLARE(ic_help_wifidevice);
LV_IMG_DECLARE(ic_indicator_unfocus);
LV_IMG_DECLARE(ic_keyboard_delete);
LV_IMG_DECLARE(ic_launcher_about);
LV_IMG_DECLARE(ic_launcher_guide);
LV_IMG_DECLARE(ic_launcher_power);
LV_IMG_DECLARE(ic_launcher_setting);
LV_IMG_DECLARE(ic_launcher_sms);
LV_IMG_DECLARE(ic_launcher_speedtest);
LV_IMG_DECLARE(ic_launcher_ssid);
LV_IMG_DECLARE(ic_launcher_info_device);
LV_IMG_DECLARE(ic_launcher_info_charger);
LV_IMG_DECLARE(ic_list_block_n);
#ifdef FEATURE_BRIGHTNESS
LV_IMG_DECLARE(ic_list_brightness_h);
LV_IMG_DECLARE(ic_list_brightness_l);
#endif
LV_IMG_DECLARE(ic_list_checkbox);
LV_IMG_DECLARE(ic_list_checkbox_selected);
LV_IMG_DECLARE(ic_list_data);
LV_IMG_DECLARE(ic_list_language);
LV_IMG_DECLARE(ic_list_networking);
LV_IMG_DECLARE(ic_list_password);
LV_IMG_DECLARE(ic_list_pin);
LV_IMG_DECLARE(ic_list_profile);
#ifdef REMOTE_WAKEUP
LV_IMG_DECLARE(ic_list_remote);
#endif
LV_IMG_DECLARE(ic_list_remove_n);
LV_IMG_DECLARE(ic_list_restore);
LV_IMG_DECLARE(ic_list_roaming);
LV_IMG_DECLARE(ic_list_sms);
LV_IMG_DECLARE(ic_list_sms_read);
LV_IMG_DECLARE(ic_list_update);
LV_IMG_DECLARE(ic_list_usage_h);
LV_IMG_DECLARE(ic_list_usage_l);
LV_IMG_DECLARE(ic_list_users);
LV_IMG_DECLARE(ic_list_wifi);
#ifdef WIFI_EXTENDER
LV_IMG_DECLARE(ic_list_wifiex);
#endif
LV_IMG_DECLARE(ic_list_wps);
LV_IMG_DECLARE(ic_list_time);
LV_IMG_DECLARE(ic_notifi_badge);
LV_IMG_DECLARE(ic_page_indicator_focus);
LV_IMG_DECLARE(ic_pop_cancel);
LV_IMG_DECLARE(ic_pop_interruptible_1);
LV_IMG_DECLARE(ic_pop_interruptible_2);
LV_IMG_DECLARE(ic_pop_interruptible_3);
LV_IMG_DECLARE(ic_pop_interruptible_4);
LV_IMG_DECLARE(ic_pop_interruptible_5);
LV_IMG_DECLARE(ic_pop_interruptible_6);
LV_IMG_DECLARE(ic_pop_interruptible_7);
LV_IMG_DECLARE(ic_pop_interruptible_8);
LV_IMG_DECLARE(ic_pop_interruptible_9);
LV_IMG_DECLARE(ic_pop_interruptible_10);
LV_IMG_DECLARE(ic_pop_notice_1);
LV_IMG_DECLARE(ic_pop_notice_2);
LV_IMG_DECLARE(ic_pop_notice_3);
LV_IMG_DECLARE(ic_pop_notice_4);
LV_IMG_DECLARE(ic_pop_notice_5);
LV_IMG_DECLARE(ic_pop_notice_6);
LV_IMG_DECLARE(ic_pop_notice_7);
LV_IMG_DECLARE(ic_pop_notice_8);
LV_IMG_DECLARE(ic_pop_notice_9);
LV_IMG_DECLARE(ic_pop_notice_10);
LV_IMG_DECLARE(ic_pop_power);
LV_IMG_DECLARE(ic_pop_question_1);
LV_IMG_DECLARE(ic_pop_question_2);
LV_IMG_DECLARE(ic_pop_question_3);
LV_IMG_DECLARE(ic_pop_question_4);
LV_IMG_DECLARE(ic_pop_question_5);
LV_IMG_DECLARE(ic_pop_question_6);
LV_IMG_DECLARE(ic_pop_question_7);
LV_IMG_DECLARE(ic_pop_question_8);
LV_IMG_DECLARE(ic_pop_question_9);
LV_IMG_DECLARE(ic_pop_question_10);
LV_IMG_DECLARE(ic_pop_restart);
LV_IMG_DECLARE(ic_speedtest_download_wb);
LV_IMG_DECLARE(ic_speedtest_pinspeed_wb);
LV_IMG_DECLARE(ic_speedtest_upload_wb);
LV_IMG_DECLARE(ic_status_battery_charging);
LV_IMG_DECLARE(ic_status_battery_charging_hd);
LV_IMG_DECLARE(ic_status_battery_level01);
LV_IMG_DECLARE(ic_status_battery_level01_hd);
LV_IMG_DECLARE(ic_status_battery_level02);
LV_IMG_DECLARE(ic_status_battery_level02_hd);
LV_IMG_DECLARE(ic_status_battery_level03);
LV_IMG_DECLARE(ic_status_battery_level03_hd);
LV_IMG_DECLARE(ic_status_battery_level04);
LV_IMG_DECLARE(ic_status_battery_level04_hd);
LV_IMG_DECLARE(ic_status_battery_low);
LV_IMG_DECLARE(ic_status_battery_low_hd);
#ifdef BT_SUPPORT
LV_IMG_DECLARE(ic_status_btdevice_level01);
LV_IMG_DECLARE(ic_status_btdevice_level01_hd);
LV_IMG_DECLARE(ic_status_btdevice_level02);
LV_IMG_DECLARE(ic_status_btdevice_level02_hd);
LV_IMG_DECLARE(ic_status_btdevice_level03);
LV_IMG_DECLARE(ic_status_btdevice_level03_hd);
LV_IMG_DECLARE(ic_status_btdevice_level04);
LV_IMG_DECLARE(ic_status_btdevice_level04_hd);
LV_IMG_DECLARE(ic_status_btdevice_level05);
LV_IMG_DECLARE(ic_status_btdevice_level05_hd);
LV_IMG_DECLARE(ic_status_btdevice_level06);
LV_IMG_DECLARE(ic_status_btdevice_level06_hd);
LV_IMG_DECLARE(ic_status_btdevice_level07);
LV_IMG_DECLARE(ic_status_btdevice_level07_hd);
LV_IMG_DECLARE(ic_status_btdevice_level08);
LV_IMG_DECLARE(ic_status_btdevice_level08_hd);
LV_IMG_DECLARE(ic_status_btdevice_level09);
LV_IMG_DECLARE(ic_status_btdevice_level09_hd);
LV_IMG_DECLARE(ic_status_btdevice_level10);
LV_IMG_DECLARE(ic_status_btdevice_level10_hd);
#endif
LV_IMG_DECLARE(ic_status_data_down);
LV_IMG_DECLARE(ic_status_data_down_hd);
LV_IMG_DECLARE(ic_status_data_ud);
LV_IMG_DECLARE(ic_status_data_ud_hd);
LV_IMG_DECLARE(ic_status_data_up);
LV_IMG_DECLARE(ic_status_data_up_hd);
LV_IMG_DECLARE(ic_status_message);
LV_IMG_DECLARE(ic_status_message_hd);
LV_IMG_DECLARE(ic_status_signal_3g);
LV_IMG_DECLARE(ic_status_signal_3g_hd);
LV_IMG_DECLARE(ic_status_signal_4g);
LV_IMG_DECLARE(ic_status_signal_4g_hd);
LV_IMG_DECLARE(ic_status_signal_4gp);
LV_IMG_DECLARE(ic_status_signal_4gp_hd);
LV_IMG_DECLARE(ic_status_signal_5g);
LV_IMG_DECLARE(ic_status_signal_5g_hd);
LV_IMG_DECLARE(ic_status_signal_5g_c);
LV_IMG_DECLARE(ic_status_signal_e);
LV_IMG_DECLARE(ic_status_signal_e_hd);
LV_IMG_DECLARE(ic_status_signal_g);
LV_IMG_DECLARE(ic_status_signal_g_hd);
LV_IMG_DECLARE(ic_status_signal_h);
LV_IMG_DECLARE(ic_status_signal_h_hd);
LV_IMG_DECLARE(ic_status_signal_hp);
LV_IMG_DECLARE(ic_status_signal_hp_hd);
LV_IMG_DECLARE(ic_status_signal_level01);
LV_IMG_DECLARE(ic_status_signal_level01_hd);
LV_IMG_DECLARE(ic_status_signal_level02);
LV_IMG_DECLARE(ic_status_signal_level02_hd);
LV_IMG_DECLARE(ic_status_signal_level03);
LV_IMG_DECLARE(ic_status_signal_level03_hd);
LV_IMG_DECLARE(ic_status_signal_level04);
LV_IMG_DECLARE(ic_status_signal_level04_hd);
LV_IMG_DECLARE(ic_status_signal_level05);
LV_IMG_DECLARE(ic_status_signal_no);
LV_IMG_DECLARE(ic_status_signal_no_hd);
LV_IMG_DECLARE(ic_status_signal_r);
LV_IMG_DECLARE(ic_status_signal_r_hd);
LV_IMG_DECLARE(ic_status_sim);
LV_IMG_DECLARE(ic_status_sim_hd);
LV_IMG_DECLARE(ic_status_sw_update);
LV_IMG_DECLARE(ic_status_wifi_s1);
LV_IMG_DECLARE(ic_status_wifi_s1_hd);
LV_IMG_DECLARE(ic_status_wifi_s2);
LV_IMG_DECLARE(ic_status_wifi_s2_hd);
LV_IMG_DECLARE(ic_status_wifi_s3);
LV_IMG_DECLARE(ic_status_wifi_s3_hd);
LV_IMG_DECLARE(ic_status_y5device_level01);
LV_IMG_DECLARE(ic_status_y5device_level01_hd);
LV_IMG_DECLARE(ic_status_y5device_level02);
LV_IMG_DECLARE(ic_status_y5device_level02_hd);
LV_IMG_DECLARE(ic_status_y5device_level03);
LV_IMG_DECLARE(ic_status_y5device_level03_hd);
LV_IMG_DECLARE(ic_status_y5device_level04);
LV_IMG_DECLARE(ic_status_y5device_level04_hd);
LV_IMG_DECLARE(ic_status_y5device_level05);
LV_IMG_DECLARE(ic_status_y5device_level05_hd);
LV_IMG_DECLARE(ic_status_y5device_level06);
LV_IMG_DECLARE(ic_status_y5device_level06_hd);
LV_IMG_DECLARE(ic_status_y5device_level07);
LV_IMG_DECLARE(ic_status_y5device_level07_hd);
LV_IMG_DECLARE(ic_status_y5device_level08);
LV_IMG_DECLARE(ic_status_y5device_level08_hd);
LV_IMG_DECLARE(ic_status_y5device_level09);
LV_IMG_DECLARE(ic_status_y5device_level09_hd);
LV_IMG_DECLARE(ic_status_y5device_level10);
LV_IMG_DECLARE(ic_status_y5device_level10_hd);
LV_IMG_DECLARE(ic_status_y5device_level11);
LV_IMG_DECLARE(ic_status_y5device_level12);
LV_IMG_DECLARE(ic_status_y5device_level13);
LV_IMG_DECLARE(ic_status_y5device_level14);
LV_IMG_DECLARE(ic_status_y5device_level15);
LV_IMG_DECLARE(ic_status_y5device_level16);
LV_IMG_DECLARE(ic_status_y5device_level17);
LV_IMG_DECLARE(ic_status_y5device_level18);
LV_IMG_DECLARE(ic_status_y5device_level19);
LV_IMG_DECLARE(ic_status_y5device_level20);
LV_IMG_DECLARE(ic_status_y5device_level21);
LV_IMG_DECLARE(ic_status_y5device_level22);
LV_IMG_DECLARE(ic_status_y5device_level23);
LV_IMG_DECLARE(ic_status_y5device_level24);
LV_IMG_DECLARE(ic_status_y5device_level25);
LV_IMG_DECLARE(ic_status_y5device_level26);
LV_IMG_DECLARE(ic_status_y5device_level27);
LV_IMG_DECLARE(ic_status_y5device_level28);
LV_IMG_DECLARE(ic_status_y5device_level29);
LV_IMG_DECLARE(ic_status_y5device_level30);
LV_IMG_DECLARE(ic_status_y5device_level31);
LV_IMG_DECLARE(ic_status_y5device_level32);
LV_IMG_DECLARE(icon_chart_1);
LV_IMG_DECLARE(icon_chart_2);
LV_IMG_DECLARE(icon_chart_3);
LV_IMG_DECLARE(icon_chart_4);
LV_IMG_DECLARE(icon_chart_5);
LV_IMG_DECLARE(icon_chart_6);
LV_IMG_DECLARE(icon_chart_7);
LV_IMG_DECLARE(icon_chart_8);
LV_IMG_DECLARE(icon_chart_9);
LV_IMG_DECLARE(icon_chart_10);
LV_IMG_DECLARE(icon_chart_11);
LV_IMG_DECLARE(icon_chart_12);
LV_IMG_DECLARE(icon_chart_13);
LV_IMG_DECLARE(icon_chart_14);
LV_IMG_DECLARE(icon_chart_15);
LV_IMG_DECLARE(icon_chart_16);
LV_IMG_DECLARE(icon_chart_17);
LV_IMG_DECLARE(icon_chart_18);
LV_IMG_DECLARE(icon_chart_19);
LV_IMG_DECLARE(icon_chart_20);
LV_IMG_DECLARE(icon_chart_21);
LV_IMG_DECLARE(icon_chart_22);
LV_IMG_DECLARE(icon_process_1);
LV_IMG_DECLARE(icon_process_2);
LV_IMG_DECLARE(icon_process_3);
LV_IMG_DECLARE(icon_process_4);
LV_IMG_DECLARE(icon_process_5);
LV_IMG_DECLARE(icon_process_6);
LV_IMG_DECLARE(icon_process_7);
LV_IMG_DECLARE(icon_process_8);
LV_IMG_DECLARE(image_page_to_page);
LV_IMG_DECLARE(Zyxel_logo_2016);
LV_IMG_DECLARE(bootup_3);
LV_IMG_DECLARE(bootup_dlink);
LV_IMG_DECLARE(bootup_zyxel);
LV_IMG_DECLARE(ic_dash_data_bg);
LV_IMG_DECLARE(ic_dash_data_level01);
LV_IMG_DECLARE(ic_dash_data_level02);
LV_IMG_DECLARE(ic_dash_data_level03);
LV_IMG_DECLARE(ic_dash_data_level04);
LV_IMG_DECLARE(ic_dash_data_level05);
LV_IMG_DECLARE(ic_dash_data_status_alert);
LV_IMG_DECLARE(ic_dash_data_status_normal);
LV_IMG_DECLARE(ic_dash_data_status_warning);
LV_IMG_DECLARE(ic_dash_data_unlimited);
LV_IMG_DECLARE(slash_date);
LV_IMG_DECLARE(slash_time);
LV_IMG_DECLARE(ic_dash_battery_alert);
LV_IMG_DECLARE(ic_dash_battery_charging);
LV_IMG_DECLARE(ic_launcher_info_charging);
LV_IMG_DECLARE(ic_launcher_info_low);
LV_IMG_DECLARE(ic_launcher_btn_pressed_01);
LV_IMG_DECLARE(ic_launcher_btn_pressed_02);
LV_IMG_DECLARE(ic_dash_device_connected);
LV_IMG_DECLARE(ic_dash_bettary_low);
LV_IMG_DECLARE(ic_dash_bettary_level04);
LV_IMG_DECLARE(ic_dash_bettary_level03);
LV_IMG_DECLARE(ic_dash_bettary_level02);
LV_IMG_DECLARE(ic_dash_bettary_level01);
LV_IMG_DECLARE(ic_dash_bettary_charging);
LV_IMG_DECLARE(ic_dash_battery);
LV_IMG_DECLARE(ic_dash_battery_low);
LV_IMG_DECLARE(ic_dash_battery_remain_1);
LV_IMG_DECLARE(ic_dash_battery_remain_2);
LV_IMG_DECLARE(ic_dash_battery_remain_3);
LV_IMG_DECLARE(ic_dash_battery_remain_4);
LV_IMG_DECLARE(ic_dash_battery_remain_5);
LV_IMG_DECLARE(ic_dash_battery_remain_6);
LV_IMG_DECLARE(ic_dash_battery_remain_7);
LV_IMG_DECLARE(ic_dash_battery_remain_alert);
LV_IMG_DECLARE(ic_dash_battery_remain_charging);
LV_IMG_DECLARE(ic_dash_battery_remain_normal);
LV_IMG_DECLARE(ic_dashboard_data);
LV_IMG_DECLARE(ic_dashboard_data_remain);
LV_IMG_DECLARE(ic_launcher_return);
LV_IMG_DECLARE(icon_offscreen_charging_7);
LV_IMG_DECLARE(icon_offscreen_charging_6);
LV_IMG_DECLARE(icon_offscreen_charging_5);
LV_IMG_DECLARE(icon_offscreen_charging_4);
LV_IMG_DECLARE(icon_offscreen_charging_3);
LV_IMG_DECLARE(icon_offscreen_charging_2);
LV_IMG_DECLARE(icon_offscreen_charging_1);
// EN Fonts
LV_FONT_DECLARE(Oswald_Medium_14);
LV_FONT_DECLARE(Oswald_Medium_22);
LV_FONT_DECLARE(Oswald_SemiBold_12);
LV_FONT_DECLARE(Oswald_SemiBold_16);
LV_FONT_DECLARE(Oswald_SemiBold_18);
LV_FONT_DECLARE(Oswald_SemiBold_20);
LV_FONT_DECLARE(Oswald_SemiBold_22);
LV_FONT_DECLARE(Oswald_SemiBold_24);
LV_FONT_DECLARE(Oswald_SemiBold_26);
LV_FONT_DECLARE(Oswald_SemiBold_30);
LV_FONT_DECLARE(Oswald_SemiBold_32);
LV_FONT_DECLARE(Oswald_SemiBold_40);
LV_FONT_DECLARE(Oswald_SemiBold_50);
// JP Fonts
LV_FONT_DECLARE(NotoSansCJKjp_Bold_16);
LV_FONT_DECLARE(NotoSansCJKjp_Bold_22);
LV_FONT_DECLARE(NotoSansCJKjp_Bold_24);
LV_FONT_DECLARE(NotoSansCJKjp_Bold_32);
LV_FONT_DECLARE(NotoSansCJKjp_Bold_40);
LV_FONT_DECLARE(NotoSansCJKjp_Medium_14);
LV_FONT_DECLARE(NotoSansCJKjp_Medium_22);
LV_FONT_DECLARE(NotoSansCJKjp_Medium_26);
LV_FONT_DECLARE(MPLUS1p_Bold_16);
LV_FONT_DECLARE(MPLUS1p_Bold_24);
LV_FONT_DECLARE(MPLUS1p_Medium_14);
LV_FONT_DECLARE(MPLUS1p_Medium_26);
// AR Fonts
LV_FONT_DECLARE(NotoSansArabic_ExtraCondensedBold_12);
LV_FONT_DECLARE(NotoSansArabic_ExtraCondensedBold_16);
LV_FONT_DECLARE(NotoSansArabic_ExtraCondensedBold_18);
LV_FONT_DECLARE(NotoSansArabic_ExtraCondensedBold_20);
LV_FONT_DECLARE(NotoSansArabic_ExtraCondensedBold_22);
LV_FONT_DECLARE(NotoSansArabic_ExtraCondensedBold_24);
LV_FONT_DECLARE(NotoSansArabic_ExtraCondensedBold_26);
LV_FONT_DECLARE(NotoSansArabic_ExtraCondensedBold_30);
LV_FONT_DECLARE(NotoSansArabic_ExtraCondensedBold_32);
LV_FONT_DECLARE(NotoSansArabic_ExtraCondensedBold_40);
LV_FONT_DECLARE(NotoSansArabic_ExtraCondensedBold_50);
LV_FONT_DECLARE(NotoSansArabic_ExtraCondensedMedium_14);
LV_FONT_DECLARE(NotoSansArabic_ExtraCondensedMedium_22);
LV_FONT_DECLARE(NotoSansArabic_ExtraCondensedMedium_24);
LV_FONT_DECLARE(NotoSansArabic_ExtraCondensedMedium_26);


enum {
    font_w_regular,
    font_w_bold
};
typedef uint8_t font_weights_t;

enum {
    font_h_12,
    font_h_14,
    font_h_16,
    font_h_18,
    font_h_20,
    font_h_22,
    font_h_24,
    font_h_26,
    font_h_30,
    font_h_32,
    font_h_40,
    font_h_50,
    font_h_count
};
typedef uint8_t font_heights_t;

lv_font_t * get_font(font_weights_t w, font_heights_t h);
lv_font_t * get_locale_font(int locale, font_weights_t w, font_heights_t h);
lv_font_t * get_locale_font_cust(font_weights_t w, font_heights_t h);

bool is_ltr();
bool is_txt_CJK(const char * txt);
bool is_txt_rtl(const char * txt);
bool is_letter_rtl(uint32_t letter);
bool is_txt_line_rtl(const char * txt, uint32_t line_start, uint32_t line_end);

int systemCmd(const char *command);
int read_node_value(char* path, char* value, int size);

void ui_cleanup();
void launch_home_behaviour();

bool is_silent_reboot();
void reset_silent_reboot();
bool is_upgrade_notified();
void set_upgrade_notified();
void start_sleep();
void stop_sleep();
bool fs_ready();
void set_storage_full_promp_flag();
bool storage_full_check();
bool is_task_debug_enabled();
void task_debug_refresh();
bool charge_mode();
bool get_reboot_screen_on();
void set_reboot_screen_on(bool enable);
void convert_mobileapcfg();
void trace_dump();
#ifdef CUST_LUXSHARE
void luxshare_init();
#endif

//for handling string exceed lable width issue
lv_coord_t get_txt_x_size(lv_obj_t * obj, const char * txt);
#define HOR_SPACE    10
#define SMALL_ICON_X 32

#endif /* LV_POCKET_ROUTER_SRC_UTIL_H_ */
