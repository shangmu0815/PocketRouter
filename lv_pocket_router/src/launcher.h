#ifndef LV_POCKET_ROUTER_SRC_LAUNCHER_H_
#define LV_POCKET_ROUTER_SRC_LAUNCHER_H_

#include "../../lvgl/lvgl.h"
#include "../../lvgl/src/lv_objx/lv_imgbtn.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../res/values/styles.h"
#include <stdbool.h>

void update_data_usage_bar();
void launcher_create(void);
void update_sms_num();
void launcher_destroy();

#define DOWNLOAD_TEST   0

enum MENU_INDEX {
    INDEX_CONN_GUIDE,
    INDEX_POWER_SAVING,
    INDEX_SSID_SETTING,
    INDEX_SMS,
    INDEX_SETTINGS,
    INDEX_ABOUT,
    INDEX_SPEEDTEST,
#if DOWNLOAD_TEST == 1
    INDEX_DOWNLOADTEST,
#endif
    INDEX_COUNT
};
#define DELAY_DEL_LAUNCHER ((uint32_t)300)

#endif /* LV_POCKET_ROUTER_SRC_LAUNCHER_H_ */
