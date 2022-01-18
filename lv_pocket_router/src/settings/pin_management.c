#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/keyboard/num_kb_col.h"
#include "lv_pocket_router/src/keyboard/en_kb.h"
#include "lv_pocket_router/src/keyboard/basic_kb.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/settings/pin_management.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/dashboard.h"

#if !defined (FEATURE_ROUTER)
static int remain_pin_count = 3;
static int remain_puk_count = 10;
#endif
static lv_style_t style_font;
lv_obj_t * liste_enable_pin;
lv_obj_t * enable_pin_sw;
lv_obj_t * enable_pin_label;

bool kb_with_close_btn = true;
int ril_return_state = RIL_ERROR_UNKNOWN;
int chk_type = SIM_PIN;
#define SIM_PIN_LENGTH SIM_PIN1_MAX_LENGTH + 1
#define SIM_PUK_LENGTH SIM_PUK1_MAX_LENGTH + 1
char enter_sim_puk_value[SIM_PUK_LENGTH];
char enter_new_sim_pin_value[SIM_PIN_LENGTH];
lv_obj_t * retry_left_info_page_win;
lv_obj_t * enter_puk_info_page_win;
char* verify_fail_str = NULL;
char* retry_left_info_page_str = NULL;
lv_obj_t * mbox;

//TODO temp to get retry_left_info_page string
char* get_retry_left_info_page_string(int type, int retry_left) {
    log_d("get_retry_left_info_page_string type:%d,retry_left:%d", type , retry_left);
    char retry_count_str[3];
    memset(retry_count_str, '\0', sizeof(retry_count_str));
    snprintf(retry_count_str, sizeof(retry_count_str), "%d", retry_left);

    const char* retry_left_msg = (
            (type == SIM_PIN) ?
                    get_string(ID_PIN_MNG_PIN_RETRY_LEFT_PAGE_MSG) :
                    get_string(ID_PIN_MNG_PUK_RETRY_LEFT_PAGE_MSG));
    int len = strlen(retry_left_msg) + strlen(retry_count_str);
    //x3 for special char support
    retry_left_info_page_str = (char*) lv_mem_alloc((sizeof(char) * len) * 3 + 1);
    memset(retry_left_info_page_str, '\0', (sizeof(char) * len) * 3 + 1);
    sprintf(retry_left_info_page_str, retry_left_msg, retry_count_str);
    return (retry_left_info_page_str == NULL) ? "" : retry_left_info_page_str;
}

//TODO temp retry_left_page_action
void retry_left_page_action(lv_obj_t * btnm, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(btnm);
    log_d("retry_left_page_action chk_type:%d,txt:%s", chk_type, txt);
    if (strcmp(txt, get_string(ID_OK)) == 0) {
        pin_puk_kb_create((chk_type == SIM_PIN ? ID_ENABLE_PIN : ID_ENABLE_PUK), false);
        retry_left_info_page_str_free_alloc();
        info_page_close_win(retry_left_info_page_win);
    }
}

//TODO temp pin puk retry left info page
void pin_puk_retry_left_info_page(int type, int retry_left) {
    chk_type = type;
    log_d("pin_puk_retry_left_info_page chk_type:%d,retry_left:%d", chk_type, retry_left);
    static const char * btns[2];
    btns[0] = get_string(ID_OK);
    btns[1] = "";
    retry_left_info_page_str = get_retry_left_info_page_string(chk_type, retry_left);
    retry_left_info_page_win = info_page_create_btmn(lv_scr_act(),
            get_string(ID_PIN_MNG),
            retry_left_info_page_str,
            btns, retry_left_page_action);
    info_page_hide_btn(retry_left_info_page_win);
}

//TODO
void retry_left_info_page_str_free_alloc() {
    if (retry_left_info_page_str != NULL) {
        lv_mem_free(retry_left_info_page_str);
        retry_left_info_page_str = NULL;
    }
}

