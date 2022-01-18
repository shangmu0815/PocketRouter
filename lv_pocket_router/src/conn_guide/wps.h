#ifndef LV_POCKET_ROUTER_SRC_CONN_GUIDE_WPS_H_
#define LV_POCKET_ROUTER_SRC_CONN_GUIDE_WPS_H_

#include "../../../lvgl/lvgl.h"
void wps_create(void);
void wps_conn_pin_create(void);
void wps_conn_create(const char *);
void wps_connect_task();
void destroy();
void est_wps_conn_popup();
void est_wps_conn_popup_task();
lv_res_t wps_conn_oot_action();

#endif /* LV_POCKET_ROUTER_SRC_CONN_GUIDE_WPS_H_ */
