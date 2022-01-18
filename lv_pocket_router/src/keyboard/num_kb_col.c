#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "num_kb_col.h"
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/keyboard/en_kb.h"
#include "lv_pocket_router/src/keyboard/basic_kb.h"
#include "lv_pocket_router/res/values/string_value.h"

#include <string.h>

#define MAX_DAYS     31
#define INPUT_X      268

static void style_create(void);
static lv_style_t style_column_bg;

lv_obj_t * root_view;
lv_obj_t * lv_input;
int mCount;
int max_len;
bool date_kb = false;
bool tick_mode = true;

//to check if kb still in tip mode
//(i.e pin column display tip instead of user entered content)
static bool tip_mode = false;
static char *tip_str = NULL;
//pin column tip string style
static lv_style_t style_pin_font_dark;
static lv_style_t style_pin_font_light;

int select_id;
void (*done_callback)(int id, const char* txt);
void style_create(void){

    //for input column style
    lv_style_copy(&style_column_bg, &lv_style_plain);
    style_column_bg.body.main_color = LV_COLOR_WHITE;
    style_column_bg.body.grad_color = LV_COLOR_WHITE;
    style_column_bg.body.border.color = LV_COLOR_SILVER;
    style_column_bg.body.border.width = 2;
    style_column_bg.body.border.opa = LV_OPA_50;
    style_column_bg.body.radius = 0;
    style_column_bg.text.color = LV_COLOR_GREYISH_BROWN;
    style_column_bg.text.font = get_font(font_w_bold, font_h_22);
    style_column_bg.text.letter_space = 1;

    lv_style_copy(&style_pin_font_dark, &lv_style_plain);
    style_pin_font_dark.text.color = LV_COLOR_GREYISH_BROWN;
#if defined(HIGH_RESOLUTION)
    style_pin_font_dark.text.font = get_font(font_w_bold, font_h_40);
#else
    style_pin_font_dark.text.font = get_font(font_w_bold, font_h_22);
#endif

    lv_style_copy(&style_pin_font_light, &lv_style_plain);
    style_pin_font_light.text.color = LV_COLOR_SILVER;
    style_pin_font_light.text.letter_space = 0;
#if defined(HIGH_RESOLUTION)
    style_pin_font_light.text.font = get_font(font_w_bold, font_h_40);
#else
    style_pin_font_light.text.font = get_font(font_w_bold, font_h_22);
#endif
}

void num_kb_col_set_lable_len(int length) {
    max_len = length;
}

void num_kb_action(lv_obj_t * btnm, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;

    uint32_t index = lv_obj_get_user_data(btnm);
    uint16_t btn_id = lv_btnm_get_active_btn(btnm);
    if(btn_id == LV_BTNM_BTN_NONE) return;

    if (index == KB_NUM_BTNM) {
        const char * num = lv_btnm_get_active_btn_text(btnm);
        //clear pin column from tip string and display user input content
        if(tip_mode){
            lv_ta_set_text(lv_input, "");
            tip_mode = false;
        }
        const char* txt = lv_ta_get_text(lv_input);
        int len = strlen(txt) + strlen(num);
        if (len <= max_len) {
            if (check_valid_date(num)) {
                lv_ta_add_text(lv_input, num);
                lv_obj_set_style(lv_input, &style_pin_font_dark);
            }
        }
        if (strcmp(lv_ta_get_text(lv_input), "") != 0 && (root_view != NULL)) {
            num_kb_enable_tick_btn(true);
        }
        //enable cursor while user entering data in input column
        lv_ta_set_cursor_type(lv_input, LV_CURSOR_LINE);
    } else if (index == KB_LEFT_BTN){
        num_col_close_win();

    } else if (index == KB_RIGHT_BTN){
        num_col_close_win();

    } else if (index == KB_HOME_BTN){
        kb_home_action();

    } else if (index == KB_DEL_BTN){
        //do nothing if in tip mode
        if(tip_mode)
            return;

        const char* txt = lv_ta_get_text(lv_input);
        int len = strlen(txt);
        if (len > 1) {
            lv_ta_del_char(lv_input);
        } else if (len == 1) {
            if (tip_str != NULL && strlen(tip_str) > 0) {
                //enter tip mode
                lv_ta_set_text(lv_input, tip_str);
                tip_mode = true;
                lv_obj_set_style(lv_input, &style_pin_font_light);
                //leaving cursor mode while enter tip mode
                lv_ta_set_cursor_type(lv_input, LV_CURSOR_NONE);
            } else {
                lv_ta_del_char(lv_input);
            }
            //set inactive to tick btn cause lv_input has no content now
            if(root_view != NULL) {
                num_kb_enable_tick_btn(false);
            }
        }
    }
}

