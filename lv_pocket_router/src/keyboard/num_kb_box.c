/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "num_kb_box.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/keyboard/basic_kb.h"

#define PWD_LENGTH       5
#define KB_BOX_SIZE      60
#define TA_SHIFT_VER     20
#define TA_SHIFT_HOR     15
#define INPUT_DONE_DELAY 600

void num_kb_box_action(lv_obj_t * btnm, lv_event_t event);
void del_click_listener(lv_obj_t * imgbtn, lv_event_t event);

lv_obj_t * root_view;
lv_obj_t * kb_txt_1;
lv_obj_t * kb_txt_2;
lv_obj_t * kb_txt_3;
lv_obj_t * kb_txt_4;
static char *password = NULL;
int mCount;

static lv_style_t style_box_bg;
static lv_style_t style_ta_bg;
void (*pwd_check_func) (char *);
int num_kb_time_box_style;
lv_obj_t ** kb_box_list[] = {&kb_txt_1, &kb_txt_2, &kb_txt_3, &kb_txt_4};

static void style_create(void){

    //for input box style
    lv_style_copy(&style_box_bg, &lv_style_plain);
    style_box_bg.body.main_color = LV_COLOR_WHITE;
    style_box_bg.body.grad_color = LV_COLOR_WHITE;
    style_box_bg.body.border.color = LV_COLOR_SILVER;
    style_box_bg.body.border.width = 2;
    style_box_bg.body.border.opa = LV_OPA_50;
    style_box_bg.body.radius = 0;

    //for input box ta style
    lv_style_copy(&style_ta_bg, &lv_style_plain);
    style_ta_bg.body.border.width = 0;
    style_ta_bg.body.opa = LV_OPA_TRANSP;
    style_ta_bg.text.color = LV_COLOR_GREYISH_BROWN;
    style_ta_bg.text.font = get_font(font_w_bold, font_h_26);
}

void num_kb_header_style(lv_event_cb_t action, lv_obj_t * r_btn, lv_obj_t * l_btn,
        lv_obj_t * r_img, lv_obj_t * l_img, bool hashomekey){

    if (action == NULL) {
        lv_obj_set_event_cb(l_btn, num_kb_box_action);
    } else {
        lv_obj_set_event_cb(l_btn, action);
    }
    if(is_ltr()){
        lv_img_set_src(l_img, &ic_headline_back);
    }else{
        lv_img_set_src(l_img, &ic_headline_back_rtl);
    }
    lv_obj_set_user_data(l_btn, KB_LEFT_BTN);

    if(hashomekey){
        lv_obj_set_user_data(r_btn, KB_HOME_BTN);
        lv_obj_set_event_cb(r_btn, num_kb_box_action);
        lv_img_set_src(r_img, &ic_headline_home);
    }else{
        lv_obj_set_hidden(r_btn, 1);
    }
}

