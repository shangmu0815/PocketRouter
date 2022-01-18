#ifndef LV_POCKET_ROUTER_SRC_SETTINGS_SIM_NETWORK_CONTROLLER_H_
#define LV_POCKET_ROUTER_SRC_SETTINGS_SIM_NETWORK_CONTROLLER_H_

#define SIM_BLACKLIST_HEADER      "blacklist"
#define SIM_BLACKLIST_MCC_HEADER  "mcc"
#define SIM_BLACKLIST_MNC_HEADER  "mnc"

enum {
    AIRPLANE_MODE_OFF = 1,
    AIRPLANE_MODE_ON = 4,
};

bool get_airplane_mode_status();
bool set_airplane_mode(int val);
void check_airplane_mode();
void init_airplane_mode();

void check_blacklist_sim();
bool is_blacklist_sim();

#endif /* LV_POCKET_ROUTER_SRC_SETTINGS_SIM_NETWORK_CONTROLLER_H_ */