void num_col_header_style(lv_event_cb_t action, lv_obj_t * r_btn, lv_obj_t * l_btn,
        lv_obj_t * r_img, lv_obj_t * l_img, bool default_ic){
    if(action != NULL){
        lv_obj_set_event_cb(l_btn, action);
        lv_obj_set_event_cb(r_btn, action);
    }else{
        lv_obj_set_event_cb(l_btn, num_kb_action);
        lv_obj_set_event_cb(r_btn, num_kb_action);
    }
    lv_obj_set_user_data(l_btn, KB_LEFT_BTN);

    if(default_ic){
        lv_obj_set_user_data(r_btn, KB_HOME_BTN);
        lv_obj_set_event_cb(r_btn, num_kb_action);
        lv_img_set_src(l_img, &ic_headline_back);
        lv_img_set_src(r_img, &ic_headline_home);
        tick_mode = false;
    }else{
        lv_obj_set_user_data(r_btn, KB_RIGHT_BTN);
        lv_img_set_src(l_img, &ic_headline_close);
        lv_img_set_src(r_img, &ic_headline_tick);
        tick_mode = true;
    }
}

// num_col_create is to create a page with numeric keyboard in the bottom and a column to show user input data
//@ headline the header text
//@default_ic if set to true left icon will be back and right icon will be home
//if set to false left will be close and right will be tick
//@ action the action to handle header btn (i.e. back, cancel, tick)
lv_obj_t * num_col_create(const void * headline, bool default_ic, lv_event_cb_t action) {
    date_kb = false;
    mCount = 0;
    max_len = 20;

    style_create();

    root_view = basic_kb_create(headline);
    lv_obj_t * r_btn = lv_obj_get_child(root_view, NULL);
    lv_obj_t * r_img = lv_img_create(r_btn, NULL);
    lv_obj_t * l_btn = lv_obj_get_child(root_view, r_btn);
    lv_obj_t * l_img = lv_img_create(l_btn, NULL);

    //set header icon/cb according to is_ltr result
    if(is_ltr()){
        num_col_header_style(action, r_btn, l_btn, r_img, l_img, default_ic);
    }else{
        num_col_header_style(action, l_btn, r_btn, l_img, r_img, default_ic);
    }

    //draw pin column
    lv_obj_t * input_col = lv_cont_create(root_view, NULL);
    lv_obj_set_style(input_col, &style_column_bg);
    lv_obj_set_size(input_col, INPUT_X, 60);
    lv_obj_align(input_col, NULL, LV_ALIGN_CENTER, 0, -10);

    lv_input = lv_ta_create(input_col, NULL);
    lv_obj_set_style(lv_input, &style_pin_font_dark);

    lv_obj_set_size(lv_input, 260, 50);
    lv_ta_set_text(lv_input, "");
    lv_ta_set_one_line(lv_input, true);
    lv_ta_set_cursor_type(lv_input, LV_CURSOR_NONE);

    //align lv_input depend on is_ltr result
    if(is_ltr()){
        lv_ta_set_text_align(lv_input, LV_LABEL_ALIGN_LEFT);
        lv_obj_align(lv_input, input_col, LV_ALIGN_IN_LEFT_MID, 5, 0);
    }else{
        lv_ta_set_text_align(lv_input, LV_LABEL_ALIGN_RIGHT);
        lv_obj_align(lv_input, input_col, LV_ALIGN_IN_RIGHT_MID, -5, 0);
    }

    //draw numeric keyboard
    basic_num_kb(root_view, num_kb_action, num_kb_action);

    return root_view;
}

