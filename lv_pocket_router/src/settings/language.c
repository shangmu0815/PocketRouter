#include <stdio.h>
#include <stdbool.h>

#include "lv_pocket_router/src/dashboard.h"
#include "lv_pocket_router/src/launcher.h"
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/settings/language.h"
#include "lv_pocket_router/res/values/styles.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"

enum {
    LANG_STRING_ID,
    LOCALE_ID,
    ID_COUNT
};

int lang_map[][ID_COUNT] = {
    { ID_LANG_EN, EN },
    { ID_LANG_JP, JP },
    { ID_LANG_AR, AR },
    { ID_LANG_FR, FR },
    { ID_LANG_DE, DE },
    { ID_LANG_NL, NL },
    { ID_LANG_PT, PT },
    { ID_LANG_SL, SL },
    { ID_LANG_IT, IT },
    { ID_LANG_ES, ES },
    { ID_LANG_RU, RU },
    { ID_LANG_ZH_CN, ZH_CN },
    { ID_LANG_ZH_TW, ZH_TW },
    { ID_LANG_PL, PL },
    { ID_LANG_HU, HU },
};

// mapping table to data_store item, must be same order as lang_map
char * ds_map[] = {
    DS_KEY_LANG_EN,
    DS_KEY_LANG_JP,
    DS_KEY_LANG_AR,
    DS_KEY_LANG_FR,
    DS_KEY_LANG_DE,
    DS_KEY_LANG_NL,
    DS_KEY_LANG_PT,
    DS_KEY_LANG_SL,
    DS_KEY_LANG_IT,
    DS_KEY_LANG_ES,
    DS_KEY_LANG_RU,
    DS_KEY_LANG_ZH_CN,
    DS_KEY_LANG_ZH_TW,
    DS_KEY_LANG_PL,
    DS_KEY_LANG_HU
};

#define LANG_MAX_LISTE          sizeof(lang_map) / (sizeof(int)*ID_COUNT)

int lang_type;
lv_obj_t * lang_liste_img[LANG_MAX_LISTE];
lv_obj_t * lang_liste[LANG_MAX_LISTE];


void choose_lang_ok_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    log_d("choose_lang_ok_action lang_type:%d\n", lang_type);
    if (lang_type == get_device_locale()) {
        return;
    }

    set_device_locale(lang_type);

    lv_win_close_event_cb(btn, LV_EVENT_PRESSED);
    //close all list except the first 1
    ui_cleanup();
    dashboard_create();
}

void language_btn_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    int i;
    int selected = lv_obj_get_user_data(btn);
    lang_type = lang_map[selected][LOCALE_ID];

    for (i = 0; i < LANG_MAX_LISTE; i++){
        if (ds_get_bool(ds_map[i]) == true) {
            lv_img_set_src(lang_liste_img[i], &btn_list_radio_n);
        }
    }
    lv_img_set_src(lang_liste_img[selected], &btn_list_radio_p);

    log_d("language_btn_action lang_type:%d\n",lang_type);
}

void init_language() {
    liste_style_create();
    lv_obj_t * win = modify_list_header(lv_scr_act(), get_string(ID_LANG),
            choose_lang_ok_action, lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    //add list element in order
    int language = lang_type = get_device_locale();
    log_d("language:%d\n",language);
    int i;
    for (i = 0; i < LANG_MAX_LISTE; i++) {
        //temp to add japan lang item
        /*if (i == JP) {
            lv_obj_set_user_data(list, JP);
        }else{
            lv_obj_set_user_data(list, EN);
        }*/
        if (ds_get_bool(ds_map[i]) == true) {
            lv_obj_set_user_data(list, i);
            lang_liste[i] = lv_liste_w_cbox_style(list, get_string(lang_map[i][LANG_STRING_ID]), language==lang_map[i][LOCALE_ID], language_btn_action, i);
            lang_liste_img[i] = lv_obj_get_child(lang_liste[i], NULL);
        }
    }
}
