/*
 * remote_wakeup.c
 *
 *  Created on: Mar 28, 2019
 *      Author: joseph
 */
#include <stdio.h>
#include <stdbool.h>
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/settings/remote_wakeup.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/util.h"

#define DS_KEY_ENABLE_REMOTE_WAKEUP     "enable_remote_wakeup"
lv_obj_t * rw_sw;

void remote_wakeup_action(lv_obj_t * sw, lv_event_cb_t event_cb)
{
    if (event_cb != LV_EVENT_CLICKED) return;

    printf("lv_sw_get_state(sw): %d\n", lv_sw_get_state(sw));
    if (lv_sw_get_state(sw)) {
        ds_set_bool(DS_KEY_ENABLE_REMOTE_WAKEUP, true);
        lv_sw_on(rw_sw, LV_ANIM_OFF);
    } else {
        ds_set_bool(DS_KEY_ENABLE_REMOTE_WAKEUP, false);
        lv_sw_off(rw_sw, LV_ANIM_OFF);
    }
    return LV_RES_OK;
}

void init_remote_wakeup(void) {

    lv_obj_t * win = default_list_header (lv_scr_act(), get_string(ID_REMOTE_WAKEUP), lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);
    lv_obj_align(list, win, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, 60 * LV_RES_OFFSET);
    lv_obj_align(list, win, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    //add list element in order

    //liste_enable_pin
    lv_obj_t * liste = lv_liste_w_switch(list, get_string(ID_REMOTE_WAKEUP), remote_wakeup_action);
    rw_sw = lv_obj_get_child(liste, NULL);

    if (ds_get_bool(DS_KEY_ENABLE_REMOTE_WAKEUP)) {
        lv_sw_on(rw_sw, LV_ANIM_OFF);
    } else {
        lv_sw_off(rw_sw, LV_ANIM_OFF);
    }

    //liste_enable_pin_note
    static lv_style_t style_font;
    lv_style_copy(&style_font, &lv_style_plain);
    style_font.text.font = get_font(font_w_bold, font_h_16);
    style_font.text.color = LV_COLOR_GREYISH_BROWN;
    style_font.text.letter_space = 1;

    lv_obj_t * remote_wakeup_note_label = lv_label_create(win, NULL);
    lv_label_set_long_mode(remote_wakeup_note_label, LV_LABEL_LONG_BREAK);
    lv_obj_set_size(remote_wakeup_note_label, 288, 120);
    lv_label_set_text(remote_wakeup_note_label, get_string(ID_REMOTE_WAKEUP_NOTE));
    lv_label_set_style(remote_wakeup_note_label, LV_LABEL_STYLE_MAIN, &style_font);
    lv_obj_align(remote_wakeup_note_label, win, LV_ALIGN_IN_TOP_LEFT, 16, 120);
}