//to enable or disable header right tick btn
void num_kb_enable_tick_btn(bool enable) {
    if (!tick_mode)
        return;

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

void set_date_keyboard_style(bool state) {
    date_kb = state;
}

//check if is date_kb, is yes, check date <= 31
bool check_valid_date(const char * num) {
    bool res = false;

    if(!date_kb){
        res = true;
    } else {
        const char* txt = lv_ta_get_text(lv_input);
        int len = strlen(txt) + strlen(num);
        char label[len];
        memset(label, '\0', sizeof(label));
        sprintf(label,"%s%s", txt, num);
        if (atoi(label) <= MAX_DAYS) {//do not let user enter 01 as date
            if (!(strlen(txt) == 0 && atoi(num) == 0))
                res = true;
        }
    }
    return res;
}

//to set pin column tip string
void num_kb_set_tip(const char * tip) {
    //save tip string for later use
    int len = strlen(tip);
    //x3 for special char support
    tip_str = lv_mem_alloc((sizeof(char) * len) * 3 + 1);
    memset(tip_str, '\0', (sizeof(char) * len) * 3 + 1);
    strcpy(tip_str, tip);

    tip_mode = true;
    lv_ta_set_text(lv_input, tip);
    lv_obj_set_style(lv_input, &style_pin_font_light);
    num_kb_enable_tick_btn(false);

    //check if kb tip were too long
    kb_tip_adjust(lv_input, tip_str, INPUT_X);
}

//set input number on lv_input
void num_col_set_lable(const void * lable) {
    //if set tip before, clear tip string, show label string instead
    if(tip_mode){
        tip_mode = false;
        lv_obj_set_style(lv_input, &style_pin_font_dark);
    }
    lv_ta_set_text(lv_input, lable);
    num_kb_enable_tick_btn(true);
    lv_ta_set_cursor_type(lv_input, LV_CURSOR_LINE);
}

//get input number from lv_input
const char* num_col_get_lable(void) {
    return lv_ta_get_text(lv_input);
}

void num_col_cleanup() {
    if (tip_str != NULL) {
        lv_mem_free(tip_str);
        tip_str = NULL;
    }
}

//to close window
void num_col_close_win() {
    num_col_cleanup();
    close_kb();
}

//to hide header left close btn
void set_kb_close_btn_hidden() {
    if (is_ltr()) {
        lv_obj_t * l_btn = lv_get_child_by_index(root_view, KB_LEFT_BTN);
        lv_obj_set_hidden(l_btn, true);
    } else {
        lv_obj_t * r_btn = lv_get_child_by_index(root_view, KB_RIGHT_BTN);
        lv_obj_set_hidden(r_btn, true);
    }
}

//return if close btn got hidden
bool get_kb_close_btn_hidden() {
    lv_obj_t * l_btn = lv_get_child_by_index(root_view, KB_LEFT_BTN);
    return lv_obj_get_hidden(l_btn);

}

bool check_input_len_fit_4_to_8(const char * input) {
    bool res = false;
    int length = strlen(input);
    if (length >= 4 && length <= 8) {
        res = true;
    }
    return res;
}

bool check_input_len_fit_8(const char * input) {
    bool res = false;
    int length = strlen(input);
    if (length == 8) {
        res = true;
    }
    return res;
}

//for pin management tick btn action
void pin_mng_r_action(lv_obj_t * btn, lv_event_t event) {

    if (event != LV_EVENT_CLICKED) return;

    if(get_tick_btn_ina_state(get_kb_tick_btn(root_view))) return;

    num_col_cleanup();
    if (done_callback != NULL) {
        (*done_callback)(select_id, lv_ta_get_text(lv_input));
    }
}

void num_kb_col_pin_mng(const void * headline, int btn_click_id,
        void (*callback)(int id, const char* txt), lv_event_cb_t action) {
    select_id = btn_click_id;
    done_callback = callback;

    lv_obj_t * kb = num_col_create(headline, false, action);
    if (is_ltr()) {
        lv_obj_t * r_btn = lv_get_child_by_index(kb, KB_RIGHT_BTN);
        lv_obj_set_event_cb(r_btn, pin_mng_r_action);
    } else {
        lv_obj_t * l_btn = lv_get_child_by_index(kb, KB_LEFT_BTN);
        lv_obj_set_event_cb(l_btn, pin_mng_r_action);
    }
}

void num_kb_col_set_select_id(int btn_click_id) {
    select_id = btn_click_id;
}
