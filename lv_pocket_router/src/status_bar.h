
#include "../../lvgl/lvgl.h"
#include <stdbool.h>

#define MAX_STATUS_BAR_LEFT_ICON 11
#define INDEX_TIME 0
#define INDEX_SIM_ABSENT 1
#define INDEX_SIGNAL_STRENGTH 2
#define INDEX_DATA_FLOW  3
#define INDEX_RADIO_TECH 4
#define INDEX_USER_HOTSPOT 5
#define INDEX_USER_BLUETTOTH 6
#define INDEX_UNREAD_MESSAGE 7
#define INDEX_WIFI_BAND 8
#define INDEX_WIFI_BAND_5G 9
#define INDEX_SW_UPDATE 10

#define MAX_STATUS_BAR_RIGHT_ICON 3
#define INDEX_BATTERY_LEVEL 0
#define INDEX_CHARING_ICON 1
#define INDEX_BATTERY_ICON 2

#define MAX_STATUS_BAR_SECOND_LEFT_ICON 1
#define INDEX_CARRIER 0

#define MAX_STATUS_BAR_SECOND_RIGHT_ICON 1
#define INDEX_SSID 0

void update_ssid();
void update_sim_signal();
void update_data_flow();
void update_radio_tech();
#ifdef BT_SUPPORT
void update_bluetooth_user_counter();
#endif
void update_hotspot_user_counter();
void update_unread_message();
void update_wifi_band();
void update_sw_update();
void update_battery_level();

enum statusbar_id {
    STATUSBAR_LEFT,
    STATUSBAR_RIGHT,
    STATUSBAR_SECOND_LEFT,
    STATUSBAR_SECOND_RIGHT
};

typedef struct {
    int x;
    int y;
} Shift;

typedef struct {
    lv_obj_t * img;
    Shift shift;
    int align;
    int h;
    int w;
} Icon;

typedef struct {
    lv_obj_t * con;
    Shift shift;
    int align;
    int h;
    int w;
} Container;

typedef struct {
    bool hidden;
    Container slot;
    Icon image;
    int id;
    void (*func)(void);
} StatusBarInfo;

StatusBarInfo statusbar_left[MAX_STATUS_BAR_LEFT_ICON];
StatusBarInfo statusbar_right[MAX_STATUS_BAR_RIGHT_ICON];

StatusBarInfo statusbar_second_left[MAX_STATUS_BAR_SECOND_LEFT_ICON];
StatusBarInfo statusbar_second_right[MAX_STATUS_BAR_SECOND_RIGHT_ICON];
