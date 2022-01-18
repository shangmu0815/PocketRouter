#include <lv_pocket_router/src/util/liste_style.h>
#include <stdio.h>
#include <time.h>
#include "../../res/values/styles.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/settings/language.h"

lv_style_t style_bar;
lv_style_t style_indic;
lv_style_t style_list_font;
lv_style_t style_bg;
lv_style_t style_btn_rel;
lv_style_t style_btn_ina;
lv_style_t style_btn_pr;
lv_style_t style_btn_cu_pr;
lv_style_t style_arrow_btn_pr;

#if defined(HIGH_RESOLUTION)
lv_style_t style_list_font_hd;
#endif

//for power off popup
lv_style_t style_pw_pr;
lv_style_t style_pw_rel;

//for switch element style
lv_style_t bg_style_ltr;
lv_style_t indic_style_ltr;
lv_style_t bg_style_rtl;
lv_style_t indic_style_rtl;
lv_style_t knob_style;

//for liste with lv_sw inactive style
lv_style_t bg_style_ltr_ina;
lv_style_t indic_style_ltr_ina;
lv_style_t knob_style_ina;
lv_style_t lv_sw_ina_fg;
lv_style_t style_list_font_ina;

//for sms style
lv_style_t style_sms_font_l;
lv_style_t style_sms_tc_font_l;
lv_style_t style_sms_font_rtl;

lv_style_t style_ta;
lv_style_t style_sb;

//for multi-lang
lv_style_t style_jp_font;
lv_style_t style_ar_font;

//to fix list touch performance issue
int clicked_id = -1;

//for profile management style
lv_style_t style_profile_font_l;
lv_style_t style_profile_font_rtl;

lv_style_t style_label_ina;

