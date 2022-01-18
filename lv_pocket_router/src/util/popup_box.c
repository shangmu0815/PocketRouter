#include <stdio.h>

#include "../keyboard/num_kb_box.h"
#include "popup_box.h"
#include "lv_pocket_router/src/util/util.h"

static lv_style_t style_bg;
static lv_style_t style_btn_bg;
static lv_style_t style_btn_rel;
static lv_style_t style_btn_pr;
static lv_style_t style_ta;
static lv_style_t style_scrl_bg;
static lv_style_t style_line;

//for gray background
static lv_style_t style_gray_bg;

//for scrl popup
static lv_style_t title_font_style;
static lv_style_t text_font_style;
static lv_style_t style_sb;

//for loading anim popup
static lv_style_t style_bar;
static lv_style_t style_indic;
lv_obj_t * bar;

static lv_obj_t * mbox;
lv_obj_t * bg;
lv_obj_t * img;
lv_obj_t * ta;
lv_obj_t * btnm;
lv_obj_t * scrl_title;
lv_obj_t * scrl_txt;

//task to update animation in popup
lv_task_t * anim_task;
//task to update loading animation in popup
lv_task_t * popup_loading_task;

//for static popup
bool static_popup = false;

int anim_cnt;
void (*popup_close_cb)();

//for static popup callback
void (*static_popup_cb)();
lv_task_t * static_popup_task;

const void * anim_notice_map[]={&ic_pop_notice_1, &ic_pop_notice_2, &ic_pop_notice_3
        , &ic_pop_notice_4, &ic_pop_notice_5, &ic_pop_notice_6, &ic_pop_notice_7
        , &ic_pop_notice_8, &ic_pop_notice_9, &ic_pop_notice_10};

const void * anim_question_map[]={&ic_pop_question_1, &ic_pop_question_2, &ic_pop_question_3
        , &ic_pop_question_4, &ic_pop_question_5, &ic_pop_question_6, &ic_pop_question_7
        , &ic_pop_question_8, &ic_pop_question_9, &ic_pop_question_10};

const void * anim_interrupt_map[]={&ic_pop_interruptible_1, &ic_pop_interruptible_2, &ic_pop_interruptible_3
        , &ic_pop_interruptible_4, &ic_pop_interruptible_5, &ic_pop_interruptible_6, &ic_pop_interruptible_7
        , &ic_pop_interruptible_8, &ic_pop_interruptible_9, &ic_pop_interruptible_10};

const void * anim_loading_map[]={&icon_process_1, &icon_process_2, &icon_process_3
        , &icon_process_4, &icon_process_5, &icon_process_6, &icon_process_7, &icon_process_8};

