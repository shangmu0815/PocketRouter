#include "en_kb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/ssid/ssid.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/keyboard/basic_kb.h"
#include "lv_pocket_router/src/keyboard/num_kb_col.h"

#define MAX_INPUT_LENGTH 20
#define HEADER_TXT_SHIFT 12
#define INPUT_X          208

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void style_create(void);

/**********************
 *  STATIC VARIABLES
 **********************/
//numeric keyboard map
static const char * num_kb_map[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ""};

//lower case keyboard map
static const char * lc_kb_map[] = {
    "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", ""
};

//upper case keyboard map
static const char * uc_kb_map[] = {
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", ""
};

//special character keyboard map
static const char * others_kb_map[] = {"!", "#", "$", "(", ")", "*", "-", ".", "/", "=","@", "[", "]", "^", "_", "'", "{", "|", "}", "~", "%", "&", "+", "\\", ":", ";", "\"", "<", ">", "\,", "?", ""};


//btn matrix for bottom row
static const char * kb_map_br[] = {
    "123", "a/A", "!#$", " ", ""
};

static const char * kb_map_br_uc[] = {
    "123", "A/a", "!#$", " ", ""
};

enum {
    NUM_KB,
    LC_KB,
    UC_KB,
    OTHER_KB
};

static lv_style_t style_btn_bg;
static lv_style_t style_btn_rel;
static lv_style_t style_btn_pr;

//for delete button
static lv_style_t style_d_btn_rel_ltr;
static lv_style_t style_d_btn_rel_rtl;
static lv_style_t style_d_btn_pr_ltr;
static lv_style_t style_d_btn_pr_rtl;

static lv_style_t style_column_bg;
//pin column tip string style
static lv_style_t style_pin_font_dark;
static lv_style_t style_pin_font_light;

static lv_obj_t * root_view;

//bottom layer of the keyboard (fixed)
static lv_obj_t * kb_br;
//current input pin
static lv_obj_t * lv_input;

//the max for kb that can slide
static const uint32_t MAX_KB = 31;

//number of en character
static const uint32_t EN_CHAR_COUNT = 26;

//number of en character
static const uint32_t NUM_CHAR_COUNT = 10;

//vertical value of one kb on upper row(can slide)
static const uint32_t KB_VER = 52 * LV_RES_OFFSET;
static const uint32_t KB_HOR = 36 * LV_RES_OFFSET;

//for real upper row kb button
lv_obj_t * kb_btn[33];
lv_obj_t * ur_label[33];
lv_obj_t * ur_page;

//to check if kb still in tip mode
//(i.e pin column display tip instead of user entered content)
bool tip_mode;
bool pwd_mode;
bool input_allow_empty;
static char *tip_str = NULL;

void (*done_callback)(int id, const char* txt);
int click_id;
bool is_uc_kb;
int mCurrentKB;
int max_length = MAX_INPUT_LENGTH;

//to fix en kb dragging issue
lv_point_t point_last;
lv_point_t point_pressed = {0,0};
bool kb_dragging = false;
bool kb_pressed = false;
int kb_click_id = -1;

static lv_style_t style_profile;
static lv_style_t style_profile_rtl;
//pin input column support chinese in profile management
bool profile_font_style = false;