lv_obj_t * num_kb_box_create(void (*fp)(char *), const void * headline, bool hashomekey, lv_event_cb_t action){

    mCount = 0;
    style_create();

    root_view = basic_kb_create(headline);
    lv_obj_t * r_btn = lv_obj_get_child(root_view, NULL);
    lv_obj_t * r_img = lv_img_create(r_btn, NULL);
    lv_obj_t * l_btn = lv_obj_get_child(root_view, r_btn);
    lv_obj_t * l_img = lv_img_create(l_btn, NULL);

    //set header icon/cb according to is_ltr result
    if(is_ltr()){
        num_kb_header_style(action, r_btn, l_btn, r_img, l_img, hashomekey);
    }else{
        num_kb_header_style(action, l_btn, r_btn, l_img, r_img, hashomekey);
    }

    //draw the 4 box
    lv_obj_t * box1 = lv_cont_create(root_view, NULL);
    lv_obj_set_size(box1, KB_BOX_SIZE, KB_BOX_SIZE);
    lv_obj_set_style(box1, &style_box_bg);
    lv_obj_align(box1, NULL, LV_ALIGN_IN_TOP_LEFT, 30, 80);
    lv_obj_t * box2 = lv_cont_create(root_view, box1);
    lv_obj_align(box2, box1, LV_ALIGN_IN_LEFT_MID, 65, 0);
    lv_obj_t * box3 = lv_cont_create(root_view, box1);
    lv_obj_align(box3, box2, LV_ALIGN_IN_LEFT_MID, 65, 0);
    lv_obj_t * box4 = lv_cont_create(root_view, box1);
    lv_obj_align(box4, box3, LV_ALIGN_IN_LEFT_MID, 65, 0);

    //first insert column
    kb_txt_1 = lv_ta_create(box1, NULL);
    lv_ta_set_pwd_mode(kb_txt_1, true);
    lv_ta_set_style(kb_txt_1, LV_TA_STYLE_BG, &style_ta_bg);
    lv_ta_set_cursor_type(kb_txt_1, LV_CURSOR_NONE);
    lv_obj_set_size(kb_txt_1, KB_BOX_SIZE, KB_BOX_SIZE);
    lv_obj_align(kb_txt_1, NULL, LV_ALIGN_CENTER, TA_SHIFT_VER, TA_SHIFT_HOR);

    //second insert column
    kb_txt_2 = lv_ta_create(box2, kb_txt_1);
    lv_obj_align(kb_txt_2, NULL, LV_ALIGN_CENTER, TA_SHIFT_VER, TA_SHIFT_HOR);

    //third insert column
    kb_txt_3 = lv_ta_create(box3, kb_txt_1);
    lv_obj_align(kb_txt_3, NULL, LV_ALIGN_CENTER, TA_SHIFT_VER, TA_SHIFT_HOR);

    //forth insert column
    kb_txt_4 = lv_ta_create(box4, kb_txt_1);
    lv_obj_align(kb_txt_4, NULL, LV_ALIGN_CENTER, TA_SHIFT_VER, TA_SHIFT_HOR);

    //enable cursor in first box
    lv_ta_set_cursor_type(kb_txt_1, LV_CURSOR_LINE);
    num_kb_reset_val();

    //draw numeric keyboard
    basic_num_kb(root_view, num_kb_box_action, del_click_listener);

    //callback function when user finish enter all 4 char
    pwd_check_func = fp;

    return root_view;
}

void kb_box_set_text(bool insert, int id, const char * txt){
    //set txt on "id" box
    lv_ta_set_text(*kb_box_list[id - 1], txt);

    //set cursor according to inset/delete mode
    if(insert){
        lv_ta_set_cursor_type(*kb_box_list[id - 1], LV_CURSOR_NONE);
        if(id != INDEX_COLIMN_4){
            lv_ta_set_cursor_type(*kb_box_list[id], LV_CURSOR_LINE);
        }
    }else{
        lv_ta_set_cursor_type(*kb_box_list[id - 1], LV_CURSOR_LINE);
        if(id != INDEX_COLIMN_4){
            lv_ta_set_cursor_type(*kb_box_list[id], LV_CURSOR_NONE);
        }
    }
}

void del_click_listener(lv_obj_t * imgbtn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    uint32_t index = lv_obj_get_user_data(imgbtn);

    //do nothing if all 4 box is empty
    if (mCount == 0) return;
    if (mCount >= 1) {
        if (index == KB_DEL_BTN) {
            kb_box_set_text(false, mCount, "");
            mCount --;
        }
    }
}

void pwd_input_done_action(){
    if(root_view != NULL) {
        password = (char *) lv_mem_alloc(PWD_LENGTH);
        memset(password, '\0', PWD_LENGTH);
        snprintf(password,PWD_LENGTH, "%s%s%s%s", lv_ta_get_text(kb_txt_1), lv_ta_get_text(kb_txt_2),
                lv_ta_get_text(kb_txt_3), lv_ta_get_text(kb_txt_4));
        pwd_check_func(password);
    }
}