void lv_style_popup_init(void){
    // close any existing popup that might be on top
    close_popup();

    //init callback func
    popup_close_cb = NULL;

    //mbox bg style
    lv_style_copy(&style_bg, &lv_style_plain);
    style_bg.body.main_color = LV_COLOR_WHITE;
    style_bg.body.grad_color = LV_COLOR_WHITE;
    style_bg.text.font = get_font(font_w_bold, font_h_18);
    style_bg.body.radius = 2;

    lv_style_copy(&style_ta, &lv_style_plain);
    style_ta.body.main_color = LV_COLOR_WHITE;
    style_ta.body.grad_color = LV_COLOR_WHITE;
    style_ta.text.font = get_font(font_w_bold, font_h_16);
    style_ta.text.color = LV_COLOR_GREYISH_BROWN;
    style_ta.text.letter_space = 1;

    //popup box btn matrix style
    lv_style_copy(&style_btn_bg, &lv_style_plain);
    style_btn_bg.body.main_color = LV_COLOR_WHITE;
    style_btn_bg.body.grad_color = LV_COLOR_WHITE;
    style_btn_bg.body.padding.left = 0;
    style_btn_bg.body.padding.right = 0;
    style_btn_bg.body.padding.top = 0;
    style_btn_bg.body.padding.bottom = 0;
    style_btn_bg.body.padding.inner = 0;
    style_btn_bg.body.radius = 2;

    //for scrl popup
    lv_style_copy(&style_scrl_bg, &lv_style_plain);
    style_scrl_bg.body.padding.left = 0;
    style_scrl_bg.body.padding.right = 0;
    style_scrl_bg.body.padding.top = 18;
    style_scrl_bg.body.padding.bottom = 18;
    style_scrl_bg.body.padding.inner = 2;//Scrollbar width

    lv_style_copy(&style_btn_rel, &lv_style_btn_rel);
    style_btn_rel.body.main_color = LV_COLOR_WHITE;
    style_btn_rel.body.grad_color = LV_COLOR_WHITE;
    style_btn_rel.body.border.color = LV_COLOR_GREYISH_BROWN;
    style_btn_rel.body.border.width = 1;
    style_btn_rel.body.border.opa = LV_OPA_COVER;
    style_btn_rel.body.border.part = LV_BORDER_TOP;
    style_btn_rel.body.radius = 0;
    style_btn_rel.text.color = LV_COLOR_GREYISH_BROWN;
    style_btn_rel.text.font = get_font(font_w_bold, font_h_20);
    style_btn_rel.text.letter_space = 1;

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.body.main_color = LV_COLOR_WHITE;
    style_btn_pr.body.grad_color = LV_COLOR_WHITE;
    style_btn_pr.body.border.color = LV_COLOR_BASE;
    style_btn_pr.body.border.width = 3;
    style_btn_pr.text.color = LV_COLOR_BASE;

    //set scrl title font style
    lv_style_copy(&title_font_style, &lv_style_plain);
    title_font_style.text.font = get_font(font_w_bold, font_h_18);
    title_font_style.text.color = LV_COLOR_GREYISH_BROWN;
    title_font_style.text.letter_space = 1;

    //set scrl text font style
    lv_style_copy(&text_font_style, &title_font_style);
    text_font_style.text.font = get_font(font_w_bold, font_h_16);

    //set scrl sb style
    lv_style_copy(&style_sb, &lv_style_plain);
    style_sb.body.main_color = LV_COLOR_BASE;
    style_sb.body.grad_color = LV_COLOR_BASE;
    style_sb.body.border.color = LV_COLOR_WHITE;
    style_sb.body.padding.inner = 2;
    style_sb.body.radius = LV_RADIUS_CIRCLE;
    style_sb.body.opa = LV_OPA_100;

    lv_style_copy(&style_gray_bg, &lv_style_plain);
    style_gray_bg.body.main_color = LV_COLOR_GREYISH_BROWN;//#greyish-brown
    style_gray_bg.body.grad_color = LV_COLOR_GREYISH_BROWN;
    style_gray_bg.body.opa = LV_OPA_60;

    //loading anim popup bar style
    lv_style_copy(&style_bar, &lv_style_pretty);
    style_bar.body.main_color = LV_COLOR_GREYISH_BROWN;
    style_bar.body.grad_color = LV_COLOR_GREYISH_BROWN;
    style_bar.body.radius = LV_RADIUS_CIRCLE;
    style_bar.body.border.color = LV_COLOR_GRAY;

    lv_style_copy(&style_indic, &lv_style_pretty);
    style_indic.body.grad_color =  LV_COLOR_BASE;
    style_indic.body.main_color=  LV_COLOR_BASE;
    style_indic.body.radius = LV_RADIUS_CIRCLE;
    style_indic.body.padding.left = 0;
    style_indic.body.padding.right = 0;
    style_indic.body.padding.top = 0;
    style_indic.body.padding.bottom = 0;

    //style for lines in between btns
    lv_style_copy(&style_line, &lv_style_plain);
    style_line.line.color = LV_COLOR_GREYISH_BROWN;
    style_line.line.width = 1;

}