void style_create(void) {
    //keyboard btnm style
    lv_style_copy(&style_btn_bg, &lv_style_plain);
    style_btn_bg.body.main_color = LV_COLOR_SILVER;
    style_btn_bg.body.grad_color = LV_COLOR_SILVER;
    style_btn_bg.body.border.color = LV_COLOR_REAL_WHITE;
    style_btn_bg.body.border.width = 1;
    style_btn_bg.body.padding.left = 0;
    style_btn_bg.body.padding.right = 0;
    style_btn_bg.body.padding.top = 0;
    style_btn_bg.body.padding.bottom = 0;
    style_btn_bg.body.padding.inner = 0;

    lv_style_copy(&style_btn_rel, &lv_style_btn_rel);
    style_btn_rel.body.main_color = LV_COLOR_SILVER;
    style_btn_rel.body.grad_color = LV_COLOR_SILVER;
    style_btn_rel.body.border.color = LV_COLOR_REAL_WHITE;
    style_btn_rel.body.border.width = 1;
    style_btn_rel.body.border.opa = LV_OPA_50;
    style_btn_rel.body.radius = 0;
#if defined(HIGH_RESOLUTION)
    style_btn_rel.text.font = get_font(font_w_bold, font_h_40);
#else
    style_btn_rel.text.font = get_font(font_w_bold, font_h_26);
#endif
    style_btn_rel.text.color = LV_COLOR_GREYISH_BROWN;
    style_btn_rel.text.letter_space = 1;

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.body.main_color = LV_COLOR_BASE;
    style_btn_pr.body.grad_color = LV_COLOR_BASE;
    style_btn_pr.text.color = LV_COLOR_WHITE;
    style_btn_pr.image.color = LV_COLOR_WHITE;
    style_btn_pr.image.intense = LV_OPA_COVER;

    //delete column style
    lv_style_copy(&style_column_bg, &lv_style_plain);
    style_column_bg.body.main_color = LV_COLOR_WHITE;
    style_column_bg.body.grad_color = LV_COLOR_WHITE;
    style_column_bg.body.border.color = LV_COLOR_SILVER;
    style_column_bg.body.border.width = 2;
    style_column_bg.body.border.opa = LV_OPA_50;
    style_column_bg.body.radius = 0;
    style_column_bg.text.color = LV_COLOR_GREYISH_BROWN;
#if defined(HIGH_RESOLUTION)
    style_column_bg.text.font = get_font(font_w_bold, font_h_40);
#else
    style_column_bg.text.font = get_font(font_w_bold, font_h_22);
#endif

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
    style_pin_font_light.text.font = get_font(font_w_bold, (is_ltr() ? font_h_22 : font_h_18));
#endif

    lv_style_copy(&style_d_btn_rel_ltr, &style_column_bg);
    //bottom, top, left
    style_d_btn_rel_ltr.body.border.part = 0x0B;

    lv_style_copy(&style_d_btn_rel_rtl, &style_d_btn_rel_ltr);
    //bottom, top, right
    style_d_btn_rel_rtl.body.border.part = 0x07;

    lv_style_copy(&style_d_btn_pr_ltr, &style_d_btn_rel_ltr);
    style_d_btn_pr_ltr.body.main_color = LV_COLOR_BASE;
    style_d_btn_pr_ltr.body.grad_color = LV_COLOR_BASE;

    lv_style_copy(&style_d_btn_pr_rtl, &style_d_btn_rel_rtl);
    style_d_btn_pr_rtl.body.main_color = LV_COLOR_BASE;
    style_d_btn_pr_rtl.body.grad_color = LV_COLOR_BASE;
    //use NotoSansCJKjp_Bold_24 font to support chinese for all language
    //when in profile management
    lv_style_copy(&style_profile, &lv_style_plain);
    style_profile.text.font = get_locale_font_cust(font_w_bold, font_h_22);
    style_profile.text.letter_space = 1;

    lv_style_copy(&style_profile_rtl, &style_profile);
    style_profile_rtl.text.font = get_locale_font(AR, font_w_bold, font_h_22);
}

void update_label(const char* value) {
    //clear pin column from tip string and display user input content
    if(tip_mode){
        en_kb_set_tip_impl(false);
    }
    const char* txt = lv_ta_get_text(lv_input);
    int len = strlen(txt) + strlen(value);

    if (len <= max_length) {
        lv_ta_add_text(lv_input, value);
        lv_obj_set_style(lv_input,
                ((!profile_font_style) ? &style_pin_font_dark : (is_ltr() ? &style_profile : &style_profile_rtl)));
    }
    if (strcmp(lv_ta_get_text(lv_input), "") != 0 && (root_view != NULL)) {
        enable_tick_btn(true);
    }
}

