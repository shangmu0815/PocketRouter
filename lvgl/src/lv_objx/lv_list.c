/**
 * @file lv_list.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_list.h"
#if LV_USE_LIST != 0

#include "../lv_core/lv_group.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_misc/lv_anim.h"
#include "../lv_misc/lv_math.h"
#include "../lv_objx/lv_cb.h"
#include "../lv_objx/lv_sw.h"

//FOR ZX53
#include "../../../lv_pocket_router/res/values/string_value.h"
#include "../../../lv_pocket_router/src/util/util.h"
#include <stdio.h>
#include "../../lvgl.h"
#include <stdlib.h>
/*********************
 *      DEFINES
 *********************/
#define LV_LIST_LAYOUT_DEF LV_LAYOUT_COL_M

#if LV_USE_ANIMATION == 0
#undef LV_LIST_DEF_ANIM_TIME
#define LV_LIST_DEF_ANIM_TIME 0
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_res_t lv_list_signal(lv_obj_t * list, lv_signal_t sign, void * param);
static lv_res_t lv_list_btn_signal(lv_obj_t * btn, lv_signal_t sign, void * param);
static void lv_list_btn_single_select(lv_obj_t * btn);
static bool lv_list_is_list_btn(lv_obj_t * list_btn);
static bool lv_list_is_list_img(lv_obj_t * list_btn);
static bool lv_list_is_list_label(lv_obj_t * list_btn);

/**********************
 *  STATIC VARIABLES
 **********************/
#if LV_USE_IMG
static lv_signal_cb_t img_signal;
#endif
static lv_signal_cb_t label_signal;
static lv_signal_cb_t ancestor_page_signal;
static lv_signal_cb_t ancestor_btn_signal;
#if LV_USE_GROUP
/*Used to make the last clicked button pressed (selected) when the list become focused and
 * `click_focus == 1`*/
static lv_obj_t * last_clicked_btn;
#endif

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a list objects
 * @param par pointer to an object, it will be the parent of the new list
 * @param copy pointer to a list object, if not NULL then the new object will be copied from it
 * @return pointer to the created list
 */
lv_obj_t * lv_list_create(lv_obj_t * par, const lv_obj_t * copy)
{
    LV_LOG_TRACE("list create started");

    /*Create the ancestor basic object*/
    lv_obj_t * new_list = lv_page_create(par, copy);
    lv_mem_assert(new_list);
    if(new_list == NULL) return NULL;

    if(ancestor_page_signal == NULL) ancestor_page_signal = lv_obj_get_signal_cb(new_list);

    lv_list_ext_t * ext = lv_obj_allocate_ext_attr(new_list, sizeof(lv_list_ext_t));
    lv_mem_assert(ext);
    if(ext == NULL) return NULL;

    ext->style_img                        = NULL;
    ext->styles_btn[LV_BTN_STATE_REL]     = &lv_style_btn_rel;
    ext->styles_btn[LV_BTN_STATE_PR]      = &lv_style_btn_pr;
    ext->styles_btn[LV_BTN_STATE_TGL_REL] = &lv_style_btn_tgl_rel;
    ext->styles_btn[LV_BTN_STATE_TGL_PR]  = &lv_style_btn_tgl_pr;
    ext->styles_btn[LV_BTN_STATE_INA]     = &lv_style_btn_ina;
    ext->single_mode                      = false;
    ext->size                             = 0;

#if LV_USE_GROUP
    ext->last_sel     = NULL;
    ext->selected_btn = NULL;
#endif

    lv_obj_set_signal_cb(new_list, lv_list_signal);

    /*Init the new list object*/
    if(copy == NULL) {
        lv_page_set_anim_time(new_list, LV_LIST_DEF_ANIM_TIME);
        lv_page_set_scrl_fit2(new_list, LV_FIT_FLOOD, LV_FIT_TIGHT);
        lv_obj_set_size(new_list, 2 * LV_DPI, 3 * LV_DPI);
        lv_page_set_scrl_layout(new_list, LV_LIST_LAYOUT_DEF);
        lv_list_set_sb_mode(new_list, LV_SB_MODE_DRAG);

        /*Set the default styles*/
        lv_theme_t * th = lv_theme_get_current();
        if(th) {
            lv_list_set_style(new_list, LV_LIST_STYLE_BG, th->style.list.bg);
            lv_list_set_style(new_list, LV_LIST_STYLE_SCRL, th->style.list.scrl);
            lv_list_set_style(new_list, LV_LIST_STYLE_SB, th->style.list.sb);
            lv_list_set_style(new_list, LV_LIST_STYLE_BTN_REL, th->style.list.btn.rel);
            lv_list_set_style(new_list, LV_LIST_STYLE_BTN_PR, th->style.list.btn.pr);
            lv_list_set_style(new_list, LV_LIST_STYLE_BTN_TGL_REL, th->style.list.btn.tgl_rel);
            lv_list_set_style(new_list, LV_LIST_STYLE_BTN_TGL_PR, th->style.list.btn.tgl_pr);
            lv_list_set_style(new_list, LV_LIST_STYLE_BTN_INA, th->style.list.btn.ina);
        } else {
            lv_list_set_style(new_list, LV_LIST_STYLE_BG, &lv_style_transp_fit);
            lv_list_set_style(new_list, LV_LIST_STYLE_SCRL, &lv_style_pretty);
        }
    } else {
        lv_list_ext_t * copy_ext = lv_obj_get_ext_attr(copy);

        lv_obj_t * copy_btn = lv_list_get_next_btn(copy, NULL);
        while(copy_btn) {
            const void * img_src = NULL;
#if LV_USE_IMG
            lv_obj_t * copy_img = lv_list_get_btn_img(copy_btn);
            if(copy_img) img_src = lv_img_get_src(copy_img);
#endif
            lv_list_add_btn(new_list, img_src, lv_list_get_btn_text(copy_btn));
            copy_btn = lv_list_get_next_btn(copy, copy_btn);
        }

        lv_list_set_style(new_list, LV_LIST_STYLE_BTN_REL, copy_ext->styles_btn[LV_BTN_STATE_REL]);
        lv_list_set_style(new_list, LV_LIST_STYLE_BTN_PR, copy_ext->styles_btn[LV_BTN_STATE_PR]);
        lv_list_set_style(new_list, LV_LIST_STYLE_BTN_TGL_REL, copy_ext->styles_btn[LV_BTN_STATE_TGL_REL]);
        lv_list_set_style(new_list, LV_LIST_STYLE_BTN_TGL_PR, copy_ext->styles_btn[LV_BTN_STATE_TGL_REL]);
        lv_list_set_style(new_list, LV_LIST_STYLE_BTN_INA, copy_ext->styles_btn[LV_BTN_STATE_INA]);

        /*Refresh the style with new signal function*/
        lv_obj_refresh_style(new_list);
    }

    LV_LOG_INFO("list created");

    return new_list;
}

/**
 * Delete all children of the scrl object, without deleting scrl child.
 * @param obj pointer to an object
 */
void lv_list_clean(lv_obj_t * obj)
{
    lv_obj_t * scrl = lv_page_get_scrl(obj);
    lv_obj_clean(scrl);
    lv_list_ext_t * ext = lv_obj_get_ext_attr(obj);
    ext->size           = 0;
}

/*======================
 * Add/remove functions
 *=====================*/

/**
 * Add a list element to the list
 * @param list pointer to list object
 * @param img_fn file name of an image before the text (NULL if unused)
 * @param txt text of the list element (NULL if unused)
 * @return pointer to the new list element which can be customized (a button)
 */
lv_obj_t * lv_list_add_btn(lv_obj_t * list, const void * img_src, const char * txt)
{
    lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
    ext->size++;
    /*Create a list element with the image an the text*/
    lv_obj_t * liste;
    liste = lv_btn_create(list, NULL);

    /*Save the original signal function because it will be required in `lv_list_btn_signal`*/
    if(ancestor_btn_signal == NULL) ancestor_btn_signal = lv_obj_get_signal_cb(liste);

    /*Set the default styles*/
    lv_btn_set_style(liste, LV_BTN_STYLE_REL, ext->styles_btn[LV_BTN_STATE_REL]);
    lv_btn_set_style(liste, LV_BTN_STYLE_PR, ext->styles_btn[LV_BTN_STATE_PR]);
    lv_btn_set_style(liste, LV_BTN_STYLE_TGL_REL, ext->styles_btn[LV_BTN_STATE_TGL_REL]);
    lv_btn_set_style(liste, LV_BTN_STYLE_TGL_PR, ext->styles_btn[LV_BTN_STATE_TGL_PR]);
    lv_btn_set_style(liste, LV_BTN_STYLE_INA, ext->styles_btn[LV_BTN_STATE_INA]);

    lv_page_glue_obj(liste, true);
    lv_btn_set_layout(liste, LV_LAYOUT_ROW_M);
    lv_btn_set_fit2(liste, LV_FIT_FLOOD, LV_FIT_TIGHT);
    lv_obj_set_protect(liste, LV_PROTECT_PRESS_LOST);
    lv_obj_set_signal_cb(liste, lv_list_btn_signal);

#if LV_USE_IMG != 0
    lv_obj_t * img = NULL;
    if(img_src) {
        img = lv_img_create(liste, NULL);
        lv_img_set_src(img, img_src);
        lv_obj_set_style(img, ext->style_img);
        lv_obj_set_click(img, false);
        if(img_signal == NULL) img_signal = lv_obj_get_signal_cb(img);
    }
#endif
    if(txt != NULL) {
        lv_coord_t btn_hor_pad = ext->styles_btn[LV_BTN_STYLE_REL]->body.padding.left -
                                 ext->styles_btn[LV_BTN_STYLE_REL]->body.padding.right;
        lv_obj_t * label = lv_label_create(liste, NULL);
        lv_label_set_text(label, txt);
        lv_obj_set_click(label, false);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SROLL_CIRC);
        lv_obj_set_width(label, liste->coords.x2 - label->coords.x1 - btn_hor_pad);
        if(label_signal == NULL) label_signal = lv_obj_get_signal_cb(label);
    }