void set_popup_cb(void (*callback)()){
    popup_close_cb = callback;
}

void close_popup_event_cb(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;
    close_popup();
}

void close_popup(){
    //skip closing static popup
    if(is_static_popup()){
        return;
    }
    if(bg != NULL){
        lv_obj_del(bg);
        bg = NULL;
    }
    if (anim_task != NULL) {
        lv_task_del(anim_task);
        anim_task = NULL;
    }
    if (popup_loading_task != NULL) {
        lv_task_del(popup_loading_task);
        popup_loading_task = NULL;
    }
    if (popup_close_cb != NULL) {
        (*popup_close_cb)();
        popup_close_cb = NULL;
    }
}

/**
 * Set the alignment of the text area.
 * @param ta pointer to a text are object
 * @param align the desired alignment from `lv_label_align_t`. (LV_LABEL_ALIGN_LEFT/CENTER/RIGHT)
 */
void popup_txt_align(lv_obj_t * ta, lv_label_align_t align)
{
    if(ta) {
        lv_ta_set_text_align(ta, align);
    }
}

//show interrupt animation
//need to call lv_task_del(anim_task); when popup close
void anim_interrupt() {
    int len = sizeof(anim_interrupt_map) / sizeof(void *);

    lv_obj_set_hidden(img, 0);
    if (anim_cnt < len) {
        lv_img_set_src(img, anim_interrupt_map[anim_cnt]);
        anim_cnt++;
    } else {
        //repeat anim
        anim_cnt = 0;
    }
}

//show notice animation
//need to call lv_task_del(anim_task); when popup close
void anim_notice() {
    int len = sizeof(anim_notice_map) / sizeof(void *);

    lv_obj_set_hidden(img, 0);
    if (anim_cnt < len) {
        lv_img_set_src(img, anim_notice_map[anim_cnt]);
        anim_cnt++;
    } else {
        //repeat anim
        anim_cnt = 0;
    }
}

//show question animation
//need to call lv_task_del(anim_task); when popup close
void anim_question() {
    int len = sizeof(anim_question_map) / sizeof(void *);

    lv_obj_set_hidden(img, 0);
    if (anim_cnt < len) {
        lv_img_set_src(img, anim_question_map[anim_cnt]);
        anim_cnt++;
    } else {
        //repeat anim
        anim_cnt = 0;
    }
}

//show loading animation
//need to call lv_task_del(anim_task); when popup close
void anim_loading() {

    int len = sizeof(anim_loading_map) / sizeof(void *);

    lv_obj_set_hidden(img, 0);
    if (anim_cnt < len) {
        lv_img_set_src(img, anim_loading_map[anim_cnt]);
        anim_cnt++;
    } else {
        //repeat anim
        anim_cnt = 0;
    }
}

//message box with title on top follow by a scrollable text area and btns in the bottom
lv_obj_t * popup_scrl_create(const void * title, const void * text, const char ** btns, lv_event_cb_t action) {
    return popup_scrl_create_impl(title, text, btns, action, NULL);
}

