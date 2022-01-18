#include <stdio.h>
#include <stdbool.h>
#include "data_roaming.h"

#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/ril/ril.h"

lv_obj_t * liste_roaming;
lv_obj_t * data_roaming_sw;

// if another popup like usb tether popup come up, our popup got closed
// so set this function callback, so we reset switch status when our popup got closed
void set_roaming_state() {
#ifdef FEATURE_ROUTER
    int enable;
    GetRoaming(&enable);
#ifdef CUST_SWITCH
    lv_obj_t * img = lv_obj_get_child(liste_roaming, NULL);
    if (enable) {
        lv_img_set_src(img, &ic_list_checkbox_selected);
    } else {
        lv_img_set_src(img, &ic_list_checkbox);
    }
#else
    if (enable) {
        lv_sw_on(data_roaming_sw, LV_ANIM_OFF);
    } else {
        lv_sw_off(data_roaming_sw, LV_ANIM_OFF);
    }
#endif
#else
#ifdef CUST_SWITCH
    lv_obj_t * img = lv_obj_get_child(liste_roaming, NULL);
    lv_img_set_src(img, &ic_list_checkbox);
#else
    lv_sw_off(data_roaming_sw, LV_ANIM_OFF);
#endif
#endif
}

void enable_roaming() {
    reset_data_usage(false);
#ifdef FEATURE_ROUTER
    //switch on data roaming here
    SetRoaming(1);
#endif
}

void reset_data_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(mbox);
    if (strcmp(txt, get_string(ID_CANCEL)) == 0) { //cancel
#ifdef CUST_SWITCH
        lv_obj_t * img = lv_obj_get_child(liste_roaming, NULL);
        lv_img_set_src(img, &ic_list_checkbox);
#else
        lv_sw_off(data_roaming_sw, LV_ANIM_OFF);
#endif
        //switch off data roaming here
#ifdef FEATURE_ROUTER
        SetRoaming(0);
#endif
    } else if (strcmp(txt, get_string(ID_OK)) == 0) { //ok
#ifdef CUST_SWITCH
        lv_obj_t * img = lv_obj_get_child(liste_roaming, NULL);
        lv_img_set_src(img, &ic_list_checkbox_selected);
#else
        lv_sw_on(data_roaming_sw, LV_ANIM_OFF);
#endif
        enable_roaming();
    }
    close_popup();
}

void data_roaming_action(lv_obj_t * sw, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    int enable = false;
#ifdef FEATURE_ROUTER
    GetRoaming(&enable);
#endif
#ifndef CUST_SWITCH
    enable = !lv_sw_get_state(sw);
#endif
    if (!enable) {
        static const char * btns[3];
        btns[0] = get_string(ID_CANCEL);
        btns[1] = get_string(ID_OK);
        btns[2] = "";
        if (ds_get_bool(DS_KEY_DATA_USAGE_MONITOR)) {
            popup_anim_que_create(get_string(ID_DATA_USAGE_RESET_CONFIRM_PROMPT),
                  btns, reset_data_action, NULL);
            set_popup_cb(set_roaming_state);
        } else {
            enable_roaming();
#ifdef CUST_SWITCH
            lv_obj_t * img = lv_obj_get_child(liste_roaming, NULL);
            lv_img_set_src(img, &ic_list_checkbox_selected);
#endif
        }
    } else {
        //switch off data roaming here
#ifdef FEATURE_ROUTER
        SetRoaming(0);
#ifdef CUST_SWITCH
        lv_obj_t * img = lv_obj_get_child(liste_roaming, NULL);
        lv_img_set_src(img, &ic_list_checkbox);
#endif
#endif
    }
}

void data_roaming_with_no_sim(void) {
    info_page_create_label_align_center(lv_scr_act(), get_string(ID_DATA_ROAMING),
            get_string(ID_DATA_ROAMING_SIM_NOT_READY));
}

void data_roaming_with_sim(void) {
    liste_style_create();

    lv_obj_t * win = default_list_header (lv_scr_act(), get_string(ID_DATA_ROAMING), lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);
#ifdef CUST_SWITCH
    liste_roaming = lv_liste_cust_switch(list,
            get_string(ID_DATA_ROAMING), data_roaming_action, false);
#else
    liste_roaming = lv_liste_w_switch(list, get_string(ID_DATA_ROAMING), data_roaming_action);
    data_roaming_sw = lv_obj_get_child(liste_roaming, NULL);
#endif

    set_roaming_state();
}

void init_data_roaming(void) {
#ifndef FEATURE_ROUTER
    data_roaming_with_sim();
#else
    if (get_sim_state() == SIM_READY) {
        data_roaming_with_sim();
    } else {
        data_roaming_with_no_sim();
    }
#endif
}