#if LV_USE_GROUP
    /* If this is the first item to be added to the list and the list is
     * focused, select it */
    {
        lv_group_t * g = lv_obj_get_group(list);
        if(ext->size == 1 && lv_group_get_focused(g) == list) {
            lv_list_set_btn_selected(list, liste);
        }
    }
#endif

    return liste;
}

/**
 * Remove the index of the button in the list
 * @param list pointer to a list object
 * @param index pointer to a the button's index in the list, index must be 0 <= index <
 * lv_list_ext_t.size
 * @return true: successfully deleted
 */
bool lv_list_remove(const lv_obj_t * list, uint16_t index)
{
    lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
    if(index >= ext->size) return false;
    uint16_t count = 0;
    lv_obj_t * e   = lv_list_get_next_btn(list, NULL);
    while(e != NULL) {
        if(count == index) {
            lv_obj_del(e);
            ext->size--;
            return true;
        }
        e = lv_list_get_next_btn(list, e);
        count++;
    }
    return false;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set single button selected mode, only one button will be selected if enabled.
 * @param list pointer to the currently pressed list object
 * @param mode, enable(true)/disable(false) single selected mode.
 */
void lv_list_set_single_mode(lv_obj_t * list, bool mode)
{
    lv_list_ext_t * ext = lv_obj_get_ext_attr(list);

    ext->single_mode = mode;
}

#if LV_USE_GROUP

/**
 * Make a button selected
 * @param list pointer to a list object
 * @param btn pointer to a button to select
 *            NULL to not select any buttons
 */
void lv_list_set_btn_selected(lv_obj_t * list, lv_obj_t * btn)
{
    lv_list_ext_t * ext = lv_obj_get_ext_attr(list);

    if(ext->selected_btn) {
        lv_btn_state_t s = lv_btn_get_state(ext->selected_btn);
        if(s == LV_BTN_STATE_PR)
            lv_btn_set_state(ext->selected_btn, LV_BTN_STATE_REL);
        else if(s == LV_BTN_STATE_TGL_PR)
            lv_btn_set_state(ext->selected_btn, LV_BTN_STATE_TGL_REL);
    }

    ext->selected_btn = btn;

    /*Don't forget which button was selected.
     * It will be restored when the list is focused.*/
    if(btn != NULL) {
        ext->last_sel = btn;
    }

    if(ext->selected_btn) {
        lv_btn_state_t s = lv_btn_get_state(ext->selected_btn);
        if(s == LV_BTN_STATE_REL)
            lv_btn_set_state(ext->selected_btn, LV_BTN_STATE_PR);
        else if(s == LV_BTN_STATE_TGL_REL)
            lv_btn_set_state(ext->selected_btn, LV_BTN_STATE_TGL_PR);

        lv_page_focus(list, ext->selected_btn, lv_list_get_anim_time(list));
    }
}

#endif

/**
 * Set a style of a list
 * @param list pointer to a list object
 * @param type which style should be set
 * @param style pointer to a style
 */
void lv_list_set_style(lv_obj_t * list, lv_list_style_t type, const lv_style_t * style)
{
    lv_list_ext_t * ext           = lv_obj_get_ext_attr(list);
    lv_btn_style_t btn_style_refr = LV_BTN_STYLE_REL;
    lv_obj_t * btn;

    switch(type) {
        case LV_LIST_STYLE_BG:
            lv_page_set_style(list, LV_PAGE_STYLE_BG, style);
            /*style change signal will call 'refr_btn_width' */
            break;
        case LV_LIST_STYLE_SCRL: lv_page_set_style(list, LV_PAGE_STYLE_SCRL, style); break;
        case LV_LIST_STYLE_SB: lv_page_set_style(list, LV_PAGE_STYLE_SB, style); break;
        case LV_LIST_STYLE_EDGE_FLASH: lv_page_set_style(list, LV_PAGE_STYLE_EDGE_FLASH, style); break;
        case LV_LIST_STYLE_BTN_REL:
            ext->styles_btn[LV_BTN_STATE_REL] = style;
            btn_style_refr                    = LV_BTN_STYLE_REL;
            break;
        case LV_LIST_STYLE_BTN_PR:
            ext->styles_btn[LV_BTN_STATE_PR] = style;
            btn_style_refr                   = LV_BTN_STYLE_PR;
            break;
        case LV_LIST_STYLE_BTN_TGL_REL:
            ext->styles_btn[LV_BTN_STATE_TGL_REL] = style;
            btn_style_refr                        = LV_BTN_STYLE_TGL_REL;
            break;
        case LV_LIST_STYLE_BTN_TGL_PR:
            ext->styles_btn[LV_BTN_STATE_TGL_PR] = style;
            btn_style_refr                       = LV_BTN_STYLE_TGL_PR;
            break;
        case LV_LIST_STYLE_BTN_INA:
            ext->styles_btn[LV_BTN_STATE_INA] = style;
            btn_style_refr                    = LV_BTN_STYLE_INA;
            break;
    }

    /*Refresh existing buttons' style*/
    if(type == LV_LIST_STYLE_BTN_PR || type == LV_LIST_STYLE_BTN_REL || type == LV_LIST_STYLE_BTN_TGL_REL ||
       type == LV_LIST_STYLE_BTN_TGL_PR || type == LV_LIST_STYLE_BTN_INA) {
        btn = lv_list_get_prev_btn(list, NULL);
        while(btn != NULL) {
            lv_btn_set_style(btn, btn_style_refr, ext->styles_btn[btn_style_refr]);
            btn = lv_list_get_prev_btn(list, btn);
        }
    }
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get single button selected mode.
 * @param list pointer to the currently pressed list object.
 */
bool lv_list_get_single_mode(lv_obj_t * list)
{
    lv_list_ext_t * ext = lv_obj_get_ext_attr(list);

    return (ext->single_mode);
}

/**
 * Get the text of a list element
 * @param btn pointer to list element
 * @return pointer to the text
 */
const char * lv_list_get_btn_text(const lv_obj_t * btn)
{
    lv_obj_t * label = lv_list_get_btn_label(btn);
    if(label == NULL) return "";
    return lv_label_get_text(label);
}

/**
 * Get the label object from a list element
 * @param btn pointer to a list element (button)
 * @return pointer to the label from the list element or NULL if not found
 */
lv_obj_t * lv_list_get_btn_label(const lv_obj_t * btn)
{
    lv_obj_t * label = lv_obj_get_child(btn, NULL);
    if(label == NULL) return NULL;

    while(lv_list_is_list_label(label) == false) {
        label = lv_obj_get_child(btn, label);
        if(label == NULL) break;
    }

    return label;
}

/**
 * Get the image object from a list element
 * @param btn pointer to a list element (button)
 * @return pointer to the image from the list element or NULL if not found
 */
lv_obj_t * lv_list_get_btn_img(const lv_obj_t * btn)
{
#if LV_USE_IMG != 0
    lv_obj_t * img = lv_obj_get_child(btn, NULL);
    if(img == NULL) return NULL;

    while(lv_list_is_list_img(img) == false) {
        img = lv_obj_get_child(btn, img);
        if(img == NULL) break;
    }

    return img;
#else
    return NULL;
#endif
}

/**
 * Get the previous button from list. (Starts from the bottom button)
 * @param list pointer to a list object
 * @param prev_btn pointer to button. Search the previous before it.
 * @return pointer to the previous button or NULL when no more buttons
 */
lv_obj_t * lv_list_get_prev_btn(const lv_obj_t * list, lv_obj_t * prev_btn)
{
    /* Not a good practice but user can add/create objects to the lists manually.
     * When getting the next button try to be sure that it is at least a button */

    lv_obj_t * btn;
    lv_obj_t * scrl = lv_page_get_scrl(list);

    btn = lv_obj_get_child(scrl, prev_btn);
    if(btn == NULL) return NULL;

    while(lv_list_is_list_btn(btn) == false) {
        btn = lv_obj_get_child(scrl, btn);
        if(btn == NULL) break;
    }

    return btn;
}

/**
 * Get the next button from list. (Starts from the top button)
 * @param list pointer to a list object
 * @param prev_btn pointer to button. Search the next after it.
 * @return pointer to the next button or NULL when no more buttons
 */
lv_obj_t * lv_list_get_next_btn(const lv_obj_t * list, lv_obj_t * prev_btn)
{
    /* Not a good practice but user can add/create objects to the lists manually.
     * When getting the next button try to be sure that it is at least a button */

    lv_obj_t * btn;
    lv_obj_t * scrl = lv_page_get_scrl(list);

    btn = lv_obj_get_child_back(scrl, prev_btn);
    if(btn == NULL) return NULL;

    while(lv_list_is_list_btn(btn) == false) {
        btn = lv_obj_get_child_back(scrl, btn);
        if(btn == NULL) break;
    }

    return btn;
}

/**
 * Get the index of the button in the list
 * @param list pointer to a list object. If NULL, assumes btn is part of a list.
 * @param btn pointer to a list element (button)
 * @return the index of the button in the list, or -1 of the button not in this list
 */
int32_t lv_list_get_btn_index(const lv_obj_t * list, const lv_obj_t * btn)
{
    int index = 0;
    if(list == NULL) {
        /* no list provided, assuming btn is part of a list */
        list = lv_obj_get_parent(lv_obj_get_parent(btn));
    }
    lv_obj_t * e = lv_list_get_next_btn(list, NULL);
    while(e != NULL) {
        if(e == btn) {
            return index;
        }
        index++;
        e = lv_list_get_next_btn(list, e);
    }
    return -1;
}

/**
 * Get the number of buttons in the list
 * @param list pointer to a list object
 * @return the number of buttons in the list
 */
uint16_t lv_list_get_size(const lv_obj_t * list)
{
    lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
    return ext->size;
}

#if LV_USE_GROUP
/**
 * Get the currently selected button
 * @param list pointer to a list object
 * @return pointer to the selected button
 */
lv_obj_t * lv_list_get_btn_selected(const lv_obj_t * list)
{
    lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
    return ext->selected_btn;
}

#endif

/**
 * Get a style of a list
 * @param list pointer to a list object
 * @param type which style should be get
 * @return style pointer to a style
 *  */
const lv_style_t * lv_list_get_style(const lv_obj_t * list, lv_list_style_t type)
{
    const lv_style_t * style = NULL;
    lv_list_ext_t * ext      = lv_obj_get_ext_attr(list);

    switch(type) {
        case LV_LIST_STYLE_BG: style = lv_page_get_style(list, LV_PAGE_STYLE_BG); break;
        case LV_LIST_STYLE_SCRL: style = lv_page_get_style(list, LV_PAGE_STYLE_SCRL); break;
        case LV_LIST_STYLE_SB: style = lv_page_get_style(list, LV_PAGE_STYLE_SB); break;
        case LV_LIST_STYLE_EDGE_FLASH: style = lv_page_get_style(list, LV_PAGE_STYLE_EDGE_FLASH); break;
        case LV_LIST_STYLE_BTN_REL: style = ext->styles_btn[LV_BTN_STATE_REL]; break;
        case LV_LIST_STYLE_BTN_PR: style = ext->styles_btn[LV_BTN_STATE_PR]; break;
        case LV_LIST_STYLE_BTN_TGL_REL: style = ext->styles_btn[LV_BTN_STATE_TGL_REL]; break;
        case LV_LIST_STYLE_BTN_TGL_PR: style = ext->styles_btn[LV_BTN_STATE_TGL_PR]; break;
        case LV_LIST_STYLE_BTN_INA: style = ext->styles_btn[LV_BTN_STATE_INA]; break;
        default: style = NULL; break;
    }

    return style;
}
/*=====================
 * Other functions
 *====================*/

/**
 * Move the list elements up by one
 * @param list pointer a to list object
 */
void lv_list_up(const lv_obj_t * list)
{
    /*Search the first list element which 'y' coordinate is below the parent
     * and position the list to show this element on the bottom*/
    lv_obj_t * scrl = lv_page_get_scrl(list);
    lv_obj_t * e;
    lv_obj_t * e_prev = NULL;

    e                 = lv_list_get_prev_btn(list, NULL);
    while(e != NULL) {
        if(e->coords.y2 <= list->coords.y2) {
            if(e_prev != NULL) {
                lv_coord_t new_y = lv_obj_get_height(list) - (lv_obj_get_y(e_prev) + lv_obj_get_height(e_prev));
                if(lv_list_get_anim_time(list) == 0) {
                    lv_obj_set_y(scrl, new_y);
                } else {
#if LV_USE_ANIMATION
                    lv_anim_t a;
                    a.var            = scrl;
                    a.start          = lv_obj_get_y(scrl);
                    a.end            = new_y;
                    a.exec_cb        = (lv_anim_exec_xcb_t)lv_obj_set_y;
                    a.path_cb        = lv_anim_path_linear;
                    a.ready_cb       = NULL;
                    a.act_time       = 0;
                    a.time           = LV_LIST_DEF_ANIM_TIME;
                    a.playback       = 0;
                    a.playback_pause = 0;
                    a.repeat         = 0;
                    a.repeat_pause   = 0;
                    lv_anim_create(&a);
#endif
                }
            }
            break;
        }
        e_prev = e;
        e      = lv_list_get_prev_btn(list, e);
    }
}

/**
 * Move the list elements down by one
 * @param list pointer to a list object
 */
void lv_list_down(const lv_obj_t * list)
{
    /*Search the first list element which 'y' coordinate is above the parent
     * and position the list to show this element on the top*/
    lv_obj_t * scrl = lv_page_get_scrl(list);
    lv_obj_t * e;
    e = lv_list_get_prev_btn(list, NULL);
    while(e != NULL) {
        if(e->coords.y1 < list->coords.y1) {
            lv_coord_t new_y = -lv_obj_get_y(e);
            if(lv_list_get_anim_time(list) == 0) {
                lv_obj_set_y(scrl, new_y);
            } else {
#if LV_USE_ANIMATION
                lv_anim_t a;
                a.var            = scrl;
                a.start          = lv_obj_get_y(scrl);
                a.end            = new_y;
                a.exec_cb        = (lv_anim_exec_xcb_t)lv_obj_set_y;
                a.path_cb        = lv_anim_path_linear;
                a.ready_cb       = NULL;
                a.act_time       = 0;
                a.time           = LV_LIST_DEF_ANIM_TIME;
                a.playback       = 0;
                a.playback_pause = 0;
                a.repeat         = 0;
                a.repeat_pause   = 0;
                lv_anim_create(&a);
#endif
            }
            break;
        }
        e = lv_list_get_prev_btn(list, e);
    }
}

/**
 * Focus on a list button. It ensures that the button will be visible on the list.
 * @param btn pointer to a list button to focus
 * @param anim_en LV_ANIM_ON: scroll with animation, LV_ANOM_OFF: without animation
 */
void lv_list_focus(const lv_obj_t * btn, lv_anim_enable_t anim)
{

#if LV_USE_ANIMATION == 0
    anim = false;
#endif

    lv_obj_t * list = lv_obj_get_parent(lv_obj_get_parent(btn));

    lv_page_focus(list, btn, anim == LV_ANIM_OFF ? 0 : lv_list_get_anim_time(list));
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Signal function of the list
 * @param list pointer to a list object
 * @param sign a signal type from lv_signal_t enum
 * @param param pointer to a signal specific variable
 * @return LV_RES_OK: the object is not deleted in the function; LV_RES_INV: the object is deleted
 */
static lv_res_t lv_list_signal(lv_obj_t * list, lv_signal_t sign, void * param)
{
    lv_res_t res;

    /* Include the ancient signal function */
    res = ancestor_page_signal(list, sign, param);
    if(res != LV_RES_OK) return res;

    if(sign == LV_SIGNAL_RELEASED || sign == LV_SIGNAL_PRESSED || sign == LV_SIGNAL_PRESSING ||
       sign == LV_SIGNAL_LONG_PRESS || sign == LV_SIGNAL_LONG_PRESS_REP) {
#if LV_USE_GROUP
        /*If pressed/released etc by a KEYPAD or ENCODER delegate signal to the button*/
        lv_indev_t * indev         = lv_indev_get_act();
        lv_indev_type_t indev_type = lv_indev_get_type(indev);
        if(indev_type == LV_INDEV_TYPE_KEYPAD ||
           (indev_type == LV_INDEV_TYPE_ENCODER && lv_group_get_editing(lv_obj_get_group(list)))) {
            /*Get the 'pressed' button*/
            lv_obj_t * btn = NULL;
            btn            = lv_list_get_prev_btn(list, btn);
            while(btn != NULL) {
                if(lv_btn_get_state(btn) == LV_BTN_STATE_PR) break;
                btn = lv_list_get_prev_btn(list, btn);
            }
            lv_list_ext_t * ext = lv_obj_get_ext_attr(list);

            /*The page receives the key presses so the events should be propagated to the selected
             * button*/
            if(btn) {
                if(sign == LV_SIGNAL_PRESSED) {
                    res = lv_event_send(btn, LV_EVENT_PRESSED, NULL);
                } else if(sign == LV_SIGNAL_PRESSING) {
                    res = lv_event_send(btn, LV_EVENT_PRESSING, NULL);
                } else if(sign == LV_SIGNAL_LONG_PRESS) {
                    res = lv_event_send(btn, LV_EVENT_LONG_PRESSED, NULL);
                } else if(sign == LV_SIGNAL_LONG_PRESS_REP) {
                    res = lv_event_send(btn, LV_EVENT_LONG_PRESSED_REPEAT, NULL);
                } else if(sign == LV_SIGNAL_RELEASED) {
#if LV_USE_GROUP
                    ext->last_sel = btn;
#endif
                    if(indev->proc.long_pr_sent == 0) {
                        res = lv_event_send(btn, LV_EVENT_SHORT_CLICKED, NULL);
                    }
                    if(lv_indev_is_dragging(indev) == false && res == LV_RES_OK) {
                        res = lv_event_send(btn, LV_EVENT_CLICKED, NULL);
                    }
                    if(res == LV_RES_OK) {
                        res = lv_event_send(btn, LV_EVENT_RELEASED, NULL);
                    }
                }
            }
        }
#endif
    } else if(sign == LV_SIGNAL_FOCUS) {

#if LV_USE_GROUP
        lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_get_act());
        /*With ENCODER select the first button only in edit mode*/
        if(indev_type == LV_INDEV_TYPE_ENCODER) {
            lv_group_t * g = lv_obj_get_group(list);
            if(lv_group_get_editing(g)) {
                lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
                if(ext->last_sel) {
                    /* Select the    last used button */
                    lv_list_set_btn_selected(list, ext->last_sel);
                } else {
                    /*Get the first button and mark it as selected*/
                    lv_list_set_btn_selected(list, lv_list_get_next_btn(list, NULL));
                }
            } else {
                lv_list_set_btn_selected(list, NULL);
            }
        }
        /*Else select the clicked button*/
        else {
            /*Mark the last clicked button (if any) as selected because it triggered the focus*/
            if(last_clicked_btn) {
                lv_list_set_btn_selected(list, last_clicked_btn);
            } else {
                lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
                if(ext->last_sel) {
                    /* Select the last used button */
                    lv_list_set_btn_selected(list, ext->last_sel);
                } else {
                    /*Get the first button and mark it as selected*/
                    lv_list_set_btn_selected(list, lv_list_get_next_btn(list, NULL));
                }
            }
        }
#endif
    } else if(sign == LV_SIGNAL_DEFOCUS) {

#if LV_USE_GROUP
        /*De-select the selected btn*/
        lv_list_set_btn_selected(list, NULL);
        last_clicked_btn    = NULL; /*button click will be set if click happens before focus*/
        lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
        ext->selected_btn   = NULL;
#endif
    } else if(sign == LV_SIGNAL_GET_EDITABLE) {
        bool * editable = (bool *)param;
        *editable       = true;
    } else if(sign == LV_SIGNAL_CONTROL) {

#if LV_USE_GROUP
        char c = *((char *)param);
        if(c == LV_KEY_RIGHT || c == LV_KEY_DOWN) {
            lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
            /*If there is a valid selected button the make the previous selected*/
            if(ext->selected_btn) {
                lv_obj_t * btn_prev = lv_list_get_next_btn(list, ext->selected_btn);
                if(btn_prev) lv_list_set_btn_selected(list, btn_prev);
            }
            /*If there is no selected button the make the first selected*/
            else {
                lv_obj_t * btn = lv_list_get_next_btn(list, NULL);
                if(btn)
                    lv_list_set_btn_selected(list,
                                             btn); /*If there are no buttons on the list then there is no first button*/
            }
        } else if(c == LV_KEY_LEFT || c == LV_KEY_UP) {
            lv_list_ext_t * ext = lv_obj_get_ext_attr(list);
            /*If there is a valid selected button the make the next selected*/
            if(ext->selected_btn != NULL) {
                lv_obj_t * btn_next = lv_list_get_prev_btn(list, ext->selected_btn);
                if(btn_next) lv_list_set_btn_selected(list, btn_next);
            }
            /*If there is no selected button the make the first selected*/
            else {
                lv_obj_t * btn = lv_list_get_next_btn(list, NULL);
                if(btn) lv_list_set_btn_selected(list, btn);
            }
        }
#endif
    } else if(sign == LV_SIGNAL_GET_TYPE) {
        lv_obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < LV_MAX_ANCESTOR_NUM - 1; i++) { /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "lv_list";
    }
    return res;
}

/**
 * Signal function of the list buttons
 * @param btn pointer to a button on the list
 * @param sign a signal type from lv_signal_t enum
 * @param param pointer to a signal specific variable
 * @return LV_RES_OK: the object is not deleted in the function; LV_RES_INV: the object is deleted
 */
static lv_res_t lv_list_btn_signal(lv_obj_t * btn, lv_signal_t sign, void * param)
{
    lv_res_t res;

    /* Include the ancient signal function */
    res = ancestor_btn_signal(btn, sign, param);
    if(res != LV_RES_OK) return res;

    if(sign == LV_SIGNAL_RELEASED) {
        lv_obj_t * list          = lv_obj_get_parent(lv_obj_get_parent(btn));
        lv_list_ext_t * ext      = lv_obj_get_ext_attr(list);
        ext->page.scroll_prop_ip = 0;

#if LV_USE_GROUP
        lv_group_t * g = lv_obj_get_group(list);
        if(lv_group_get_focused(g) == list && lv_indev_is_dragging(lv_indev_get_act()) == false) {
            /* Is the list is focused then be sure only the button being released
             * has a pressed state to indicate the selected state on the list*/
            lv_obj_t * btn_i = lv_list_get_prev_btn(list, NULL);
            while(btn_i) {
                lv_btn_state_t s = lv_btn_get_state(btn_i);
                if(s == LV_BTN_STATE_PR)
                    lv_btn_set_state(btn_i, LV_BTN_STATE_REL);
                else if(s == LV_BTN_STATE_TGL_PR)
                    lv_btn_set_state(btn_i, LV_BTN_STATE_TGL_REL);
                btn_i = lv_list_get_prev_btn(list, btn_i);
            }

            /*Make the released button "selected"*/
            lv_list_set_btn_selected(list, btn);
        }

        /* If `click_focus == 1` then LV_SIGNAL_FOCUS need to know which button triggered the focus
         * to mark it as selected (pressed state)*/
        last_clicked_btn = btn;
#endif
        if(lv_indev_is_dragging(lv_indev_get_act()) == false && ext->single_mode) {
            lv_list_btn_single_select(btn);
        }
    } else if(sign == LV_SIGNAL_PRESS_LOST) {
        lv_obj_t * list          = lv_obj_get_parent(lv_obj_get_parent(btn));
        lv_list_ext_t * ext      = lv_obj_get_ext_attr(list);
        ext->page.scroll_prop_ip = 0;
    } else if(sign == LV_SIGNAL_CLEANUP) {

#if LV_USE_GROUP
        lv_obj_t * list = lv_obj_get_parent(lv_obj_get_parent(btn));
        lv_obj_t * sel  = lv_list_get_btn_selected(list);
        if(sel == btn) lv_list_set_btn_selected(list, lv_list_get_next_btn(list, btn));
#endif
    }

    return res;
}

/**
 * Make a single button selected in the list, deselect others.
 * @param btn pointer to the currently pressed list btn object
 */
static void lv_list_btn_single_select(lv_obj_t * btn)
{
    lv_obj_t * list = lv_obj_get_parent(lv_obj_get_parent(btn));

    lv_obj_t * e = lv_list_get_next_btn(list, NULL);
    do {
        if(e == btn) {
            lv_btn_set_state(e, LV_BTN_STATE_TGL_REL);
        } else {
            lv_btn_set_state(e, LV_BTN_STATE_REL);
        }
        e = lv_list_get_next_btn(list, e);
    } while(e != NULL);
}

/**
 * Check if this is really a list button or another object.
 * @param list_btn List button
 */
static bool lv_list_is_list_btn(lv_obj_t * list_btn)
{
    lv_obj_type_t type;

    lv_obj_get_type(list_btn, &type);
    uint8_t cnt;
    for(cnt = 0; cnt < LV_MAX_ANCESTOR_NUM; cnt++) {
        if(type.type[cnt] == NULL) break;
        if(!strcmp(type.type[cnt], "lv_btn")) return true;
    }
    return false;
}

/**
 * Check if this is really a list label or another object.
 * @param list_label List label
 */
static bool lv_list_is_list_label(lv_obj_t * list_label)
{
    lv_obj_type_t type;

    lv_obj_get_type(list_label, &type);
    uint8_t cnt;
    for(cnt = 0; cnt < LV_MAX_ANCESTOR_NUM; cnt++) {
        if(type.type[cnt] == NULL) break;
        if(!strcmp(type.type[cnt], "lv_label")) return true;
    }
    return false;
}

/**
 * Check if this is really a list image or another object.
 * @param list_image List image
 */
static bool lv_list_is_list_img(lv_obj_t * list_img)
{
    lv_obj_type_t type;

    lv_obj_get_type(list_img, &type);
    uint8_t cnt;
    for(cnt = 0; cnt < LV_MAX_ANCESTOR_NUM; cnt++) {
        if(type.type[cnt] == NULL) break;
        if(!strcmp(type.type[cnt], "lv_img")) return true;
    }
    return false;
}

/**
 * Create a list objects for ZX53 Pocket Router
 */
lv_obj_t * lv_list_create_for_PR(lv_obj_t * par, const lv_obj_t * copy)
{
    LV_LOG_TRACE("lv_list_create_for_PR started");

    /*Create the ancestor basic object*/
    lv_obj_t * new_list = lv_page_create(par, copy);
    lv_mem_assert(new_list);
    if(new_list == NULL) return NULL;

    if(ancestor_page_signal == NULL) ancestor_page_signal = lv_obj_get_signal_cb(new_list);

    lv_list_ext_t * ext = lv_obj_allocate_ext_attr(new_list, sizeof(lv_list_ext_t));
    lv_mem_assert(ext);
    if(ext == NULL) return NULL;

    ext->style_img                        = NULL;
    ext->style_img2                       = NULL;//++
    ext->styles_btn[LV_BTN_STATE_REL]     = &lv_style_btn_rel;
    ext->styles_btn[LV_BTN_STATE_PR]      = &lv_style_btn_pr;
    ext->styles_btn[LV_BTN_STATE_TGL_REL] = &lv_style_btn_tgl_rel;
    ext->styles_btn[LV_BTN_STATE_TGL_PR]  = &lv_style_btn_tgl_pr;
    ext->styles_btn[LV_BTN_STATE_INA]     = &lv_style_btn_ina;
    ext->single_mode                      = false;
    ext->size                             = 0;

#if LV_USE_GROUP
    ext->last_sel     = NULL;
    ext->selected_btn = NULL;
#endif

    lv_obj_set_signal_cb(new_list, lv_list_signal);

    /*Init the new list object*/
    if(copy == NULL) {
        lv_page_set_anim_time(new_list, LV_LIST_DEF_ANIM_TIME);
        lv_page_set_scrl_fit2(new_list, LV_FIT_FLOOD, LV_FIT_TIGHT);
        lv_obj_set_size(new_list, 2 * LV_DPI, 3 * LV_DPI);
        lv_page_set_scrl_layout(new_list, LV_LIST_LAYOUT_DEF);
        lv_list_set_sb_mode(new_list, LV_SB_MODE_HIDE);

        /*Set the default styles*/
        lv_theme_t * th = lv_theme_get_current();
        if(th) {
            lv_list_set_style(new_list, LV_LIST_STYLE_BG, th->style.list.bg);
            lv_list_set_style(new_list, LV_LIST_STYLE_SCRL, th->style.list.scrl);
            lv_list_set_style(new_list, LV_LIST_STYLE_SB, th->style.list.sb);
            lv_list_set_style(new_list, LV_LIST_STYLE_BTN_REL, th->style.list.btn.rel);
            lv_list_set_style(new_list, LV_LIST_STYLE_BTN_PR, th->style.list.btn.pr);
            lv_list_set_style(new_list, LV_LIST_STYLE_BTN_TGL_REL, th->style.list.btn.tgl_rel);
            lv_list_set_style(new_list, LV_LIST_STYLE_BTN_TGL_PR, th->style.list.btn.tgl_pr);
            lv_list_set_style(new_list, LV_LIST_STYLE_BTN_INA, th->style.list.btn.ina);
        } else {
            lv_list_set_style(new_list, LV_LIST_STYLE_BG, &lv_style_transp_fit);
            lv_list_set_style(new_list, LV_LIST_STYLE_SCRL, &lv_style_pretty);
        }
    } else {
        lv_list_ext_t * copy_ext = lv_obj_get_ext_attr(copy);

        lv_obj_t * copy_btn = lv_list_get_next_btn(copy, NULL);
        while(copy_btn) {
            const void * img_src = NULL;
#if USE_LV_IMG
            lv_obj_t * copy_img = lv_list_get_btn_img(copy_btn);
            if(copy_img) img_src = lv_img_get_src(copy_img);
#endif
            //lv_list_add(new_list, img_src, lv_list_get_btn_text(copy_btn), lv_btn_get_action(copy_btn, LV_BTN_ACTION_CLICK));
            lv_list_add_btn(new_list, img_src, lv_list_get_btn_text(copy_btn));
            // new_btn = lv_btn_create(new_list, copy_btn);
            copy_btn = lv_list_get_next_btn(copy, copy_btn);
        }

        lv_list_set_style(new_list, LV_LIST_STYLE_BTN_REL, copy_ext->styles_btn[LV_BTN_STATE_REL]);
        lv_list_set_style(new_list, LV_LIST_STYLE_BTN_PR, copy_ext->styles_btn[LV_BTN_STATE_PR]);
        lv_list_set_style(new_list, LV_LIST_STYLE_BTN_TGL_REL, copy_ext->styles_btn[LV_BTN_STATE_TGL_REL]);
        lv_list_set_style(new_list, LV_LIST_STYLE_BTN_TGL_PR, copy_ext->styles_btn[LV_BTN_STATE_TGL_REL]);
        lv_list_set_style(new_list, LV_LIST_STYLE_BTN_INA, copy_ext->styles_btn[LV_BTN_STATE_INA]);

        /*Refresh the style with new signal function*/
        lv_obj_refresh_style(new_list);
    }

    lv_list_set_style(new_list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(new_list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    LV_LOG_INFO("lv_list_create_for_PR created");

    return new_list;
}

/**
 * Add a list element to the list for ZX53 Pocket Router
 */

// [ZX53] customized list start
lv_obj_t * lv_list_add_for_PR(lv_obj_t * list, const void * img_src,
        const void * img_src2, const char * txt,
        lv_event_cb_t list_release_action, lv_list_selector_t selector_info,
        int item_id)
{
    lv_style_t * style = lv_obj_get_style(list);
    lv_list_ext_t * ext = lv_obj_get_ext_attr(list);

    /*Create a list element with the image an the text*/
    lv_obj_t * liste;
    liste = lv_btn_create(list, NULL);

    /*Save the original signal function because it will be required in `lv_list_btn_signal`*/
    if(ancestor_btn_signal == NULL) ancestor_btn_signal = lv_obj_get_signal_cb(liste);

    //lv_btn_set_action(liste, LV_BTN_ACTION_CLICK, list_release_action);
    lv_obj_set_event_cb(liste, list_release_action);
    //lv_btn_set_action(liste, LV_BTN_ACTION_LONG_PR, long_press_list_release_action);//for long press

    lv_page_glue_obj(liste, true);
    lv_btn_set_layout(liste, LV_LAYOUT_ROW_M);
    lv_obj_set_protect(liste, LV_PROTECT_PRESS_LOST);
    lv_obj_set_signal_cb(liste, lv_list_btn_signal);

    /*Create styles for the buttons*/
    static lv_style_t style_btn_rel;
    static lv_style_t style_btn_pr;

    lv_style_copy(&style_btn_rel, &lv_style_btn_rel);
    style_btn_rel.body.main_color = LV_COLOR_WHITE;
    style_btn_rel.body.grad_color = LV_COLOR_WHITE;
    style_btn_rel.body.border.color = LV_COLOR_SILVER;
    style_btn_rel.body.border.width = 2;
    style_btn_rel.body.border.part = LV_BORDER_BOTTOM;
    style_btn_rel.body.padding.inner = LIST_OBJ_PADDING;
    //style_btn_rel.body.padding.hor = LIST_OBJ_PADDING;
    //style_btn_rel.body.padding.ver = LIST_OBJ_PADDING;
    style_btn_rel.body.radius = 0;
    style_btn_rel.text.color = lv_color_hex(0x4c4c4c);
    style_btn_rel.text.letter_space = 1;

#if defined(HIGH_RESOLUTION)
    style_btn_rel.text.font = get_font(font_w_bold, font_h_40);
#else
    style_btn_rel.text.font = get_font(font_w_regular, font_h_22);
#endif

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.body.border.color = LV_COLOR_ORANGE;
    style_btn_pr.text.color = LV_COLOR_ORANGE;
    style_btn_pr.body.border.width = 3;
    style_btn_pr.body.padding.inner = LIST_OBJ_PADDING;
    //style_btn_pr.body.padding.hor = LIST_OBJ_PADDING;
    //style_btn_pr.body.padding.ver = LIST_OBJ_PADDING;
    style_btn_pr.body.border.opa = LV_OPA_COVER;
    style_btn_pr.image.color = LV_COLOR_ORANGE;
    style_btn_pr.image.intense = LV_OPA_COVER;
    /*Set the new button styles*/
    lv_btn_set_style(liste, LV_BTN_STYLE_REL, &style_btn_rel);
    lv_btn_set_style(liste, LV_BTN_STYLE_PR, &style_btn_pr);

    lv_obj_set_size(liste, LISTE_X, LISTE_Y);

    //Can check list element by API lv_obj_get_free_num(obj)
    //when in called back function list_release_action
    lv_obj_set_user_data(liste, item_id);

    lv_obj_t * img = NULL;
    /*Style of the list element images on buttons*/
    if (img_src) {
        img = lv_img_create(liste, NULL);
        lv_img_set_src(img, img_src);
        lv_obj_set_style(img, ext->style_img);
        lv_obj_set_click(img, false);
        if(img_signal == NULL) img_signal = lv_obj_get_signal_cb(img);
    }
    /*Style of the list element check boxes on buttons on LEFT*/
    if (selector_info.type == SELECTOR_TYPE_CB && selector_info.pos == LEFT) {
        static lv_style_t cb1_styles[_LV_BTN_STATE_NUM];
        lv_style_copy(&cb1_styles[LV_BTN_STATE_REL], &lv_style_plain);
        cb1_styles[LV_BTN_STATE_REL].body.radius = LV_DPI / 20;
        cb1_styles[LV_BTN_STATE_REL].body.border.width = 1;
        cb1_styles[LV_BTN_STATE_REL].body.border.color = LV_COLOR_GRAY;
        cb1_styles[LV_BTN_STATE_REL].body.main_color = LV_COLOR_WHITE;
        cb1_styles[LV_BTN_STATE_REL].body.grad_color = LV_COLOR_SILVER;

        lv_style_copy(&cb1_styles[LV_BTN_STATE_PR], &cb1_styles[LV_BTN_STATE_REL]);
        cb1_styles[LV_BTN_STATE_PR].body.main_color = LV_COLOR_SILVER;
        cb1_styles[LV_BTN_STATE_PR].body.grad_color = LV_COLOR_GRAY;

        lv_style_copy(&cb1_styles[LV_BTN_STATE_TGL_REL], &cb1_styles[LV_BTN_STATE_REL]);
        cb1_styles[LV_BTN_STATE_TGL_REL].body.border.width = 4;
        cb1_styles[LV_BTN_STATE_TGL_REL].body.border.color = LV_COLOR_WHITE;
        cb1_styles[LV_BTN_STATE_TGL_REL].body.border.opa = LV_OPA_70;
        cb1_styles[LV_BTN_STATE_TGL_REL].body.main_color = LV_COLOR_ORANGE;
        cb1_styles[LV_BTN_STATE_TGL_REL].body.grad_color = LV_COLOR_ORANGE;

        lv_style_copy(&cb1_styles[LV_BTN_STATE_TGL_PR], &cb1_styles[LV_BTN_STATE_TGL_REL]);
        cb1_styles[LV_BTN_STATE_TGL_PR].body.border.color = LV_COLOR_SILVER;
        cb1_styles[LV_BTN_STATE_TGL_PR].body.border.opa = LV_OPA_70;
        cb1_styles[LV_BTN_STATE_TGL_PR].body.main_color = LV_COLOR_ORANGE;
        cb1_styles[LV_BTN_STATE_TGL_PR].body.grad_color = LV_COLOR_ORANGE;

        lv_style_copy(&cb1_styles[LV_BTN_STATE_INA], &cb1_styles[LV_BTN_STATE_TGL_REL]);
        cb1_styles[LV_BTN_STATE_INA].body.border.width = 1;
        cb1_styles[LV_BTN_STATE_INA].body.border.color = LV_COLOR_GRAY;
        cb1_styles[LV_BTN_STATE_INA].body.main_color = LV_COLOR_SILVER;
        cb1_styles[LV_BTN_STATE_INA].body.grad_color = LV_COLOR_SILVER;

        lv_obj_t * cb1 = lv_cb_create(liste, NULL);
        lv_cb_set_text(cb1, "");
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_REL, &cb1_styles[LV_BTN_STATE_REL]);
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_PR, &cb1_styles[LV_BTN_STATE_PR]);
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_TGL_REL, &cb1_styles[LV_BTN_STATE_TGL_REL]);
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_TGL_PR, &cb1_styles[LV_BTN_STATE_TGL_PR]);
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_INA, &cb1_styles[LV_BTN_STATE_INA]);
        lv_obj_set_event_cb(cb1, selector_info.selector_release_action);
        if (selector_info.enabled) {
            lv_cb_set_checked(cb1, selector_info.enabled);
        }
        //Can check check box objects by API lv_obj_get_free_num(obj)
        //when in called back function selector_release_action
        lv_obj_set_user_data(cb1, item_id);
        //record check box status
        if (selector_info.state_updator != NULL) selector_info.state_updator(cb1, LV_EVENT_PRESSED);
    }
    /*Style of the list element text on buttons*/
    if (txt != NULL) {
        lv_obj_t * label = lv_label_create(liste, NULL);
        lv_label_set_text(label, txt);
        lv_obj_set_click(label, false);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SROLL);
        //modify the text lv_obj_set_width
        if (img_src2 == NULL && selector_info.type == SELECTOR_TYPE_DEFAULT) {
            lv_obj_set_width(label, (liste->coords.x2) - (label->coords.x1));
        } else {
            lv_obj_set_width(label, (((liste->coords.x2) / 1.21) - (label->coords.x1)));

        }
        if(label_signal == NULL) label_signal = lv_obj_get_signal_cb(label);
    }
    /*Style of the list element check boxes on buttons on RIGHT*/
    if (selector_info.type == SELECTOR_TYPE_CB && selector_info.pos == RIGHT) {
        static lv_style_t cb1_styles[_LV_BTN_STATE_NUM];
        lv_style_copy(&cb1_styles[LV_BTN_STATE_REL], &lv_style_plain);
        cb1_styles[LV_BTN_STATE_REL].body.radius = LV_DPI / 20;
        cb1_styles[LV_BTN_STATE_REL].body.border.width = 1;
        cb1_styles[LV_BTN_STATE_REL].body.border.color = LV_COLOR_GRAY;
        cb1_styles[LV_BTN_STATE_REL].body.main_color = LV_COLOR_WHITE;
        cb1_styles[LV_BTN_STATE_REL].body.grad_color = LV_COLOR_SILVER;

        lv_style_copy(&cb1_styles[LV_BTN_STATE_PR], &cb1_styles[LV_BTN_STATE_REL]);
        cb1_styles[LV_BTN_STATE_PR].body.main_color = LV_COLOR_SILVER;
        cb1_styles[LV_BTN_STATE_PR].body.grad_color = LV_COLOR_GRAY;

        lv_style_copy(&cb1_styles[LV_BTN_STATE_TGL_REL], &cb1_styles[LV_BTN_STATE_REL]);
        cb1_styles[LV_BTN_STATE_TGL_REL].body.border.width = 4;
        cb1_styles[LV_BTN_STATE_TGL_REL].body.border.color = LV_COLOR_WHITE;
        cb1_styles[LV_BTN_STATE_TGL_REL].body.border.opa = LV_OPA_70;
        cb1_styles[LV_BTN_STATE_TGL_REL].body.main_color = LV_COLOR_ORANGE;
        cb1_styles[LV_BTN_STATE_TGL_REL].body.grad_color = LV_COLOR_ORANGE;

        lv_style_copy(&cb1_styles[LV_BTN_STATE_TGL_PR], &cb1_styles[LV_BTN_STATE_TGL_REL]);
        cb1_styles[LV_BTN_STATE_TGL_PR].body.border.color = LV_COLOR_SILVER;
        cb1_styles[LV_BTN_STATE_TGL_PR].body.border.opa = LV_OPA_70;
        cb1_styles[LV_BTN_STATE_TGL_PR].body.main_color = LV_COLOR_ORANGE;
        cb1_styles[LV_BTN_STATE_TGL_PR].body.grad_color = LV_COLOR_ORANGE;

        lv_style_copy(&cb1_styles[LV_BTN_STATE_INA], &cb1_styles[LV_BTN_STATE_TGL_REL]);
        cb1_styles[LV_BTN_STATE_INA].body.border.width = 1;
        cb1_styles[LV_BTN_STATE_INA].body.border.color = LV_COLOR_GRAY;
        cb1_styles[LV_BTN_STATE_INA].body.main_color = LV_COLOR_SILVER;
        cb1_styles[LV_BTN_STATE_INA].body.grad_color = LV_COLOR_SILVER;

        lv_obj_t * cb1 = lv_cb_create(liste, NULL);
        lv_cb_set_text(cb1, "");
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_REL, &cb1_styles[LV_BTN_STATE_REL]);
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_PR, &cb1_styles[LV_BTN_STATE_PR]);
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_TGL_REL, &cb1_styles[LV_BTN_STATE_TGL_REL]);
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_TGL_PR, &cb1_styles[LV_BTN_STATE_TGL_PR]);
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_INA, &cb1_styles[LV_BTN_STATE_INA]);
        lv_obj_set_event_cb(cb1, selector_info.selector_release_action);
        if (selector_info.enabled) {
            lv_cb_set_checked(cb1, selector_info.enabled);
        }
        //Can check check box objects by API lv_obj_get_free_num(obj)
        //when in called back function selector_release_action
        lv_obj_set_user_data(cb1, item_id);
        //record check box status
        if (selector_info.state_updator != NULL) selector_info.state_updator(cb1, LV_EVENT_PRESSED);
    }
    /*Style of the list element images2 on buttons*/
    lv_obj_t * img2 = NULL;
    if (img_src2) {
        img2 = lv_img_create(liste, NULL);
        lv_img_set_src(img2, img_src2);
        lv_obj_set_style(img2, ext->style_img2);
        lv_obj_set_click(img2, false);
        if(img_signal == NULL) img_signal = lv_obj_get_signal_cb(img2);
    }

    if (selector_info.type == SELECTOR_TYPE_SW) {
        /*Create styles for the switch*/
        static lv_style_t bg_style;
        static lv_style_t indic_style;
        static lv_style_t knob_on_style;
        static lv_style_t knob_off_style;
        lv_style_copy(&bg_style, &lv_style_pretty);
        bg_style.body.main_color = LV_COLOR_SILVER;
        bg_style.body.grad_color = LV_COLOR_SILVER;
        bg_style.body.border.color = LV_COLOR_GREYISH_BROWN;
        bg_style.body.border.width = 2;

        lv_style_copy(&indic_style, &lv_style_pretty_color);
        indic_style.body.radius = LV_RADIUS_CIRCLE;
        indic_style.body.main_color = LV_COLOR_ORANGE;
        indic_style.body.grad_color = LV_COLOR_ORANGE;
        //indic_style.body.padding.hor = 0;
        //indic_style.body.padding.ver = 0;

        lv_style_copy(&knob_off_style, &lv_style_pretty);
        knob_off_style.body.radius = LV_RADIUS_CIRCLE;
        knob_off_style.body.main_color = LV_COLOR_GREYISH_BROWN;
        knob_off_style.body.grad_color = LV_COLOR_GREYISH_BROWN;
        knob_off_style.body.shadow.width = 0;
        knob_off_style.body.shadow.type = LV_SHADOW_BOTTOM;

        lv_style_copy(&knob_on_style, &lv_style_pretty_color);
        knob_on_style.body.radius = LV_RADIUS_CIRCLE;
        knob_on_style.body.main_color = LV_COLOR_GREYISH_BROWN;
        knob_on_style.body.grad_color = LV_COLOR_GREYISH_BROWN;
        knob_on_style.body.shadow.width = 0;
        knob_on_style.body.shadow.type = LV_SHADOW_BOTTOM;

        /*Create a switch and apply the styles*/
        lv_obj_t *sw1 = lv_sw_create(liste, NULL);
        lv_sw_set_style(sw1, LV_SW_STYLE_BG, &bg_style);
        lv_sw_set_style(sw1, LV_SW_STYLE_INDIC, &indic_style);
        lv_sw_set_style(sw1, LV_SW_STYLE_KNOB_ON, &knob_on_style);
        lv_sw_set_style(sw1, LV_SW_STYLE_KNOB_OFF, &knob_off_style);
        lv_obj_set_event_cb(sw1, selector_info.selector_release_action);

        if (selector_info.enabled) {
            lv_sw_on(sw1, LV_ANIM_ON);
        } else {
            lv_sw_off(sw1, LV_ANIM_ON);
        }
        //Can check switch objects by API lv_obj_get_free_num(obj)
        //when in called back function sw_release_action
        lv_obj_set_user_data(sw1, item_id);
    }
    return liste;
}
// [ZX53] customized list end

