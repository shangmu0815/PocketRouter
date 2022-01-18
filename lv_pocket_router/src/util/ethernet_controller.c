#include <fcntl.h>
#include "lv_pocket_router/src/util/ethernet_controller.h"

static lv_obj_t * lv_insert_ethernet_task = NULL;

bool is_ethernet_connected() {
    FILE *fp = NULL;
    fp = popen("ethtool eth0 | grep detected", "r");
    if (fp == NULL) {
        log_e("error dump ethtool info");
        return false;
    }

    char buffer[50];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    pclose(fp);

    log_d("ethernet connection: %s", buffer);

    char* state = strstr(buffer, "Link detected: ");
    if (state != NULL &&
        strncmp(state + strlen("Link detected: "), "yes", strlen("yes")) == 0) {
        return true;
    }
    return false;
}

void stop_insert_task() {
    if (lv_insert_ethernet_task != NULL) {
        lv_task_del(lv_insert_ethernet_task);
        lv_insert_ethernet_task = NULL;
    }
}

void remove_ethernet_module() {
#if USE_FBDEV || USE_ANDROID_FBDEV
    stop_insert_task();
    if (!is_ethernet_connected()) {
        log_d("remove ethernet module");
        //systemCmd("rmmod emac_dwc_eqos.ko");
        systemCmd("ethtool -s eth0 wol d");
    }
#endif
}

void insert_ethernet() {
    lv_insert_ethernet_task = NULL;
    log_d("insert ethernet module");
    //systemCmd("insmod /lib/modules/4.14.117-perf/extra/emac_dwc_eqos.ko");
    systemCmd("ethtool -s eth0 wol g");
}

void insert_ethernet_module() {
    stop_insert_task();
    lv_insert_ethernet_task = lv_task_create(insert_ethernet, 3000, LV_TASK_PRIO_LOW, NULL);
    lv_task_once(lv_insert_ethernet_task);
}

void remove_ethernet_kmod() {
    if (systemCmd("rmmod emac_dwc_eqos.ko") != 0) {
        log_e("Remove Ethernet module failed!");
    }
}
