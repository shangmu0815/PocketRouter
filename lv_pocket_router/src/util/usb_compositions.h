#ifndef LV_POCKET_ROUTER_SRC_UTIL_USB_COMPOSITIONS_H_
#define LV_POCKET_ROUTER_SRC_UTIL_USB_COMPOSITIONS_H_
#include "../../../lvgl/lvgl.h"
#include "lv_pocket_router/src/util/util.h"

void usb_compositions_create(void);
void usb_compositions_close(void);

typedef enum {
    USB_STATE_UNKNOWN           = 0,
    USB_STATE_NOT_ATTACHED      = 1, // not attached
    USB_STATE_DEFAULT           = 2, // default
    USB_STATE_ADDRESSED         = 3, // addressed
    USB_STATE_CONFIGURED        = 4, // configured
} usb_state_t;
usb_state_t read_usb_state();

void start_adbd();
bool usb_debug_enabled();
void enable_usb_debug();
#ifdef CUST_LUXSHARE
void luxshare_usb_composition_init();
#endif

#endif /* LV_POCKET_ROUTER_SRC_UTIL_USB_COMPOSITIONS_H_ */
