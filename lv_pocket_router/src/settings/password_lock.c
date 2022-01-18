#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/settings/password_lock.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/keyboard/num_kb_box.h"
#include "lv_pocket_router/src/keyboard/basic_kb.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/res/values/styles.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"

#define DS_KEY_PASSWORD_LOCK_ENABLE "password_lock_enable"
#define DS_KEY_PASSWORD_LOCK_VALUE  "password_lock_value"
#define PWD_LENGTH 5
#define PWD_BOX    4

//static const uint32_t INDEX_LEFT_BTN = 5;
static lv_style_t style_font;
static lv_obj_t * liste_enable_password_lock;
static lv_obj_t * enable_password_lock_label;
static lv_obj_t * password_lock_modified_btn;
static lv_obj_t * password_lock_root_view;
static lv_obj_t * enable_password_lock_sw;

static bool check_enable_password_lock_enter_pw_done;
static bool check_enable_password_lock_confirm_pw_done;
static bool check_modified_password_lock_current_pw_done;
static bool check_modified_password_lock_new_pw_done;
static bool check_modified_password_lock_confirm_pw_done;
static bool check_disable_password_lock_current_pw_done;
static bool check_enable_password_lock;
static bool check_modified_password_lock;
static bool check_disable_password_lock;

static char enable_pw_lock_confirm_pw_data[PWD_LENGTH];
static char enable_pw_lock_enter_pw_data[PWD_LENGTH];
static char modified_current_pw_data[PWD_LENGTH];
static char modified_new_pw_data[PWD_LENGTH];
static char modified_confirm_pw_data[PWD_LENGTH];
static char disable_pw_lock_current_pw_data[PWD_LENGTH];

void enable_password_lock_action(lv_obj_t * sw, lv_event_cb_t event_cb) {
    if (event_cb != LV_EVENT_CLICKED) return;

    const char* str;

#ifdef CUST_SWITCH
    lv_obj_t * img = lv_obj_get_child(liste_enable_password_lock, NULL);
    if (ds_get_bool(DS_KEY_PASSWORD_LOCK_ENABLE)){
        //disable
        lv_img_set_src(img, &ic_list_checkbox);
        check_disable_password_lock = true;
        str = get_string(ID_PW_LOCK_CURRENT_PW);
    } else {
        //enable
        lv_img_set_src(img, &ic_list_checkbox_selected);
        check_enable_password_lock = true;
        str = get_string(ID_PW_LOCK_ENTER_PW);
    }
#else
    if (lv_sw_get_state(sw)) {
        check_enable_password_lock = true;
        str = get_string(ID_PW_LOCK_ENTER_PW);
    } else {
        check_disable_password_lock = true;
        str = get_string(ID_PW_LOCK_CURRENT_PW);
    }
#endif
    password_lock_root_view = num_kb_box_create(pw_input_done_action,
            str, true, pw_lock_num_kb_box_btnm_action);
}

void pw_lock_modified_action(lv_obj_t * btn, lv_event_cb_t event_cb) {
    if (event_cb != LV_EVENT_CLICKED) return;

    password_lock_root_view = num_kb_box_create(pw_input_done_action,
            get_string(ID_PW_LOCK_CURRENT_PW), true, pw_lock_num_kb_box_btnm_action);

    check_modified_password_lock = true;
    log_d("pw_lock_modified_action check_modified_password_lock:%d\n",check_modified_password_lock);
}

void pw_lock_num_kb_box_btnm_action(lv_obj_t * btn, lv_event_cb_t event_cb) {
    if (event_cb != LV_EVENT_CLICKED) return;

    uint32_t index = lv_obj_get_user_data(btn);
    //keyboard cancel
    if (index == KB_LEFT_BTN) {
        //check_enable_password_lock if not completely
        if (check_enable_password_lock == true) {
            if (check_enable_password_lock_enter_pw_done == false
                    || check_enable_password_lock_confirm_pw_done == false) {
                ds_set_bool(DS_KEY_PASSWORD_LOCK_ENABLE, false);
#ifdef CUST_SWITCH
                lv_obj_t * img = lv_obj_get_child(liste_enable_password_lock, NULL);
                lv_img_set_src(img, &ic_list_checkbox);
#else
                lv_sw_off(enable_password_lock_sw, LV_ANIM_OFF);
#endif
            }
        }
        //check_modified_password_lock if not completely
        if (check_modified_password_lock == true) {
            if (check_modified_password_lock_current_pw_done == false
                    || check_modified_password_lock_new_pw_done == false
                    || check_modified_password_lock_confirm_pw_done == false) {
                ds_set_bool(DS_KEY_PASSWORD_LOCK_ENABLE, true);
#ifdef CUST_SWITCH
                lv_obj_t * img = lv_obj_get_child(liste_enable_password_lock, NULL);
                lv_img_set_src(img, &ic_list_checkbox_selected);
#else
                lv_sw_on(enable_password_lock_sw, LV_ANIM_OFF);
#endif
            }
        }
        //check_disable_password_lock if not completely
        if (check_disable_password_lock == true) {
            if (check_disable_password_lock_current_pw_done == false) {
                ds_set_bool(DS_KEY_PASSWORD_LOCK_ENABLE, true);
#ifdef CUST_SWITCH
                lv_obj_t * img = lv_obj_get_child(liste_enable_password_lock, NULL);
                lv_img_set_src(img, &ic_list_checkbox_selected);
#else
                lv_sw_on(enable_password_lock_sw, LV_ANIM_OFF);
#endif
            }
        }
        pw_lock_clear_config();
        num_kb_box_close();
    }
}