void init_pin_management(void) {
    //set kb_with_close_btn config as true
    kb_with_close_btn = true;
#if defined (FEATURE_ROUTER)
    int sim_state = get_sim_state();
    log_d("init_pin_management sim_state:%d", sim_state);
    if (sim_state == READY || sim_state == PIN_REQUIRED
            || sim_state == PUK_REQUIRED) {
        pin_management_with_sim_ready();
    } else {
        pin_management_with_sim_not_ready();
    }
#else
    log_d("init_pin_management DS_KEY_SIM_PIN_VALUE:%s", ds_get_value(DS_KEY_SIM_PIN_VALUE));
    if (strcmp("", ds_get_value(DS_KEY_SIM_PIN_VALUE)) == 0) {
        ds_set_value(DS_KEY_SIM_PIN_VALUE, "0000"); //only for UI test
    }
    log_d("init_pin_management DS_KEY_SIM_PUK_VALUE:%s", ds_get_value(DS_KEY_SIM_PUK_VALUE));
    if (strcmp("", ds_get_value(DS_KEY_SIM_PUK_VALUE)) == 0) {
        ds_set_value(DS_KEY_SIM_PUK_VALUE, "12345678"); //only for UI test
    }
    pin_management_with_sim_ready();
#endif
}

void pin_management_with_sim_not_ready(void) {
    info_page_create_label_align_center(lv_scr_act(), get_string(ID_PIN_MNG),
            get_string(ID_PIN_MNG_SIM_NOT_READY));
}

void pin_management_with_sim_ready(void) {
    liste_style_create();
    lv_obj_t * win = default_list_header (lv_scr_act(), get_string(ID_PIN_MNG), lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);
    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, 60 * LV_RES_OFFSET);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);
    //add list element in order
    //liste_enable_pin
#ifdef CUST_SWITCH
    liste_enable_pin = lv_liste_cust_switch(list,
            get_string(ID_PIN_MNG_ENABLE_PIN), enable_pin_switch, false);
#else
    liste_enable_pin = lv_liste_w_switch(list, get_string(ID_PIN_MNG_ENABLE_PIN), enable_pin_switch);
    enable_pin_sw = lv_obj_get_child(liste_enable_pin, NULL);
#endif

#if defined (FEATURE_ROUTER)
    set_switch_state();
#else
    log_d("pin_management_with_sim_ready DS_KEY_ENABLE_SIM_PIN:%d", ds_get_bool(DS_KEY_ENABLE_SIM_PIN));
#ifdef CUST_SWITCH
    if (liste_enable_pin != NULL) {
        lv_obj_t * img = lv_obj_get_child(liste_enable_pin, NULL);
        if (ds_get_bool(DS_KEY_ENABLE_SIM_PIN) == true) {
            lv_img_set_src(img, &ic_list_checkbox_selected);
        } else {
            lv_img_set_src(img, &ic_list_checkbox);
        }
    }
#else
    (ds_get_bool(DS_KEY_ENABLE_SIM_PIN) == true) ? lv_sw_on(enable_pin_sw, LV_ANIM_OFF) : lv_sw_off(enable_pin_sw, LV_ANIM_OFF);
#endif
#endif
    //liste_enable_pin_note
    lv_style_copy(&style_font, &lv_style_plain);
#ifdef CUST_DLINK
    style_font.text.font = get_font(font_w_bold, font_h_20);
#else
    style_font.text.font = get_font(font_w_bold, font_h_16);
#endif
    style_font.text.color = LV_COLOR_GREYISH_BROWN;
    style_font.text.letter_space = 1;

    enable_pin_label = lv_label_create(win, NULL);
    lv_label_set_long_mode(enable_pin_label, LV_LABEL_LONG_BREAK);
    lv_obj_set_size(enable_pin_label, 288, 120);
    lv_label_set_text(enable_pin_label, get_string(ID_PIN_MNG_ENABLE_PIN_NOTE));
    lv_label_set_style(enable_pin_label, LV_LABEL_STYLE_MAIN, &style_font);

    //re-align depend on is_ltr result
    if(is_ltr()){
        lv_label_set_align(enable_pin_label, LV_LABEL_ALIGN_LEFT);
        lv_obj_align(enable_pin_label,
                liste_enable_pin, LV_ALIGN_OUT_BOTTOM_LEFT, LISTE_SHIFT, HOR_SPACE);
    }else{
        lv_label_set_align(enable_pin_label, LV_LABEL_ALIGN_RIGHT);
        lv_obj_align(enable_pin_label,
                liste_enable_pin, LV_ALIGN_OUT_BOTTOM_RIGHT, -LISTE_SHIFT, HOR_SPACE);
    }
}

