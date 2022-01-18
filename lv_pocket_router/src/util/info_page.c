#include <assert.h>
#include <stdio.h>

#include "info_page.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/ril/ril.h"

static lv_style_t style_ta_bold;
static lv_style_t style_ta;
static lv_style_t style_ta_sms;
static lv_style_t style_ta_sms_rtl;
static lv_style_t style_ta_profile;
static lv_style_t style_ta_profile_rtl;
static lv_style_t style_bg;
static lv_style_t style_bg_header;
//TODO for RTL test only
static lv_style_t style_btn_rel;
static lv_style_t style_btn_pr;
static lv_style_t style_btnm_rel;
static lv_style_t style_btnm_pr;
static lv_style_t style_sb;
static lv_style_t style_content_bg;

lv_obj_t * header_r_btn;
lv_obj_t * header_l_btn;
lv_obj_t * pin_puk_retry_left_win;
lv_obj_t * info_page_pin_mng_win;
//for home key action
#define MAX_PAGE_NUM            5
#define CACHE_INDEX_INVALID     255
#define INFO_PAGE_HOR           280

typedef struct {
    int cnt;
    lv_obj_t * btn[MAX_PAGE_NUM];
    lv_event_cb_t action[MAX_PAGE_NUM];
    // to keep Back btn info for Delete(garbage) btn (ie, for trigger Back action after additional action)
    lv_obj_t * added_list_back_btn;
    lv_event_cb_t added_action;
} InfoPageBack;

InfoPageBack ipBackBtnCache = { .cnt = 0 };

static const uint32_t INDEX_HOME_BTN = 0;
static const uint32_t INDEX_BACK_BTN = 1;
static const uint32_t INDEX_TICK_BTN = 2;
static const uint32_t INDEX_EDIT_BTN = 3;

void info_page_add_back_cache(lv_obj_t * btn, lv_event_cb_t action) {
    assert(ipBackBtnCache.cnt < MAX_PAGE_NUM);
    ipBackBtnCache.btn[ipBackBtnCache.cnt] = btn;
    ipBackBtnCache.action[ipBackBtnCache.cnt] = action;
    ipBackBtnCache.cnt++;
}

void info_page_rm_back_cache(lv_obj_t * btn) {
    bool found = false;
    int i;
    for (i = 0; i < ipBackBtnCache.cnt; i++) {
        if (found) {
            ipBackBtnCache.action[i - 1] = ipBackBtnCache.action[i];
            ipBackBtnCache.btn[i - 1] = ipBackBtnCache.btn[i];
            continue;
        }

        if (ipBackBtnCache.btn[i] == btn) {
            found = true;
            ipBackBtnCache.action[i] = NULL;
            ipBackBtnCache.btn[i] = NULL;
        }
    }
    if (found) {
        ipBackBtnCache.action[ipBackBtnCache.cnt - 1] = NULL;
        ipBackBtnCache.btn[ipBackBtnCache.cnt - 1] = NULL;
        ipBackBtnCache.cnt--;
    }
}

int info_page_lookup_back_cache(lv_obj_t * btn) {
    int i;
    for (i = 0; i < ipBackBtnCache.cnt; i++) {
        if (ipBackBtnCache.btn[i] == btn) {
            return i;
        }
    }
    return CACHE_INDEX_INVALID;
}

