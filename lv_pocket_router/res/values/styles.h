/*
 * styles.h
 *
 *  Created on: Dec 20, 2018
 *      Author: pingwen_kao
 */

#ifndef LV_POCKET_ROUTER_RES_VALUES_STYLES_H_
#define LV_POCKET_ROUTER_RES_VALUES_STYLES_H_

#include "../../../lvgl/src/lv_misc/lv_color.h"
#include "../../../lvgl/src/lv_core/lv_style.h"

lv_style_t style_scrollbar;
lv_style_t style_win_page;
lv_style_t style_win_header;

lv_style_t style_statusbar_bg;
lv_style_t style_slot_bg;

lv_style_t style_btn_release;
lv_style_t style_btn_pressed;

lv_style_t style_bottom;

void init_pocket_router_style(void);

#endif /* LV_POCKET_ROUTER_RES_VALUES_STYLES_H_ */
