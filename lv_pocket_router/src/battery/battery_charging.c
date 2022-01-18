#include <stdio.h>

#include "battery_charging.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/battery/battery_info.h"

static lv_style_t black_bg;
static lv_style_t title_style;
static lv_obj_t * bg = NULL;
lv_obj_t * img;
lv_obj_t * label;
int anim_cnt = 0;

const void * battery_anim_map[]={&icon_offscreen_charging_1, &icon_offscreen_charging_2, &icon_offscreen_charging_3
        , &icon_offscreen_charging_4, &icon_offscreen_charging_5, &icon_offscreen_charging_6, &icon_offscreen_charging_7
        , &icon_offscreen_charging_7, &icon_offscreen_charging_7, &icon_offscreen_charging_7, &icon_offscreen_charging_7};
int length = sizeof(battery_anim_map) / sizeof(void *);

void init_style(void){
    lv_style_copy(&black_bg, &lv_style_plain);
    black_bg.body.main_color = LV_COLOR_DARK_GRAY;
    black_bg.body.grad_color = LV_COLOR_DARK_GRAY;

    lv_style_copy(&title_style, &lv_style_plain);
    title_style.text.font = get_font(font_w_bold, font_h_26);
    title_style.text.color = LV_COLOR_WHITE_SMOKE;
    title_style.text.letter_space = 1;
}

void update_anim() {
    //update battery anim
    if (anim_cnt < length) {
        lv_img_set_src(img, battery_anim_map[anim_cnt]);
        anim_cnt++;
    } else {
        //repeat anim
        anim_cnt = 0;
    }
}

//update battery level & led light
void update_capacity() {
    //update battery level
    char data[5];
    memset(data, 0, sizeof(data));
    int battery = get_battery_info();
    sprintf(data, "%d%s", battery, "%");
    lv_label_set_text(label, data);
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 20);
}

void create_charge_mode(){
    if(bg != NULL){
        return;
    }
    log_d("create charging only mode UI");

    init_style();
    bg = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(bg, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style(bg, &black_bg);

    img = lv_img_create(bg, NULL);
    lv_obj_set_size(img, 160, 160);
    lv_img_set_src(img, &icon_offscreen_charging_1);
    lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, 0);

    label = lv_label_create(bg, NULL);
    lv_label_set_text(label, "");
    lv_obj_set_size(label, 50, 26);
    lv_obj_set_style(label, &title_style);
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 20);
    update_capacity();

    //start anim
    lv_task_create(update_anim, 110, LV_TASK_PRIO_MID, NULL);
    lv_task_create(update_capacity, 3000, LV_TASK_PRIO_MID, NULL);
}