//delete button action
void del_btn_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char* txt = lv_ta_get_text(lv_input);
    int len = strlen(txt);

    if(tip_mode)
        return;

    if (len > 1) {
        lv_ta_del_char(lv_input);
    } else if (len == 1) {
        if (tip_str != NULL && strlen(tip_str) > 0) {
            //enter tip mode
            en_kb_set_tip_impl(true);
        } else {
            lv_ta_del_char(lv_input);
        }
    }
}

//use "normal" param to decide weather to set real PR style or not
void en_kb_set_btn_style(int id, bool normal){

    if(id < 0) return;

    if(normal){
        lv_btn_set_style(kb_btn[id], LV_BTN_STYLE_PR, &style_btn_pr);
    }else{
        lv_btn_set_style(kb_btn[id], LV_BTN_STYLE_PR, &style_btn_rel);
    }

}

//return true if x_diff > 0 i.e is dragging
bool get_x_diff(lv_indev_t * indev, lv_point_t point_last){
    lv_point_t point_act;
    lv_indev_get_point(indev, &point_act);
    lv_coord_t x_diff = point_act.x - point_last.x;

    return x_diff != 0 ? true:false;
}

//upper row en kb action
//1. we will set normal PR style for clicked btn
//2. btn will not show PR effect if user dragging
//3. skip user input content if user dragging
void ur_kb_action(lv_obj_t * btn, lv_event_t event, int kb_style) {
    lv_indev_t * indev = lv_indev_get_act();
    if (!indev) return;

    uint8_t id = lv_obj_get_user_data(btn);

    if(kb_pressed){
        if(get_x_diff(indev, point_pressed)){
            kb_dragging = true;
            en_kb_set_btn_style(id, false);
            en_kb_set_btn_style(kb_click_id, false);
        } else{
           if((kb_click_id == id) && !kb_dragging){
               //btn clicked, set to normal PR style
               en_kb_set_btn_style(id, true);
           }
        }
    }
    if(event == LV_EVENT_PRESSED){
        if(point_pressed.x == 0 && point_pressed.y == 0){
            kb_pressed = true;
            lv_indev_get_point(indev, &point_pressed);
        }
        if(kb_click_id < 0){
            kb_click_id = id;
            lv_indev_get_point(indev, &point_last);
        }

    } else if (event == LV_EVENT_CLICKED) {
        if(get_x_diff(indev, point_last)){
            log_d("user dragging, skip en_kb click");
            return;
        }
        switch (kb_style) {
            case NUM_KB:
                update_label(num_kb_map[id]);
                break;
            case LC_KB:
                update_label(lc_kb_map[id]);
                break;
            case UC_KB:
                update_label(uc_kb_map[id]);
                break;
            case OTHER_KB:
                update_label(others_kb_map[id]);
                break;
        }

    } else if(event == LV_EVENT_PRESS_LOST || event == LV_EVENT_RELEASED) {
        kb_dragging = false;
        en_kb_set_btn_style(id, false);

        if(event == LV_EVENT_RELEASED){
            point_pressed.x = 0;
            point_pressed.y = 0;
            kb_pressed = false;
            kb_click_id = -1;
        }
    }
}

//numeric keyboard action
void en_kb_action(lv_obj_t * btn, lv_event_t event) {
    ur_kb_action(btn, event, NUM_KB);
}

//lower case keyboard action
void lc_kb_action(lv_obj_t * btn, lv_event_t event) {
    ur_kb_action(btn, event, LC_KB);
}

//upper case keyboard action
void uc_kb_action(lv_obj_t * btn, lv_event_t event) {
    ur_kb_action(btn, event, UC_KB);
}

//other character keyboard action
void others_kb_action(lv_obj_t * btn, lv_event_t event) {
    ur_kb_action(btn, event, OTHER_KB);
}

//other character keyboard action
void space_btn_action(lv_obj_t * btn, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;

    update_label(" ");
}