void style_create(void) {

    lv_style_copy(&style_ta_bold, &lv_style_plain);
    style_ta_bold.text.font = get_font(font_w_bold, font_h_20);
    style_ta_bold.text.letter_space = 1;

    lv_style_copy(&style_ta, &lv_style_plain);
    style_ta.text.color = LV_COLOR_GREYISH_BROWN;
#if defined(HIGH_RESOLUTION)
    style_ta.text.font = get_font(font_w_bold, font_h_40);
#else
    lv_style_copy(&style_ta, &lv_style_plain);
#ifdef CUST_DLINK
    style_ta.text.font = get_font(font_w_bold, font_h_20);
#else
    style_ta.text.font = get_font(font_w_bold, font_h_16);
#endif
    style_ta.text.letter_space = 1;
#endif

    lv_style_copy(&style_ta_sms, &lv_style_plain);
#ifdef JP_AR_FONT
    style_ta_sms.text.font = get_font(font_w_bold, font_h_22);
#else
    style_ta_sms.text.font = get_locale_font_cust(font_w_bold, font_h_22);
#endif
    style_ta_sms.text.letter_space = 1;

    lv_style_copy(&style_ta_sms_rtl, &style_ta_sms);
#ifdef JP_AR_FONT
    style_ta_sms_rtl.text.font = get_font(font_w_bold, font_h_22);
#else
    style_ta_sms_rtl.text.font = get_locale_font(AR, font_w_bold, font_h_22);
#endif

    lv_style_copy(&style_ta_profile, &lv_style_plain);
    style_ta_profile.text.font = get_locale_font_cust(font_w_bold, font_h_22);
    style_ta_profile.text.letter_space = 1;

    lv_style_copy(&style_ta_profile_rtl, &style_ta_profile);
    style_ta_profile_rtl.text.font = get_locale_font(AR, font_w_bold, font_h_22);

    //background style
    lv_style_copy(&style_bg, &lv_style_plain);
    style_bg.body.main_color = LV_COLOR_WHITE;
    style_bg.body.grad_color = LV_COLOR_WHITE;
    style_bg.body.padding.left = 0;
    style_bg.body.padding.right = 0;
    style_bg.body.padding.top = 0;
    style_bg.body.padding.bottom = 0;
    style_bg.body.padding.inner = 0;

    //main content style
    lv_style_copy(&style_content_bg, &lv_style_plain);
    style_content_bg.body.main_color = LV_COLOR_WHITE;
    style_content_bg.body.grad_color = LV_COLOR_WHITE;
    style_content_bg.body.padding.left = 16;
    style_content_bg.body.padding.right = 0;
    style_content_bg.body.padding.top = 0;
    style_content_bg.body.padding.bottom = 0;

    //header border line style
    lv_style_copy(&style_bg_header, &lv_style_plain);
    style_bg_header.body.main_color = LV_COLOR_WHITE;
    style_bg_header.body.grad_color = LV_COLOR_WHITE;
    style_bg_header.body.border.part = LV_BORDER_BOTTOM;
    style_bg_header.body.border.color = LV_COLOR_SILVER;
    style_bg_header.body.border.width = 2;
    style_bg_header.text.color = LV_COLOR_MATTERHORN;
#if defined(HIGH_RESOLUTION)
    style_bg_header.text.font = get_font(font_w_bold, font_h_40);
#else
    style_bg_header.text.font = get_font(font_w_bold, font_h_24);
    style_bg_header.text.letter_space = 1;
#endif
    //header icon style
    lv_style_copy(&style_btn_rel, &lv_style_plain);
    style_btn_rel.body.main_color = LV_COLOR_WHITE;
    style_btn_rel.body.grad_color = LV_COLOR_WHITE;
    style_btn_rel.body.radius = 0;
    style_btn_rel.text.color = LV_COLOR_BLACK;
    style_btn_rel.text.letter_space = 1;

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.image.color = LV_COLOR_BASE;
    style_btn_pr.image.intense = LV_OPA_COVER;

    //button matrix style
    lv_style_copy(&style_btnm_rel, &style_btn_rel);
    style_btnm_rel.body.border.color = LV_COLOR_SILVER;
    style_btnm_rel.body.border.width = 2;
    style_btnm_rel.text.font = get_font(font_w_bold, font_h_22);
    style_btnm_rel.text.color = LV_COLOR_GREYISH_BROWN;
    style_btnm_rel.text.letter_space = 1;
    style_btnm_rel.body.border.part = LV_BORDER_TOP;

    lv_style_copy(&style_btnm_pr, &style_btnm_rel);
    style_btnm_pr.body.main_color = LV_COLOR_WHITE;
    style_btnm_pr.body.grad_color = LV_COLOR_WHITE;
    style_btnm_pr.text.color = LV_COLOR_BASE;
    style_btnm_pr.body.border.width = 3;
    style_btnm_pr.body.border.color = LV_COLOR_BASE;

    //scroll bar style
    lv_style_copy(&style_sb, &lv_style_plain);
    style_sb.body.main_color = LV_COLOR_BASE;
    style_sb.body.grad_color = LV_COLOR_BASE;
    style_sb.body.radius = LV_RADIUS_CIRCLE;
    style_sb.body.opa = LV_OPA_100;
    style_sb.body.padding.inner = 2;//Scrollbar width
}

