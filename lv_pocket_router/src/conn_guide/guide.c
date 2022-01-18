#include <stdio.h>  /*For printf in the action*/
#include "guide.h"
#include "../util/info_page.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/conn_guide/guide_wps.h"
#include "lv_pocket_router/src/util/page_anim.h"

//conn guide next btn action
void conn_guide_action(lv_obj_t * btn, lv_event_cb_t event_cb){
    if (event_cb != LV_EVENT_CLICKED) return;

    guide_wps_create();
}

void conn_guide_create(void) {
    //static const char * btns[] ={"Next", ""};
    static const char *btns[2];
    btns[0] = get_string(ID_NEXT);
    btns[1] = "";
    info_page_create_btmn(lv_scr_act(), get_string(ID_CONN_GUIDE),
            get_string(ID_CONN_GUIDE_NOTE), btns, conn_guide_action);

    //show page to page exit part
    page_anim_exit();
}

