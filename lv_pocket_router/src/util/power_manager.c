#include "power_manager.h"
#include <stdio.h>

#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/display/display.h"

void set_screen_timeout(int value /* ms */) {
    int timeout = (value > 0)? value : TIME_60S;
    ds_set_int(DS_KEY_SCREEN_OFF_TIME, timeout);
    notify_screen_timeout_changed(timeout);
}

int get_brightness() {
    return ds_get_int(DS_KEY_BRIGHTNESS);
}

void set_brightness(int value) {
    // Set min as 30
    int brightness = (value>BACKLIGHT_MIN)?
        (value>BACKLIGHT_MAX? BACKLIGHT_MAX:value) : BACKLIGHT_MIN;

    ds_set_int(DS_KEY_BRIGHTNESS, brightness);
    notify_brightness_changed(brightness);
}

int get_screen_timeout() {
    if(charge_mode()){
        return TIME_60S;
    }else{
        int timeout = ds_get_int(DS_KEY_SCREEN_OFF_TIME);
        if (timeout == 0) timeout = SCN_OFF_DEFAULT;
        return timeout;
    }
}