void num_kb_reset_val(){
    lv_ta_set_text(kb_txt_1, "");
    lv_ta_set_text(kb_txt_2, "");
    lv_ta_set_text(kb_txt_3, "");
    lv_ta_set_text(kb_txt_4, "");
}
//refresh title string and clear all 4 box's value
void num_kb_refresh_title(const char * title) {
    log_d("num_kb title = %s", title);
    kb_update_headline(title);

    num_kb_reset_val();
    mCount = 0;
}

//need to call cleanup if user overwrite back key action
void num_kb_box_cleanup(){
    if (password != NULL){
        lv_mem_free(password);
        password = NULL;
    }
    if(root_view != NULL) {
        root_view = NULL;
    }
}

void num_kb_box_close() {
    num_kb_box_cleanup();
    close_kb();
}

void num_kb_box_action(lv_obj_t * btnm, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    uint32_t index = lv_obj_get_user_data(btnm);
    if (index == KB_LEFT_BTN) {
        num_kb_box_cleanup();

    } else if (index == KB_HOME_BTN) {
        num_kb_box_cleanup();
        kb_home_action();

    } else if (index == KB_NUM_BTNM && (mCount < 4)) {
        //do nothing if all 4 box is full
        const char * txt = lv_btnm_get_active_btn_text(btnm);
        if (txt != NULL) {
            mCount ++;
            kb_box_set_text(true, mCount, txt);
            if (mCount == INDEX_COLIMN_4) {
                lv_task_t *  task = lv_task_create(pwd_input_done_action, INPUT_DONE_DELAY, LV_TASK_PRIO_MID, NULL);
                lv_task_once(task);
            }
        }
    }
}

void num_kb_header_style_default(lv_event_cb_t action, lv_obj_t * r_btn,
        lv_obj_t * l_btn, lv_obj_t * r_img, lv_obj_t * l_img) {
    lv_obj_set_event_cb(l_btn, action);
    lv_obj_set_event_cb(r_btn, action);
    lv_obj_set_user_data(l_btn, KB_LEFT_BTN);
    lv_obj_set_user_data(r_btn, KB_RIGHT_BTN);
    lv_img_set_src(l_img, &ic_headline_close);
    lv_img_set_src(r_img, &ic_headline_tick);
}

