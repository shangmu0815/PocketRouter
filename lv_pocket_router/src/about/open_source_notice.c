/*
 * open_source_notice.c
 *
 *  Created on: Mar 18, 2019
 *      Author: joseph
 */

#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/info_page.h"

void show_open_source_notice(void) {
    info_page_create(lv_scr_act(), get_string(ID_OPEN_SOURCE_NOTICE),
            get_string(ID_OPEN_SOURCE_NOTICE_NOTE));
}

void show_third_party_notices(void) {
    info_page_create(lv_scr_act(), get_string(ID_THIRD_PARTY_NOTICE),
            get_string(ID_THIRD_PARTY_NOTICE_NOTE));
}
