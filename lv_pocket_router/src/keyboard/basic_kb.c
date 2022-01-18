#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "basic_kb.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/liste_style.h"

static lv_style_t style_line;
static lv_style_t style_header;
static lv_style_t style_bg;
static lv_style_t style_bg_pr;
static lv_style_t style_btn_bg;
static lv_style_t style_btn_rel;
static lv_style_t style_btn_pr;

static const lv_point_t p[] = {{0, 49}, {320, 49}};
static const char * btnm_map[] = {"0", "1", "2", "3", "4", "\20","\n",
                             "5", "6", "7", "8", "9", "\20", "\n", ""};
//basic keyboard root view
lv_obj_t * root;

void close_kb(){
    if(root != NULL){
        lv_obj_del(root);
        root = NULL;
    }
}

void basic_kb_style(void) {
    //draw header line
    lv_style_copy(&style_line, &lv_style_plain);
    style_line.line.color = LV_COLOR_SILVER;

    lv_style_copy(&style_header, &lv_style_plain);
    style_header.text.font = get_font(font_w_bold, font_h_24);
    style_header.text.letter_space = 1;
    style_header.text.color = LV_COLOR_MATTERHORN;

    lv_style_copy(&style_bg, &lv_style_plain);
    style_bg.body.main_color = LV_COLOR_WHITE;
    style_bg.body.grad_color = LV_COLOR_WHITE;
    style_bg.body.padding.top = 0;

    lv_style_copy(&style_bg_pr, &lv_style_plain);
    style_bg_pr.body.main_color = LV_COLOR_WHITE;
    style_bg_pr.body.grad_color = LV_COLOR_WHITE;
    style_bg_pr.image.color = LV_COLOR_BASE;
    style_bg_pr.image.intense = LV_OPA_COVER;

    //keyboard style
    lv_style_copy(&style_btn_bg, &lv_style_plain);
    style_btn_bg.body.main_color = LV_COLOR_SILVER;
    style_btn_bg.body.grad_color = LV_COLOR_SILVER;
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
    style_btn_rel.text.color = LV_COLOR_GREYISH_BROWN;
    style_btn_rel.text.font = get_font(font_w_bold, font_h_26);

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.body.main_color = LV_COLOR_BASE;
    style_btn_pr.body.grad_color = LV_COLOR_BASE;
    style_btn_pr.text.color = LV_COLOR_WHITE;
    style_btn_pr.image.color = LV_COLOR_WHITE;
    style_btn_pr.image.intense = LV_OPA_COVER;
}

lv_obj_t * basic_kb_create(const void * headline){
    basic_kb_style();

    //draw background
    root = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(root, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style(root, &style_bg);

    //draw headline line
    lv_obj_t * line = lv_line_create(root, NULL);
    lv_obj_set_pos(line, 0, 0);
    lv_line_set_points(line, p, 2);
    lv_line_set_style(line, LV_LABEL_STYLE_MAIN, &style_line);

    //draw headline text
    lv_obj_t * headline_txt = lv_label_create(root, NULL);
    lv_obj_set_size(headline_txt, 220, 40);
    lv_label_set_text(headline_txt, headline);
    lv_obj_set_style(headline_txt, &style_header);

    //if title too long, set lable to scroll circle mode
    composite_title_adjust(headline_txt, NULL, headline, NULL, LV_SW_PR_X + HOR_SPACE*2);
    lv_obj_align(headline_txt, NULL, LV_ALIGN_IN_TOP_MID, 0, 12);

    //draw headline left button
    lv_obj_t * l_btn = lv_btn_create(root, NULL);
    lv_obj_set_size(l_btn, 32, 32);
    lv_obj_align(l_btn, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);
    lv_btn_set_style(l_btn, LV_BTN_STYLE_REL, &style_bg);
    lv_btn_set_style(l_btn, LV_BTN_STYLE_PR, &style_bg_pr);
    lv_btn_set_style(l_btn, LV_BTN_STYLE_INA, &style_bg);

    //draw headline right button
    lv_obj_t * r_btn = lv_btn_create(root, l_btn);
    lv_obj_align(r_btn, NULL, LV_ALIGN_IN_TOP_RIGHT, -10, 10);

    return root;
}

void basic_num_kb(lv_obj_t * par, lv_event_cb_t kb_action, lv_event_cb_t del_action){
    //draw numeric keyboard
    lv_obj_t * num_btn = lv_btnm_create(par, NULL);
    lv_btnm_set_map(num_btn, btnm_map);
    lv_obj_set_event_cb(num_btn, kb_action);
    lv_obj_set_user_data(num_btn, KB_NUM_BTNM);
    lv_obj_set_size(num_btn, LV_HOR_RES_MAX, LV_VER_RES_MAX / 2);

    lv_btnm_set_style(num_btn, LV_BTNM_STYLE_BG, &style_btn_bg);
    lv_btnm_set_style(num_btn, LV_BTNM_STYLE_BTN_REL, &style_btn_rel);
    lv_btnm_set_style(num_btn, LV_BTNM_STYLE_BTN_PR, &style_btn_pr);
    lv_obj_align(num_btn, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, LV_VER_RES_MAX / 6);

    //draw delete icon
    lv_obj_t * del_btn = lv_btn_create(par, NULL);
    lv_obj_set_user_data(del_btn, KB_DEL_BTN);

    lv_obj_set_event_cb(del_btn, del_action);
    lv_obj_set_size(del_btn, LV_HOR_RES_MAX / 6, LV_VER_RES_MAX / 3);

    lv_btn_set_style(del_btn, LV_BTN_STYLE_REL, &style_btn_rel);
    lv_btn_set_style(del_btn, LV_BTN_STYLE_PR, &style_btn_pr);
    lv_obj_align(del_btn, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -1, 0);

    lv_obj_t * del_img = lv_img_create(del_btn, NULL);
    lv_img_set_src(del_img, &ic_keyboard_delete);
    lv_obj_align(del_img, NULL, LV_ALIGN_CENTER, 0, 0);
}

void kb_update_headline(const char * headline) {
    lv_obj_t * title = lv_get_child_by_index(root, KB_TITLE);
    lv_label_set_text(title, headline);
    lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_MID, 0, 12);
}

//to handle home key for all kb
void kb_home_action(){
    //close all pages and list win underneath
    close_kb();
    //close_all_pages();
    //close_all_lists(0);
    launch_home_behaviour();
}

//if kb tip too long, set lable to scroll circle mode
void kb_tip_adjust(lv_obj_t * ta, const char * tip, int obj_x){
    lv_coord_t txt_x = get_txt_x_size(ta, tip);
    lv_coord_t total = txt_x + HOR_SPACE*2;

    if(total > obj_x){
        lv_coord_t new_txt_x = obj_x - HOR_SPACE*2;
        lv_ta_ext_t * ext = lv_obj_get_ext_attr(ta);
        lv_label_set_long_mode(ext->label, LV_LABEL_LONG_SROLL_CIRC);
        lv_obj_set_width(ta, new_txt_x);
    }
}