void set_switch_state() {
    int sim_pin1_state = get_sim_pin1_state();
    log_d("set_switch_state sim_pin1_state:%d", sim_pin1_state);
#ifdef CUST_SWITCH
    if (liste_enable_pin != NULL) {
        lv_obj_t * img = lv_obj_get_child(liste_enable_pin, NULL);
        if (sim_pin1_state == PIN_ENABLED_VERIFIED) {
            lv_img_set_src(img, &ic_list_checkbox_selected);
        } else {
            lv_img_set_src(img, &ic_list_checkbox);
        }
    }
#else
    if (enable_pin_sw != NULL) {
        (sim_pin1_state == PIN_ENABLED_VERIFIED) ? lv_sw_on(enable_pin_sw, LV_ANIM_OFF) : lv_sw_off(enable_pin_sw, LV_ANIM_OFF);
    }
#endif
}

//TODO
void enable_pin_switch_error_action(lv_obj_t * mbox/*btn*/, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(mbox);
    log_d("enable_pin_switch_error_action pressed: %s", txt);
    if (strcmp(txt, get_string(ID_OK)) == 0) {
#ifdef CUST_SWITCH
        if (liste_enable_pin != NULL) {
            lv_obj_t * img = lv_obj_get_child(liste_enable_pin, NULL);
            lv_img_set_src(img, &ic_list_checkbox);
        }
#else
        if (enable_pin_sw != NULL) lv_sw_off(enable_pin_sw, LV_ANIM_OFF);
#endif
        close_popup();
#if !defined (FEATURE_ROUTER)
        ds_set_bool(DS_KEY_ENABLE_SIM_PIN, false);
#endif
    }
}

//TODO temp to show enable_pin_switch_error_notice when enable_pin_switch error
void enable_pin_switch_error_notice() {
    static const char * btns[2];
    btns[0] = get_string(ID_OK);
    btns[1] = "";
    popup_anim_not_create(get_string(ID_PIN_MNG_ENABLE_PIN_SWITCH_ERR), btns,
            enable_pin_switch_error_action, NULL);
}

void enable_pin_switch(lv_obj_t * sw, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;
#if defined (FEATURE_ROUTER)
        int sim_state = get_sim_state();
        log_d("enable_pin_switch sim_state:%d,kb_with_close_btn:%d", sim_state,kb_with_close_btn);
        if (sim_state == READY || sim_state == PIN_REQUIRED) {
            //sim pin verify case
            pin_puk_kb_create(ID_ENABLE_PIN, kb_with_close_btn);
        } else if (sim_state == PUK_REQUIRED) {
            //sim puk verify case
            pin_puk_kb_create(ID_ENABLE_PUK, kb_with_close_btn);
        } else {
            //switch error case
            enable_pin_switch_error_notice();
        }
#else
        //only for UI test
        log_d("enable_pin_switch remain_pin_count:%d,remain_puk_count:%d", remain_pin_count,remain_puk_count);
        log_d("enable_pin_switch kb_with_close_btn:%d", kb_with_close_btn);
        if (remain_puk_count == 0) {
            //puk error 10 times case
            verify_fail_str = get_pin_puk_verify_fail_string(SIM_PUK, remain_puk_count);
            static const char * btns[2];
            btns[0] = get_string(ID_PIN_MNG_INCORRECT_CONFIRM);
            btns[1] = "";
            popup_scrl_create_impl(get_string(ID_PIN_MNG_INCORRECT_PUK),
                    verify_fail_str, btns, retry_puk_error_10_times_action,
                    NULL);
        } else if (remain_pin_count == 0) {
            //sim puk verify case
            pin_puk_kb_create(ID_ENABLE_PUK, kb_with_close_btn);
        } else {
            //sim pin verify case
            pin_puk_kb_create(ID_ENABLE_PIN, kb_with_close_btn);
        }
#endif
}