//for home key action, close all page windows
lv_res_t close_all_pages(void)
{
    int i;
    for (i = ipBackBtnCache.cnt-1; i >= 0; i--) {
        if (ipBackBtnCache.btn[i]) {
            lv_event_cb_t back_action;
            back_action = ipBackBtnCache.action[i];
            back_action(ipBackBtnCache.btn[i], LV_EVENT_CLICKED);
        }
    }
    ipBackBtnCache.cnt = 0;
    log_d("close_all_pages 0");

    return LV_RES_INV;
}

//define header action
static void header_action(lv_obj_t * btn, lv_event_t event_cb)
{
    if (event_cb != LV_EVENT_CLICKED) return;

    uint8_t id = lv_obj_get_user_data(btn);
    if (id == INDEX_HOME_BTN) {
        //lv_win_close_event_cb(btn, LV_EVENT_RELEASED);
        //close_all_pages();
        //close_all_lists(0);
        launch_home_behaviour();
    } else if (id == INDEX_BACK_BTN){
        lv_win_close_event_cb(btn, LV_EVENT_RELEASED);

        //remove ipBackBtnCache.cnt since info page already been deleted
        info_page_rm_back_cache(btn);
        log_d("info_Page back %d", ipBackBtnCache.cnt);
    }
}

void info_page_additional_action(lv_obj_t * btn, lv_event_t event_cb)
{
    if (event_cb != LV_EVENT_CLICKED) return;

    lv_event_cb_t action;
    action = ipBackBtnCache.added_action;
    action(btn, LV_EVENT_CLICKED);
    header_action(ipBackBtnCache.added_list_back_btn, LV_EVENT_CLICKED);
}

void custom_action(lv_obj_t * btn, lv_event_t event_cb)
{
    if (event_cb != LV_EVENT_CLICKED) return;

    int index = info_page_lookup_back_cache(btn);
    // invalid means non Back btn case, should be Home btn
    if (index != CACHE_INDEX_INVALID) {
        lv_event_cb_t back_action;
        assert(index < MAX_PAGE_NUM);
        back_action = ipBackBtnCache.action[index];
        back_action(btn, LV_EVENT_CLICKED);
        info_page_rm_back_cache(btn);
    }
    header_action(btn, LV_EVENT_CLICKED);
}

lv_obj_t * info_page_basic (lv_obj_t * win, const void * headline, lv_event_cb_t r_action,
        lv_event_cb_t l_action, lv_event_cb_t back_cache_action, const void * r_img, const void * l_img){
    style_create();
    lv_win_set_title(win, headline);
    lv_win_set_style(win, LV_WIN_STYLE_BG, &style_bg);
    lv_win_set_style(win, LV_WIN_STYLE_SB, &style_sb);
    lv_win_set_style(win, LV_WIN_STYLE_CONTENT, &style_content_bg);
    lv_win_set_style(win, LV_WIN_STYLE_BTN_REL, &style_btn_rel);
    lv_win_set_style(win, LV_WIN_STYLE_BTN_PR, &style_btn_pr);
    lv_win_set_sb_mode(win, LV_SB_MODE_AUTO);
    lv_win_set_style(win, LV_WIN_STYLE_HEADER, &style_bg_header);

    if(is_ltr()){
        log_d("info_page LTR");
        //draw right icon, default set to home
        if(r_img == NULL){
            header_r_btn = lv_win_add_btn(win, &ic_headline_home);
            lv_obj_set_user_data(header_r_btn, INDEX_HOME_BTN);
        }else{
            header_r_btn = lv_win_add_btn(win, r_img);
        }
        //draw left icon, default set to back
        if(l_img == NULL){
            header_l_btn = lv_win_add_btn(win, &ic_headline_back);
            lv_obj_set_user_data(header_l_btn, INDEX_BACK_BTN);
        }else{
            header_l_btn = lv_win_add_btn(win, l_img);
        }
        lv_obj_set_event_cb(header_r_btn, r_action);
        lv_obj_set_event_cb(header_l_btn, l_action);

        //add back cache for back btn
        info_page_add_back_cache(header_l_btn, back_cache_action);
    }else {
        log_d("info_page RTL");
        //draw right icon, default set to back
        if(l_img == NULL){
            header_r_btn = lv_win_add_btn(win, &ic_headline_back_rtl);
            lv_obj_set_user_data(header_r_btn, INDEX_BACK_BTN);
        }else{
            header_r_btn = lv_win_add_btn(win, l_img);
        }
        //draw left icon, default set to home
        if(r_img == NULL){
            header_l_btn = lv_win_add_btn(win, &ic_headline_home);
            lv_obj_set_user_data(header_l_btn, INDEX_HOME_BTN);
        }else{
            header_l_btn = lv_win_add_btn(win, r_img);
        }
        lv_obj_set_event_cb(header_r_btn, l_action);
        lv_obj_set_event_cb(header_l_btn, r_action);

        //add back cache for back btn
        info_page_add_back_cache(header_r_btn, back_cache_action);
    }
    //set title lable to scroll circle mode if too long
    win_title_adjust(win, headline);

    return win;
}

