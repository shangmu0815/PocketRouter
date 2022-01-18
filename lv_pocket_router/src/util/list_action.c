/*********************
 *      INCLUDES
 *********************/
#include <assert.h>
#include <stdio.h>
#include "lv_pocket_router/res/values/string_value.h"
#include <stdbool.h>
#include "lvgl/lvgl.h"
#include "lvgl/src/lv_objx/lv_list.h"
#include "list_action.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/about/about.h"
#include "lv_pocket_router/src/about/device_information.h"
#include "lv_pocket_router/src/about/caution_for_using_5g.h"
#include "lv_pocket_router/src/about/about_help.h"
#include "lv_pocket_router/src/about/open_source_notice.h"
#include "lv_pocket_router/src/settings/data_roaming.h"
#include "lv_pocket_router/src/settings/language.h"
#include "lv_pocket_router/src/settings/wifi_extender.h"
#include "lv_pocket_router/src/settings/network_settings.h"
#include "lv_pocket_router/src/settings/wifi.h"
#include "lv_pocket_router/src/sms/sms.h"
#include "lv_pocket_router/src/settings/password_lock.h"
#include "lv_pocket_router/src/keyboard/num_kb_box.h"
#include "lv_pocket_router/src/settings/remote_wakeup.h"
#include "lv_pocket_router/src/settings/restore_default.h"
#include "lv_pocket_router/src/settings/data_usage.h"
#include "lv_pocket_router/src/settings/pin_management.h"
#include "lv_pocket_router/src/keyboard/en_kb.h"
#include "lv_pocket_router/src/settings/update.h"
#include "lv_pocket_router/src/settings/bluetooth_settings.h"
#include "lv_pocket_router/src/settings/profile_management.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"

static lv_style_t style_bg_header;
static lv_style_t style_btn_rel;
static lv_style_t style_btn_pr;
static lv_style_t style_btn_ina;

//for home key action
#define MAX_LIST_PAGE_NUM       5
#define CACHE_INDEX_INVALID     255

typedef struct {
    int cnt;
    lv_obj_t * btn[MAX_LIST_PAGE_NUM];
    lv_event_cb_t action[MAX_LIST_PAGE_NUM];
    // to keep Back btn info for Tick/Delete(garbage) type of list btn (ie, for trigger Back action after additional action)
    lv_obj_t * added_list_back_btn;
    lv_event_cb_t added_action;
} ListActionBack;

ListActionBack laBackBtnCache = { .cnt = 0 };

void list_action_add_back_cache(lv_obj_t * btn, lv_event_cb_t action) {
    assert(laBackBtnCache.cnt < MAX_LIST_PAGE_NUM);
    laBackBtnCache.btn[laBackBtnCache.cnt] = btn;
    laBackBtnCache.action[laBackBtnCache.cnt] = action;
    laBackBtnCache.cnt++;
}

void list_action_rm_back_cache(lv_obj_t * btn) {
    bool found = false;
    int i;
    for (i = 0; i < laBackBtnCache.cnt; i++) {
        if (found) {
            laBackBtnCache.action[i - 1] = laBackBtnCache.action[i];
            laBackBtnCache.btn[i - 1] = laBackBtnCache.btn[i];
            continue;
        }

        if (laBackBtnCache.btn[i] == btn) {
            found = true;
            laBackBtnCache.action[i] = NULL;
            laBackBtnCache.btn[i] = NULL;
        }
    }
    if (found) {
        laBackBtnCache.action[laBackBtnCache.cnt - 1] = NULL;
        laBackBtnCache.btn[laBackBtnCache.cnt - 1] = NULL;
        laBackBtnCache.cnt--;
    }
}

int list_action_lookup_back_cache(lv_obj_t * btn) {
    int i;
    for (i = 0; i < laBackBtnCache.cnt; i++) {
        if (laBackBtnCache.btn[i] == btn) {
            return i;
        }
    }
    return CACHE_INDEX_INVALID;
}