//pin_puk_verify_result_cb will be triggered if execute below ril SUCCESS
//1.enable_disable_sim_pin,verify_sim_pin in sim_pin_verify()
//2.verify_sim_puk in sim_puk_verify()
void pin_puk_verify_result_cb(int sim_pin_req, int error, int retry_left) {
    log_d("pin_puk_verify_result_cb sim_pin_req:%d,error:%d,retry_left:%d",
            sim_pin_req, error, retry_left);
    if (sim_pin_req == SIM_PIN_REQ_ENABLE_PIN
            || sim_pin_req == SIM_PIN_REQ_DISABLE_PIN
            || sim_pin_req == SIM_PIN_REQ_VERIFY_PIN
            || sim_pin_req == SIM_PIN_REQ_VERIFY_PUK) {
        if (error == RIL_SUCCESS) {
            mbox = popup_anim_not_create(get_string(ID_LOADING), NULL, NULL, NULL);
            lv_task_t * task = lv_task_create(close_delay_popup, 1000, LV_TASK_PRIO_MID, NULL);
            lv_task_once(task);
            //set_switch_state();
            //start dashboard after pin/puk verify pass
            //if(get_kb_close_btn_hidden()){
            //    log_d("restart dashboard from pin management");
            //    db_destroy();
            //    dashboard_create();
            //}
            //start dashboard after pin/puk verify pass
            //num_col_close_win();
        } else {
            verify_fail_notice(((sim_pin_req == SIM_PIN_REQ_VERIFY_PUK) ? SIM_PUK : SIM_PIN),retry_left);
        }
    }
}

void verify_fail_notice(int type, int retry_left) {
    chk_type = type;
    log_d("verify_fail_notice chk_type:%d,retry_left:%d", chk_type, retry_left);
    verify_fail_str = get_pin_puk_verify_fail_string(
            ((chk_type == SIM_PIN) ? SIM_PIN : SIM_PUK), retry_left);

    //retry_left != 0(pin error < 3 times or puk error < 10 times)
    if (retry_left != 0) {
        static const char * btns[3];
        btns[0] = get_string(ID_CANCEL);
        btns[1] = get_string(ID_PIN_MNG_INCORRECT_CONFIRM);
        btns[2] = "";

        //set content to scroll circle mode for JP
        if(get_device_locale() == JP){
            popup_anim_que_long_create(verify_fail_str, btns, verify_fail_error_action, NULL);
        }else{
            popup_anim_que_create(verify_fail_str, btns, verify_fail_error_action, NULL);
        }
    } else {
        //retry_left ==0(pin error 3 times or puk error 10 times)
        static const char * btns[2];
        btns[0] = get_string(ID_PIN_MNG_INCORRECT_CONFIRM);
        btns[1] = "";
        popup_scrl_create_impl(
                ((chk_type == SIM_PIN) ?
                        get_string(ID_PIN_MNG_INCORRECT_PIN) :
                        get_string(ID_PIN_MNG_INCORRECT_PUK)), verify_fail_str,
                btns,
                ((chk_type == SIM_PIN) ?
                        incorrect_pin_threes_times_action :
                        retry_puk_error_10_times_action),
                NULL);
    }
}

void verify_fail_error_action(lv_obj_t * mbox/*btn*/, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(mbox);
    log_d("verify_fail_error_action chk_type:%d,txt:%s", chk_type,txt);
    //lv_obj_t * mbox = lv_mbox_get_from_btn(btn);
    if (strcmp(txt, get_string(ID_PIN_MNG_INCORRECT_CONFIRM)) == 0 || strcmp(txt, get_string(ID_CANCEL)) == 0) {
        verify_fail_str_free_alloc();
        //reuse kb,renew title, tip and callback id to Enable PIN or Enable PUK
        ((chk_type == SIM_PIN) ?
                set_pin_puk_kb_state(ID_ENABLE_PIN) :
                set_pin_puk_kb_state(ID_ENABLE_PUK));
        close_popup();
    }
}

void incorrect_pin_threes_times_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    const char * txt = lv_btnm_get_active_btn_text(mbox);
    //lv_obj_t * mbox = lv_mbox_get_from_btn(btn);
    if (strcmp(txt, get_string(ID_PIN_MNG_INCORRECT_CONFIRM)) == 0) {
        verify_fail_str_free_alloc();
        //incorrect PIN for 3 times,show please enter your PUK page
        static const char * btns[2];
        btns[0] = get_string(ID_PIN_MNG_ENTER_PUK);
        btns[1] = "";
        enter_puk_info_page_win = info_page_create_btmn(lv_scr_act(),
                get_string(ID_PIN_MNG),
                get_string(ID_PIN_MNG_PLS_ENTER_YOUR_PUK_NOTE), btns,
                enter_puk_info_page_action);

        log_d("incorrect_pin_threes_times_action kb_with_close_btn:%d", kb_with_close_btn);
        if (!kb_with_close_btn) {
            //hide back and home btn when trigger from main
            info_page_hide_btn(enter_puk_info_page_win);
        }
#if !defined (FEATURE_ROUTER)
        ds_set_bool(DS_KEY_ENABLE_SIM_PIN, false);
#endif
#ifdef CUST_SWITCH
        if (liste_enable_pin != NULL) {
            lv_obj_t * img = lv_obj_get_child(liste_enable_pin, NULL);
            lv_img_set_src(img, &ic_list_checkbox);
        }
#else
        if (enable_pin_sw != NULL) lv_sw_off(enable_pin_sw, LV_ANIM_OFF);
#endif
        //close Enable PIN kb
        num_col_close_win();
        close_popup();
    }
}

