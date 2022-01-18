/*
 * time_setting.h
 *
 */

#ifndef LV_POCKET_ROUTER_SRC_SETTINGS_TIME_SETTING_H_
#define LV_POCKET_ROUTER_SRC_SETTINGS_TIME_SETTING_H_
enum TIME_SETTING_CHK_TYPES {
    YEAR_TYPE,
    MONTH_DATE_TYPE,
    HOUR_MIN_TYPE,
    AM_PM_TYPE,
    UNKNOWN_TYPE
};
struct time
{
  int sec;
  int min;
  int hour;
  int day;
  int mon;
  int year;
};
char* get_month_date();
char* get_hour_minute();
void year_kb_action(lv_obj_t * btnm, lv_event_t event);
void month_date_kb_action(lv_obj_t * btnm, lv_event_t event);
void hour_minute_kb_action(lv_obj_t * btnm, lv_event_t event);
void year_create();
void month_date_create();
void hour_minute_create();
void am_pm_btn_action(lv_obj_t * btn, lv_event_t event);
void am_pm_confirm_action(lv_obj_t * btn, lv_event_t event);
void am_pm_create();
void year_action(lv_obj_t * btn, lv_event_t event);
void month_date_action(lv_obj_t * btn, lv_event_t event);
void hour_minute_action(lv_obj_t * btn, lv_event_t event);
void am_pm_action(lv_obj_t * btn, lv_event_t event);
void update_year_label();
void update_month_date_label();
void update_hour_minute_label();
void update_am_pm_label();
void time_setting_label_refresh();
void time_setting_label_refresh_task();
void time_setting_close_cb(lv_obj_t * btn, lv_event_t event);
void time_setting_create();
char* get_current_year();
char* get_current_month();
char* get_current_date();
char* get_current_hour();
char* get_current_hour_24hformat();
char* get_current_minute();
char* get_current_sec();
char* get_current_am_pm();
int hour_convert_check(int hr, int am_pm_type);
void set_time(struct time *datetime);
void time_setting_err_action(lv_obj_t * mbox, lv_event_t event);
void time_setting_err_notice(int type);
#endif /* LV_POCKET_ROUTER_SRC_SETTINGS_TIME_SETTING_H_ */