// [ZX53] Start Add cb and sw support in lv_list
lv_obj_t* list_content_action(lv_obj_t * list, const void * img_src,
          const void * img_src2, const char * txt,
          lv_event_cb_t list_release_action, lv_list_selector_t selector_info,
          int item_id)
{
    return lv_list_add_for_PR(list, img_src, img_src2,
            txt, list_release_action, selector_info, item_id);
}
// [ZX53] End Add cb and sw support in lv_list

// [ZX53] customized list start
lv_obj_t * lv_list_add_for_combo(lv_obj_t * list, const void * img_src,
        const void * img_src2, const char * txt,
        lv_event_cb_t list_release_action, lv_list_selector_t selector_info,
        int item_id)
{
    char* data_info;
    lv_obj_t * label;
    lv_obj_t * label2;
    lv_style_t * style = lv_obj_get_style(list);
    lv_list_ext_t * ext = lv_obj_get_ext_attr(list);

    /*Create a list element with the image an the text*/
    lv_obj_t * liste;
    liste = lv_btn_create(list, NULL);

    /*Save the original signal function because it will be required in `lv_list_btn_signal`*/
    if(ancestor_btn_signal == NULL) ancestor_btn_signal = lv_obj_get_signal_cb(liste);

    //lv_btn_set_action(liste, LV_BTN_ACTION_CLICK, list_release_action);
    lv_obj_set_event_cb(liste, list_release_action);
    //lv_btn_set_action(liste, LV_BTN_ACTION_LONG_PR, selector_info.long_press_list_release_action);//for long press

    lv_page_glue_obj(liste, true);
    lv_btn_set_layout(liste, LV_LAYOUT_ROW_M);
    lv_obj_set_protect(liste, LV_PROTECT_PRESS_LOST);
    lv_obj_set_signal_cb(liste, lv_list_btn_signal);

    /*Create styles for the buttons*/
    static lv_style_t style_btn_rel;
    static lv_style_t style_btn_pr;

    lv_style_copy(&style_btn_rel, &lv_style_btn_rel);
    style_btn_rel.body.main_color = LV_COLOR_WHITE;
    style_btn_rel.body.grad_color = LV_COLOR_WHITE;
    style_btn_rel.body.border.color = LV_COLOR_SILVER;
    style_btn_rel.body.border.width = 2;
    style_btn_rel.body.border.part = LV_BORDER_BOTTOM;
    style_btn_rel.body.padding.inner = LIST_OBJ_PADDING;
    //style_btn_rel.body.padding.hor = LIST_OBJ_PADDING;
    //style_btn_rel.body.padding.ver = LIST_OBJ_PADDING;
    style_btn_rel.body.radius = 0;
    style_btn_rel.text.color = lv_color_hex(0x4c4c4c);

#if defined(HIGH_RESOLUTION)
    style_btn_rel.text.font = get_font(font_w_bold, font_h_40);
#else
    style_btn_rel.text.font = get_font(font_w_regular, font_h_22);
#endif

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.body.border.color = LV_COLOR_ORANGE;
    style_btn_pr.text.color = LV_COLOR_ORANGE;
    style_btn_pr.body.border.width = 3;
    style_btn_pr.body.padding.inner = LIST_OBJ_PADDING;
    //style_btn_pr.body.padding.hor = LIST_OBJ_PADDING;
    //style_btn_pr.body.padding.ver = LIST_OBJ_PADDING;
    style_btn_pr.body.border.opa = LV_OPA_COVER;
    style_btn_pr.image.color = LV_COLOR_ORANGE;
    style_btn_pr.image.intense = LV_OPA_COVER;
    /*Set the new button styles*/
    lv_btn_set_style(liste, LV_BTN_STYLE_REL, &style_btn_rel);
    lv_btn_set_style(liste, LV_BTN_STYLE_PR, &style_btn_pr);

    lv_obj_set_size(liste, LISTE_X, LISTE_Y);

    //Can check list element by API lv_obj_get_free_num(obj)
    //when in called back function list_release_action
    lv_obj_set_user_data(liste, item_id);

    lv_obj_t * img = NULL;
    /*Style of the list element images on buttons*/
    if (img_src) {
        img = lv_img_create(liste, NULL);
        lv_img_set_src(img, img_src);
        lv_obj_set_style(img, ext->style_img);
        lv_obj_set_click(img, false);
        if(img_signal == NULL) img_signal = lv_obj_get_signal_cb(img);
    }
    /*Style of the list element check boxes on buttons on LEFT*/
    if (selector_info.type == SELECTOR_TYPE_CB && selector_info.pos == LEFT) {
        static lv_style_t cb1_styles[_LV_BTN_STATE_NUM];
        lv_style_copy(&cb1_styles[LV_BTN_STATE_REL], &lv_style_plain);
        cb1_styles[LV_BTN_STATE_REL].body.radius = LV_DPI / 20;
        cb1_styles[LV_BTN_STATE_REL].body.border.width = 1;
        cb1_styles[LV_BTN_STATE_REL].body.border.color = LV_COLOR_GRAY;
        cb1_styles[LV_BTN_STATE_REL].body.main_color = LV_COLOR_WHITE;
        cb1_styles[LV_BTN_STATE_REL].body.grad_color = LV_COLOR_SILVER;

        lv_style_copy(&cb1_styles[LV_BTN_STATE_PR], &cb1_styles[LV_BTN_STATE_REL]);
        cb1_styles[LV_BTN_STATE_PR].body.main_color = LV_COLOR_SILVER;
        cb1_styles[LV_BTN_STATE_PR].body.grad_color = LV_COLOR_GRAY;

        lv_style_copy(&cb1_styles[LV_BTN_STATE_TGL_REL], &cb1_styles[LV_BTN_STATE_REL]);
        cb1_styles[LV_BTN_STATE_TGL_REL].body.border.width = 4;
        cb1_styles[LV_BTN_STATE_TGL_REL].body.border.color = LV_COLOR_WHITE;
        cb1_styles[LV_BTN_STATE_TGL_REL].body.border.opa = LV_OPA_70;
        cb1_styles[LV_BTN_STATE_TGL_REL].body.main_color = LV_COLOR_ORANGE;
        cb1_styles[LV_BTN_STATE_TGL_REL].body.grad_color = LV_COLOR_ORANGE;

        lv_style_copy(&cb1_styles[LV_BTN_STATE_TGL_PR], &cb1_styles[LV_BTN_STATE_TGL_REL]);
        cb1_styles[LV_BTN_STATE_TGL_PR].body.border.color = LV_COLOR_SILVER;
        cb1_styles[LV_BTN_STATE_TGL_PR].body.border.opa = LV_OPA_70;
        cb1_styles[LV_BTN_STATE_TGL_PR].body.main_color = LV_COLOR_ORANGE;
        cb1_styles[LV_BTN_STATE_TGL_PR].body.grad_color = LV_COLOR_ORANGE;

        lv_style_copy(&cb1_styles[LV_BTN_STATE_INA], &cb1_styles[LV_BTN_STATE_TGL_REL]);
        cb1_styles[LV_BTN_STATE_INA].body.border.width = 1;
        cb1_styles[LV_BTN_STATE_INA].body.border.color = LV_COLOR_GRAY;
        cb1_styles[LV_BTN_STATE_INA].body.main_color = LV_COLOR_SILVER;
        cb1_styles[LV_BTN_STATE_INA].body.grad_color = LV_COLOR_SILVER;

        lv_obj_t * cb1 = lv_cb_create(liste, NULL);
        lv_cb_set_text(cb1, "");
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_REL, &cb1_styles[LV_BTN_STATE_REL]);
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_PR, &cb1_styles[LV_BTN_STATE_PR]);
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_TGL_REL, &cb1_styles[LV_BTN_STATE_TGL_REL]);
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_TGL_PR, &cb1_styles[LV_BTN_STATE_TGL_PR]);
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_INA, &cb1_styles[LV_BTN_STATE_INA]);
        lv_obj_set_event_cb(cb1, selector_info.selector_release_action);
        if (selector_info.enabled) {
            lv_cb_set_checked(cb1, selector_info.enabled);
        }
        //Can check check box objects by API lv_obj_get_free_num(obj)
        //when in called back function selector_release_action
        lv_obj_set_user_data(cb1, item_id);
        //record check box status
        if (selector_info.state_updator != NULL) selector_info.state_updator(cb1, LV_EVENT_PRESSED);
    }
    /*Style of the list element text on buttons*/
    if (txt != NULL) {
        if (selector_info.type == SELECTOR_TYPE_COMBO_TYPE1) {
            char* data_info1 = txt;
            char* data_info2 = "\n";
            char* data_info3 = selector_info.metadata;
            data_info = lv_mem_alloc(strlen(data_info1) + strlen(data_info2) + strlen(data_info3) + 1);
            sprintf(data_info, "%s%s%s", data_info1, data_info2, data_info3);
        } else {
            data_info = txt;
        }
        label = lv_label_create(liste, NULL);
        lv_label_set_text(label, data_info);
        lv_obj_set_click(label, false);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SROLL);
        //modify the text lv_obj_set_width
        if (img_src2 == NULL && selector_info.type == SELECTOR_TYPE_DEFAULT) {
            lv_obj_set_width(label, (liste->coords.x2) - (label->coords.x1));
        } else {
            lv_obj_set_width(label, (((liste->coords.x2) / 1.18) - (label->coords.x1)));
        }
        if(label_signal == NULL) label_signal = lv_obj_get_signal_cb(label);
        lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
    }
    if (selector_info.type == SELECTOR_TYPE_COMBO_TYPE2) {
        label2 = lv_label_create(label, NULL);
        lv_label_set_text(label2, selector_info.metadata);
        lv_obj_set_click(label2, false);
        lv_label_set_long_mode(label2, LV_LABEL_LONG_SROLL);
        lv_obj_align(label2, NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);
    }
    /*Style of the list element check boxes on buttons on RIGHT*/
    if (selector_info.type == SELECTOR_TYPE_CB && selector_info.pos == RIGHT) {
        static lv_style_t cb1_styles[_LV_BTN_STATE_NUM];
        lv_style_copy(&cb1_styles[LV_BTN_STATE_REL], &lv_style_plain);
        cb1_styles[LV_BTN_STATE_REL].body.radius = LV_DPI / 20;
        cb1_styles[LV_BTN_STATE_REL].body.border.width = 1;
        cb1_styles[LV_BTN_STATE_REL].body.border.color = LV_COLOR_GRAY;
        cb1_styles[LV_BTN_STATE_REL].body.main_color = LV_COLOR_WHITE;
        cb1_styles[LV_BTN_STATE_REL].body.grad_color = LV_COLOR_SILVER;

        lv_style_copy(&cb1_styles[LV_BTN_STATE_PR], &cb1_styles[LV_BTN_STATE_REL]);
        cb1_styles[LV_BTN_STATE_PR].body.main_color = LV_COLOR_SILVER;
        cb1_styles[LV_BTN_STATE_PR].body.grad_color = LV_COLOR_GRAY;

        lv_style_copy(&cb1_styles[LV_BTN_STATE_TGL_REL], &cb1_styles[LV_BTN_STATE_REL]);
        cb1_styles[LV_BTN_STATE_TGL_REL].body.border.width = 4;
        cb1_styles[LV_BTN_STATE_TGL_REL].body.border.color = LV_COLOR_WHITE;
        cb1_styles[LV_BTN_STATE_TGL_REL].body.border.opa = LV_OPA_70;
        cb1_styles[LV_BTN_STATE_TGL_REL].body.main_color = LV_COLOR_ORANGE;
        cb1_styles[LV_BTN_STATE_TGL_REL].body.grad_color = LV_COLOR_ORANGE;

        lv_style_copy(&cb1_styles[LV_BTN_STATE_TGL_PR], &cb1_styles[LV_BTN_STATE_TGL_REL]);
        cb1_styles[LV_BTN_STATE_TGL_PR].body.border.color = LV_COLOR_SILVER;
        cb1_styles[LV_BTN_STATE_TGL_PR].body.border.opa = LV_OPA_70;
        cb1_styles[LV_BTN_STATE_TGL_PR].body.main_color = LV_COLOR_ORANGE;
        cb1_styles[LV_BTN_STATE_TGL_PR].body.grad_color = LV_COLOR_ORANGE;

        lv_style_copy(&cb1_styles[LV_BTN_STATE_INA], &cb1_styles[LV_BTN_STATE_TGL_REL]);
        cb1_styles[LV_BTN_STATE_INA].body.border.width = 1;
        cb1_styles[LV_BTN_STATE_INA].body.border.color = LV_COLOR_GRAY;
        cb1_styles[LV_BTN_STATE_INA].body.main_color = LV_COLOR_SILVER;
        cb1_styles[LV_BTN_STATE_INA].body.grad_color = LV_COLOR_SILVER;

        lv_obj_t * cb1 = lv_cb_create(liste, NULL);
        lv_cb_set_text(cb1, "");
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_REL, &cb1_styles[LV_BTN_STATE_REL]);
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_PR, &cb1_styles[LV_BTN_STATE_PR]);
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_TGL_REL, &cb1_styles[LV_BTN_STATE_TGL_REL]);
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_TGL_PR, &cb1_styles[LV_BTN_STATE_TGL_PR]);
        lv_cb_set_style(cb1, LV_CB_STYLE_BOX_INA, &cb1_styles[LV_BTN_STATE_INA]);
        lv_obj_set_event_cb(cb1, selector_info.selector_release_action);
        if (selector_info.enabled) {
            lv_cb_set_checked(cb1, selector_info.enabled);
        }
        //Can check check box objects by API lv_obj_get_free_num(obj)
        //when in called back function selector_release_action
        lv_obj_set_user_data(cb1, item_id);
        //record check box status
        if (selector_info.state_updator != NULL) selector_info.state_updator(cb1, LV_EVENT_PRESSED);
    }
    /*Style of the list element images2 on buttons*/
    lv_obj_t * img2 = NULL;
    if (img_src2) {
        img2 = lv_img_create(liste, NULL);
        lv_img_set_src(img2, img_src2);
        lv_obj_set_style(img2, ext->style_img2);
        lv_obj_set_click(img2, false);
        if(img_signal == NULL) img_signal = lv_obj_get_signal_cb(img2);
    }
    if (selector_info.type == SELECTOR_TYPE_SW) {
        /*Create styles for the switch*/
        static lv_style_t bg_style;
        static lv_style_t indic_style;
        static lv_style_t knob_on_style;
        static lv_style_t knob_off_style;
        lv_style_copy(&bg_style, &lv_style_pretty);
        bg_style.body.main_color = LV_COLOR_SILVER;
        bg_style.body.grad_color = LV_COLOR_SILVER;
        bg_style.body.border.color = LV_COLOR_GREYISH_BROWN;
        bg_style.body.border.width = 2;

        lv_style_copy(&indic_style, &lv_style_pretty_color);
        indic_style.body.radius = LV_RADIUS_CIRCLE;
        indic_style.body.main_color = LV_COLOR_ORANGE;
        indic_style.body.grad_color = LV_COLOR_ORANGE;
        //indic_style.body.padding.hor = 0;
        //indic_style.body.padding.ver = 0;

        lv_style_copy(&knob_off_style, &lv_style_pretty);
        knob_off_style.body.radius = LV_RADIUS_CIRCLE;
        knob_off_style.body.main_color = LV_COLOR_GREYISH_BROWN;
        knob_off_style.body.grad_color = LV_COLOR_GREYISH_BROWN;
        knob_off_style.body.shadow.width = 0;
        knob_off_style.body.shadow.type = LV_SHADOW_BOTTOM;

        lv_style_copy(&knob_on_style, &lv_style_pretty_color);
        knob_on_style.body.radius = LV_RADIUS_CIRCLE;
        knob_on_style.body.main_color = LV_COLOR_GREYISH_BROWN;
        knob_on_style.body.grad_color = LV_COLOR_GREYISH_BROWN;
        knob_on_style.body.shadow.width = 0;
        knob_on_style.body.shadow.type = LV_SHADOW_BOTTOM;

        /*Create a switch and apply the styles*/
        lv_obj_t *sw1 = lv_sw_create(liste, NULL);
        lv_sw_set_style(sw1, LV_SW_STYLE_BG, &bg_style);
        lv_sw_set_style(sw1, LV_SW_STYLE_INDIC, &indic_style);
        lv_sw_set_style(sw1, LV_SW_STYLE_KNOB_ON, &knob_on_style);
        lv_sw_set_style(sw1, LV_SW_STYLE_KNOB_OFF, &knob_off_style);
        lv_obj_set_event_cb(sw1, selector_info.selector_release_action);

        if (selector_info.enabled) {
            lv_sw_on(sw1, LV_ANIM_ON);
        } else {
            lv_sw_off(sw1, LV_ANIM_ON);
        }
        //Can check switch objects by API lv_obj_get_free_num(obj)
        //when in called back function sw_release_action
        lv_obj_set_user_data(sw1, item_id);
    }
    if (selector_info.type == SELECTOR_TYPE_COMBO_TYPE1) {
        if (data_info != NULL) {
            lv_mem_free(data_info);
        }
    }
    return liste;
}
// [ZX53] customized list end
#endif