void retry_puk_error_10_times_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(mbox);
    log_d("retry_puk_error_10_times_action pressed: %s", txt);
    //lv_obj_t * mbox = lv_mbox_get_from_btn(btn);
    if (strcmp(txt, get_string(ID_PIN_MNG_INCORRECT_CONFIRM)) == 0) {
        verify_fail_str_free_alloc();
        close_popup();
#ifdef CUST_SWITCH
        if (liste_enable_pin != NULL) {
            lv_obj_t * img = lv_obj_get_child(liste_enable_pin, NULL);
            lv_img_set_src(img, &ic_list_checkbox);
        }
#else
        if (enable_pin_sw != NULL) lv_sw_off(enable_pin_sw, LV_ANIM_OFF);
#endif
        num_col_close_win();
        //close all pages
        close_all_pages();
#if !defined (FEATURE_ROUTER)
        //set enable_sim_pin false
        ds_set_bool(DS_KEY_ENABLE_SIM_PIN, false);
#endif
    }
}

char* get_pin_puk_verify_fail_string(int type, int retry_left) {
    log_d("get_pin_puk_verify_fail_string type:%d,retry_left:%d", type, retry_left);
    if (retry_left != 0) {
        char retry_count_str[2];
        memset(retry_count_str, '\0', sizeof(retry_count_str));
        sprintf(retry_count_str, "%d", retry_left);
        const char* retry_fail_msg = (
                (type == SIM_PIN) ?
                        get_string(ID_PIN_MNG_INCORRECT_PIN_ERROR) :
                        get_string(ID_PIN_MNG_INCORRECT_PUK_ERROR));
        int len = strlen(retry_fail_msg) + strlen(retry_count_str);
        //x3 for special char support
        verify_fail_str = (char*) lv_mem_alloc((sizeof(char) * len) * 3 + 1);
        memset(verify_fail_str, '\0', (sizeof(char) * len) * 3 + 1);
        sprintf(verify_fail_str, "%s%s", retry_fail_msg, retry_count_str);
    } else {
        const char* retry_fail_msg = (
                (type == SIM_PIN) ?
                        get_string(ID_PIN_MNG_INCORRECT_PIN_3_TIMES) :
                        get_string(ID_PIN_MNG_INCORRECT_PUK_10_TIMES));
        int len = strlen(retry_fail_msg);
        //x3 for special char support
        verify_fail_str = (char*) lv_mem_alloc((sizeof(char) * len) * 3 + 1);
        memset(verify_fail_str, '\0', (sizeof(char) * len) * 3 + 1);
        sprintf(verify_fail_str, "%s", retry_fail_msg);
    }
    return (verify_fail_str == NULL) ? "" : verify_fail_str;
}

void verify_fail_str_free_alloc() {
    if (verify_fail_str != NULL) {
        lv_mem_free(verify_fail_str);
        verify_fail_str = NULL;
    }
}

void pin_puk_kb_close_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    num_col_close_win();
#if defined (FEATURE_ROUTER)
    set_switch_state();
#else
#ifdef CUST_SWITCH
    if (liste_enable_pin != NULL) {
        lv_obj_t * img = lv_obj_get_child(liste_enable_pin, NULL);
        if (ds_get_bool(DS_KEY_ENABLE_SIM_PIN) == true) {
            lv_img_set_src(img, &ic_list_checkbox_selected);
        } else {
            lv_img_set_src(img, &ic_list_checkbox);
        }
    }
#else
    if (ds_get_bool(DS_KEY_ENABLE_SIM_PIN) == true) {
        if (enable_pin_sw != NULL) lv_sw_on(enable_pin_sw, LV_ANIM_OFF);
    } else {
        if (enable_pin_sw != NULL) lv_sw_off(enable_pin_sw, LV_ANIM_OFF);
    }
