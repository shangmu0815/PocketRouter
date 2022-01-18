#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/battery/battery_info.h"
#include "lv_pocket_router/src/power_saving/battery_optimize.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/util.h"

#ifdef BATTERY_OPTIMIZE_SUPPORT
static lv_style_t style_font;
static lv_obj_t * liste_switch;

void battery_optimize_warning(void) {
    if (!ds_get_bool(DS_KEY_BATTERY_OPTIMIZE)) return;

    popup_anim_not_create(get_string(ID_BATTERY_OPTIMIZE_WARNING), NULL, NULL, NULL);
    set_static_popup(true);
    lv_task_t * task = lv_task_create(close_static_popup, 5000, LV_TASK_PRIO_MID, NULL);
    lv_task_once(task);
}

void optimize_action(lv_obj_t * sw, lv_event_cb_t event_cb) {
    if (event_cb != LV_EVENT_CLICKED) return;

#ifdef CUST_SWITCH
    set_cust_switch_state(liste_switch, DS_KEY_BATTERY_OPTIMIZE);
    bool enable = ds_get_bool(DS_KEY_BATTERY_OPTIMIZE);
    set_charging_fv(enable);
#else
    bool enable = lv_sw_get_state(sw);
    ds_set_bool(DS_KEY_BATTERY_OPTIMIZE, enable);
    set_charging_fv(enable);
#endif
}

void battery_optimize_create(void) {
    liste_style_create();

    lv_obj_t * win = default_list_header(lv_scr_act(), 
               get_string(ID_BATTERY_OPTIMIZE), lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, 60 * LV_RES_OFFSET);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    bool enabled = ds_get_bool(DS_KEY_BATTERY_OPTIMIZE);
#ifdef CUST_SWITCH
    liste_switch = lv_liste_cust_switch(list,
          get_string(ID_BATTERY_OPTIMIZE), optimize_action, enabled);
    lv_obj_t * img = lv_obj_get_child(liste_switch, NULL);
    if (enabled) {
        lv_img_set_src(img, &ic_list_checkbox_selected);
    } else {
        lv_img_set_src(img, &ic_list_checkbox);
    }
#else
    liste_switch = lv_liste_w_switch(list,
          get_string(ID_BATTERY_OPTIMIZE), optimize_action);
    lv_obj_t * sw_toggle = lv_obj_get_child(liste_switch, NULL);
    if (enabled) {
        lv_sw_on(sw_toggle, LV_ANIM_OFF);
    } else {
        lv_sw_off(sw_toggle, LV_ANIM_OFF);
    }
#endif

    lv_style_copy(&style_font, &lv_style_plain);
#ifdef CUST_DLINK
    style_font.text.font = get_font(font_w_bold, font_h_20);
#else
    style_font.text.font = get_font(font_w_bold, font_h_16);
#endif
    style_font.text.color = LV_COLOR_GREYISH_BROWN;
    style_font.text.letter_space = 1;

    lv_obj_t * enable_sw_label = lv_label_create(win, NULL);
    lv_label_set_long_mode(enable_sw_label, LV_LABEL_LONG_BREAK);
    lv_obj_set_size(enable_sw_label, 288, 120);
    lv_label_set_text(enable_sw_label, get_string(ID_BATTERY_OPTIMIZE_NOTE));
    lv_label_set_style(enable_sw_label, LV_LABEL_STYLE_MAIN, &style_font);

    //re-align depend on is_ltr result
    if(is_ltr()){
        lv_label_set_align(enable_sw_label, LV_LABEL_ALIGN_LEFT);
        lv_obj_align(enable_sw_label,
               liste_switch, LV_ALIGN_OUT_BOTTOM_LEFT, LISTE_SHIFT, HOR_SPACE);
    }else{
        lv_label_set_align(enable_sw_label, LV_LABEL_ALIGN_RIGHT);
        lv_obj_align(enable_sw_label,
               liste_switch, LV_ALIGN_OUT_BOTTOM_RIGHT, -LISTE_SHIFT, HOR_SPACE);
    }
}

#endif /* BATTERY_OPTIMIZE_SUPPORT */
