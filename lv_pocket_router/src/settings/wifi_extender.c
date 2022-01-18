#include <stdio.h>
#include <stdbool.h>
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/settings/wifi_extender.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/util.h"

lv_obj_t * mbox;
int loading;
static lv_style_t style_bg;
lv_obj_t * wifi_extender_sw;
lv_obj_t * wifi_extender;

#define BT_TURNS_ON false

void wifi_extender_sw_release_action(lv_obj_t * sw, lv_event_cb_t event_cb)
{
    if (event_cb != LV_EVENT_CLICKED) return;

#ifdef CUST_SWITCH
    set_cust_switch_state(wifi_extender, DS_KEY_WIFI_EXTENDER);
    bool enable = ds_get_bool(DS_KEY_WIFI_EXTENDER);
    if(enable){
        wifi_extender_loading_animation();
    }else{
        wifi_extender_available_or_unavailable_anim(false);
    }
#else
    if (lv_sw_get_state(sw) == true) {
        ds_set_bool(DS_KEY_WIFI_EXTENDER, true);
        lv_sw_on(wifi_extender_sw, LV_ANIM_OFF);
        wifi_extender_loading_animation();
    }
    if (lv_sw_get_state(sw) == false) {
        ds_set_bool(DS_KEY_WIFI_EXTENDER, false);
        lv_sw_off(wifi_extender_sw, LV_ANIM_OFF);
        wifi_extender_available_or_unavailable_anim(false);
    }
#endif
}

void init_wifi_extender(void) {
    if (BT_TURNS_ON == true) {
        wifi_extender_with_bt();
    } else {
        wifi_extender_with_no_bt();
    }
}

void wifi_extender_with_bt(void) {
    info_page_create_label_align_center(lv_scr_act(), get_string(ID_WIFI_EXTENDER),
            get_string(ID_WIFI_ERR_DUE_TO_BT_ON));
}

void wifi_extender_with_no_bt(void) {
    liste_style_create();

    lv_obj_t * win = default_list_header (lv_scr_act(), get_string(ID_WIFI_EXTENDER), lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    bool enable = ds_get_bool(DS_KEY_WIFI_EXTENDER);
#ifdef CUST_SWITCH
    wifi_extender = lv_liste_cust_switch(list, get_string(ID_WIFI_EXTENDER),
            wifi_extender_sw_release_action, enable);
#else
    wifi_extender = lv_liste_w_switch(list, get_string(ID_WIFI_EXTENDER), wifi_extender_sw_release_action);
    wifi_extender_sw = lv_obj_get_child(wifi_extender, NULL);
    if (enable) {
        lv_sw_on(wifi_extender_sw, LV_ANIM_OFF);
    } else {
        lv_sw_off(wifi_extender_sw, LV_ANIM_OFF);
    }
#endif
}


//TODO draft version, need modify later
void wifi_extender_loading() {
    printf("wifi_extender_loading %d \n", loading);

    if (loading <= 100) {
        update_loading_bar(mbox, loading);
        loading++;
    } else {
        //close task and popup when finish
        close_popup();
        wifi_extender_available_or_unavailable_anim(true);
    }
}

void wifi_extender_loading_animation(void) {
    loading = 0;
     mbox = popup_anim_loading_create(get_string(ID_WIFI_EXTENDER), get_string(ID_LOADING));
     popup_loading_task_create(wifi_extender_loading, 50, LV_TASK_PRIO_MID, NULL);
}

void wifi_extender_available_or_unavailable_anim(bool wifi_extender_config) {
    lv_obj_t * mbox1 = lv_mbox_create(lv_scr_act(), NULL);
    if(wifi_extender_config== true){
        lv_mbox_set_text(mbox1, get_string(ID_WIFI_EXTENDER_AVAILABLE_NW));
    }else{
        lv_mbox_set_text(mbox1, get_string(ID_WIFI_EXTENDER_UNAVAILABLE_NW));
    }
    lv_style_copy(&style_bg, &lv_style_plain);
    style_bg.body.main_color = LV_COLOR_WHITE;
    style_bg.body.grad_color = LV_COLOR_WHITE;
    style_bg.text.font = get_font(font_w_bold, font_h_18);
    lv_mbox_set_style(mbox1, LV_MBOX_STYLE_BG, &style_bg);
    lv_obj_set_width(mbox1, 120);
    lv_obj_align(mbox1, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_mbox_set_anim_time(mbox1, 500);
    lv_mbox_start_auto_close(mbox1, 500);
}
