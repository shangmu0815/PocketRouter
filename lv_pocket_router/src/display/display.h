#include <stdbool.h>

#define BACKLIGHT_MAX 255
#define BACKLIGHT_MIN 30

#define SCREEN_ON  1
#define SCREEN_OFF 0
#define SCREEN_TURNING_OFF 2

int get_power_state();
void set_keep_screen_on(bool state);
void reset_screen_timeout();
void reset_timeout_task();
void notify_screen_timeout_changed(int value);
void notify_brightness_changed(int value);
void turn_off_screen();
void turn_on_screen();
void init_display(int s, int b);
void backlight_off();
void charge_mode_turn_on_screen();
void charge_mode_turn_off_screen();