lv_obj_t * num_kb_time_box_create(int time_box_style, const void * headline,
        lv_event_cb_t action) {
    style_create();
    root_view = basic_kb_create(headline);
    lv_obj_t * r_btn = lv_obj_get_child(root_view, NULL);
    lv_obj_t * r_img = lv_img_create(r_btn, NULL);
    lv_obj_t * l_btn = lv_obj_get_child(root_view, r_btn);
    lv_obj_t * l_img = lv_img_create(l_btn, NULL);

    //set header icon/cb according to is_ltr result
    if(is_ltr()){
        num_kb_header_style_default(action, r_btn, l_btn, r_img, l_img);
    }else{
        num_kb_header_style_default(action, l_btn, r_btn, l_img, r_img);
    }
    //disable tick
    num_kb_box_enable_tick_btn(false);
    //draw the 4 box
    lv_obj_t * box1 = lv_cont_create(root_view, NULL);
    lv_obj_set_size(box1, KB_BOX_SIZE, KB_BOX_SIZE);
    lv_obj_set_style(box1, &style_box_bg);
    lv_obj_align(box1, NULL, LV_ALIGN_IN_TOP_LEFT, 30, 80);
    lv_obj_t * box2 = lv_cont_create(root_view, box1);
    lv_obj_align(box2, box1, LV_ALIGN_IN_LEFT_MID, 65, 0);
    lv_obj_t * box3 = lv_cont_create(root_view, box1);
    lv_obj_align(box3, box2, LV_ALIGN_IN_LEFT_MID,
            (time_box_style == KB_YEAR) ? 65 : 70, 0);
    lv_obj_t * box4 = lv_cont_create(root_view, box1);
    lv_obj_align(box4, box3, LV_ALIGN_IN_LEFT_MID, 65, 0);

    //first insert column
    kb_txt_1 = lv_ta_create(box1, NULL);
    lv_ta_set_style(kb_txt_1, LV_TA_STYLE_BG, &style_ta_bg);
    lv_ta_set_cursor_type(kb_txt_1, LV_CURSOR_NONE);
    lv_obj_set_size(kb_txt_1, KB_BOX_SIZE, KB_BOX_SIZE);
    lv_obj_align(kb_txt_1, NULL, LV_ALIGN_CENTER, TA_SHIFT_VER, TA_SHIFT_HOR);

    //second insert column
    kb_txt_2 = lv_ta_create(box2, kb_txt_1);
    lv_obj_align(kb_txt_2, NULL, LV_ALIGN_CENTER, TA_SHIFT_VER, TA_SHIFT_HOR);

    if (time_box_style == KB_MONTH_DATE) {
        lv_obj_t * img = lv_img_create(root_view, NULL);
        lv_img_set_src(img, &slash_date);
        lv_obj_align(img, box2, LV_ALIGN_IN_LEFT_MID, 57, 0);
    }
    if (time_box_style == KB_HOUR_MINUTE) {
        lv_obj_t * img = lv_img_create(root_view, NULL);
        lv_img_set_src(img, &slash_time);
        lv_obj_align(img, box2, LV_ALIGN_IN_LEFT_MID, 57, 0);
    }

    //third insert column
    kb_txt_3 = lv_ta_create(box3, kb_txt_1);
    lv_obj_align(kb_txt_3, NULL, LV_ALIGN_CENTER, TA_SHIFT_VER, TA_SHIFT_HOR);

    //forth insert column
    kb_txt_4 = lv_ta_create(box4, kb_txt_1);
    lv_obj_align(kb_txt_4, NULL, LV_ALIGN_CENTER, TA_SHIFT_VER, TA_SHIFT_HOR);
    if (time_box_style == KB_YEAR) {
        num_kb_time_box_year_default_val();
        lv_ta_set_cursor_type(kb_txt_3, LV_CURSOR_LINE);
    } else {
        num_kb_reset_val();
        lv_ta_set_cursor_type(kb_txt_1, LV_CURSOR_LINE);
    }

    //draw numeric keyboard
    basic_num_kb(root_view, num_kb_time_box_action, time_box_del_listener);

    num_kb_time_box_style = time_box_style;
    log_d("num_kb_time_box_style:%d",num_kb_time_box_style);
    return root_view;
}

void num_kb_time_box_year_default_val() {
    lv_ta_set_text(kb_txt_1, "2");
    lv_ta_set_text(kb_txt_2, "0");
    lv_ta_set_text(kb_txt_3, "");
    lv_ta_set_text(kb_txt_4, "");
    mCount = 2;
}

