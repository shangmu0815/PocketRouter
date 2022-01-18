/*
 * string.c
 *
 *  Created on: Dec 11, 2018
 *      Author: pingwen_kao
 */
#include "lv_pocket_router/res/values/string_value.h"
#include "string_table.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/debug_log.h"

static int rtl_direction[] = {AR};

int current_locale = EN;
bool ltr = true;

bool isltr() {
    return ltr;
}

void update_directionality(int locale) {
    int i;
    int rtl_count = sizeof(rtl_direction) / sizeof(int);

    for (i = 0; i < rtl_count; i++) {
        if (rtl_direction[i] == locale) {
            ltr = false;
            return;
        }
    }
    ltr = true;
}

void set_device_locale(int locale) {
    current_locale = locale;
    update_directionality(current_locale);
    ds_set_int(DS_KEY_LANGUAGE, locale);
    return;
}

int get_device_locale() {
    return current_locale;
}

void config_locale() {
    current_locale = ds_get_int(DS_KEY_LANGUAGE);
    update_directionality(current_locale);
    log_d("config locale to %d", current_locale);
}

const char* get_string_impl(int res_id, int locale_id) {
    int locale = (locale_id < 0) ? get_device_locale() : locale_id;
    if (string_table[res_id][locale][0] == 0) {
        return string_table[res_id][EN];
    } else {
        return string_table[res_id][locale];
    }
}

const char* get_string(int res_id) {
    return get_string_impl(res_id, -1);
}
const char* get_string_locale(int res_id, int locale) {
    return get_string_impl(res_id, locale);
}