void password_lock_create(void) {
    liste_style_create();
    pw_lock_clear_config();

    lv_obj_t * win = default_list_header (lv_scr_act(), get_string(ID_PW_LOCK), lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, 60 * LV_RES_OFFSET);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    //add list element in order
    //liste_enable_password_lock
    bool enable = ds_get_bool(DS_KEY_PASSWORD_LOCK_ENABLE);
#ifdef CUST_SWITCH
    liste_enable_password_lock = lv_liste_cust_switch(list,
            get_string(ID_PW_LOCK_ENABLE_PW_LOCK), enable_password_lock_action, enable);
#else
    liste_enable_password_lock = lv_liste_w_switch(list, get_string(ID_PW_LOCK_ENABLE_PW_LOCK), enable_password_lock_action);
    enable_password_lock_sw = lv_obj_get_child(liste_enable_password_lock, NULL);

    if (enable) {
        lv_sw_on(enable_password_lock_sw, LV_ANIM_OFF);
    } else {
        lv_sw_off(enable_password_lock_sw, LV_ANIM_OFF);
    }
#endif

    //liste_enable_password_lock_note
    lv_style_copy(&style_font, &lv_style_plain);
#ifdef CUST_DLINK
    style_font.text.font = get_font(font_w_bold, font_h_20);
#else
    style_font.text.font = get_font(font_w_bold, font_h_16);
#endif
    style_font.text.color = LV_COLOR_GREYISH_BROWN;
    style_font.text.letter_space = 1;

    enable_password_lock_label = lv_label_create(win, NULL);
    lv_label_set_long_mode(enable_password_lock_label, LV_LABEL_LONG_BREAK);
    lv_obj_set_size(enable_password_lock_label, 288, 120);
    lv_label_set_text(enable_password_lock_label, get_string(ID_PW_LOCK_ENABLE_PW_LOCK_NOTE));
    lv_label_set_style(enable_password_lock_label, LV_LABEL_STYLE_MAIN, &style_font);

    //re-align depend on is_ltr result
    if(is_ltr()){
        lv_label_set_align(enable_password_lock_label, LV_LABEL_ALIGN_LEFT);
        lv_obj_align(enable_password_lock_label,
                liste_enable_password_lock, LV_ALIGN_OUT_BOTTOM_LEFT, LISTE_SHIFT, HOR_SPACE);
    }else{
        lv_label_set_align(enable_password_lock_label, LV_LABEL_ALIGN_RIGHT);
        lv_obj_align(enable_password_lock_label,
                liste_enable_password_lock, LV_ALIGN_OUT_BOTTOM_RIGHT, -LISTE_SHIFT, HOR_SPACE);
    }

    //draw check_for_update button
    static lv_style_t style_btn_rel;
    static lv_style_t style_btn_pr;
    lv_style_copy(&style_btn_rel, &lv_style_btn_rel);
    style_btn_rel.body.main_color = LV_COLOR_WHITE;
    style_btn_rel.body.grad_color = LV_COLOR_WHITE;
    style_btn_rel.body.border.color = LV_COLOR_SILVER;
    style_btn_rel.body.border.width = 2;
    style_btn_rel.body.border.opa = LV_OPA_COVER;
    style_btn_rel.body.radius = 0;
    style_btn_rel.text.color = LV_COLOR_GREYISH_BROWN;
    style_btn_rel.text.font = get_font(font_w_bold, font_h_22);
    style_btn_rel.text.letter_space = 1;
    style_btn_rel.body.border.part = LV_BORDER_TOP;

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.body.main_color = LV_COLOR_WHITE;
    style_btn_pr.body.grad_color = LV_COLOR_WHITE;
    style_btn_pr.body.border.color = LV_COLOR_BASE;
    style_btn_pr.text.color = LV_COLOR_BASE;

    password_lock_modified_btn = lv_btn_create(win, NULL);

    lv_label_set_text(lv_label_create(password_lock_modified_btn, NULL), get_string(ID_PW_LOCK_MODIFIED));
    lv_obj_set_event_cb(password_lock_modified_btn, pw_lock_modified_action);
    lv_obj_set_size(password_lock_modified_btn, 320 * LV_RES_OFFSET, 50 * LV_RES_OFFSET);
    lv_btn_set_style(password_lock_modified_btn, LV_BTN_STYLE_REL, &style_btn_rel);
    lv_btn_set_style(password_lock_modified_btn, LV_BTN_STYLE_PR, &style_btn_pr);
    lv_obj_align(password_lock_modified_btn, win, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    //hide password_lock_modified_btn
    lv_obj_set_hidden(password_lock_modified_btn, true);
    log_d("password_lock_createds_get_bool(DS_KEY_PASSWORD_LOCK_ENABLE):%d\n", ds_get_bool(DS_KEY_PASSWORD_LOCK_ENABLE));
    if (ds_get_bool(DS_KEY_PASSWORD_LOCK_ENABLE) == true) {
        lv_obj_set_hidden(password_lock_modified_btn, false);
#ifdef CUST_SWITCH
        lv_obj_t * img = lv_obj_get_child(liste_enable_password_lock, NULL);
        lv_img_set_src(img, &ic_list_checkbox_selected);
#else
        lv_sw_on(enable_password_lock_sw, LV_ANIM_OFF);
#endif
    } else {
        lv_obj_set_hidden(password_lock_modified_btn, true);
#ifdef CUST_SWITCH
        lv_obj_t * img = lv_obj_get_child(liste_enable_password_lock, NULL);
        lv_img_set_src(img, &ic_list_checkbox);
#else
        lv_sw_off(enable_password_lock_sw, LV_ANIM_OFF);
#endif
    }
}

//reset all config for pw lock
void pw_lock_clear_config(){
    check_enable_password_lock_enter_pw_done = false;
    check_enable_password_lock_confirm_pw_done = false;
    check_modified_password_lock_current_pw_done = false;
    check_modified_password_lock_new_pw_done = false;
    check_modified_password_lock_confirm_pw_done = false;
    check_disable_password_lock_current_pw_done = false;
    check_enable_password_lock = false;
    check_modified_password_lock = false;
    check_disable_password_lock = false;
}

void pw_input_done_action(char * pw_info){
    log_d("pw_info: %s \n",pw_info);

#ifdef CUST_SWITCH
    bool enable = !ds_get_bool(DS_KEY_PASSWORD_LOCK_ENABLE);
#else
    bool enable = lv_sw_get_state(enable_password_lock_sw);
#endif
    //do something if all 4 box is full
    if (strlen(pw_info) == PWD_BOX) {
        if (check_enable_password_lock == true) {
            log_d("check_enable_password_lock check_enable_password_lock_enter_pw_done: %d \n",check_enable_password_lock_enter_pw_done);
            if (check_enable_password_lock_enter_pw_done == false
                    && enable) {
                num_kb_refresh_title(get_string(ID_PW_LOCK_CONFIRM_PW));

                memset(enable_pw_lock_enter_pw_data, 0, PWD_LENGTH);
                strcpy(enable_pw_lock_enter_pw_data, pw_info);
                if (pw_info != NULL) {
                    lv_mem_free(pw_info);
                    pw_info = NULL;
                }
                check_enable_password_lock_enter_pw_done = true;

            } else if (check_enable_password_lock_enter_pw_done == true && check_enable_password_lock_confirm_pw_done == false && enable) {
                memset(enable_pw_lock_confirm_pw_data, 0, PWD_LENGTH);
                strcpy(enable_pw_lock_confirm_pw_data, pw_info);
                if (pw_info != NULL) {
                    lv_mem_free(pw_info);
                    pw_info = NULL;
                }
                if (strcmp(enable_pw_lock_enter_pw_data, enable_pw_lock_confirm_pw_data) == 0) {
                    lv_obj_set_hidden(password_lock_modified_btn, false);
                    num_kb_box_close();
                    //enable_password_lock_switch = true;
                    //check_enable_password_lock_confirm_pw_done = true;

                    //save password lock value
                    ds_set_bool(DS_KEY_PASSWORD_LOCK_ENABLE, true);
                    ds_set_value(DS_KEY_PASSWORD_LOCK_VALUE, enable_pw_lock_confirm_pw_data);

                    //enable pw lock ok,clear all config
                    pw_lock_clear_config();
                } else {
                    num_kb_refresh_title(get_string(ID_PW_LOCK_PW_INCORRECT));
                    check_enable_password_lock_confirm_pw_done= false;
                }
            }
        } else if(check_modified_password_lock == true) {
#ifdef CUST_SWITCH
            enable = ds_get_bool(DS_KEY_PASSWORD_LOCK_ENABLE);
#endif
            log_d("check_modified_password_lock check_modified_password_lock_current_pw_done: %d\n",check_modified_password_lock_current_pw_done);
            log_d("check_modified_password_lock lv_sw_get_state(enable_password_lock_sw): %d\n", enable);
            if (check_modified_password_lock_current_pw_done == false && enable)
            {
                memset(modified_current_pw_data, 0, PWD_LENGTH);
                strcpy(modified_current_pw_data, pw_info);
                if (pw_info != NULL) {
                    lv_mem_free(pw_info);
                    pw_info = NULL;
                }
                if (strcmp(modified_current_pw_data, ds_get_value(DS_KEY_PASSWORD_LOCK_VALUE)) == 0) {
                    num_kb_refresh_title(get_string(ID_PW_LOCK_NEW_PW));
                    check_modified_password_lock_current_pw_done= true;
                } else {
                    num_kb_refresh_title(get_string(ID_PW_LOCK_PW_INCORRECT));
                    check_modified_password_lock_current_pw_done= false;
                }
            } else if (check_modified_password_lock_current_pw_done == true && check_modified_password_lock_new_pw_done == false && enable) {
                memset(modified_new_pw_data, 0, PWD_LENGTH);
                strcpy(modified_new_pw_data, pw_info);
                if (pw_info != NULL) {
                    lv_mem_free(pw_info);
                    pw_info = NULL;
                }
                num_kb_refresh_title(get_string(ID_PW_LOCK_CONFIRM_PW));
                check_modified_password_lock_new_pw_done= true;
            } else if (check_modified_password_lock_current_pw_done == true && check_modified_password_lock_new_pw_done == true && check_modified_password_lock_confirm_pw_done == false && enable) {
                memset(modified_confirm_pw_data, 0, PWD_LENGTH);
                strcpy(modified_confirm_pw_data, pw_info);
                if (pw_info != NULL) {
                    lv_mem_free(pw_info);
                    pw_info = NULL;
                }
                if (strcmp(modified_new_pw_data, modified_confirm_pw_data) == 0) {
                    //save password lock value
                    ds_set_bool(DS_KEY_PASSWORD_LOCK_ENABLE, true);
                    ds_set_value(DS_KEY_PASSWORD_LOCK_VALUE, modified_confirm_pw_data);

                    num_kb_box_close();
                    //modified pw lock ok,clear all config
                    pw_lock_clear_config();
                } else {
                    num_kb_refresh_title(get_string(ID_PW_LOCK_PW_INCORRECT));
                    check_modified_password_lock_confirm_pw_done= false;
                }
            }
        } else if (check_disable_password_lock == true) {
            log_d("check_disable_password_lock check_disable_password_lock_current_pw_done: %d\n",check_disable_password_lock_current_pw_done);
            log_d("check_disable_password_lock lv_sw_get_state(enable_password_lock_sw): %d\n", enable);
            if (check_disable_password_lock_current_pw_done ==false && !enable) {
                memset(disable_pw_lock_current_pw_data, 0, PWD_LENGTH);
                strcpy(disable_pw_lock_current_pw_data, pw_info);
                if (pw_info != NULL) {
                    lv_mem_free(pw_info);
                    pw_info = NULL;
                }
                //need to get the latest pw to check

                if (strcmp(disable_pw_lock_current_pw_data, ds_get_value(DS_KEY_PASSWORD_LOCK_VALUE)) == 0) {
                    //save password lock value
                    ds_set_bool(DS_KEY_PASSWORD_LOCK_ENABLE, false);
                    ds_set_value(DS_KEY_PASSWORD_LOCK_VALUE, "");

                    num_kb_box_close();
                    lv_obj_set_hidden(password_lock_modified_btn, true);
                    //disable pw lock ok,clear all config
                    pw_lock_clear_config();

                } else {
                    num_kb_refresh_title(get_string(ID_PW_LOCK_PW_INCORRECT));
                    check_disable_password_lock_current_pw_done= false;
                }
            }
        }
    }
}