void en_kb_set_lable_length(int length) {
    max_length = length;
}

//change callback id in case you want to reuse kb
void en_kb_set_cb_id(int id){
    click_id = id;
}

//set tip string and do necessary setting before enable/disable tip mode
void en_kb_set_tip_impl(bool enable){
    if(enable){
        lv_ta_set_text(lv_input, tip_str);
        lv_ta_set_cursor_type(lv_input, LV_CURSOR_NONE);
        lv_obj_set_style(lv_input, &style_pin_font_light);

        //check if kb tip were too long
        kb_tip_adjust(lv_input, tip_str, INPUT_X);

        //always make tick btn inactive while in tip mode
        enable_tick_btn(false);
    } else{
        lv_ta_set_text(lv_input, "");
        lv_ta_set_cursor_type(lv_input, LV_CURSOR_LINE);
        lv_obj_set_style(lv_input, &style_pin_font_dark);
    }
    if(pwd_mode){
        lv_ta_set_pwd_mode(lv_input, !enable);
    }
    tip_mode = enable;
}

//also allow input to be empty i.e.tick button always enable
void en_kb_input_allow_empty() {
    enable_tick_btn(true);
    input_allow_empty = true;
}

//set input column into tip string
void en_kb_set_tip(const char * tip) {
    //save tip string for later use
    int len = strlen(tip);
    //x3 for special char support(ex:arabic..)
    tip_str = lv_mem_alloc((sizeof(char) * len) * 3 + 1);
    memset(tip_str, '\0', (sizeof(char) * len) * 3 + 1);
    strcpy(tip_str, tip);
    en_kb_set_tip_impl(true);
}

//hide kb's header left cancel btn
void en_kb_hide_cancel_btn(lv_obj_t * kb) {
    lv_obj_t * l_btn;
    l_btn = lv_get_child_by_index(kb, KB_LEFT_BTN);
    lv_obj_set_hidden(l_btn, 1);
}

void en_kb_set_lable(const void * txt) {
    //if set tip before, clear tip string, show label string instead
    if(tip_mode){
        en_kb_set_tip_impl(false);
    }
    lv_ta_set_text(lv_input, txt);
    if (profile_font_style == true) {
        lv_obj_set_style(lv_input, (is_ltr() ? &style_profile : &style_profile_rtl));
    }
    enable_tick_btn(true);
}

//entry point for setting SSID pwd mode
void en_kb_set_pwd_lable(const char* txt) {
    en_kb_set_lable(txt);
    lv_ta_set_pwd_mode(lv_input, true);
    pwd_mode = true;
}

void en_kb_cleanup(){
    if (tip_str != NULL) {
        lv_mem_free(tip_str);
        tip_str = NULL;
    }
}

void en_kb_cb_func(){
    if (done_callback != NULL) {
        (*done_callback)(click_id, lv_ta_get_text(lv_input));
    }
}

//to close window
void en_kb_close_win(lv_obj_t * kb, lv_event_t event_cb) {
    if (event_cb != LV_EVENT_CLICKED) return;

    en_kb_cleanup();
    max_length = MAX_INPUT_LENGTH;
    close_kb();

}

void en_kb_done_action(lv_obj_t * kb, lv_event_t event) {

    if (event != LV_EVENT_CLICKED) return;

    if(get_tick_btn_ina_state(get_kb_tick_btn(root_view))) return;

    en_kb_cb_func();
    //en_kb_close_win(kb, LV_EVENT_CLICKED);

}

//for reusing en_kb
void en_kb_reuse_action(lv_obj_t * kb, lv_event_t event) {

    if (event != LV_EVENT_CLICKED) return;

    if(get_tick_btn_ina_state(get_kb_tick_btn(root_view))) return;

    input_allow_empty = false;
    en_kb_cleanup();
    en_kb_cb_func();

}