//add a scrollable text area in info page
lv_obj_t * info_page_scrl_txt(lv_obj_t * win, const void * text, const char ** btnm){

    lv_obj_t * page = lv_win_get_content(win);
    lv_obj_t * label = lv_label_create(page, NULL);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(label, INFO_PAGE_HOR);
    info_page_set_label(label, text);

    //set button matrix style
    if (btnm != NULL) {
        lv_win_set_style(win, LV_WIN_STYLE_BTNM_BG, &style_bg);
        lv_win_set_style(win, LV_WIN_STYLE_BTNM_REL, &style_btnm_rel);
        lv_win_set_style(win, LV_WIN_STYLE_BTNM_PR, &style_btnm_pr);
        lv_obj_set_size(page, 320, 120);
    }else{
        lv_obj_set_size(page, 320, 164);
    }
    lv_obj_align(page, win, LV_ALIGN_IN_TOP_LEFT, 0, 60);

    if(is_ltr()){
        lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
    }else{
        lv_label_set_align(label, LV_LABEL_ALIGN_RIGHT);
    }
    return label;
}

void info_page_set_label(lv_obj_t * label, const void * text){
    lv_label_set_text(label, text);
    lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style_ta);
    //align right if rtl
    if(!is_ltr()){
        lv_label_set_align(label, LV_LABEL_ALIGN_RIGHT);
    }
}

//to create a basic info page without no button
lv_obj_t * info_page_create(lv_obj_t * par, const void * headline, const void * text){
    return info_page_create_btmn(par, headline, text, NULL, NULL);
}

lv_obj_t * info_page_create_btmn(lv_obj_t * par, const void * headline,
        const void * text, const char ** btnm, lv_event_cb_t action) {

    lv_obj_t * win;
    if (btnm == NULL) {
        //plain info page without btn matrix
        win = lv_win_create(par, NULL);
    } else if (action != NULL) {
        win = lv_win_btn_create(par, btnm, action);
    }
    win = info_page_basic(win, headline, header_action, header_action, header_action, NULL, NULL);
    info_page_scrl_txt(win, text, btnm);

    log_d("info_page_create_btmn_impl %d", ipBackBtnCache.cnt);
    return win;
}