#endif
#endif
}

//sim pin1 verify
void sim_pin_verify(const char *pin1) {
#if defined (FEATURE_ROUTER)
    int sim_pin1_state = get_sim_pin1_state();
    log_d("sim_pin_verify pin1:%s,sim_pin1_state:%d", pin1,sim_pin1_state);
    if (sim_pin1_state == PIN_DISABLED) {
        //try to enable sim pin
        ril_return_state = enable_disable_sim_pin(1, pin1);
    } else if (sim_pin1_state == PIN_ENABLED_NOT_VERIFIED) {
        //pin try to enable but now not verified state
        ril_return_state = verify_sim_pin(pin1);
    } else if (sim_pin1_state == PIN_ENABLED_VERIFIED) {
        //try to disable sim pin
        ril_return_state = enable_disable_sim_pin(0, pin1);
    }
    log_d("sim_pin_verify ril_return_state:%d", ril_return_state);
    //1.
    //pin_puk_verify_result_cb will be triggered if call
    //enable_disable_sim_pin,verify_sim_pin SUCCESS
    //2.
    //pin_puk_verify_result_cb will not be triggered if any RIL_ERROR occurs
    //show ril_error_notice and set kb state to Enable PIN for user to retry it
    if (ril_return_state != RIL_SUCCESS) {
        ril_error_notice();
        set_pin_puk_kb_state(ID_ENABLE_PIN);
    }
#else
    log_d("sim_pin_verify pin1:%s,DS_KEY_SIM_PIN_VALUE:%s", pin1,ds_get_value(DS_KEY_SIM_PIN_VALUE));
    //for UI test if input == sim pin
    if (strcmp(pin1, ds_get_value(DS_KEY_SIM_PIN_VALUE))== 0) {
        bool value = lv_sw_get_state(enable_pin_sw);
        ds_set_bool(DS_KEY_ENABLE_SIM_PIN, value);
        num_col_close_win();
        reset_retry_state();
    }
    else {
        //if input != sim pin
        remain_pin_count--;
        if (remain_pin_count < 0) remain_pin_count = 0;
        log_d("sim_pin_verify remain_pin_count:%d", remain_pin_count);
        verify_fail_notice(SIM_PIN, remain_pin_count);
    }
#endif
}

//sim puk verify
void sim_puk_verify(char *puk1, char *new_pin1) {
#if defined (FEATURE_ROUTER)
    log_d("sim_puk_verify puk1:%s,new_pin1:%s", puk1, new_pin1);
    ril_return_state = verify_sim_puk(puk1,new_pin1);
    log_d("sim_puk_verify ril_return_state:%d", ril_return_state);
    //1.
    //pin_puk_verify_result_cb will be triggered if call verify_sim_puk SUCCESS
    //2.
    //pin_puk_verify_result_cb will not be triggered if any RIL_ERROR occurs
    //show ril_error_notice and set kb state to Enable PUK for user to retry it
    if (ril_return_state != RIL_SUCCESS) {
        ril_error_notice();
        set_pin_puk_kb_state(ID_ENABLE_PUK);
    }
#else
    //if input == sim puk
    log_d("sim_puk_verify puk1:%s,DS_KEY_SIM_PUK_VALUE:%s", puk1,ds_get_value(DS_KEY_SIM_PUK_VALUE));
    if (strcmp(puk1, ds_get_value(DS_KEY_SIM_PUK_VALUE)) == 0) {
#ifdef CUST_SWITCH
        if (liste_enable_pin != NULL) {
            lv_obj_t * img = lv_obj_get_child(liste_enable_pin, NULL);
            lv_img_set_src(img, &ic_list_checkbox_selected);
        }
#else
        if (enable_pin_sw != NULL) lv_sw_on(enable_pin_sw, LV_ANIM_OFF);
#endif
        ds_set_bool(DS_KEY_ENABLE_SIM_PIN, true);
        ds_set_value(DS_KEY_SIM_PIN_VALUE, new_pin1);
        num_col_close_win();
        reset_retry_state();
    } else {
        //if input != sim puk
        remain_puk_count--;
        if (remain_puk_count < 0) remain_puk_count = 0;
        log_d("sim_puk_verify remain_puk_count:%d", remain_puk_count);
        verify_fail_notice(SIM_PUK, remain_puk_count);
    }
#endif
}

