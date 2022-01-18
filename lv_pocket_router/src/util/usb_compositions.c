#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "lv_pocket_router/src/util/usb_compositions.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/util/popup_box.h"

#define USB_DEBUG_CMD                   "sh sbin/usb/compositions/901D"
#define USB_MASS_STORAGE_CMD            "sh sbin/usb/compositions/F000"
#define USB_TETHER_WO_ADB_CMD           "sh sbin/usb/compositions/9057"
#define USB_TETHER_WITH_ADB_CMD         "sh sbin/usb/compositions/9024"
#define USB_TETHER_MAC_CMD              "sh sbin/usb/compositions/9064"

#ifdef USB_DEBUG_ENABLING
#define USB_TETHER_CMD                  USB_TETHER_WO_ADB_CMD
#define USB_TETHER_CANCEL_CMD           USB_MASS_STORAGE_CMD
#else
#define USB_TETHER_CMD                  USB_TETHER_WITH_ADB_CMD
#define USB_TETHER_CANCEL_CMD           USB_DEBUG_CMD
#endif

static lv_obj_t * usb_comp_obj = NULL;
static uint32_t last_usb_comp_t = 0;

void start_adbd() {
#if USB_COMPOSITIONS
#ifdef USB_DEBUG_ENABLING
    if (usb_debug_enabled()) {
        lv_task_t * task = lv_task_create(enable_usb_debug, 1000, LV_TASK_PRIO_MID, NULL);
        lv_task_once(task);
    }
#endif

    FILE *fp = popen("ps | grep [a]dbd", "r");
    if (fp == NULL) {
        log_e("error ps grep for adbd");
        return;
    }

    char buffer[100];
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        systemCmd(USB_TETHER_CMD);
    }
    pclose(fp);
#endif
}

bool usb_debug_enabled() {
    return ds_get_bool(DS_KEY_USB_DEBUG);
}

void enable_usb_debug() {
#ifdef FEATURE_ROUTER
    int res = systemCmd("echo 901D > /etc/usb/boot_hsusb_comp");
    if (res == 0) {
        res = systemCmd(USB_DEBUG_CMD);
        if (res == 0) {
            ds_set_bool(DS_KEY_USB_DEBUG, true);
        }
    }
#endif
}

usb_state_t read_usb_state() {
    int fd = open("/sys/devices/platform/a600000.ssusb/a600000.dwc3/udc/a600000.dwc3/state", O_RDONLY);
    if (fd < 0) {
        log_e("open usb state node error");
        return USB_STATE_UNKNOWN;
    }

    char buf[20];
    int len = read(fd, buf, sizeof(buf));
    if (len < 0) {
        log_e("error reading usb state node");
        close(fd);
        return USB_STATE_UNKNOWN;
    }
    close(fd);

    if(strncmp(buf, "configured", strlen("configured")) == 0) {
        return USB_STATE_CONFIGURED;
    } else if(strncmp(buf, "not attached", strlen("not attached")) == 0) {
        return USB_STATE_NOT_ATTACHED;
    } else if(strncmp(buf, "default", strlen("default")) == 0) {
        return USB_STATE_DEFAULT;
    } else if(strncmp(buf, "addressed", strlen("addressed")) == 0) {
        return USB_STATE_ADDRESSED;
    }
    return USB_STATE_UNKNOWN;
}

void enable_usb_tether_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(mbox);
    if (strcmp(txt, get_string(ID_OK)) == 0 || strcmp(txt, get_string(ID_USB_TETHER_OTHERS)) == 0) {
        systemCmd(USB_TETHER_CMD);
    } else if (strcmp(txt, get_string(ID_USB_TETHER_MAC)) == 0) {
        systemCmd(USB_TETHER_MAC_CMD);
    } else {
#ifdef USB_DEBUG_ENABLING
        char *cmd = (usb_debug_enabled() ? USB_DEBUG_CMD : USB_TETHER_CANCEL_CMD);
        systemCmd(cmd);
#else
        systemCmd(USB_TETHER_CANCEL_CMD);
#endif
    }
    last_usb_comp_t = lv_tick_get();
    usb_comp_obj = NULL;
    close_popup();
}

void usb_compositions_close(void) {
#if USB_COMPOSITIONS
    if (usb_comp_obj != NULL) {
        usb_comp_obj = NULL;
        close_popup();
    }
#endif
}

void usb_compositions_create(void) {
#if USB_COMPOSITIONS
    if(is_static_popup()){
        log_d("static popup exist, skip usb compositions create");
        return;
    }
    if (usb_comp_obj == NULL) {
        // skip show usb popup if receives usb event within 3 sec
        uint32_t t = lv_tick_elaps(last_usb_comp_t);
        if (t < 3000) {
            return;
        }
        log_d("last usb compositions popup elapsed time is %d ms", t);

        static const char * btns[3];
#ifdef CUST_DLINK
        btns[0] = get_string(ID_USB_TETHER_OTHERS);
        btns[1] = get_string(ID_USB_TETHER_MAC);
#else
        btns[0] = get_string(ID_CANCEL);
        btns[1] = get_string(ID_OK);
#endif
        btns[2] = "";

        usb_comp_obj = popup_scrl_create_impl(get_string(ID_USB_TETHER_PROMPT_HEADER),
                             get_string(ID_USB_TETHER_PROMPT),
                             btns, enable_usb_tether_action, NULL);
        set_popup_cb(usb_compositions_close);
    }
#endif
}

#ifdef CUST_LUXSHARE
void luxshare_usb_composition_init() {
    systemCmd(USB_TETHER_WITH_ADB_CMD);
    systemCmd("echo 9024 > /etc/usb/boot_hsusb_comp");
}
#endif