//for data usage page in Setting
lv_obj_t * info_page_create_btmn_label_refresh(lv_obj_t * par, const void * headline,
        const void * text1, const void * text2, const char ** btnm, lv_event_cb_t custom_cb, lv_event_cb_t action) {

    lv_obj_t * win;
    if (action != NULL) {
        win = lv_win_btn_create(par, btnm, action);
    }
    win = info_page_basic(win, headline, custom_action, custom_action, custom_cb, NULL, NULL);

    lv_obj_t * label1 = info_page_scrl_txt(win, text1, btnm);
    lv_obj_set_size(label1, 280, 30);
    lv_obj_align(label1, win, LV_ALIGN_IN_TOP_MID, 0, 60);

    lv_obj_t * label2 = lv_label_create(win, label1);
    lv_obj_set_size(label2, 280, 100);
    info_page_set_label(label2, text2);
    lv_obj_align(label2, label1, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    lv_label_set_align(label1, LV_LABEL_ALIGN_CENTER);
    lv_label_set_style(label1, LV_LABEL_STYLE_MAIN, &style_ta_bold);

    log_d("info_page_create_btmn_label_refresh %d", ipBackBtnCache.cnt);
    return label1;
}

lv_obj_t * info_page_sms_create(lv_obj_t * par, const void * headline, const void * text, lv_event_cb_t action, int encoding){
    lv_obj_t * win = lv_win_create(par, NULL);

    win = info_page_basic(win, headline, info_page_additional_action,
            header_action, header_action, &ic_headline_delete, NULL);

    lv_obj_t * label = info_page_scrl_txt(win, text, NULL);
    //support multi language display
    if(is_ltr() && !is_txt_rtl(text)){
        lv_obj_set_style(label, &style_ta_sms);
    }else {
        lv_obj_set_style(label, &style_ta_sms_rtl);
        lv_label_set_align(label, LV_LABEL_ALIGN_RIGHT);
    }

    // for handling back btn after f garbage callback
    ipBackBtnCache.added_action = action;
    if(is_ltr()){
        ipBackBtnCache.added_list_back_btn = header_l_btn;
    }else{
        ipBackBtnCache.added_list_back_btn = header_r_btn;
    }

    log_d("info_page_sms_create %d", ipBackBtnCache.cnt);
    return win;
}

lv_obj_t * info_page_create_label_align_center (lv_obj_t * par, const void * headline, const void * text){

    lv_obj_t * win = lv_win_create(par, NULL);
    win = info_page_basic(win, headline, header_action, header_action, header_action, NULL, NULL);

    //remove scroll bar
    lv_win_set_sb_mode(win, LV_SB_MODE_OFF);

    lv_obj_t * page = lv_win_get_content(win);
    lv_obj_t * label = lv_label_create(page, NULL);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(label, 200);

    lv_label_set_text(label, text);
    lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style_ta);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(label, win, LV_ALIGN_CENTER, 0, 0);

    log_d("info_page_create_label_align_center %d", ipBackBtnCache.cnt);
    return win;
}

lv_obj_t * info_page_create_confirm_profile_info(lv_obj_t * par, const void * headline, const void * text, lv_event_cb_t action)
{
    lv_obj_t * win = lv_win_create(par, NULL);
    win = info_page_basic(win, headline, action, header_action, action, &ic_headline_tick, NULL);

    //draw info main text area
    lv_obj_t * ta = lv_ta_create(win, NULL);
    lv_obj_set_size(ta, INFO_PAGE_HOR, 164);
    lv_ta_set_text(ta, text);
    lv_ta_set_style(ta, LV_TA_STYLE_SB, &style_sb);
    lv_ta_set_style(ta, LV_TA_STYLE_BG,
            (is_ltr() ? &style_ta_profile : &style_ta_profile_rtl));
    //set info page scroll bar to auto mode(i.e always show sb)
    lv_ta_set_sb_mode(ta, LV_SB_MODE_AUTO);
    lv_ta_set_cursor_type(ta, LV_CURSOR_HIDDEN);
    lv_ta_set_cursor_pos(ta, 0);

    if(is_ltr())
        lv_obj_set_user_data(header_r_btn, INDEX_TICK_BTN);
    else
        lv_obj_set_user_data(header_l_btn, INDEX_TICK_BTN);

    log_d("info_page_create_confirm_profile_info %d", ipBackBtnCache.cnt);
    return win;
}