lv_obj_t * popup_scrl_create_impl(const void * title, const void * text, const char ** btns, lv_event_cb_t action, lv_event_cb_t action2) {

    lv_style_popup_init();

    //draw dark gray background
    bg = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(bg, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style(bg, &style_gray_bg);

    //create container for popup box
    mbox = lv_cont_create(bg, NULL);
    lv_obj_set_size(mbox, 280, 200);
    lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);

    //add title
    scrl_title = lv_label_create(mbox, NULL);
    lv_obj_set_size(scrl_title, 240, 20);
    lv_label_set_text(scrl_title, title);
    lv_label_set_align(scrl_title, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_style(scrl_title, &title_font_style);
    lv_obj_align(scrl_title, mbox, LV_ALIGN_IN_TOP_MID, 0, 18);

    //draw info main text area
    scrl_txt = lv_ta_create(mbox, NULL);
    lv_obj_set_size(scrl_txt, 240, 100);
    lv_ta_set_text(scrl_txt, text);
    lv_ta_set_style(scrl_txt, LV_TA_STYLE_SB, &style_sb);
    lv_ta_set_style(scrl_txt, LV_TA_STYLE_BG, &style_scrl_bg);
    lv_ta_set_sb_mode(scrl_txt, LV_SB_MODE_AUTO);
    lv_ta_set_cursor_type(scrl_txt, LV_CURSOR_HIDDEN);
    lv_ta_set_cursor_pos(scrl_txt, 0);
    lv_obj_set_style(scrl_txt, &text_font_style);
    lv_obj_align(scrl_txt, scrl_title, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    //re-align scroll text UI depend on is_ltr result
    if(is_ltr()){
        lv_ta_set_text_align(scrl_txt, LV_LABEL_ALIGN_LEFT);
    }else{
        lv_ta_set_text_align(scrl_txt, LV_LABEL_ALIGN_RIGHT);
    }

    //add btnm, put it in the bottom of the popup
    btnm = lv_btnm_create(mbox, NULL);
    lv_obj_set_size(btnm, 280, 48);

    //check if need reverse order for rtl
    set_reverse_btnm_map(btnm, btns);

    if (action != NULL) {
        lv_obj_set_event_cb(btnm, action);
    } else if (action2 != NULL) {
        lv_obj_set_event_cb(btnm, action2);
    } else {
        lv_obj_set_event_cb(btnm, close_popup_event_cb);
    }

    lv_obj_align(btnm, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -2);
    draw_popup_btnm_line(btnm);

    //set button matrix
    if (btns) {
        lv_btnm_set_style(btnm, LV_BTNM_STYLE_BG, &style_btn_bg);
        lv_btnm_set_style(btnm, LV_BTNM_STYLE_BTN_REL, &style_btn_rel);
        lv_btnm_set_style(btnm, LV_BTNM_STYLE_BTN_PR, &style_btn_pr);
    }
    //set mbox style
    lv_obj_set_style(mbox, &style_bg);

    return mbox;
}

//to let user update popup scroll text area
void popup_scrl_update(const void * text){
    lv_ta_set_text(scrl_txt, text);
}

//plain popup with ic_pop_notice anim, close the popup with "time" ms
lv_obj_t * popup_anim_not_plain_create(const void * text, int32_t time) {

    mbox = popup_img_create_impl(anim_notice_map[0], text, NULL, NULL, NULL);
    lv_obj_t * ta = lv_obj_get_child(mbox, NULL);
    lv_obj_t * image = lv_obj_get_child(mbox, ta);

    lv_img_set_src(image, anim_notice_map[0]);
    lv_obj_set_hidden(image, 0);

    //adjust UI because this popup did not include btnm
    lv_obj_align(image, mbox, LV_ALIGN_CENTER, 0, -20);
    lv_obj_align(ta, image, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    //start animation task
    anim_cnt = 0;
    anim_task = lv_task_create(anim_notice, 100, LV_TASK_PRIO_MID, NULL);

    lv_task_t * close_task;
    close_task = lv_task_create(close_popup, time, LV_TASK_PRIO_MID, NULL);
    lv_task_once(close_task);

    return mbox;
}

//for popup_anim_not_create long content
lv_obj_t * popup_anim_not_long_create(const void * text, const char ** btns, lv_event_cb_t action, lv_event_cb_t action2) {
    mbox = popup_anim_not_create(text, btns, action, action2);

    lv_obj_t * label  = lv_ta_get_label(ta);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SROLL_CIRC);

    return mbox;
}

//ic_pop_notice anim
lv_obj_t * popup_anim_not_create(const void * text, const char ** btns, lv_event_cb_t action, lv_event_cb_t action2) {

    mbox = popup_img_create_impl(anim_notice_map[0], text, btns, action, action2);

    lv_img_set_src(img, anim_notice_map[0]);
    lv_obj_set_hidden(img, 0);
    lv_obj_align(img, mbox, LV_ALIGN_IN_TOP_MID, 0, 25);

    //start animation task
    anim_cnt = 0;
    anim_task = lv_task_create(anim_notice, 100, LV_TASK_PRIO_MID, NULL);

    return mbox;
}

//for popup_anim_que_create long content
lv_obj_t * popup_anim_que_long_create(const void * text, const char ** btns, lv_event_cb_t action, lv_event_cb_t action2) {
    mbox = popup_anim_que_create(text, btns, action, action2);

    lv_obj_t * label  = lv_ta_get_label(ta);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SROLL_CIRC);

    return mbox;
}

//ic_pop_question anim
lv_obj_t * popup_anim_que_create(const void * text, const char ** btns, lv_event_cb_t action, lv_event_cb_t action2) {

    mbox = popup_img_create_impl(anim_question_map[0], text, btns, action, action2);

    lv_img_set_src(img, anim_question_map[0]);
    lv_obj_set_hidden(img, 0);
    lv_obj_align(img, mbox, LV_ALIGN_IN_TOP_MID, 0, 25);

    //start animation task
    anim_cnt = 0;
    anim_task = lv_task_create(anim_question, 100, LV_TASK_PRIO_MID, NULL);

    return mbox;
}

//check if need reverse btnm order depend on is_ltr result
void set_reverse_btnm_map(lv_obj_t * btnm, const char ** btns){
    if(is_ltr()){
        lv_btnm_set_map(btnm, btns);
        return;
    }

    int i;
    int cnt = 0;
    for(i = 0; strlen(btns[i]) > 0; i++) {
        cnt++;
    }
    if(cnt > 1){
        //reverse btns order
        const char* temp;
        int j = cnt - 1;
        for (i = 0; i < j; i++) {
            temp = btns[i];
            btns[i] = btns[j];
            btns[j] = temp;
            j--;
        }
    }
    lv_btnm_set_map(btnm, btns);
}

//customized message box with image on top, text in the middle and btn on the bottom
lv_obj_t * popup_img_create(const void * src_img, const void * text, const char ** btns, lv_event_cb_t action) {
    return popup_img_create_impl(src_img, text, btns, action, NULL);
}

lv_obj_t * popup_img_create_impl(const void * src_img, const void * text, const char ** btns, lv_event_cb_t action, lv_event_cb_t action2) {

    lv_style_popup_init();

    //draw dark gray background
    bg = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(bg, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style(bg, &style_gray_bg);

    //create a container for popup box
    lv_obj_t * mbox = lv_cont_create(bg, NULL);
    lv_obj_set_size(mbox, 280, 200);
    lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);

    //add image res on the top of the popup
    img = lv_img_create(mbox, NULL);
    lv_obj_set_size(img, 65, 65);
    lv_img_set_src(img, src_img);
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_MID, 0, 22);

    //draw info main text area
    ta = lv_ta_create(mbox, NULL);
    lv_ta_set_text(ta, text);
    lv_ta_set_text_align(ta, LV_LABEL_ALIGN_CENTER);
    lv_ta_set_style(ta, LV_TA_STYLE_BG, &style_ta);
    lv_ta_set_cursor_type(ta, LV_CURSOR_HIDDEN);
    lv_ta_set_cursor_pos(ta, 0);
    lv_ta_set_sb_mode(ta, LV_SB_MODE_OFF);
    lv_obj_set_size(ta, 240, 50);
    lv_obj_align(ta, NULL, LV_ALIGN_CENTER, 0, 20);

    if (btns != NULL){
        //draw btnm, put it in the bottom of the popup
        btnm = lv_btnm_create(mbox, NULL);
        lv_obj_set_size(btnm, 280, 48);

        //check if need reverse order for rtl
        set_reverse_btnm_map(btnm, btns);

        if (action != NULL) {
            lv_obj_set_event_cb(btnm, action);
        } else if (action2 != NULL) {
            lv_obj_set_event_cb(btnm, action2);
        } else {
            lv_obj_set_event_cb(btnm, close_popup_event_cb);
        }

        lv_obj_align(btnm, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -2);
        draw_popup_btnm_line(btnm);

        //set button matrix
        if (btns) {
            lv_btnm_set_style(btnm, LV_BTNM_STYLE_BG, &style_btn_bg);
            lv_btnm_set_style(btnm, LV_BTNM_STYLE_BTN_REL, &style_btn_rel);
            lv_btnm_set_style(btnm, LV_BTNM_STYLE_BTN_PR, &style_btn_pr);
        }
    } else {
        //increase text area cause the popup did not include btnm
        lv_obj_set_size(ta, 240, 120);
    }
    //set mbox style
    lv_obj_set_style(mbox, &style_bg);

    return mbox;
}