//Init header style
void style_create_(void) {

    //header border line style
    lv_style_copy(&style_bg_header, &lv_style_plain);
    style_bg_header.body.main_color = LV_COLOR_WHITE;
    style_bg_header.body.grad_color = LV_COLOR_WHITE;
    style_bg_header.body.border.part = LV_BORDER_BOTTOM;
    style_bg_header.body.border.color = LV_COLOR_SILVER;
    style_bg_header.body.border.width = 2;
    style_bg_header.text.font = get_font(font_w_bold, font_h_24);
    style_bg_header.text.color = LV_COLOR_MATTERHORN;
    style_bg_header.text.letter_space = 1;

    //header icon style
    lv_style_copy(&style_btn_rel, &lv_style_plain);
    style_btn_rel.body.main_color = LV_COLOR_WHITE;
    style_btn_rel.body.grad_color = LV_COLOR_WHITE;
    style_btn_rel.body.radius = 0;
    style_btn_rel.text.color = LV_COLOR_MATTERHORN;
    style_btn_rel.image.color = LV_COLOR_MATTERHORN;
    style_btn_rel.image.intense = LV_OPA_COVER;

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.image.color = LV_COLOR_BASE;
    style_btn_pr.image.intense = LV_OPA_COVER;

    lv_style_copy(&style_btn_ina, &style_btn_rel);
    style_btn_ina.image.color = LV_COLOR_NOBEL;
    style_btn_ina.image.intense = LV_OPA_COVER;
}

void list_action_win_close_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    int index = list_action_lookup_back_cache(btn);
    // invalid means non Back btn case, should be Tick/Delete(garbage) btn
    if (index != CACHE_INDEX_INVALID) {
        lv_event_cb_t back_action;
        assert(index < MAX_LIST_PAGE_NUM);
        back_action = laBackBtnCache.action[index];
        list_action_rm_back_cache(btn);
        if (back_action != lv_win_close_event_cb) {
            back_action(btn, LV_EVENT_CLICKED);
        }

        //exception for sms list, let sms close by itself
        if (back_action != sms_list_back_action)
            lv_win_close_event_cb(btn, LV_EVENT_RELEASED);
    }
}

//for home key action, close lists from "start" param in list_array
lv_res_t close_all_lists(int start)
{
    int i;
    for (i = laBackBtnCache.cnt-1; i >= start; i--) {
        if (laBackBtnCache.btn[i]) {
            list_action_win_close_action(laBackBtnCache.btn[i], LV_EVENT_CLICKED);
        }
    }
    laBackBtnCache.cnt = start;
    printf("close_all_lists %d \n", laBackBtnCache.cnt);

    return LV_RES_INV;
}

void list_action_additional_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    lv_event_cb_t action = laBackBtnCache.added_action;
    action(btn, LV_EVENT_CLICKED);
    list_action_win_close_action(laBackBtnCache.added_list_back_btn, LV_EVENT_CLICKED);
}

//define header home key action
void home_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    //if (btn != NULL) {
    //    lv_win_close_event_cb(btn, LV_EVENT_RELEASED);
    //}
    //close_all_lists(0);
    //close_all_pages();
    launch_home_behaviour();
}

void win_title_adjust(lv_obj_t * win, const char * headline){
    lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
    lv_coord_t txt_x = get_txt_x_size(ext->title, headline);
    lv_coord_t total = txt_x + SMALL_ICON_X*2 + HOR_SPACE*4;

    if(total > LV_HOR_RES_MAX){
        lv_coord_t new_txt_x = LV_HOR_RES_MAX - total + txt_x;
        lv_label_set_long_mode(ext->title, LV_LABEL_LONG_SROLL_CIRC);
        lv_obj_set_width(ext->title, new_txt_x);
        lv_obj_align(ext->title, ext->header, LV_ALIGN_CENTER, 0, 0);
    }
}

/**
 * Create list header
 * @param par pointer to an object, it will be the parent of the new window
 * @param headline text of header title
 * @param r_img image icon on the right side of the title
 * @param l_img image icon on the left side of the title
 * @return pointer to the created window
 */