//bottom row action
void br_btn_action(lv_obj_t * btnm, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    uint8_t i;
    const char * txt = lv_btnm_get_active_btn_text(btnm);
    uint16_t btn_idx = lv_btnm_get_active_btn(btnm);

    if(strcmp(txt, kb_map_br[0]) == 0) {
        mCurrentKB = NUM_KB;
        for (i = 0; i < NUM_CHAR_COUNT; i++) {
            lv_label_set_text(ur_label[i], num_kb_map[i]);
            lv_obj_set_event_cb(kb_btn[i], en_kb_action);
        }

    } else if (strcmp(txt, kb_map_br[1]) == 0 || strcmp(txt, kb_map_br_uc[1]) == 0) {
        if (is_uc_kb) {
            mCurrentKB = LC_KB;
            is_uc_kb = false;
            lv_btnm_set_map(kb_br, kb_map_br);
            for (i = 0; i < EN_CHAR_COUNT; i++) {
                lv_label_set_text(ur_label[i], lc_kb_map[i]);
                lv_obj_set_event_cb(kb_btn[i], lc_kb_action);
            }
        } else {
            mCurrentKB = UC_KB;
            is_uc_kb = true;
            lv_btnm_set_map(kb_br, kb_map_br_uc);
            for (i = 0; i < EN_CHAR_COUNT; i++) {
                lv_label_set_text(ur_label[i], uc_kb_map[i]);
                lv_obj_set_event_cb(kb_btn[i], uc_kb_action);
            }
        }
    } else if (strcmp(txt, kb_map_br[2]) == 0) {
        mCurrentKB = OTHER_KB;
        for (i = 0; i < MAX_KB; i++) {
            lv_label_set_text(ur_label[i], others_kb_map[i]);
            lv_obj_set_event_cb(kb_btn[i], others_kb_action);
        }
    }

    ur_kb_adjust(mCurrentKB);

    //we clear all btn's toggle state and set again after a/A or A/a been set
    lv_btnm_clear_btn_ctrl_all(btnm, LV_BTNM_CTRL_TGL_STATE);
    lv_btnm_set_btn_ctrl(btnm, btn_idx, LV_BTNM_CTRL_TGL_STATE);
}

//adjust upper row key board layout, hide button if not using in current kb
void ur_kb_adjust(int curr){
    uint8_t i;
    if(curr == NUM_KB) {
        //re-arrange not needed button (after 0~9) outside UI
        for(i = NUM_CHAR_COUNT; i < MAX_KB; i++) {
            lv_obj_set_hidden(kb_btn[i], 1);
        }
    } else if(curr == LC_KB || curr == UC_KB) {
        //re-arrange correct UI for a~z
        for(i = NUM_CHAR_COUNT; i < EN_CHAR_COUNT; i++) {
            lv_obj_set_hidden(kb_btn[i], 0);
        }
        //re-arrange not needed button after a~z
        for(i = EN_CHAR_COUNT; i < MAX_KB; i++) {
            lv_obj_set_hidden(kb_btn[i], 1);
        }
    } else if(curr == OTHER_KB) {
        //re-arrange all button for !#$ kb to correct UI
        for(i = NUM_CHAR_COUNT; i < MAX_KB; i++) {
            lv_obj_set_hidden(kb_btn[i], 0);
        }
    }
}

//check if tick btn is currently inactive
bool get_tick_btn_ina_state(lv_obj_t * tick_btn){
    //if input column allow empty string
    if(input_allow_empty)
        return false;

    if(lv_btn_get_state(tick_btn) == LV_BTN_STATE_INA){
        return true;
    }else{
        return false;
    }
}

lv_obj_t * get_kb_tick_btn(lv_obj_t * root){
    if(is_ltr()){
        return lv_get_child_by_index(root, KB_RIGHT_BTN);
    }else{
        return lv_get_child_by_index(root, KB_LEFT_BTN);
    }
}

