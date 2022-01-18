/*
 * caution_for_using_5g.c
 *
 *  Created on: Mar 18, 2019
 *      Author: joseph
 */

#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/info_page.h"

void show_caution_for_using_5G(void) {
    info_page_create(lv_scr_act(), get_string(ID_ABOUT_5G_CAUTION_HEADER),
            get_string(ID_ABOUT_5G_CAUTION_INFO));
}
