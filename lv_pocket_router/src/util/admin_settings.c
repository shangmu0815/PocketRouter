
#include <stdio.h>
#include <stdlib.h>
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/admin_settings.h"
#include "lv_pocket_router/src/keyboard/en_kb.h"
#include "lv_pocket_router/src/keyboard/basic_kb.h"

#define ADMIN_LOGIN_MAX_LEN 20
static lv_obj_t * kb;
char admin[ADMIN_LOGIN_MAX_LEN];
char password[ADMIN_LOGIN_MAX_LEN];

void admin_login_cb(int id, const char* txt){
    if (id == ID_ADMIN_COLUMN){
        strcpy(admin, txt);
        log_d("admin %s", admin);

        //reuse the kb, renew title, tip and cb id
        kb_update_headline(get_string(ID_PASSWORD));
        en_kb_set_cb_id(ID_PASSWORD_COLUMN);
        en_kb_set_tip(get_string(ID_KB_PWD_TIP));
    } else if(id == ID_PASSWORD_COLUMN){
        strcpy(password, txt);
        log_d("password %s", password);

        //close keyboard page
        en_kb_close_win(NULL, LV_EVENT_CLICKED);
    }
}

//Entry point for create admin login page
void admin_login_create(){
    kb = en_kb_reuse_create(get_string(ID_ADMIN_SETTING), ID_ADMIN_COLUMN, admin_login_cb, NULL);

    en_kb_set_tip(get_string(ID_KB_ADMIN_TIP));
    en_kb_set_lable_length(ADMIN_LOGIN_MAX_LEN);
    //hide left cancel btn
    en_kb_hide_cancel_btn(kb);
}