//to enable or disable header right tick btn
void enable_tick_btn(bool enable) {
    lv_obj_t * tick_btn = get_kb_tick_btn(root_view);
    lv_obj_t * img = lv_obj_get_child(tick_btn, NULL);

    //always enable tick if allow empty
    if(input_allow_empty)
        enable = true;

    if(enable) {
        lv_img_set_src(img, &ic_headline_tick);
        lv_btn_set_state(tick_btn, LV_BTN_STATE_REL);
    } else {
        lv_img_set_src(img, &ic_headline_tick_dis);
        lv_btn_set_state(tick_btn, LV_BTN_STATE_INA);
    }
}

/* To initialize upper row keyboard that can slide
 * we create 31 button in the beginning and set label to each button
 * while user choose different kb (i.e:123, a/A, !#$)
 * for the unneeded button, we hide those in ur_kb_adjust
 * we set default keyboard to lower case English character
 */
void ur_kb_create(void) {
    ur_page = cust_lv_page_create(root_view, NULL);

    lv_page_set_sb_mode(ur_page, LV_SB_MODE_HIDE);
    lv_obj_align(ur_page, NULL, LV_ALIGN_IN_BOTTOM_LEFT, -8, -11);

    lv_page_set_scrl_fit(ur_page, LV_FIT_TIGHT/*true, true*/);
    lv_obj_set_size(ur_page, LV_HOR_RES_MAX + 20, 80 * LV_RES_OFFSET);

    kb_btn[0] = lv_btn_create(ur_page, NULL);
    lv_obj_set_size(kb_btn[0], KB_VER, KB_HOR);
    lv_obj_align(kb_btn[0], ur_page, LV_ALIGN_CENTER, 0, 0);

    ur_label[0] = lv_label_create(kb_btn[0], NULL);
    lv_label_set_text(ur_label[0], lc_kb_map[0]);
    lv_obj_set_user_data(kb_btn[0], 0);
    lv_obj_set_drag_parent(kb_btn[0], true);
    lv_obj_set_event_cb(kb_btn[0], lc_kb_action);

    lv_btn_set_style(kb_btn[0], LV_BTN_STYLE_REL, &style_btn_rel);
    lv_btn_set_style(kb_btn[0], LV_BTN_STYLE_PR, &style_btn_rel);

    uint8_t i;
    for (i = 1; i < EN_CHAR_COUNT; i++) {
        kb_btn[i] = lv_btn_create(ur_page, kb_btn[0]);
        lv_obj_align(kb_btn[i], kb_btn[0], LV_ALIGN_OUT_RIGHT_MID, (i -1) * KB_VER, 0);

        ur_label[i] = lv_label_create(kb_btn[i], NULL);
        lv_label_set_text(ur_label[i], lc_kb_map[i]);
        lv_obj_set_drag_parent(kb_btn[i], true);
        lv_obj_set_user_data(kb_btn[i], i);

        lv_obj_set_event_cb(kb_btn[i], lc_kb_action);
    }

    //to init kb no.26~31
    for (i = EN_CHAR_COUNT; i < MAX_KB; i++) {
    kb_btn[i] = lv_btn_create(ur_page, kb_btn[0]);
        lv_obj_align(kb_btn[i], kb_btn[0], LV_ALIGN_OUT_RIGHT_MID, (i -1) * KB_VER, 0);
        ur_label[i] = lv_label_create(kb_btn[i], NULL);
        lv_label_set_text(ur_label[i], others_kb_map[i]);
        lv_obj_set_drag_parent(kb_btn[i], true);
        lv_obj_set_user_data(kb_btn[i], i);

        lv_obj_set_event_cb(kb_btn[i], others_kb_action);
    }
}