void num_kb_time_box_action(lv_obj_t * btnm, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;

    uint32_t index = lv_obj_get_user_data(btnm);
    const char * txt = lv_btnm_get_active_btn_text(btnm);
    log_d("num_kb_time_box_action txt:%s",txt);
    if (txt != NULL && index == KB_NUM_BTNM && (mCount < 4)) {
        //do nothing if all 4 box is full
        mCount ++;
        if (mCount == INDEX_COLIMN_1) {//box1
            //year case
            if(num_kb_time_box_style == KB_YEAR) {
                if (strcmp(lv_ta_get_text(kb_txt_1), "") == 0 && (root_view != NULL)) {
                    kb_box_set_text(true, mCount, txt);
                }
            }
            //month & date or hour & minute cases
            if (num_kb_time_box_style == KB_MONTH_DATE || num_kb_time_box_style == KB_HOUR_MINUTE) {
                //month & date field1 = 0 or field1 = 1
                //hour & minute field1 = 0 or field1 = 1
                if (atoi(txt) == 0 || atoi(txt) == 1) {
                    kb_box_set_text(true, mCount, txt);
                } else {
                    mCount--;//don't set txt if not meet condition
                }
            }
        } else if (mCount == INDEX_COLIMN_2) {//box2
            //year case
            if(num_kb_time_box_style == KB_YEAR) {
                if (strcmp(lv_ta_get_text(kb_txt_2), "") == 0 && (root_view != NULL)) {
                    kb_box_set_text(true, mCount, txt);
                }
            }
            //month & date or hour & minute cases
            if(num_kb_time_box_style == KB_MONTH_DATE || num_kb_time_box_style == KB_HOUR_MINUTE) {
                //month & date field1 = 0,field2 = 1~9 ==> (01~09)
                //month & date field1 = 1,field2 = 0~2 ==> (10~12)
                //hour & minute field1 = 0,field2 = 1~9 ==> (01~09)
                //hour & minute field1 = 1,field2 = 0~2 ==> (10~12)
                if ((atoi(lv_ta_get_text(kb_txt_1)) == 0
                        && ((atoi(txt) <= 9) && (atoi(txt) >= 1)))
                        || (atoi(lv_ta_get_text(kb_txt_1)) == 1
                                && (atoi(txt) <= 2 && atoi(txt) >= 0))) {
                    kb_box_set_text(true, mCount, txt);
                } else {
                    mCount--;//don't set txt if not meet condition
                }
            }
        } else if (mCount == INDEX_COLIMN_3) {//box3
            //year case
            if(num_kb_time_box_style == KB_YEAR) {
                //workaround to limit time setting before 2037/12/31 to avoid error
                if(atoi(txt) <= 3 && atoi(txt) >= 0) {
                    kb_box_set_text(true, mCount, txt);
                } else {
                    mCount--;//don't set txt if not meet condition
                }
            }
            //month & date case
            if(num_kb_time_box_style == KB_MONTH_DATE) {
                //month & date field3 = 0~3(Feb is 0~2)
                if (((atoi(lv_ta_get_text(kb_txt_1)) == 0)
                        && (atoi(lv_ta_get_text(kb_txt_2)) == 2)
                        && atoi(txt) <= 2 && atoi(txt) >= 0)//(Feb is 0~2)
                        || (((atoi(lv_ta_get_text(kb_txt_1)) == 0//(except Feb is 0~3)
                                && atoi(lv_ta_get_text(kb_txt_2)) != 2)
                        || atoi(lv_ta_get_text(kb_txt_1)) == 1)
                                && atoi(txt) <= 3 && atoi(txt) >= 0)) {
                    kb_box_set_text(true, mCount, txt);
                } else {
                    mCount--;//don't set txt if not meet condition
                }
            }
            //hour & minute case
            if(num_kb_time_box_style == KB_HOUR_MINUTE) {
                //hour & minute field3 = 0~5
                if (atoi(txt) <= 5 && atoi(txt) >= 0) {
                    kb_box_set_text(true, mCount, txt);
                } else {
                    mCount--;//don't set txt if not meet condition
                }
            }
        } else if (mCount == INDEX_COLIMN_4) {//box4
            //year case
            if (num_kb_time_box_style == KB_YEAR) {
                //workaround to limit time setting before 2037/12/31 to avoid error
                if (((atoi(lv_ta_get_text(kb_txt_3)) == 0
                        || atoi(lv_ta_get_text(kb_txt_3)) == 1
                        || atoi(lv_ta_get_text(kb_txt_3)) == 2)
                        && atoi(txt) <= 9)
                        || (atoi(lv_ta_get_text(kb_txt_3)) == 3
                                && atoi(txt) <= 7)) {
                    kb_box_set_text(true, mCount, txt);
                    num_kb_box_enable_tick_btn(true);
                } else {
                    mCount--;//don't set txt if not meet condition
                }
            }
            //month date case
            if(num_kb_time_box_style == KB_MONTH_DATE) {
                //month & date field3 = 0,field4 = 1~9 ==> (01~09)
                //month & date field3 = 1,field4 = 0~9 ==> (10~19)
                //month & date field3 = 2,field4 = 0~9 ==> (20~29)
                //month & date field3 = 3,field4 = 0~1 ==> (30~31)
                if ((atoi(lv_ta_get_text(kb_txt_3)) == 0 && atoi(txt) <= 9
                        && atoi(txt) >= 1)
                        || ((atoi(lv_ta_get_text(kb_txt_3)) == 1
                                || atoi(lv_ta_get_text(kb_txt_3)) == 2)
                                && atoi(txt) <= 9 && atoi(txt) >= 0)
                        || (atoi(lv_ta_get_text(kb_txt_3)) == 3
                                && atoi(txt) <= 1 && atoi(txt) >= 0)) {
                    kb_box_set_text(true, mCount, txt);
                    num_kb_box_enable_tick_btn(true);
                } else {
                    mCount--;//don't set txt if not meet condition
                }
            }
            //hour & minute case
            if (num_kb_time_box_style == KB_HOUR_MINUTE) {
                kb_box_set_text(true, mCount, txt);
                num_kb_box_enable_tick_btn(true);
            }
        }
    }
    //log_d("num_kb_time_box_action mCount:%d",mCount);
}

