#include <stdbool.h>

#define QC_PROMPT_CHECK_INTERVAL              5 // 5 sec

bool qc_prompt_checking();
void quick_charge_prompt();
bool show_quick_charge();
int get_battery_info();
bool is_charging();
void update_led(bool charging);
int get_battery_capacity();
int get_battery_temperature();
int get_charging_current();
int get_cpu_temperature();
int get_driver_workqueue_current();
int get_register_1340();
int get_battery_present();
bool battery_protect_shutdown();
void reset_emergency_temp_high();
int get_emergency_temp_high();
int get_sim_change();
void reset_sim_change();
bool get_battery_protect_flag();
void battery_protect_prompt();
#ifdef BATTERY_OPTIMIZE_SUPPORT
void set_charging_fv(bool enable);
void init_charging_fv();
bool get_bat_opt_notify_flag();
void battery_optimize_prompt();
void update_charging_timer(bool charging);
void battery_optimize_reminder();
#endif
