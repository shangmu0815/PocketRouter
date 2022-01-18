
#include "../../lvgl/lvgl.h"
#include <stdbool.h>

typedef enum {
    DASHBOARD_BG,
    LAUNCHER_BG
} status_bg_view;

void refresh_status_bar_list(int id);
void create_statusbar(lv_obj_t * root, lv_color_t font_color, lv_color_t bg_color, status_bg_view bg_view);
void statusbar_init();
void statusbar_reset();
void update_ssid_address();
void update_battery_label();