lv_obj_t * info_page_create_btmn_impl_profile_with_edit(lv_obj_t * par, const void * headline,
        const void * text, const char ** btnm, lv_event_cb_t custom_cb, lv_event_cb_t action,
        lv_event_cb_t edit_action) {

    lv_obj_t * win;

    win = lv_win_btn_create(par, btnm, action);
    win = info_page_basic(win, headline, custom_action, custom_action, custom_cb, NULL, NULL);

    //remove scroll bar
    lv_win_set_sb_mode(win, LV_SB_MODE_OFF);

    //draw header_r2_btn icon on header
    lv_obj_t * header = lv_obj_get_parent(header_r_btn);
    lv_obj_t * edit_btn = lv_btn_create(header, NULL);
    lv_obj_set_size(edit_btn, 32, 32);
    lv_btn_set_style(edit_btn, LV_BTN_STYLE_REL, &style_btn_rel);
    lv_btn_set_style(edit_btn, LV_BTN_STYLE_PR, &style_btn_pr);
    lv_obj_set_event_cb(edit_btn, edit_action);
    lv_obj_set_user_data(edit_btn, INDEX_EDIT_BTN);

    lv_obj_t * edit_img = lv_img_create(edit_btn, NULL);
    lv_obj_set_size(edit_img, 32, 32);
    lv_img_set_src(edit_img, &ic_headline_edit);

    //adjust header title position
    lv_win_ext_t * ext = lv_obj_get_ext_attr(win);

    lv_obj_t * page = lv_win_get_content(win);
    lv_obj_t * label = lv_label_create(page, NULL);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(label, INFO_PAGE_HOR);

    lv_label_set_text(label, text);
    lv_label_set_style(label, LV_LABEL_STYLE_MAIN,
            (is_ltr() ? &style_ta_profile : &style_ta_profile_rtl));
    //align right if rtl
    if(is_ltr()){
        lv_obj_align(edit_btn, header_r_btn, LV_ALIGN_OUT_LEFT_MID, 0, 0);
        lv_obj_align(ext->title, NULL, LV_ALIGN_CENTER, -10, 0);
        lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
        lv_obj_align(label, header, LV_ALIGN_OUT_BOTTOM_LEFT, 16, 12);
    }else{
        lv_obj_align(edit_btn, header_l_btn, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
        lv_obj_align(ext->title, NULL, LV_ALIGN_CENTER, 10, 0);
        lv_label_set_align(label, LV_LABEL_ALIGN_RIGHT);
        lv_obj_align(label, header, LV_ALIGN_OUT_BOTTOM_RIGHT, -16, 12);
    }

    //set button matrix style
    lv_win_set_style(win, LV_WIN_STYLE_BTNM_BG, &style_bg);
    lv_win_set_style(win, LV_WIN_STYLE_BTNM_REL, &style_btnm_rel);
    lv_win_set_style(win, LV_WIN_STYLE_BTNM_PR, &style_btnm_pr);

    log_d("info_page_create_btmn_impl_profile_with_edit %d", ipBackBtnCache.cnt);
    return win;
}

//hide left and right btn
void info_page_hide_btn(lv_obj_t * win) {
    if (win != NULL) {
        lv_obj_t * header = lv_get_child_by_index(win, 2);   //all header
        lv_obj_t * r_btn = NULL;
        lv_obj_t * l_btn = NULL;
        if(is_ltr()){
            r_btn = lv_get_child_by_index(header, 2); //right btn
            l_btn = lv_get_child_by_index(header, 3); //left btn
        }else{
            l_btn = lv_get_child_by_index(header, 2);
            r_btn = lv_get_child_by_index(header, 3);
        }

        lv_obj_set_hidden(r_btn, true);
        lv_obj_set_hidden(l_btn, true);
    }
}

void info_page_close_win(lv_obj_t * win) {
    if (win != NULL) {
        lv_obj_t * header = lv_get_child_by_index(win, 2);
        lv_obj_t * l_btn = NULL;
        if(is_ltr()){
            l_btn = lv_get_child_by_index(header, 3);
        }else{
            l_btn = lv_get_child_by_index(header, 2);
        }

        lv_win_close_event_cb(l_btn, LV_EVENT_RELEASED);
        //remove ipBackBtnCache.cnt since info page already been deleted
        info_page_rm_back_cache(l_btn);
    }
}