void draw_popup_btnm_line (lv_obj_t * btnm){

    //if more than one btn, draw line between them
    lv_btnm_ext_t * ext_btnm = lv_obj_get_ext_attr(btnm);
    uint16_t btn_cnt = ext_btnm->btn_cnt;
    if (btn_cnt == 2) {
        static lv_point_t p[] = {{140, 10}, {140, 40}};
        lv_obj_t * line = lv_line_create(btnm, NULL);
        lv_line_set_points(line, p, 2);
        lv_line_set_style(line, LV_LINE_STYLE_MAIN, &style_line);
    }

    if (btn_cnt == 3) {
        static lv_point_t p1[] = {{93, 10}, {93, 40}};
        static lv_point_t p2[] = {{186, 10}, {186, 40}};

        lv_obj_t * line1 = lv_line_create(btnm, NULL);
        lv_line_set_points(line1, p1, 2);
        lv_line_set_style(line1, LV_LINE_STYLE_MAIN, &style_line);

        lv_obj_t * line2 = lv_line_create(btnm, line1);
        lv_line_set_points(line2, p2, 2);
    }
}

//create interrupt popup with anim on top, text in the middle and btnm on the bottom
lv_obj_t * popup_anim_int_create(const void * text, const char ** btns, lv_event_cb_t action) {
    return popup_anim_int_create_impl(text, btns, action, NULL);
}