//"Enter new PIN" != "Confirm PIN" action
void new_pin_not_match_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    const char * txt = lv_btnm_get_active_btn_text(mbox);
    log_d("new_pin_not_match_action pressed: %s", txt);
    //lv_obj_t * mbox = lv_mbox_get_from_btn(btn);
    if (strcmp(txt, get_string(ID_OK)) == 0) {
        //reuse kb,renew title, tip and callback id to Confirm PIN
        set_pin_puk_kb_state(ID_CONFIRM_PIN);
        close_popup();
    }
}

//"Enter new PIN" != "Confirm PIN"
void new_pin_not_match_notice() {
    static const char * btns[2];
    btns[0] = get_string(ID_OK);
    btns[1] = "";
    popup_scrl_create_impl(get_string(ID_PIN_MNG_INCORRECT_PIN),
            get_string(ID_PIN_MNG_ENTER_PIN_NOT_EQL_CONFIRM_PIN),
            btns, new_pin_not_match_action, NULL);
}

//pin puk keyboard callback
void pin_puk_kb_cb(int id, const char* pin_puk_info) {
    log_d("pin_puk_kb_action id:%d,pin_puk_info:%s", id, pin_puk_info);
    ///////////////////////////////////////////////////////////////
    //tap tick btn when in Enable PIN state case
    if (id == ID_ENABLE_PIN) {
        if (check_input_len_fit_4_to_8(pin_puk_info)) {
            sim_pin_verify(pin_puk_info);
        } else {
            len_err_notice(SIM_PIN);
        }
    }
    ///////////////////////////////////////////////////////////////
    //tap tick btn when in Enable PUK state case
    if (id == ID_ENABLE_PUK) {
        if (check_input_len_fit_8(pin_puk_info)) {
            //temp save Enter PUK
            memset(enter_sim_puk_value, '\0', sizeof(enter_sim_puk_value));
            strcpy(enter_sim_puk_value, pin_puk_info);
            //reuse kb,renew title, tip and callback id to Enter new PIN
            set_pin_puk_kb_state(ID_ENTER_NEW_PIN);
        } else {
            len_err_notice(SIM_PUK);
        }

    }
    ///////////////////////////////////////////////////////////////
    //tap tick btn when in Enter new PIN state case
    if (id == ID_ENTER_NEW_PIN) {
        if (check_input_len_fit_4_to_8(pin_puk_info)) {
            //temp save Enter new PIN
            memset(enter_new_sim_pin_value, '\0', sizeof(enter_new_sim_pin_value));
            strcpy(enter_new_sim_pin_value, pin_puk_info);
            //reuse kb,renew title, tip and callback id to Confirm PIN
            set_pin_puk_kb_state(ID_CONFIRM_PIN);
        } else {
            len_err_notice(SIM_PIN);
        }
    }
    ///////////////////////////////////////////////////////////////
    //tap tick btn when in Confirm PIN state case
    if (id == ID_CONFIRM_PIN) {
        if (check_input_len_fit_4_to_8(pin_puk_info)) {
            //compare the "enter new pin" and "confirm pin"
            if (strcmp(pin_puk_info, enter_new_sim_pin_value) == 0) {
                //"enter new pin" == "confirm pin"
                //go to sim puk verify
                sim_puk_verify(enter_sim_puk_value, enter_new_sim_pin_value);
            } else {
                //"Enter new PIN" != "Confirm PIN"
                new_pin_not_match_notice();
            }
        } else {
            len_err_notice(SIM_PIN);
        }
    }
    ///////////////////////////////////////////////////////////////
}

//pin or puk kb create
void pin_puk_kb_create(int kb_type, bool with_close_btn) {
    kb_with_close_btn = with_close_btn;
    log_d("pin_puk_kb_create kb_with_close_btn:%d", kb_with_close_btn);
    int click_id = ((kb_type == ID_ENABLE_PIN) ? ID_ENABLE_PIN : ID_ENABLE_PUK);//callback id
    const char* headerStr = ((kb_type == ID_ENABLE_PIN) ? get_string(ID_PIN_MNG_ENTER_PIN) : get_string(ID_PIN_MNG_ENABLE_PUK));//title
    const char* tipStr = ((kb_type == ID_ENABLE_PIN) ? get_string(ID_KB_4TO8_DIGIT_TIP) : get_string(ID_KB_PUK_TIP));//tip
    num_kb_col_pin_mng(headerStr, click_id, pin_puk_kb_cb, pin_puk_kb_close_action);
    num_kb_set_tip(tipStr);
    ((kb_type == ID_ENABLE_PIN) ? num_kb_col_set_lable_len(SIM_PIN1_MAX_LENGTH) : num_kb_col_set_lable_len(SIM_PUK1_MAX_LENGTH));
    if (!kb_with_close_btn) {
        set_kb_close_btn_hidden();
    }
}