void en_kb_set_header_style(lv_event_cb_t tick_action, lv_event_cb_t close_action,
        lv_obj_t * tick_btn, lv_obj_t * close_btn, lv_obj_t * tick_img, lv_obj_t * close_img){
    if(close_action!= NULL){
        lv_obj_set_event_cb(close_btn, close_action);
    }else{
        lv_obj_set_event_cb(close_btn, en_kb_close_win);
    }
    if(tick_action != NULL){
        lv_obj_set_event_cb(tick_btn, tick_action);
    }else{
        lv_obj_set_event_cb(tick_btn, en_kb_done_action);
    }
    lv_img_set_src(close_img, &ic_headline_close);
    lv_img_set_src(tick_img, &ic_headline_tick_dis);
    lv_btn_set_state(tick_btn, LV_BTN_STATE_INA);
}

// to create en_kb that let user reuse, en_kb_reuse_action make sure
// win won't be closed if click r btn
lv_obj_t * en_kb_reuse_create(const void * headline, int btn_click_id,
        void (*callback)(int id, const char* txt), lv_event_cb_t l_action) {
    return en_kb_create_impl(headline, btn_click_id, callback,
            en_kb_reuse_action, l_action);
}

lv_obj_t * en_kb_create(const void * headline, int btn_click_id,
        void (*callback)(int id, const char* txt)) {
    return en_kb_create_impl(headline, btn_click_id, callback,
            en_kb_done_action, en_kb_close_win);
}