//create interrupt popup with anim on top, text in the middle and btnm on the bottom
lv_obj_t * popup_anim_int_create_impl(const void * text, const char ** btns, lv_event_cb_t action, lv_event_cb_t action2) {

    mbox = popup_img_create_impl(anim_interrupt_map[0], text, btns, action, action2);
    lv_obj_set_hidden(img, 0);
    lv_obj_align(img, mbox, LV_ALIGN_IN_TOP_MID, 0, 25);
    //start animation task
    anim_cnt = 0;
    anim_task = lv_task_create(anim_interrupt, 100, LV_TASK_PRIO_MID, NULL);

    return mbox;
}

// hide loading bar
void hide_loading_bar(lv_obj_t * box) {
    lv_obj_t * txt = lv_obj_get_child(box, NULL);
    lv_obj_t * bbar = lv_obj_get_child(box, txt);
    lv_obj_set_hidden(bbar, true);
}

//update loading anim popup bar percentage
void update_loading_bar(lv_obj_t * box, int16_t value) {

    lv_obj_t * txt = lv_obj_get_child(box, NULL);
    lv_obj_t * bbar = lv_obj_get_child(box, txt);
    lv_bar_set_value(bbar, value, false);

}

// set loading message font size
void update_anim_loading_msg_font(lv_obj_t * box, uint8_t font_weight, uint8_t font_height) {
    static lv_style_t new_font_style;
    lv_style_copy(&new_font_style, &title_font_style);
    new_font_style.text.font = get_font(font_weight, font_height);

    lv_obj_t * msg = lv_obj_get_child(box, NULL);
    lv_obj_set_size(msg, 240, 30);
    lv_obj_set_style(msg, &new_font_style);
}

