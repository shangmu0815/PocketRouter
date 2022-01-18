#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/util/led.h"
#include "lv_pocket_router/src/util/power_menu.h"

typedef enum {
    LED_STATE_OFF,
    LED_STATE_POWER_OFF,
    LED_STATE_NO_POWER,
    LED_STATE_BATTERY_LOW,
    LED_STATE_RECHARGE,
    LED_STATE_BATTERY_FULL,
    LED_STATE_NETWORK_DISCONNECT,
    LED_STATE_5G,
    LED_STATE_LTE
} led_state_t;

typedef enum {
    LED_OFF,
    LED_RED,
    LED_GREEN,
    LED_ORANGE
} led_color_t;

typedef enum {
    LED_ON,
    LED_BLINK,
    LED_QUICK_BLINK
} led_action_t;

static led_state_t led_state = LED_STATE_OFF;
lv_task_t * toggle_task = NULL;


void led_off() {
    // reset toggle timer
    if(toggle_task != NULL){
        lv_task_del(toggle_task);
        toggle_task = NULL;
    }

    // turn off led red
    systemCmd("echo 0x0 > /sys/class/gpio/gpio97/value");
    // turn off led green
    systemCmd("echo 0xC440 > /sys/kernel/debug/regmap/spmi0-08/address");
    systemCmd("echo 0x0 > /sys/kernel/debug/regmap/spmi0-08/data");
    systemCmd("echo 0xbe46 > /sys/kernel/debug/regmap/spmi0-09/address");
    systemCmd("echo 0x0 > /sys/kernel/debug/regmap/spmi0-09/data");
    // turn off led red blink
    systemCmd("/oem/script/led_off.sh 97");
}

void led_orange() {
    led_off();
    led_red_on(LED_ON);
    led_green_on(LED_ON);
}

void led_toggle() {
    if (led_state != LED_STATE_NETWORK_DISCONNECT) return;

    static led_color_t color = LED_OFF;
    if (color == LED_RED) {
        led_green();
        color = LED_GREEN;
    } else {
        led_red();
        color = LED_RED;
    }
    toggle_task = lv_task_create(led_toggle, 1000, LV_TASK_PRIO_LOW, NULL);
    lv_task_once(toggle_task);
}

void led_red() {
    led_red_action(LED_ON);
}

void led_red_action(led_action_t action) {
    led_off();
    led_red_on(action);
}

void led_red_on(led_action_t action) {
    if (action == LED_BLINK) {
        systemCmd("/oem/script/led_start.sh 97 1 1&");
    } else if (action == LED_QUICK_BLINK) {
        systemCmd("/oem/script/led_start.sh 97 0.5 0.5&");
    } else {
        // turn on led red
        systemCmd("echo 97 > /sys/class/gpio/export");
        systemCmd("echo out > /sys/class/gpio/gpio97/direction");
        systemCmd("echo 0x1 > /sys/class/gpio/gpio97/value");
    }
}

void led_green() {
    led_green_action(LED_ON);
}

void led_green_action(led_action_t action) {
    led_off();
    led_green_on(action);
}