void enter_puk_info_page_action(lv_obj_t * btnm, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    const char * txt = lv_btnm_get_active_btn_text(btnm);

    if (strcmp(txt, get_string(ID_PIN_MNG_ENTER_PUK)) == 0) {//if press Enter PUK button
        log_d("enter_puk_info_page_action kb_with_close_btn:%d",kb_with_close_btn);
        pin_puk_kb_create(ID_ENABLE_PUK, kb_with_close_btn);
        //close enter_puk_info_page
        info_page_close_win(enter_puk_info_page_win);
    }
}

void len_err_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    log_d("len_err_action chk_type:%d",chk_type);
    num_col_set_lable("");
    const char* err_msg = (chk_type == SIM_PIN) ?
                    get_string(ID_KB_4TO8_DIGIT_TIP) :
                    get_string(ID_KB_PUK_TIP);
    num_kb_set_tip(err_msg);//set tip
    close_popup();
}

void len_err_notice(int type) {
    chk_type = type;
    static const char *btns[2];
    btns[0] = get_string(ID_OK);
    btns[1] = "";
    popup_anim_not_create(
            ((chk_type == SIM_PIN) ?
                    get_string(ID_PIN_MNG_PIN_LEN_ERR) :
                    get_string(ID_PIN_MNG_PUK_LEN_ERR)), btns, len_err_action,
                    NULL);
}

#if !defined (FEATURE_ROUTER)
void reset_retry_state() {
    remain_pin_count = 3;
    remain_puk_count = 10;
}
#endif

void set_pin_puk_kb_state_impl(int headline, int id, int tip, int max_len) {
    num_col_set_lable("");
    kb_update_headline(get_string(headline));
    num_kb_col_set_select_id(id);
    num_kb_set_tip(get_string(tip));
    num_kb_col_set_lable_len(max_len);
}

//reuse the kb, renew title, tip and callback id
void set_pin_puk_kb_state(int kb_type) {
    log_d("set_pin_puk_kb_state kb_type:%d",kb_type);
    if (kb_type == ID_ENABLE_PIN) {
        set_pin_puk_kb_state_impl(ID_PIN_MNG_ENTER_PIN,
                ID_ENABLE_PIN, ID_KB_4TO8_DIGIT_TIP, SIM_PIN1_MAX_LENGTH);
    }
    if (kb_type == ID_ENABLE_PUK) {
        set_pin_puk_kb_state_impl(ID_PIN_MNG_ENABLE_PUK,
                ID_ENABLE_PUK, ID_KB_PUK_TIP, SIM_PUK1_MAX_LENGTH);
    }
    if (kb_type == ID_ENTER_NEW_PIN) {
        set_pin_puk_kb_state_impl(ID_PIN_MNG_ENTER_NEW_PIN,
                ID_ENTER_NEW_PIN, ID_KB_4TO8_DIGIT_TIP, SIM_PIN1_MAX_LENGTH);
    }
    if (kb_type == ID_CONFIRM_PIN) {
        set_pin_puk_kb_state_impl(ID_PIN_MNG_CONFIRM_PIN,
                ID_CONFIRM_PIN, ID_KB_4TO8_DIGIT_TIP, SIM_PIN1_MAX_LENGTH);
    }
}

//TODO
void ril_error_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    const char * txt = lv_btnm_get_active_btn_text(mbox);
    log_d("ril_error_action pressed:%s", txt);
    if (strcmp(txt, get_string(ID_OK)) == 0) {
        close_popup();
    }
}

//TODO temp to show ril_error_notice when sim_pin_verify() or sim_puk_verify() Error
void ril_error_notice() {
    static const char * btns[2];
    btns[0] = get_string(ID_OK);
    btns[1] = "";
    popup_anim_not_create(get_string(ID_RIL_ERR),btns, ril_error_action, NULL);
}

void close_delay_popup() {
    close_popup();
    set_switch_state();
    num_col_close_win();
}
