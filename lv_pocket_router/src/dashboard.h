#ifndef LV_POCKET_ROUTER_SRC_DASHBOARD_H_
#define LV_POCKET_ROUTER_SRC_DASHBOARD_H_
#include <stdio.h>
#include "launcher.h"
#include "../res/values/styles.h"

#define DELAY_DEL_DB ((uint32_t)300)

void init_dashboard_style(void);
void dashboard_create(void);
void db_statusbar_create(void);
void db_main_create(void);
void db_cleanup();
void db_callback(lv_obj_t * tabview, uint16_t act_id);
void db_set_img_src(lv_obj_t * img, const void * src_img);
void check_password(char * pwd);
void set_inner_pb(int is_left, int p);
void db_destroy();
const char* abbr_weekday_name(char* weekday_name);

enum DB_CIR_MAP {
    DB_LEFT_CIR,
    DB_MID_CIR,
    DB_RIGHT_CIR
};

#endif /* LV_POCKET_ROUTER_SRC_DASHBOARD_H_ */