void led_green_on(led_action_t action) {
    // turn on led green
    systemCmd("echo 0xC446 > /sys/kernel/debug/regmap/spmi0-08/address");
    systemCmd("echo 0x80 > /sys/kernel/debug/regmap/spmi0-08/data");
    systemCmd("echo 0xC444 > /sys/kernel/debug/regmap/spmi0-08/address");
    systemCmd("echo 0x2 > /sys/kernel/debug/regmap/spmi0-08/data");
    systemCmd("echo 0xC440 > /sys/kernel/debug/regmap/spmi0-08/address");
    systemCmd("echo 0x1 > /sys/kernel/debug/regmap/spmi0-08/data");
    systemCmd("echo 0xbe41 > /sys/kernel/debug/regmap/spmi0-09/address");
    if (action == LED_BLINK || action == LED_QUICK_BLINK) {
        systemCmd("echo 0x01 > /sys/kernel/debug/regmap/spmi0-09/data");
    } else { //LED_ON
        systemCmd("echo 0x03 > /sys/kernel/debug/regmap/spmi0-09/data");
    }
    systemCmd("echo 0xbe42 > /sys/kernel/debug/regmap/spmi0-09/address");
    if (action == LED_BLINK) {
        systemCmd("echo 0x04 > /sys/kernel/debug/regmap/spmi0-09/data");
    } else if (action == LED_QUICK_BLINK) {
        systemCmd("echo 0x03 > /sys/kernel/debug/regmap/spmi0-09/data");
    } else { //LED_ON
        systemCmd("echo 0x01 > /sys/kernel/debug/regmap/spmi0-09/data");
    }
    systemCmd("echo 0xbe44 > /sys/kernel/debug/regmap/spmi0-09/address");
    systemCmd("echo 0x62 > /sys/kernel/debug/regmap/spmi0-09/data");
    systemCmd("echo 0xbe45 > /sys/kernel/debug/regmap/spmi0-09/address");
    systemCmd("echo 0x08 > /sys/kernel/debug/regmap/spmi0-09/data");
    systemCmd("echo 0xbe46 > /sys/kernel/debug/regmap/spmi0-09/address");
    systemCmd("echo 0x80 > /sys/kernel/debug/regmap/spmi0-09/data");
    systemCmd("echo 0xbe47 > /sys/kernel/debug/regmap/spmi0-09/address");
    systemCmd("echo 0x01 > /sys/kernel/debug/regmap/spmi0-09/data");
}

void power_off_led_complete() {
    log_d("led: power off led flash completed, do power off");
    led_off();
    power_off();
}


bool state_change(led_state_t state) {
    if (led_state == state) return;

    led_state = state;

    switch (state) {
        case LED_STATE_POWER_OFF:
            log_d("led: Power Off");
            led_green_action(LED_BLINK);
            lv_task_t * pwr_off_task = lv_task_create(power_off_led_complete, 2700, LV_TASK_PRIO_LOW, NULL);
            lv_task_once(pwr_off_task);
            break;
        case LED_STATE_NO_POWER:
            log_d("led: No Power");
            led_red_action(LED_QUICK_BLINK);
            break;
        case LED_STATE_BATTERY_LOW:
            log_d("led: Battery Low");
            led_red_action(LED_BLINK);
            break;
        case LED_STATE_RECHARGE:
            log_d("led: Recharge");
            led_red();
            break;
        case LED_STATE_BATTERY_FULL:
            log_d("led: Battery Full");
            led_green();
            break;
        case LED_STATE_NETWORK_DISCONNECT:
            log_d("led: Network Disconnected");
            led_toggle();
            break;
        case LED_STATE_5G:
            log_d("led: 5G");
            led_green();
            break;
        case LED_STATE_LTE:
            log_d("led: LTE");
            led_orange();
            break;
        case LED_STATE_OFF:
            log_d("led: off");
            led_off();
            break;
    }
}

void update_custom_led() {
    // priority 1
    if (!is_charging() && get_battery_info() == 0) {
        state_change(LED_STATE_NO_POWER);
        return;
    }
    if (get_reboot_screen_on()) {
        state_change(LED_STATE_POWER_OFF);
        return;
    }

    // priority 2
    if (!is_charging() && get_battery_info() <= 10) {
        state_change(LED_STATE_BATTERY_LOW);
        return;
    }
    if (is_charging() && (0 == get_register_1340())) {
        if (get_battery_info() < 100) {
            state_change(LED_STATE_RECHARGE);
        } else {
            state_change(LED_STATE_BATTERY_FULL);
        }
        return;
    }

    // priority 3
    if (!is_wwan_connected()) {
        state_change(LED_STATE_NETWORK_DISCONNECT);
        return;
    } else {
        int radioTech = get_radio_tech();
        if (radioTech == DATA_BEARER_TECH_TYPE_5G) {
            state_change(LED_STATE_5G);
            return;
        } else if (radioTech == DATA_BEARER_TECH_TYPE_LTE) {
            state_change(LED_STATE_LTE);
            return;
        }
    }

    // none of above case, then turn off led
    state_change(LED_STATE_OFF);
}