lv_obj_t * list_header_create(lv_obj_t * par, const void * headline, const void * r_img, const void * l_img
    , lv_event_cb_t r_action, lv_event_cb_t l_action)
{
    style_create_();

    lv_obj_t * win = lv_win_create(par, NULL);
    lv_win_set_title(win, headline);
    lv_win_set_style(win, LV_WIN_STYLE_BG, &lv_style_plain);
    lv_win_set_style(win, LV_WIN_STYLE_CONTENT, &lv_style_transp_tight);
    lv_win_set_style(win, LV_WIN_STYLE_HEADER, &style_bg_header);

    lv_obj_t * back_btn = NULL;
    if(is_ltr()){
        //draw right button icon
        lv_obj_t * r_btn = NULL;
        if (r_img == &ic_headline_tick) {
            r_btn = lv_win_add_btn(win, r_img);
            lv_obj_set_event_cb(r_btn, list_action_additional_action);
        } else {
            r_btn = lv_win_add_btn(win, r_img);
            lv_obj_set_event_cb(r_btn, r_action);
        }
        //draw left button icon
        back_btn = lv_win_add_btn(win, l_img);
        lv_obj_set_event_cb(back_btn, list_action_win_close_action);

    }else{
        //draw right button icon
        if(l_img == &ic_headline_back) {
            back_btn = lv_win_add_btn(win, &ic_headline_back_rtl);
        } else{
            back_btn = lv_win_add_btn(win, l_img);
        }
        lv_obj_set_event_cb(back_btn, list_action_win_close_action);
        lv_obj_t * l_btn = NULL;
        //draw left button icon
        if (r_img == &ic_headline_tick) {
            l_btn = lv_win_add_btn(win, r_img);
            lv_obj_set_event_cb(l_btn, list_action_additional_action);
        } else {
            l_btn = lv_win_add_btn(win, r_img);
            lv_obj_set_event_cb(l_btn, r_action);
        }
    }
    //set title lable to scroll circle mode if too long
    win_title_adjust(win, headline);

    //set button style
    lv_win_set_style(win, LV_WIN_STYLE_BTN_REL, &style_btn_rel);
    lv_win_set_style(win, LV_WIN_STYLE_BTN_PR, &style_btn_pr);

    //remove scroll bar
    lv_win_set_sb_mode(win, LV_SB_MODE_OFF);

    //save all the list win
    list_action_add_back_cache(back_btn, l_action);
    // for handling back btn after finish tick callback
    if (r_img == &ic_headline_tick) {
        laBackBtnCache.added_action = r_action;
        laBackBtnCache.added_list_back_btn = back_btn;
    }

    printf("list_header_create %d \n", laBackBtnCache.cnt);
    return win;
}