void liste_style_create(void) {

    lv_style_copy(&style_jp_font, &lv_style_plain);
    style_jp_font.text.font = get_locale_font(JP, font_w_regular, font_h_22);
    style_jp_font.text.color = LV_COLOR_MATTERHORN;

    lv_style_copy(&style_ar_font, &lv_style_plain);
    style_ar_font.text.font = get_locale_font(AR, font_w_bold, font_h_22);
    style_ar_font.text.color = LV_COLOR_MATTERHORN;

    lv_style_copy(&style_bar, &lv_style_pretty);
    style_bar.body.main_color = LV_COLOR_SILVER;
    style_bar.body.grad_color = LV_COLOR_SILVER;
    style_bar.body.radius = LV_RADIUS_CIRCLE;
    style_bar.body.border.color = LV_COLOR_SILVER;

    lv_style_copy(&style_indic, &lv_style_pretty);
    style_indic.body.grad_color =  LV_COLOR_BASE;
    style_indic.body.main_color=  LV_COLOR_BASE;
    style_indic.body.radius = LV_RADIUS_CIRCLE;
    style_indic.body.padding.left = 0;
    style_indic.body.padding.right = 0;
    style_indic.body.padding.top = 0;
    style_indic.body.padding.bottom = 0;

    lv_style_copy(&style_list_font, &lv_style_plain);
    style_list_font.text.font = get_font(font_w_regular, font_h_22);
    style_list_font.text.color = LV_COLOR_MATTERHORN;
    style_list_font.text.letter_space = 1;

#if defined(HIGH_RESOLUTION)
    lv_style_copy(&style_list_font_hd, &lv_style_plain);
    style_list_font_hd.text.font = get_font(font_w_bold, font_h_40);
    style_list_font_hd.text.color = LV_COLOR_MATTERHORN;
#endif

    //for sms preview font style
    lv_style_copy(&style_sms_font_l, &style_list_font);
    style_sms_font_l.text.font = get_font(font_w_bold, font_h_16);

    //background style
    lv_style_copy(&style_bg, &lv_style_plain);
    style_bg.body.main_color = LV_COLOR_WHITE;
    style_bg.body.grad_color = LV_COLOR_WHITE;
    style_bg.body.padding.top = 0;
    style_bg.body.padding.bottom = 0;

    lv_style_copy(&style_btn_rel, &lv_style_btn_rel);
    style_btn_rel.body.main_color = LV_COLOR_WHITE;
    style_btn_rel.body.grad_color = LV_COLOR_WHITE;
    style_btn_rel.body.border.color = LV_COLOR_SILVER;
    style_btn_rel.body.border.width = 2;
    style_btn_rel.body.border.opa = LV_OPA_COVER;
    style_btn_rel.body.border.part = LV_BORDER_BOTTOM;
    style_btn_rel.body.radius = 0;
    style_btn_rel.text.font = get_font(font_w_regular, font_h_22);
    style_btn_rel.text.color = LV_COLOR_MATTERHORN;
    style_btn_rel.text.letter_space = 1;

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.body.border.color = LV_COLOR_BASE;
    style_btn_pr.body.border.opa = LV_OPA_COVER;
    style_btn_pr.body.border.width = 3;
    style_btn_pr.text.color = LV_COLOR_BASE;

    lv_style_copy(&style_btn_ina, &style_btn_rel);
    style_btn_ina.body.border.color = LV_COLOR_SILVER;
    style_btn_ina.text.color = LV_COLOR_SILVER;
    style_btn_ina.text.color = LV_COLOR_SILVER;
    style_btn_ina.image.color = LV_COLOR_SILVER;
    style_btn_ina.image.intense = LV_OPA_COVER;

    lv_style_copy(&style_arrow_btn_pr, &style_btn_pr);
    style_arrow_btn_pr.image.color = LV_COLOR_BASE;
    style_arrow_btn_pr.image.intense = LV_OPA_COVER;

    lv_style_copy(&style_btn_cu_pr, &style_btn_pr);
    style_btn_cu_pr.image.color = LV_COLOR_BASE;
    style_btn_cu_pr.image.intense = LV_OPA_COVER;

    lv_style_copy(&bg_style_ltr, &lv_style_pretty);
    bg_style_ltr.body.radius = LV_RADIUS_CIRCLE;
    bg_style_ltr.body.main_color = LV_COLOR_SILVER;
    bg_style_ltr.body.grad_color = LV_COLOR_SILVER;
    bg_style_ltr.body.border.color = LV_COLOR_GREYISH_BROWN;
    bg_style_ltr.body.border.width = 2;
    bg_style_ltr.body.border.opa = LV_OPA_COVER;
    bg_style_ltr.body.padding.left = 0;
    bg_style_ltr.body.padding.right = 0;
    bg_style_ltr.body.padding.top = 0;
    bg_style_ltr.body.padding.bottom = 0;
    bg_style_ltr.body.opa = LV_OPA_COVER;

    lv_style_copy(&indic_style_ltr, &lv_style_pretty_color);
    indic_style_ltr.body.radius = LV_RADIUS_CIRCLE;
    indic_style_ltr.body.main_color = LV_COLOR_SWITCH;
    indic_style_ltr.body.grad_color = LV_COLOR_SWITCH;
    indic_style_ltr.body.padding.left = 2;
    indic_style_ltr.body.padding.right = 2;
    indic_style_ltr.body.padding.top = 2;
    indic_style_ltr.body.padding.bottom = 2;
    indic_style_ltr.body.opa = LV_OPA_COVER;
    indic_style_ltr.body.border.width = 1;
    indic_style_ltr.body.border.opa = LV_OPA_COVER;
    indic_style_ltr.body.border.color = LV_COLOR_SWITCH;

    lv_style_copy(&bg_style_rtl, &bg_style_ltr);
    bg_style_rtl.body.main_color = LV_COLOR_BASE;
    bg_style_rtl.body.grad_color = LV_COLOR_BASE;

    lv_style_copy(&indic_style_rtl, &indic_style_ltr);
    indic_style_rtl.body.main_color = LV_COLOR_SILVER;
    indic_style_rtl.body.grad_color = LV_COLOR_SILVER;
    indic_style_rtl.body.border.color = LV_COLOR_SILVER;

    lv_style_copy(&knob_style, &lv_style_pretty);
    knob_style.body.radius = LV_RADIUS_CIRCLE;
    knob_style.body.main_color = LV_COLOR_GREYISH_BROWN;
    knob_style.body.grad_color = LV_COLOR_GREYISH_BROWN;
    knob_style.body.shadow.width = 0;
    knob_style.body.shadow.type = LV_SHADOW_BOTTOM;

    lv_style_copy(&style_ta, &lv_style_plain);
    style_ta.text.color = LV_COLOR_GREYISH_BROWN;
    lv_style_copy(&style_ta, &lv_style_plain);
    style_ta.text.font = get_font(font_w_bold, font_h_16);
    style_ta.text.letter_space = 1;

    //scroll bar style
    lv_style_copy(&style_sb, &lv_style_plain);
    style_sb.body.main_color = LV_COLOR_BASE;
    style_sb.body.grad_color = LV_COLOR_BASE;
    style_sb.body.border.color = LV_COLOR_WHITE;
    style_sb.body.border.width = 1;
    style_sb.body.padding.inner = 4;
    style_sb.body.padding.left = 0;
    style_sb.body.padding.right = 0;
    style_sb.body.border.opa = LV_OPA_70;
    style_sb.body.radius = LV_RADIUS_CIRCLE;
    style_sb.body.opa = LV_OPA_100;

    //style for disable liste with lv_sw object
    lv_style_copy(&style_list_font_ina, &style_list_font);
    style_list_font_ina.text.color = LV_COLOR_SILVER;
    style_list_font_ina.image.color = LV_COLOR_SILVER;
    style_list_font_ina.image.intense = LV_OPA_COVER;

    lv_style_copy(&bg_style_ltr_ina, &bg_style_ltr);
    bg_style_ltr_ina.body.border.color = LV_COLOR_SILVER;

    lv_style_copy(&indic_style_ltr_ina, &indic_style_ltr);
    indic_style_ltr_ina.body.main_color = LV_COLOR_SILVER;
    indic_style_ltr_ina.body.grad_color = LV_COLOR_SILVER;
    indic_style_ltr_ina.body.border.color = LV_COLOR_SILVER;

    lv_style_copy(&knob_style_ina, &knob_style);
    knob_style_ina.body.main_color = LV_COLOR_SILVER;
    knob_style_ina.body.grad_color = LV_COLOR_SILVER;

    lv_style_copy(&lv_sw_ina_fg, &lv_style_plain);
    lv_sw_ina_fg.body.opa = LV_OPA_TRANSP;

    lv_style_copy(&style_label_ina, &style_ta);
    style_label_ina.body.border.color = LV_COLOR_SILVER;
    style_label_ina.text.color = LV_COLOR_SILVER;
}

lv_obj_t * lv_get_child_by_index(lv_obj_t * base, int target) {
    if (target == 0 || target > lv_obj_count_children(base)) {
        return NULL;
    }

    int count = lv_obj_count_children(base);
    int tar = count - target + 1; 

    int i = 1;
    lv_obj_t * obj;
    for (obj = lv_ll_get_head(&base->child_ll); obj != NULL; obj = lv_ll_get_next(&base->child_ll, obj)) {
        if (i++ == tar) {
            return obj;
        }
    }
    return NULL;
}

//to fix list touch performance issue we will bypass useless event
void liste_style_action(lv_obj_t * liste, lv_event_t event){
    if (event == LV_EVENT_PRESSED){
        clicked_id = lv_obj_get_user_data(liste);

    } else if(event == LV_EVENT_CLICKED){
        if(clicked_id >= 0){
            //click now
            log_d("liste_style click %d now", clicked_id);
            lv_event_cb_t cb = lv_obj_get_origin_event_cb(liste);
            lv_event_send_func(cb, liste, event, lv_event_get_data());
        }
        clicked_id = -1;
    }
}

void page_style_action(lv_obj_t * list, lv_event_t event){
    //reset click id if user start drag action
    clicked_id = -1;
}