//for delay create static popup if one already exist
void delay_create_static_popup(){
    if((bg == NULL) || !is_static_popup()){
        log_d("create delayed static popup now");

        //can create static popup now
        if (static_popup_cb != NULL) {
            (*static_popup_cb)();
            static_popup_cb = NULL;
        }
        if (static_popup_task != NULL) {
            lv_task_del(static_popup_task);
            static_popup_task = NULL;
        }
    }else{
        log_d("previous static popup still exist, waiting...");
    }
}

void create_static_popup(void (*callback)()){
    if(bg != NULL && is_static_popup() && callback != NULL){
        log_d("static popup already exist, delayed showing popup");
        static_popup_task = lv_task_create(delay_create_static_popup, 1000, LV_TASK_PRIO_MID, NULL);
        static_popup_cb = callback;
    }else{
        log_d("static popup not exist, create now");
        if (callback != NULL) {
            (*callback)();
        }
    }
}

//enable/disable a static popup that will not be closed during suspend/resume
void set_static_popup(bool is_static){
    if(bg != NULL){
        static_popup = is_static;
    }
}

bool is_static_popup(){
    if(bg != NULL){
        return static_popup;
    }
    return false;
}

void move_static_popup_to_foreground(){
    if(bg != NULL && is_static_popup()){
        lv_obj_move_foreground(bg);
    }
}

void close_static_popup(){
    set_static_popup(false);
    close_popup();
}

//create loading anim popup
lv_obj_t * popup_anim_loading_create(const void * text, const void * message) {

    lv_style_popup_init();

    //draw dark gray background
    bg = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(bg, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style(bg, &style_gray_bg);

    //create a container for popup box
    lv_obj_t * mbox = lv_cont_create(bg, NULL);
    lv_obj_set_size(mbox, 280, 200);
    lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);

    //draw title
    lv_obj_t * title = lv_label_create(mbox, NULL);
    lv_obj_set_size(title, 240, 30);
    lv_obj_set_style(title, &text_font_style);
    lv_label_set_text(title, text);
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);

    //add image anim below title
    img = lv_img_create(mbox, NULL);
    lv_obj_set_size(img, 72, 72);
    lv_img_set_src(img, anim_loading_map[0]);
    lv_obj_set_pos(img, 10, 80);
    lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_hidden(img, 1);

    //add loading bar
    bar = lv_bar_create(mbox, NULL);
    lv_bar_set_range(bar, 0, 100);
    lv_obj_set_pos(bar, 10, 10);
    lv_obj_set_size(bar, 250, 5);
    lv_bar_set_value(bar, 0, false);
    lv_bar_set_style(bar, LV_BAR_STYLE_BG, &style_bar);
    lv_bar_set_style(bar, LV_BAR_STYLE_INDIC, &style_indic);
    lv_obj_align(bar, img, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    //set loading message below anim
    lv_obj_t * msg = lv_label_create(mbox, title);
    lv_label_set_long_mode(msg, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_width(msg, 250);

    lv_label_set_text(msg, message);
    lv_obj_align(msg, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -20);

    //set mbox style
    lv_obj_set_style(mbox, &style_bg);

    //start animation task
    anim_cnt = 0;
    anim_task = lv_task_create(anim_loading, 125, LV_TASK_PRIO_MID, NULL);

    return mbox;
}

lv_task_t * popup_loading_task_create(lv_task_cb_t task_cb, uint32_t period, lv_task_prio_t prio, void * user_data) {
    popup_loading_task = lv_task_create(task_cb, period, prio, user_data);
    return popup_loading_task;
}