void time_box_del_listener(lv_obj_t * imgbtn, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;
    uint32_t index = lv_obj_get_user_data(imgbtn);
    //do nothing if all 4 box is empty
    if (mCount == 0) return;

    if (mCount >= 1) {
        if (index == KB_DEL_BTN) {
            if ((mCount == INDEX_COLIMN_1 && num_kb_time_box_style != KB_YEAR)
                    || (mCount == INDEX_COLIMN_2 && num_kb_time_box_style != KB_YEAR)
                    || mCount == INDEX_COLIMN_3
                    || mCount == INDEX_COLIMN_4) {
                kb_box_set_text(false, mCount, "");
            }
            if ((num_kb_time_box_style != KB_YEAR)
                    || ((num_kb_time_box_style == KB_YEAR) && mCount > 2)) {
                mCount--;
            }
        }
    }
    //log_d("time_box_del_listener mCount:%d",mCount);
    num_kb_box_enable_tick_btn(false);
}

void set_num_kb_time_box_style(int style) {
    num_kb_time_box_style = style;
}

void num_kb_time_box_close() {
    set_num_kb_time_box_style(KB_UNKNOWN);
    close_kb();
    mCount = 0;
}

const char* get_num_kb_box(int index) {
    const char* res;
    switch (index) {
    case INDEX_COLIMN_1:
        res = lv_ta_get_text(kb_txt_1);
        break;
    case INDEX_COLIMN_2:
        res = lv_ta_get_text(kb_txt_2);
        break;
    case INDEX_COLIMN_3:
        res = lv_ta_get_text(kb_txt_3);
        break;
    case INDEX_COLIMN_4:
        res = lv_ta_get_text(kb_txt_4);
        break;
    }
    return res;
}

bool check_date_valid(int year, int month, int date) {
    log_d("check_date_valid year:%d month:%d date:%d", year, month, date);
    //workaround to limit time setting before 2037/12/31 to avoid error
    if (year > 2037) return false;
    int days_of_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2) {
        (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0)) ?
                days_of_month[1] += 1 : days_of_month[1];//leap year check
    }
    if (month > 12 || month < 1 || date > days_of_month[month - 1] || date < 1) {
        return false;
    }
    return true;
}

//to enable or disable header right tick btn
void num_kb_box_enable_tick_btn(bool enable) {
    lv_obj_t * tick_btn = get_kb_tick_btn(root_view);
    lv_obj_t * img = lv_obj_get_child(tick_btn, NULL);
    if(enable) {
        lv_img_set_src(img, &ic_headline_tick);
        lv_btn_set_state(tick_btn, LV_BTN_STYLE_REL);
    } else {
        lv_img_set_src(img, &ic_headline_tick_dis);
        lv_btn_set_state(tick_btn, LV_BTN_STATE_INA);
    }
}