lv_obj_t * lv_liste_basic(lv_obj_t * list, lv_event_cb_t action, int id){
    lv_obj_t * liste = lv_btn_create(list, NULL);
    lv_obj_t * page = lv_page_get_scrl(list);
    lv_obj_set_event_cb(page, page_style_action);

    lv_btn_set_style(liste, LV_BTN_STYLE_REL, &style_btn_rel);
    lv_btn_set_style(liste, LV_BTN_STYLE_PR, &style_arrow_btn_pr);

    lv_page_glue_obj(liste, true);
    lv_btn_set_layout(liste, LV_LAYOUT_OFF);
    if(action != NULL){
        lv_obj_set_origin_event_cb(liste, action);
        lv_obj_set_event_cb(liste, liste_style_action);
    }
    if(id > 0){
        lv_obj_set_user_data(liste, id);
    }

    lv_obj_set_protect(liste, LV_PROTECT_PRESS_LOST);
    lv_obj_set_size(liste, LISTE_X, LISTE_Y);

    return liste;
}

//create list element title and set pos based on is_ltr
lv_obj_t * lv_liste_create_title(lv_obj_t * par, const char * txt){

    lv_obj_t * title =  lv_label_create(par, NULL);
    lv_obj_set_size(title, 240, 48);
    lv_label_set_text(title, txt);

    lv_obj_align(title, NULL,
            (is_ltr() ? LV_ALIGN_IN_LEFT_MID : LV_ALIGN_IN_RIGHT_MID),
            (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 0);
    lv_label_set_align(title, is_ltr() ? LV_LABEL_ALIGN_LEFT : LV_LABEL_ALIGN_RIGHT);

    return title;
}

//for long title and short width obj
void simple_title_adjust(lv_obj_t * title, const char * txt){
    simple_title_adjust_impl(title, txt, 0);
}

void simple_title_adjust_impl(lv_obj_t * title, const char * txt, int extra_x){
    composite_title_adjust(title, NULL, txt, NULL, LV_SW_PR_X + extra_x);
}

void composite_title_adjust(lv_obj_t * title, lv_obj_t * value, const char * txt, const char * val, int extra_x){

    //if title too long, set lable to scroll circle mode
    lv_coord_t txt_x = get_txt_x_size(title, txt);
    lv_coord_t val_x = 0;
    if(value != NULL) val_x = get_txt_x_size(value, val);
    lv_coord_t total = txt_x + val_x + HOR_SPACE*4 + extra_x;

    if (total > LV_HOR_RES_MAX) {
        lv_coord_t new_txt_x = LV_HOR_RES_MAX - total + txt_x;
        lv_label_set_long_mode(title, LV_LABEL_LONG_SROLL_CIRC);
        lv_obj_set_width(title, new_txt_x);

        lv_obj_align(title, NULL,
                (is_ltr() ? LV_ALIGN_IN_LEFT_MID : LV_ALIGN_IN_RIGHT_MID),
                (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 0);
    }
}

//create a list element with title on the left, arrow icon on far
//right and a value locate on the left side of arrow icon
lv_obj_t * lv_liste_w_arrow(lv_obj_t * list, const char * txt, const char * val, lv_event_cb_t action){
    lv_obj_t * liste;
    liste = lv_liste_basic(list, action, -1);

    //set text on far left
    lv_obj_t * title = lv_liste_create_title(liste, txt);

    //set arrow icon on far right
    lv_obj_t * img = lv_img_create(liste, NULL);
    lv_obj_set_size(img, 32, 32);
    lv_obj_set_click(img, false);
    lv_img_set_src(img, (is_ltr() ? &ic_arrow : &ic_arrow_rtl));
    lv_obj_align(img, NULL,
            (is_ltr() ? LV_ALIGN_IN_RIGHT_MID : LV_ALIGN_IN_LEFT_MID),
            (is_ltr() ? -HOR_SPACE : HOR_SPACE), 0);

    //set text on far left
    lv_obj_t * value =  lv_label_create(liste, NULL);
    lv_obj_set_size(value, 114, 48);
    lv_label_set_text(value, val);
    //lv_obj_set_style(value, &style_list_font);
    lv_obj_align(value, img,
            (is_ltr() ? LV_ALIGN_OUT_LEFT_MID : LV_ALIGN_OUT_RIGHT_MID), 0, 0);

    //if title too long, set lable to scroll circle mode
    composite_title_adjust(title, value, txt, val, SMALL_ICON_X);

    return liste;
}

void lv_liste_w_arrow_align(lv_obj_t * lable){

    lv_obj_t * liste = lv_obj_get_parent(lable);
    lv_obj_t * img = lv_obj_get_child(liste, lable);
    lv_obj_align(img, NULL,
            (is_ltr() ? LV_ALIGN_IN_RIGHT_MID : LV_ALIGN_IN_LEFT_MID),
            (is_ltr() ? -HOR_SPACE : HOR_SPACE), 0);
    lv_obj_align(lable, img,
            (is_ltr() ? LV_ALIGN_OUT_LEFT_MID : LV_ALIGN_OUT_RIGHT_MID), 0, 0);
}

// liste with title/value sometimes may need to adjust to scroll mode
// and sometimes don't, so we reset long mode before checking
void lv_liste_w_arrow_align_scroll(lv_obj_t * liste, int res_txt, char* val){
    lv_obj_t * title = lv_get_child_by_index(liste, 1);
    lv_obj_t * value = lv_get_child_by_index(liste, 3);
    lv_label_set_long_mode(title, LV_LABEL_LONG_EXPAND);
    composite_title_adjust(title, value, get_string(res_txt), val, SMALL_ICON_X);

    lv_liste_w_arrow_align(value);
}

void lv_liste_w_arrow_align_liste(lv_obj_t * liste){

    lv_obj_t * img = lv_obj_get_child(liste, NULL);
    lv_obj_t * lable = lv_obj_get_child(liste, img);

    lv_obj_align(img, NULL,
            (is_ltr() ? LV_ALIGN_IN_RIGHT_MID : LV_ALIGN_IN_LEFT_MID),
            (is_ltr() ? -HOR_SPACE : HOR_SPACE), 0);
    lv_obj_align(lable, NULL,
            (is_ltr() ? LV_ALIGN_IN_LEFT_MID : LV_ALIGN_IN_RIGHT_MID),
            (is_ltr() ? HOR_SPACE : -HOR_SPACE), 0);
}

//for disable liste with lv_sw object
void lv_liste_w_sw_ina(lv_obj_t * liste){
    lv_obj_t * sw = lv_obj_get_child(liste, NULL);
    lv_obj_t * title = lv_obj_get_child(liste, sw);
    lv_obj_set_style(title, &style_list_font_ina);

#ifdef CUST_SWITCH
    lv_obj_set_style(sw, &style_list_font_ina);
#else
    lv_sw_set_style(sw, LV_SW_STYLE_BG, &bg_style_ltr_ina);
    lv_sw_set_style(sw, LV_SW_STYLE_INDIC, &indic_style_ltr_ina);
    lv_sw_set_style(sw, LV_SW_STYLE_KNOB_ON, &knob_style_ina);
    lv_sw_set_style(sw, LV_SW_STYLE_KNOB_OFF, &knob_style_ina);
#endif
    //set an inactive foreground to disable sw movement
    lv_obj_t * ina_fg = lv_cont_create(liste, NULL);
    lv_obj_set_size(ina_fg, LISTE_X, LISTE_Y);
    lv_obj_set_style(ina_fg, &lv_sw_ina_fg);
}

//create a list element with title on far left and check box (replace switch if CUST_SW) on far right
lv_obj_t * lv_liste_cust_switch(lv_obj_t * list, const char * txt, lv_event_cb_t action, bool enable){

    lv_obj_t * liste;
    liste = lv_liste_w_cbox(list, txt, enable, action, 0);

    lv_obj_t * img = lv_obj_get_child(liste, NULL);
    lv_img_set_src(img, enable? &ic_list_checkbox_selected : &ic_list_checkbox);

    return liste;
}

void set_cust_switch_state(lv_obj_t * obj, char * key){
    lv_obj_t * img = lv_obj_get_child(obj, NULL);
    if (ds_get_bool(key)){
        //disable
        lv_img_set_src(img, &ic_list_checkbox);
        ds_set_bool(key, false);
    } else {
        //enable
        lv_img_set_src(img, &ic_list_checkbox_selected);
        ds_set_bool(key, true);
    }
}

//create a list element with title on the left and switch on far right
lv_obj_t * lv_liste_w_switch(lv_obj_t * list, const char * txt, lv_event_cb_t action) {

    lv_obj_t * liste;
    liste = lv_liste_basic(list, NULL, -1);

#if defined(HIGH_RESOLUTION)
    lv_obj_set_size(liste, LIST_OBJ_HEIGHT, LISTE_Y * (LV_RES_OFFSET/2));
#else
    lv_obj_set_size(liste, LISTE_X, LISTE_Y);
#endif

    //set text on far left
    lv_obj_t * title = lv_liste_create_title(liste, txt);
#if defined(HIGH_RESOLUTION)
    lv_obj_set_style(title, &style_list_font_hd);
#else
    lv_obj_set_style(title, &style_list_font);
#endif

    lv_obj_t *sw = lv_sw_create(liste, NULL);
    //TODO switch on/off in rtl not finish
    if(true){//is_ltr()
        lv_sw_set_style(sw, LV_SW_STYLE_BG, &bg_style_ltr);
        lv_sw_set_style(sw, LV_SW_STYLE_INDIC, &indic_style_ltr);
    }else{
        lv_sw_set_style(sw, LV_SW_STYLE_BG, &bg_style_rtl);
        lv_sw_set_style(sw, LV_SW_STYLE_INDIC, &indic_style_rtl);
    }
    lv_sw_set_style(sw, LV_SW_STYLE_KNOB_ON, &knob_style);
    lv_sw_set_style(sw, LV_SW_STYLE_KNOB_OFF, &knob_style);
    lv_obj_set_event_cb(sw, action);
#if defined(HIGH_RESOLUTION)
    lv_obj_set_size(sw, 200, 40);
#endif
    //TODO have switch direction issue
    lv_obj_align(sw, NULL,
            (is_ltr() ? LV_ALIGN_IN_RIGHT_MID : LV_ALIGN_IN_LEFT_MID),
            (is_ltr() ? -LISTE_SHIFT : LISTE_SHIFT), 0);

    //if title too long, set lable to scroll circle mode
    simple_title_adjust(title, txt);

    return liste;
}

//create a list element with title on far left and check box on far right
lv_obj_t * lv_liste_w_cbox(lv_obj_t * list, const char * txt, bool enable, lv_event_cb_t action, int id){

    lv_obj_t * liste;
    liste = lv_liste_basic(list, action, id);

    //set text on far left
    lv_obj_t * title = lv_liste_create_title(liste, txt);
    lv_obj_set_style(title, &style_list_font);

    //if title too long, set lable to scroll circle mode
    simple_title_adjust(title, txt);

    //set arrow icon on far right
    lv_obj_t * img = lv_img_create(liste, NULL);
    lv_obj_set_size(img, 32, 32);
    lv_obj_align(img, NULL,
            (is_ltr() ? LV_ALIGN_IN_RIGHT_MID : LV_ALIGN_IN_LEFT_MID),
            (is_ltr() ? -HOR_SPACE : HOR_SPACE), 0);

    if (enable)
        lv_img_set_src(img, &btn_list_radio_p);
    else
        lv_img_set_src(img, &btn_list_radio_n);

    return liste;
}

//for Settings language
lv_obj_t * lv_liste_w_cbox_style(lv_obj_t * list, const char * txt, bool enable, lv_event_cb_t action, int id){
    lv_obj_t * liste = lv_liste_w_cbox(list, txt, enable, action, id);
    lv_obj_t * title = lv_get_child_by_index(liste, 1);
    bool realign = true;

    //apply style for different font
    if(strcmp(txt, get_string(ID_LANG_JP)) == 0){
        lv_obj_set_style(title, &style_jp_font);
    }else if(strcmp(txt, get_string(ID_LANG_AR)) == 0){
        lv_obj_set_style(title, &style_ar_font);
    }else if(strcmp(txt, get_string(ID_LANG_ZH_CN)) == 0 || strcmp(txt, get_string(ID_LANG_ZH_TW)) == 0){
        lv_obj_set_style(title, &style_jp_font);
    }else{
        realign = false;
    }
    //re-align title
    if(realign){
        lv_obj_align(title, NULL,
                (is_ltr() ? LV_ALIGN_IN_LEFT_MID : LV_ALIGN_IN_RIGHT_MID),
                (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 0);
        lv_label_set_align(title, is_ltr() ? LV_LABEL_ALIGN_LEFT : LV_LABEL_ALIGN_RIGHT);
    }
    return liste;
}

//count the max char sms preview txt could show
uint16_t sms_prev_txt_cnt (const char * txt, const lv_font_t * font, lv_coord_t letter_space, lv_coord_t max_width)
{
    if(txt == NULL) return 0;
    if(font == NULL) return 0;

    uint32_t i                   = 0;
    uint32_t i_next              = 0;
    lv_coord_t cur_w             = 0;
    uint32_t letter_w;
    uint32_t letter              = 0;
    uint32_t letter_next         = 0;
    letter_next = lv_txt_encoded_next(txt, &i_next);

    while(txt[i] != '\0') {
        letter      = letter_next;
        i           = i_next;
        letter_next = lv_txt_encoded_next(txt, &i_next);
        letter_w = lv_font_get_glyph_width(font, letter, letter_next);
        cur_w += letter_w;
        if(letter_w > 0) {
            cur_w += letter_space;
        }
        if(cur_w > max_width) {
            i--;
            break;
        }
    }
    return i;
}

//create a list element to show sms message
lv_obj_t * lv_liste_sms(lv_obj_t * list, const char * title, const char * preview, int state, int id, int encoding){

    lv_obj_t * liste;
    liste = lv_liste_basic(list, NULL, id);
    //for showing sms that include AR language in ltr env
    bool ltr =  is_ltr() && !is_txt_rtl(preview);

    //set state icon on far right
    lv_obj_t * state_img = lv_img_create(liste, NULL);
    lv_obj_set_size(state_img, 32, 32);
    if (!state) {
        lv_img_set_src(state_img, &ic_list_sms);
    } else {
        lv_img_set_src(state_img, &ic_list_sms_read);
    }

    lv_obj_align(state_img, NULL,
            (ltr ? LV_ALIGN_IN_LEFT_MID : LV_ALIGN_IN_RIGHT_MID),
            (ltr ? LISTE_SHIFT : -LISTE_SHIFT), 0);

    //set sms title as upper text
    lv_obj_t * utxt =  lv_label_create(liste, NULL);
    lv_obj_set_size(utxt, 204, 24);
    lv_label_set_text(utxt, title);
    lv_obj_set_style(utxt, &style_list_font);
    lv_obj_align(utxt, state_img,
            (ltr ? LV_ALIGN_OUT_RIGHT_TOP : LV_ALIGN_OUT_LEFT_TOP),
            (ltr ? LISTE_SHIFT : -LISTE_SHIFT), -10);

    //set sms preview as lower text
    lv_obj_t * ltxt =  lv_label_create(liste, NULL);
    lv_label_set_long_mode(ltxt, LV_LABEL_LONG_CROP);
    lv_obj_set_size(ltxt, 204, 24);
    lv_style_copy(&style_sms_tc_font_l, &style_list_font);
#ifdef JP_AR_FONT
    style_sms_tc_font_l.text.font = get_font(font_w_bold, font_h_22);
#else
    style_sms_tc_font_l.text.font = get_locale_font_cust(font_w_bold, font_h_22);
#endif

    lv_style_copy(&style_sms_font_rtl, &style_list_font);
#ifdef JP_AR_FONT
    style_sms_font_rtl.text.font = get_font(font_w_bold, font_h_22);
#else
    style_sms_font_rtl.text.font = get_locale_font(AR, font_w_bold, font_h_22);
#endif

#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    /*if(encoding == RIL_SMS_ENCODING_7BIT){
        lv_obj_set_style(ltxt, &style_sms_font_l);
    } else if(encoding == RIL_SMS_ENCODING_16BIT){
        //for TC
        lv_obj_set_style(ltxt, &style_sms_tc_font_l);
    } else if(encoding == RIL_SMS_ENCODING_8BIT){
        //8bit encoding currently not supported, may show unexpected result
        lv_obj_set_style(ltxt, &style_sms_font_l);
    } else if(encoding == RIL_SMS_ENCODING_UNKNOWN){
        //unknown encoding, may show unexpected result
        //modified RIL_SMS_ENCODING_UNKNOWN to style_sms_tc_font_l
        //this is for MT sms with TC in it
        lv_obj_set_style(ltxt, &style_sms_tc_font_l);
    }*/
#endif
    //TODO temp fix for ar language missing in sms issue
    if(ltr){
        lv_obj_set_style(ltxt, &style_sms_tc_font_l);
    }else{
        lv_obj_set_style(ltxt, &style_sms_font_rtl);
    }

    //try show max preview txt with ... attached in the end
    const lv_style_t * style = lv_obj_get_style(ltxt);
    //minus "..." to count max length, for ".", letter_w=5, letter_space=1 => (204-5*3-2)
    int len = sms_prev_txt_cnt(preview, style->text.font, style->text.letter_space, 187);
    char preview_tmp[len +1];
    char preview_short[len + 4];
    memset(preview_tmp, 0, len + 1);
    memset(preview_short, 0, len + 4);
    strncpy(preview_tmp, preview, len);
    if(ltr){
       sprintf(preview_short, "%s...", preview_tmp);
    } else {
       sprintf(preview_short, "%s", preview_tmp);
    }
    lv_label_set_text(ltxt, preview_short);

    lv_obj_align(ltxt, state_img,
            (ltr ? LV_ALIGN_OUT_RIGHT_BOTTOM : LV_ALIGN_OUT_LEFT_BOTTOM),
            (ltr ? LISTE_SHIFT : -LISTE_SHIFT), 10);
    lv_label_set_align(ltxt, ltr ? LV_LABEL_ALIGN_LEFT : LV_LABEL_ALIGN_RIGHT);

    //set arrow icon on far right
    lv_obj_t * img = lv_img_create(liste, NULL);
    lv_obj_set_size(img, 32, 32);
    lv_obj_set_click(img, false);
    lv_img_set_src(img, (ltr ? &ic_arrow : &ic_arrow_rtl));
    lv_obj_align(img, NULL,
            (ltr ? LV_ALIGN_IN_RIGHT_MID : LV_ALIGN_IN_LEFT_MID),
            (ltr ? -HOR_SPACE : HOR_SPACE), 0);

    return liste;
}

lv_obj_t * lv_liste_w_checkbox(lv_obj_t * list, const char * txt, bool enable, lv_event_cb_t action, int id) {

    lv_obj_t * liste;
    liste = lv_liste_basic(list, action, id);

    //set arrow icon on far right
    lv_obj_t * img = lv_img_create(liste, NULL);
    lv_obj_set_size(img, 32, 32);
    lv_obj_align(img, NULL,
            (is_ltr() ? LV_ALIGN_IN_LEFT_MID : LV_ALIGN_IN_RIGHT_MID),
            (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 0);

    if (enable)
        lv_img_set_src(img, &ic_list_checkbox_selected);
    else
        lv_img_set_src(img, &ic_list_checkbox);

    //set text on far left
    lv_obj_t * title =  lv_label_create(liste, NULL);
    lv_obj_set_size(title, 250, 48);
    lv_label_set_text(title, txt);
    //lv_obj_set_style(title, &style_list_font);
    lv_obj_align(title, img,
            (is_ltr() ? LV_ALIGN_OUT_RIGHT_MID : LV_ALIGN_OUT_LEFT_MID),
            (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 0);

    return liste;
}

lv_obj_t * lv_liste_w_scrl_txt(lv_obj_t * list, const char * txt){

    lv_obj_t * page = lv_page_create(list, NULL);
    if (get_device_locale() == JP) {
        lv_obj_set_size(page, 320, 150);
    } else {
        lv_obj_set_size(page, 320, 130);
    }
    lv_page_set_style(page, LV_PAGE_STYLE_BG, &style_bg);
    lv_page_set_style(page, LV_PAGE_STYLE_SCRL, &style_bg);
    lv_page_set_style(page, LV_PAGE_STYLE_SB, &style_sb);

    lv_obj_t * label = lv_label_create(page, NULL);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_label_set_text(label, txt);
    lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style_ta);
    if (get_device_locale() == JP) {
        lv_obj_set_size(label, 300, 150);
    } else {
        lv_obj_set_size(label, 300, 130);
    }

    if(is_ltr()){
        lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
    }else{
        lv_label_set_align(label, LV_LABEL_ALIGN_RIGHT);
    }
    lv_page_set_scroll_propagation(page, true);

    return list;
}

void lv_liste_w_arrow_ina(lv_obj_t * liste, bool enable)
{
    if(enable){
        lv_btn_set_style(liste, LV_BTN_STYLE_REL, &style_btn_rel);
        lv_btn_set_state(liste, LV_BTN_STATE_REL);
    }else{
        lv_btn_set_style(liste, LV_BTN_STYLE_INA, &style_btn_ina);
        lv_btn_set_state(liste, LV_BTN_STATE_INA);
    }
}

//create a list element to show connected user
char * lv_liste_conn_usr(lv_obj_t * list, const char * title, const char * preview, lv_event_cb_t action, int id){

    lv_obj_t * liste;
    liste = lv_liste_basic(list, action, id);

    //set state icon on far left
    lv_obj_t * block_img = lv_img_create(liste, NULL);
    lv_obj_set_size(block_img, 32, 32);
    lv_img_set_src(block_img, &ic_list_profile);
    lv_obj_align(block_img, NULL,
            (is_ltr() ? LV_ALIGN_IN_LEFT_MID : LV_ALIGN_IN_RIGHT_MID),
            (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 0);

    //set connected user name as upper text
    lv_obj_t * utxt =  lv_label_create(liste, NULL);
    lv_obj_set_size(utxt, 204, 24);
    lv_label_set_text(utxt, title);
    lv_obj_set_style(utxt, &style_list_font);
    lv_obj_align(utxt, block_img,
            (is_ltr() ? LV_ALIGN_OUT_RIGHT_TOP : LV_ALIGN_OUT_LEFT_TOP),
            (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), -5);

    //set connected user duration as lower text
    lv_obj_t * ltxt =  lv_label_create(liste, NULL);
    lv_obj_set_size(ltxt, 204, 20);
    lv_label_set_text(ltxt, preview);
    lv_obj_set_style(ltxt, &style_sms_font_l);
    lv_obj_align(ltxt, block_img,
            (is_ltr() ? LV_ALIGN_OUT_RIGHT_BOTTOM : LV_ALIGN_OUT_LEFT_BOTTOM),
            (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 5);

    //set block icon on far right
    lv_obj_t * img = lv_img_create(liste, NULL);
    lv_obj_set_size(img, 32, 32);
    lv_obj_set_click(img, false);
    lv_img_set_src(img, &ic_list_block_n);
    lv_obj_align(img, NULL,
            (is_ltr() ? LV_ALIGN_IN_RIGHT_MID : LV_ALIGN_IN_LEFT_MID),
            (is_ltr() ? -HOR_SPACE : HOR_SPACE), 0);

    return title;
}

//create a list element to show connected user
char * lv_liste_blk_usr(lv_obj_t * list, const char * title, lv_event_cb_t action, int id){

    lv_obj_t * liste;
    liste = lv_liste_basic(list, action, id);

    //set state icon on far right
    lv_obj_t * block_img = lv_img_create(liste, NULL);
    lv_obj_set_size(block_img, 32, 32);
    lv_img_set_src(block_img, &ic_list_users);
    lv_obj_align(block_img, NULL,
            (is_ltr() ? LV_ALIGN_IN_LEFT_MID : LV_ALIGN_IN_RIGHT_MID),
            (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 0);

    //set connected user name
    lv_obj_t * utxt =  lv_label_create(liste, NULL);
    lv_obj_set_size(utxt, 204, 24);
    lv_label_set_text(utxt, title);
    lv_obj_set_style(utxt, &style_list_font);
    lv_obj_align(utxt, block_img,
            (is_ltr() ? LV_ALIGN_OUT_RIGHT_MID : LV_ALIGN_OUT_LEFT_MID),
            (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 0);

    //set block icon on far right
    lv_obj_t * img = lv_img_create(liste, NULL);
    lv_obj_set_size(img, 32, 32);
    lv_obj_set_click(img, false);
    lv_img_set_src(img, &ic_list_remove_n);
    lv_obj_align(img, NULL,
            (is_ltr() ? LV_ALIGN_IN_RIGHT_MID : LV_ALIGN_IN_LEFT_MID),
            (is_ltr() ? -HOR_SPACE : HOR_SPACE), 0);

    return title;

}

//for ssid name/password liste only
lv_obj_t * lv_liste_w_arrow_w_item_id_ssid(lv_obj_t * list, const char * txt, const char * val, lv_event_cb_t action, int id){
    lv_obj_t * liste;
    liste = lv_liste_w_arrow_w_item_id(list, txt, val, action, id);
    lv_obj_t * title = lv_obj_get_child_back(liste, NULL);
    if(is_ltr()) {
        //below part only apply in ssid name/password ltr cases
        lv_label_set_long_mode(title, LV_LABEL_LONG_SROLL_CIRC);
        lv_obj_set_width(title, 260);
    }else{
        //below part only apply in ssid name/password rtl cases
        lv_obj_t * img = lv_obj_get_child_back(liste, title);
        lv_obj_t * value = lv_obj_get_child_back(liste, img);
        lv_label_set_long_mode(title, LV_LABEL_LONG_CROP);
        lv_obj_set_width(title, 140);
        lv_label_set_long_mode(value, LV_LABEL_LONG_SROLL_CIRC);
        lv_obj_set_width(value, 130);
    }
    lv_obj_align(title, NULL,
            (is_ltr() ? LV_ALIGN_IN_LEFT_MID : LV_ALIGN_IN_RIGHT_MID),
            (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 0);
    return liste;
}

lv_obj_t * lv_liste_w_arrow_w_item_id(lv_obj_t * list, const char * txt, const char * val, lv_event_cb_t action, int id){

    lv_obj_t * liste;
    liste = lv_liste_basic(list, action, id);

    //set text on far left
    lv_obj_t * title = lv_liste_create_title(liste, txt);

    //if title too long, set lable to scroll circle mode
    simple_title_adjust(title, txt);

    //set arrow icon on far right
    lv_obj_t * img = lv_img_create(liste, NULL);
    lv_obj_set_size(img, 32, 32);
    lv_obj_set_click(img, false);
    lv_img_set_src(img, (is_ltr() ? &ic_arrow : &ic_arrow_rtl));
    lv_obj_align(img, NULL,
            (is_ltr() ? LV_ALIGN_IN_RIGHT_MID : LV_ALIGN_IN_LEFT_MID),
            (is_ltr() ? -HOR_SPACE : HOR_SPACE), 0);

    if (val != NULL) {
        //set text on far left
        lv_obj_t * value =  lv_label_create(liste, NULL);
        lv_obj_set_size(value, 114, 48);
        lv_label_set_text(value, val);
        //lv_obj_set_style(value, &style_list_font);
        lv_obj_align(value, img,
                (is_ltr() ? LV_ALIGN_OUT_LEFT_MID : LV_ALIGN_OUT_RIGHT_MID), 0, 0);

        //if title too long, set lable to scroll circle mode
        composite_title_adjust(title, value, txt, val, SMALL_ICON_X);
    }

    return liste;
}

//create a list element to show settings
lv_obj_t * lv_liste_settings(lv_obj_t * list, const void * img_src,
        const char * txt, lv_event_cb_t action, int id) {

    lv_obj_t * liste;
    liste = lv_liste_basic(list, action, id);

    lv_obj_t * state_img;
    lv_obj_t * img;
    if (img_src != NULL) {
        //set state icon on far left
        state_img = lv_img_create(liste, NULL);
        lv_obj_set_size(state_img, 32, 32);
        lv_img_set_src(state_img, img_src);
        lv_obj_align(state_img, NULL,
                (is_ltr() ? LV_ALIGN_IN_LEFT_MID : LV_ALIGN_IN_RIGHT_MID),
                (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 0);
    }
    //set text on far left
    lv_obj_t * title = lv_liste_create_title(liste, txt);
    simple_title_adjust_impl(title, txt, LV_SW_PR_X);

    //no need set arrow icon if img_src == NULL
    if (img_src != NULL) {
        //set arrow icon on far right
        img = lv_img_create(liste, NULL);
        lv_obj_set_size(img, 32, 32);
        lv_obj_set_click(img, false);
        lv_img_set_src(img, (is_ltr() ? &ic_arrow : &ic_arrow_rtl));
        lv_obj_align(title, state_img,
                (is_ltr() ? LV_ALIGN_OUT_RIGHT_MID : LV_ALIGN_OUT_LEFT_MID),
                (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 0);
        lv_obj_align(img, NULL,
                (is_ltr() ? LV_ALIGN_IN_RIGHT_MID : LV_ALIGN_IN_LEFT_MID),
                (is_ltr() ? -HOR_SPACE : HOR_SPACE), 0);
    }
    return liste;
}

//create a list element to show settings information
lv_obj_t * lv_liste_setting_info(lv_obj_t * list, const void * img_src,
        const char * txt, lv_event_cb_t action, int id) {

    lv_obj_t * liste;
    lv_obj_t * arrow;
    liste = lv_liste_settings(list, img_src, txt, action, id);

    //hide arrow icon
    arrow = lv_get_child_by_index(liste, 3);
    lv_obj_set_hidden(arrow, 1);

    //if title too long, set lable to scroll circle mode
    lv_obj_t * title = lv_obj_get_child(liste, arrow);
    lv_obj_t * img = lv_obj_get_child(liste, title);
    simple_title_adjust(title, txt);

    lv_obj_align(title, img,
            (is_ltr() ? LV_ALIGN_OUT_RIGHT_MID : LV_ALIGN_OUT_LEFT_MID),
            (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 0);

    //remove liste press effect
    lv_btn_set_style(liste, LV_BTN_STYLE_PR, &style_btn_rel);

    return liste;
}

//create a list element to show settings information in 2 lines
lv_obj_t * lv_liste_setting_double_info(lv_obj_t * list, const char * txt1, const char * txt2) {

    lv_obj_t * liste;
    liste = lv_liste_basic(list, NULL, -1);

    //remove liste press effect
    lv_btn_set_style(liste, LV_BTN_STYLE_PR, &style_btn_rel);

    //set title on top
    lv_obj_t * title =  lv_label_create(liste, NULL);
    lv_obj_set_size(title, 200, 24);
    lv_label_set_text(title, txt1);

    //set data on bottom
    lv_obj_t * data =  lv_label_create(liste, NULL);
    lv_label_set_text(data, txt2);
    lv_label_set_long_mode(data, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_width(data, 300);

    lv_obj_align(title, NULL,
            (is_ltr() ? LV_ALIGN_IN_TOP_LEFT : LV_ALIGN_IN_TOP_RIGHT),
            (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 5);
    lv_obj_align(data, NULL,
            (is_ltr() ? LV_ALIGN_IN_BOTTOM_LEFT : LV_ALIGN_IN_BOTTOM_RIGHT),
            (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), -5);

    return liste;
}

//create a list element with title on far left and check box on far right only for profile management
lv_obj_t * lv_liste_profile_w_cbox(lv_obj_t * list, const char * txt, bool enable, lv_event_cb_t action, int id){
    lv_obj_t * liste;
    liste = lv_liste_basic(list, action, id);

    //set text on far left
    lv_obj_t * title = lv_liste_create_title(liste, txt);

    //use NotoSansCJKjp_Bold_24 font to show chinese for all language
    lv_style_copy(&style_profile_font_l, &lv_style_plain);
    style_profile_font_l.text.font = get_locale_font_cust(font_w_bold, font_h_22);
    style_profile_font_l.text.letter_space = 1;

    lv_style_copy(&style_profile_font_rtl, &lv_style_plain);
    style_profile_font_rtl.text.font = get_locale_font(AR, font_w_bold, font_h_22);

    lv_obj_set_style(title,
            (is_ltr() ? &style_profile_font_l : &style_profile_font_rtl));
    //re-align title
    lv_obj_align(title, NULL,
            (is_ltr() ? LV_ALIGN_IN_LEFT_MID : LV_ALIGN_IN_RIGHT_MID),
            (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 0);
    lv_label_set_align(title, is_ltr() ? LV_LABEL_ALIGN_LEFT : LV_LABEL_ALIGN_RIGHT);

    //if title too long, set lable to scroll circle mode
    simple_title_adjust(title, txt);
    //set arrow icon on far right
    lv_obj_t * img = lv_img_create(liste, NULL);
    lv_obj_set_size(img, 32, 32);
    lv_obj_align(img, NULL,
            (is_ltr() ? LV_ALIGN_IN_RIGHT_MID : LV_ALIGN_IN_LEFT_MID),
            (is_ltr() ? -HOR_SPACE : HOR_SPACE), 0);

    if (enable)
        lv_img_set_src(img, &btn_list_radio_p);
    else
        lv_img_set_src(img, &btn_list_radio_n);

    return liste;
}

lv_obj_t * lv_liste_profile_w_checkbox(lv_obj_t * list, const char * txt, bool enable, lv_event_cb_t action, int id) {
    lv_obj_t * liste;
    liste = lv_liste_basic(list, action, id);

    //set arrow icon on far right
    lv_obj_t * img = lv_img_create(liste, NULL);
    lv_obj_set_size(img, 32, 32);
    lv_obj_align(img, NULL,
            (is_ltr() ? LV_ALIGN_IN_LEFT_MID : LV_ALIGN_IN_RIGHT_MID),
            (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 0);

    if (enable)
        lv_img_set_src(img, &ic_list_checkbox_selected);
    else
        lv_img_set_src(img, &ic_list_checkbox);

    //set text on far left
    lv_obj_t * title =  lv_label_create(liste, NULL);
    lv_obj_set_size(title, 250, 48);
    lv_label_set_text(title, txt);
    //use NotoSansCJKjp_Bold_24 font to support chinese for all language
    //when in profile management
    lv_style_copy(&style_profile_font_l, &style_list_font);
    style_profile_font_l.text.font = get_locale_font_cust(font_w_bold, font_h_22);
    style_profile_font_l.text.letter_space = 1;

    lv_style_copy(&style_profile_font_rtl, &style_list_font);
    style_profile_font_rtl.text.font = get_locale_font(AR, font_w_bold, font_h_22);

    lv_obj_set_style(title,
            (is_ltr() ? &style_profile_font_l : &style_profile_font_rtl));

    lv_obj_align(title, img,
            (is_ltr() ? LV_ALIGN_OUT_RIGHT_MID : LV_ALIGN_OUT_LEFT_MID),
            (is_ltr() ? LISTE_SHIFT : -LISTE_SHIFT), 0);
    lv_label_set_align(title, is_ltr() ? LV_LABEL_ALIGN_LEFT : LV_LABEL_ALIGN_RIGHT);
    return liste;
}

lv_obj_t * lv_liste_w_scrl_txt_ina(lv_obj_t * list, const char * txt, bool inactive) {

    lv_obj_t * page = lv_page_create(list, NULL);
    lv_obj_set_size(page, 320, 130);
    lv_page_set_style(page, LV_PAGE_STYLE_BG, &style_bg);
    lv_page_set_style(page, LV_PAGE_STYLE_SCRL, &style_bg);
    lv_page_set_style(page, LV_PAGE_STYLE_SB, &style_sb);

    lv_obj_t * label = lv_label_create(page, NULL);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_label_set_text(label, txt);
    lv_label_set_style(label, LV_LABEL_STYLE_MAIN,((inactive) ? &style_label_ina : &style_ta));
    lv_obj_set_size(label, 300, 130);

    if(is_ltr()){
        lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
    }else{
        lv_label_set_align(label, LV_LABEL_ALIGN_RIGHT);
    }
    lv_page_set_scroll_propagation(page, true);

    return list;
}
