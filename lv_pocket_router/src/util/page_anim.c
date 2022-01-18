#include "page_anim.h"
#include <stdio.h>

#include "lv_pocket_router/src/util/util.h"
#define PAGE_ANIM_SHIFT     35
#define PAGE_ANIM_BAR_NUM   4

//bar1 ~ bar4 were set from top to bottom
lv_obj_t * bar1;
lv_obj_t * bar2;
lv_obj_t * bar3;
lv_obj_t * bar4;
lv_obj_t ** page_anim_bar[]={ &bar1, &bar2, &bar3, &bar4 };

void page_anim_cleanup(){
    int i;
    for(i = 0; i < PAGE_ANIM_BAR_NUM; i ++) {
        if (*page_anim_bar[i] != NULL) {
            lv_obj_del(*page_anim_bar[i]);
            *page_anim_bar[i] = NULL;
            lv_anim_del(*page_anim_bar[i], NULL);
        }
    }
}

//to create page to page animation, bar1 ~ bar4 were set from top to bottom
void page_anim_create() {
    page_anim_cleanup();

    bar1 = lv_img_create(lv_scr_act(), NULL);
    lv_obj_set_size(bar1, LV_HOR_RES_MAX, LV_VER_RES_MAX / 4);
    lv_img_set_src(bar1, &image_page_to_page);
    bar2 = lv_img_create(lv_scr_act(), bar1);
    bar3 = lv_img_create(lv_scr_act(), bar1);
    bar4 = lv_img_create(lv_scr_act(), bar1);

    lv_obj_align(bar1, NULL, LV_ALIGN_IN_TOP_RIGHT, PAGE_ANIM_SHIFT, 0);
    lv_obj_align(bar2, NULL, LV_ALIGN_IN_TOP_LEFT, - PAGE_ANIM_SHIFT, LV_VER_RES_MAX / 4);
    lv_obj_align(bar3, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, PAGE_ANIM_SHIFT, - LV_VER_RES_MAX / 4);
    lv_obj_align(bar4, NULL, LV_ALIGN_IN_BOTTOM_LEFT, - PAGE_ANIM_SHIFT, 0);
}

//to create page to page entrance animation
void page_anim_enter(){
    page_anim_create(true);

    page_animate_impl(bar1, PAGE_ANIM_FLOAT_LEFT, ANIM_TIME, 0, false);
    page_animate_impl(bar2, PAGE_ANIM_FLOAT_RIGHT, ANIM_TIME, 0, false);
    page_animate_impl(bar3, PAGE_ANIM_FLOAT_LEFT, ANIM_TIME, 0, false);
    page_animate_impl(bar4, PAGE_ANIM_FLOAT_RIGHT, ANIM_TIME, 0, false);
}

//to create page to page exit animation
void page_anim_exit(){
    page_anim_create(false);

    page_animate_impl(bar1, PAGE_ANIM_FLOAT_LEFT, ANIM_TIME, ANIM_DELAY_TIME, true);
    page_animate_impl(bar2, PAGE_ANIM_FLOAT_RIGHT, ANIM_TIME, ANIM_DELAY_TIME, true);
    page_animate_impl(bar3, PAGE_ANIM_FLOAT_LEFT, ANIM_TIME, ANIM_DELAY_TIME, true);
    page_animate_impl(bar4, PAGE_ANIM_FLOAT_RIGHT, ANIM_TIME, ANIM_DELAY_TIME, true);
}

void page_animate_impl(lv_obj_t * obj, int type, uint16_t time, uint16_t delay, bool anim_out){
    lv_obj_t * par = lv_obj_get_parent(obj);
    lv_anim_t a;
    a.var = obj;
    a.time = time;
    a.act_time = (int32_t) - delay;
    a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_x;
    a.path_cb = lv_anim_path_linear;
    a.ready_cb = (lv_anim_ready_cb_t)page_anim_cleanup;
    a.playback_pause = 0;
    a.repeat_pause = 0;
    a.playback = 0;
    a.repeat = 0;
    a.user_data = NULL;
    a.end = lv_obj_get_x(obj);

    switch(type) {
        case PAGE_ANIM_FLOAT_LEFT:
            a.start = -lv_obj_get_width(obj);
            break;
        case PAGE_ANIM_FLOAT_RIGHT:
            a.start = lv_obj_get_width(par);
            break;
        case PAGE_ANIM_FLOAT_BOTTOM:
            a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
            a.start = lv_obj_get_height(par);
            a.end = lv_obj_get_y(obj);
            break;
        case PAGE_ANIM_FLOAT_TOP:
            a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
            a.start = -lv_obj_get_height(obj);
            a.end = lv_obj_get_y(obj);
            break;
    }
    //Swap start and end in case of ANIM OUT
    if(anim_out != false) {
        int32_t tmp = a.start;
        a.start = a.end;
        a.end = tmp;
    }
    lv_anim_create(&a);
}
