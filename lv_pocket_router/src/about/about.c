/*
 * about.c
 *
 *  Created on: Mar 18, 2019
 *      Author: joseph
 */

#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lvgl/lvgl.h"
#include "lv_pocket_router/src/about/device_information.h"
#include "lv_pocket_router/src/about/caution_for_using_5g.h"
#include "lv_pocket_router/src/about/about_help.h"
#include "lv_pocket_router/src/about/open_source_notice.h"
#include "lv_pocket_router/src/util/page_anim.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/liste_style.h"

enum ABOUT_IDS {
    ITEMS_ID_DEVICE_INFORMATION,
#ifdef ABOUT_CAUTION_5G
    ITEMS_ID_CAUTION_FOR_USING_5G,
#endif
    ITEMS_ID_HELP,
    ITEMS_ID_OPEN_SOURCE_NOTICE,
    ITEMS_ID_THIRD_PARTY_NOTICES,
    ABOUT_IDS_COUNT
};

lv_obj_t* btn_list[ABOUT_IDS_COUNT];

void about_list_release_action(lv_obj_t * list_btn, lv_event_cb_t event_cb)
{
    if (event_cb != LV_EVENT_CLICKED) return;

    int item_id = lv_obj_get_user_data(list_btn);
    log_d("about_list_release_action item_id:%d", item_id);
    if (item_id == ITEMS_ID_DEVICE_INFORMATION) {
        show_device_information();
#ifdef ABOUT_CAUTION_5G
    } else if (item_id == ITEMS_ID_CAUTION_FOR_USING_5G) {
        show_caution_for_using_5G();
#endif
    } else if (item_id == ITEMS_ID_HELP) {
        show_Help();
    } else if (item_id == ITEMS_ID_OPEN_SOURCE_NOTICE) {
        show_open_source_notice();
    } else if (item_id == ITEMS_ID_THIRD_PARTY_NOTICES) {
        show_third_party_notices();
    }
}

void show_about_list(void) {
    /*Show About List*/
    liste_style_create();
    //Draw about page header 320x50
    lv_obj_t * win = default_list_header (lv_scr_act(), get_string(ID_LAUNCHER_ABOUT), lv_win_close_event_cb);
    lv_obj_t * about_list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(about_list, LV_SB_MODE_OFF);
    lv_list_set_style(about_list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(about_list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(about_list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(about_list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(about_list, LV_LAYOUT_OFF);

    /*Style of the list element text on buttons*/
    const char *elementStr[] = { get_string(ID_ABOUT_DEVICE_INFO),
#ifdef ABOUT_CAUTION_5G
                                 get_string(ID_ABOUT_5G_CAUTION_HEADER),
#endif
                                 get_string(ID_HELP), get_string(ID_OPEN_SOURCE_NOTICE),
                                 get_string(ID_THIRD_PARTY_NOTICE)};
    /*Style of the list element size on buttons*/
    int elementNum = sizeof(elementStr) / sizeof(char *);
    /*item_id for check which item select or press*/
    int item_id[] = { ITEMS_ID_DEVICE_INFORMATION,
#ifdef ABOUT_CAUTION_5G
                      ITEMS_ID_CAUTION_FOR_USING_5G,
#endif
                      ITEMS_ID_HELP, ITEMS_ID_OPEN_SOURCE_NOTICE, ITEMS_ID_THIRD_PARTY_NOTICES };
    /*list content*/
    int i;
    for (i = 0; i < elementNum; i++) {
        btn_list[i] = lv_liste_w_arrow_w_item_id(about_list, elementStr[i], NULL,
                about_list_release_action, item_id[i]);
    }

    //show page to page exit part
    page_anim_exit();
}
