/*
 * styles.c
 *
 *  Created on: Dec 20, 2018
 *      Author: pingwen_kao
 */
#include "styles.h"

void init_pocket_router_style(void){
    /*Create a scroll bar style*/
    lv_style_copy(&style_scrollbar, &lv_style_plain);
    style_scrollbar.body.main_color = LV_COLOR_BLACK;
    style_scrollbar.body.grad_color = LV_COLOR_BLACK;
    style_scrollbar.body.border.color = LV_COLOR_WHITE;
    style_scrollbar.body.border.width = 1;
    style_scrollbar.body.border.opa = LV_OPA_70;
    style_scrollbar.body.padding.left = 0;
    style_scrollbar.body.padding.right = 0;
    style_scrollbar.body.padding.top = 0;
    style_scrollbar.body.padding.bottom = 0;
    style_scrollbar.body.padding.inner = 0;
    style_scrollbar.body.radius = LV_RADIUS_CIRCLE;
    style_scrollbar.body.opa = LV_OPA_60;

    lv_style_copy(&style_win_page, &lv_style_plain);
    style_win_page.body.main_color = lv_color_hex(0xD6E4F9);
    style_win_page.body.grad_color = lv_color_hex(0x487fb7);
    style_win_page.text.color = LV_COLOR_BLACK;
    style_win_page.body.padding.inner = 0;
    style_win_page.body.padding.left = 0;
    style_win_page.body.padding.right = 0;
    style_win_page.body.padding.top = 0;
    style_win_page.body.padding.bottom = 0;

    lv_style_copy(&style_win_header, &lv_style_plain);
    style_win_header.body.main_color = LV_COLOR_RED;
    style_win_header.body.grad_color = LV_COLOR_MAROON;
    style_win_header.body.padding.top = 0;
    style_win_header.body.padding.bottom = 0;
    style_win_header.body.padding.inner = 0;
    style_win_header.text.color = LV_COLOR_BLACK;

    lv_style_copy(&style_btn_release, &lv_style_btn_rel);
    style_btn_release.body.radius = 0;
    style_btn_release.text.color = LV_COLOR_WHITE;
    style_btn_release.line.rounded = 0;

    lv_style_copy(&style_btn_pressed, &lv_style_btn_pr);
    style_btn_pressed.body.radius = 0;
    style_btn_pressed.text.color = LV_COLOR_WHITE;
    style_btn_pressed.line.rounded = 0;


    lv_style_copy(&style_statusbar_bg, &lv_style_plain);
    style_statusbar_bg.body.main_color = lv_color_hex(0x00000000);
    style_statusbar_bg.body.grad_color = lv_color_hex(0x00000000);
    style_statusbar_bg.body.padding.top = 0;
    style_statusbar_bg.text.color = lv_color_hex(0xffffff);

    lv_style_copy(&style_slot_bg, &lv_style_plain);
    style_slot_bg.body.main_color = lv_color_hex(0x00000000);
    style_slot_bg.body.grad_color = lv_color_hex(0x00000000);
    style_slot_bg.text.color = LV_COLOR_WHITE;

    lv_style_copy(&style_bottom, &lv_style_pretty_color);
    style_bottom.line.rounded = 0;
    style_bottom.body.radius = 0;
    style_bottom.body.padding.inner = 0;
    style_bottom.body.padding.left = 0;
    style_bottom.body.padding.right = 0;
    style_bottom.body.padding.top = 0;
    style_bottom.body.padding.bottom = 0;

}
