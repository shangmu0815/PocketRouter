#ifndef LV_POCKET_ROUTER_SRC_UTIL_POWER_MANAGER_H_
#define LV_POCKET_ROUTER_SRC_UTIL_POWER_MANAGER_H_

#define MAX_TIME_INTEGER        3  //ie, max is 120s
enum { // in seconds
    TIME_15S = 15000,
    TIME_30S = 30000,
    TIME_60S = 60000,
    TIME_120S = 120000,
    TIME_10MIN = 600000,
    TIME_20MIN = 1200000,
    TIME_30MIN = 1800000
};

#define SCN_OFF_DEFAULT     TIME_60S
#define WIFI_DURA_DEFAULT   TIME_10MIN

int get_brightness();
int get_screen_timeout();
void set_brightness(int brightness);
void set_screen_timeout(int value);

#endif /* LV_POCKET_ROUTER_SRC_UTIL_POWER_MANAGER_H_ */