lv_obj_t * en_kb_create_impl(const void * headline, int btn_click_id,
        void (*callback)(int id, const char* txt), lv_event_cb_t r_action, lv_event_cb_t l_action) {
    click_id = btn_click_id;
    done_callback = callback;

    tip_mode = false;
    pwd_mode = false;
    input_allow_empty = false;

    //default use lower case kb
    is_uc_kb = false;

    style_create();

    root_view = basic_kb_create(headline);
    lv_obj_t * r_btn = lv_obj_get_child(root_view, NULL);
    lv_obj_t * r_img = lv_img_create(r_btn, NULL);
    lv_obj_t * l_btn = lv_obj_get_child(root_view, r_btn);
    lv_obj_t * l_img = lv_img_create(l_btn, NULL);

    //set header icon/cb according to is_ltr result
    if(is_ltr()){
        en_kb_set_header_style(r_action, l_action, r_btn, l_btn, r_img, l_img);
    }else{
        en_kb_set_header_style(r_action, l_action, l_btn, r_btn, l_img, r_img);
    }

    //draw pin column
    lv_obj_t * input_column = lv_cont_create(root_view, NULL);
    lv_obj_set_size(input_column, INPUT_X * LV_RES_OFFSET, 60 * LV_RES_OFFSET);
    lv_obj_set_style(input_column, &style_column_bg);
    lv_input = lv_ta_create(input_column, NULL);
    lv_obj_set_size(lv_input, 200 * LV_RES_OFFSET, 30 * LV_RES_OFFSET);
    lv_obj_align(lv_input, input_column, LV_ALIGN_IN_LEFT_MID, 5, 0);
    lv_ta_set_text(lv_input, "");
    lv_ta_set_one_line(lv_input, true);
    lv_ta_set_sb_mode(lv_input, LV_SB_MODE_OFF);

    //draw delete button
    lv_obj_t * del_btn = lv_btn_create(root_view, NULL);
    lv_obj_set_event_cb(del_btn,
            (!profile_font_style ? del_btn_action : profile_del_btn_action));
    lv_obj_set_size(del_btn, 60 * LV_RES_OFFSET, 60 * LV_RES_OFFSET);
    //fix pin & delete column draw connected border twice issue
    if(is_ltr()){
        lv_btn_set_style(del_btn, LV_BTN_STYLE_REL, &style_d_btn_rel_ltr);
        lv_btn_set_style(del_btn, LV_BTN_STYLE_PR, &style_d_btn_pr_ltr);
    }else{
        lv_btn_set_style(del_btn, LV_BTN_STYLE_REL, &style_d_btn_rel_rtl);
        lv_btn_set_style(del_btn, LV_BTN_STYLE_PR, &style_d_btn_pr_rtl);
    }
    lv_obj_t * del_img = lv_img_create(del_btn, NULL);

    //re-align for rtl
    if(is_ltr()){
        //align input column & input ta
        lv_ta_set_text_align(lv_input, LV_LABEL_ALIGN_LEFT);
        lv_obj_align(input_column, NULL, LV_ALIGN_CENTER, -30, -10);
        //align delete btn
        lv_obj_align(del_btn, input_column, LV_ALIGN_IN_RIGHT_MID, 60, 0);
        lv_img_set_src(del_img, &btn_ime_delete_n);
    }else{
        //TODO ISSUE: sb not shown for rtl when input exceeds pin column width
        lv_ta_set_text_align(lv_input, LV_LABEL_ALIGN_RIGHT);
        lv_obj_align(input_column, NULL, LV_ALIGN_CENTER, 30, -10);
        lv_obj_align(del_btn, input_column, LV_ALIGN_IN_LEFT_MID, -60, 0);
        lv_img_set_src(del_img, &btn_ime_delete_n_rtl);
    }

    //draw keyboard upper layer (can slide)
    //default set to lower case English kb
    ur_kb_create();
    ur_kb_adjust(LC_KB);

    //draw keyboard bottom layer (fixed, can't slide)
    kb_br = lv_btnm_create(root_view, NULL);
    lv_btnm_set_map(kb_br, kb_map_br);
    lv_obj_set_event_cb(kb_br, br_btn_action);
    lv_obj_set_size(kb_br, LV_HOR_RES_MAX, KB_HOR);

    lv_btnm_set_style(kb_br, LV_BTNM_STYLE_BG, &style_btn_bg);
    lv_btnm_set_style(kb_br, LV_BTNM_STYLE_BTN_REL, &style_btn_rel);
    lv_btnm_set_style(kb_br, LV_BTNM_STYLE_BTN_PR, &style_btn_pr);

    //set default toggled to a/A btn
    lv_btnm_set_btn_ctrl(kb_br, 1, LV_BTNM_CTRL_TGL_STATE);

    lv_btnm_set_style(kb_br, LV_BTNM_STYLE_BTN_TGL_REL, &style_btn_pr);
    lv_btnm_set_style(kb_br, LV_BTNM_STYLE_BTN_TGL_PR, &style_btn_pr);

    lv_obj_align(kb_br, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

    //draw space button
    lv_obj_t * space_btn = lv_btn_create(root_view, NULL);
    lv_obj_set_event_cb(space_btn, space_btn_action);
    lv_obj_set_size(space_btn, 80, 36);
    lv_obj_align(space_btn, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 240, 0);

    lv_btn_set_style(space_btn, LV_BTN_STYLE_REL, &style_btn_rel);
    lv_btn_set_style(space_btn, LV_BTN_STYLE_PR, &style_btn_pr);

    lv_obj_t * space_img = lv_img_create(space_btn, NULL);
    lv_img_set_src(space_img, &btn_tab_space_n);

    return root_view;
}

bool get_tip_mode() {
    return tip_mode;
}

//config to decide whether using NotoSansCJKjp_Bold_24 font or not
//when in profile management
void en_kb_set_profile_font_style(bool enable) {
    profile_font_style = enable;
}

//delete button action for profile feature
void profile_del_btn_action(lv_obj_t * btn, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;

    const char* txt = lv_ta_get_text(lv_input);
    int len = strlen(txt);

    if(tip_mode)
        return;

    if ((len == 3 && is_txt_CJK(txt) == true)) {
        if (tip_str != NULL && strlen(tip_str) > 0) {
            //enter tip mode
            en_kb_set_tip_impl(true);
        } else {
            lv_ta_del_char(lv_input);
        }
    } else {
        if (len > 1) {
            lv_ta_del_char(lv_input);
        } else if (len == 1) {
            if (tip_str != NULL && strlen(tip_str) > 0) {
                //enter tip mode
                en_kb_set_tip_impl(true);
            } else {
                lv_ta_del_char(lv_input);
            }
        }
    }
}