//list header with back on left, home on right and a garbage btn next to home
lv_obj_t * del_list_header_create(lv_obj_t * par, const void * headline,
        lv_event_cb_t l_action, lv_event_cb_t del_action)
{
    lv_obj_t * win = list_header_create(par, headline,
        &ic_headline_home, &ic_headline_back, home_action, l_action);

    lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
    lv_obj_t * l_btn = lv_obj_get_child(ext->header, NULL);
    lv_obj_t * r_btn = lv_obj_get_child(ext->header, l_btn);

    //draw garbage icon on header
    lv_obj_t * gar_btn = lv_btn_create(ext->header, l_btn);
    lv_obj_set_event_cb(gar_btn, del_action);
    //set garbage btn default state to INA
    lv_btn_set_style(gar_btn, LV_BTN_STYLE_INA, &style_btn_ina);
    lv_btn_set_state(gar_btn, LV_BTN_STATE_INA);

    lv_obj_t * gar_img = lv_img_create(gar_btn, NULL);
    lv_img_set_src(gar_img, &ic_headline_delete_dis);

    if(is_ltr()){
        lv_obj_align(ext->title, ext->header, LV_ALIGN_CENTER, -10, 0);
        //because we switch win header btn order for rtl cases
        //in ltr, home was in far right, and in far left if in rtl
#ifdef CUST_DLINK
        lv_obj_align(gar_btn, r_btn, LV_ALIGN_OUT_LEFT_MID, -15, 0);
#else
        lv_obj_align(gar_btn, r_btn, LV_ALIGN_OUT_LEFT_MID, 0, 0);
#endif
    } else{
        lv_obj_align(ext->title, ext->header, LV_ALIGN_CENTER, 10, 0);
#ifdef CUST_DLINK
        lv_obj_align(gar_btn, l_btn, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
#else
        lv_obj_align(gar_btn, l_btn, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
#endif
    }

    return win;
}

//to create info page for sms, it include a garbage btn next to home
lv_obj_t * sms_list_header_create(lv_obj_t * par, const void * headline, lv_event_cb_t l_action, lv_event_cb_t del_action)
{
    lv_obj_t * win = del_list_header_create(par, headline, l_action, del_action);
    lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
    lv_obj_t * gar_btn = lv_obj_get_child(ext->header, NULL);
    lv_obj_t * l_btn = lv_obj_get_child(ext->header, gar_btn);
    lv_obj_t * r_btn = lv_obj_get_child(ext->header, l_btn);

    //exception for sms list to handle home by itself
    if(is_ltr()){
        lv_obj_set_event_cb(r_btn, sms_list_home_action);
    }else{
        lv_obj_set_event_cb(l_btn, sms_list_home_action);
    }
    printf("sms_list_header_create %d \n", laBackBtnCache.cnt);

    return win;
}

/**
 * Add default list header with back button on the left
 * and home button on the right, header title shows in the middle
 */
lv_obj_t * default_list_header(lv_obj_t * par, const void * headline, lv_event_cb_t l_action)
{
    lv_obj_t * win = list_header_create(par, headline,
        &ic_headline_home, &ic_headline_back, home_action, l_action);

    return win;
}

/**
 * Add list header for page required modification with
 * cancel button on the left and saved button on the right,
 * header title shows in the middle
 */
lv_obj_t * modify_list_header(lv_obj_t * par, const void * headline
    , lv_event_cb_t r_action, lv_event_cb_t l_action)

{
    lv_obj_t * win = list_header_create(par, headline,
        &ic_headline_tick, &ic_headline_close, r_action, l_action);

    return win;
}

/**
 * Add list header for page that required delete action,
 * it include back button on the left and trash can button
 * on the right, header title shows in the middle
 */
lv_obj_t * del_list_header(lv_obj_t * par, const void * headline
    , lv_event_cb_t r_action, lv_event_cb_t l_action)

{
    lv_obj_t * win = list_header_create(par, headline,
        &ic_headline_delete, &ic_headline_back, r_action, l_action);

    return win;
}

/**
 * Add list header for page that required to block connected user,
 * it include back button on the left and delete user button
 * on the right, header title shows in the middle
 */
lv_obj_t * block_list_header(lv_obj_t * par, const void * headline
    , lv_event_cb_t r_action, lv_event_cb_t l_action)

{
    lv_obj_t * win = list_header_create(par, headline,
        &ic_headline_block, &ic_headline_back, r_action, l_action);

    return win;
}

lv_obj_t * delete_profile_list_header_create(lv_obj_t * par, const void * headline, lv_event_cb_t action, lv_event_cb_t close_action)
{
    lv_obj_t * win = del_list_header_create(par, headline, close_action, list_action_additional_action);
    lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
    lv_obj_t * gar_btn = lv_obj_get_child(ext->header, NULL);
    lv_obj_t * home_btn = lv_obj_get_child(ext->header, gar_btn);
    lv_obj_t * back_btn = lv_obj_get_child(ext->header, home_btn);

    //TODO need to move below check to profile mng
    lv_btn_set_state(gar_btn, LV_BTN_STATE_REL);

    // for handling back btn after finish garbage callback
    laBackBtnCache.added_action = action;
    laBackBtnCache.added_list_back_btn = back_btn;

    printf("delete_profile_list_header_create %d \n", laBackBtnCache.cnt);
    return win;
}
