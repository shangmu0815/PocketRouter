#include <libxml/tree.h>
#include <libxml/xmlsave.h>
#include <libxml/xmlwriter.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/settings/profile_management.h"
#include "lv_pocket_router/src/keyboard/basic_kb.h"
#include "lv_pocket_router/src/keyboard/en_kb.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/res/values/styles.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/settings/profile_management_data_store.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"

static const uint32_t INDEX_BACK_BTN = 1;
static const uint32_t INDEX_TICK_BTN = 2;

#define WITH_SIM_CARD true

enum PROFILE_MANAGEMENT_BTN_IDS {
    ID_PROFILE_CREATE,
    ID_PROFILE_SELECT,
    ID_PROFILE_DELETE,
    ID_PROFILE_EDIT
};
int btn_type = ID_PROFILE_CREATE;
lv_obj_t * profile_management_page_win;
lv_obj_t * en_kb_create_for_profile_manage_root_view;

lv_obj_t * headline_txt;
lv_obj_t * r_btn_for_profile_manage;
lv_obj_t * l_btn_for_profile_manage;

char *profile_info_delim = "`";

lv_obj_t * info_page_create_confirm_profile_info_win;

char delete_profile_name_info[PROFILE_NAME_MAX_LENGTH + 1];
lv_obj_t * delete_profile_name_page_win;

char select_profile_name_info[PROFILE_NAME_MAX_LENGTH + 1];
lv_obj_t * select_profile_name_page_win;

////////////////////////////////////////////////////////
#define MAX_LISTE 50
lv_obj_t * profile_liste_img[MAX_LISTE];
lv_obj_t * profile_liste[MAX_LISTE];
char * profile_map[MAX_LISTE];

lv_obj_t * delete_profile_liste_img[MAX_LISTE];
lv_obj_t * delete_profile_liste[MAX_LISTE];
char * delete_profile_map[MAX_LISTE];

lv_task_t * profile_management_loading_task;
lv_obj_t * profile_management_loading_task_mbox;
int profile_management_loading_task_loading;

bool create_profile_name_done = false;
bool create_apn_name_done = false;
bool create_user_name_done = false;
bool create_password_done = false;

lv_task_t * init_profile_management_loading_task;
lv_obj_t * init_profile_management_loading_task_mbox;
int init_profile_management_loading_task_loading;

#define DS_KEY_TMP_SAVE_PROFILE_ACTION              "tmp_save_profile_action"
#define DS_KEY_ORIG_MCC                             "orig_mcc"
#define DS_KEY_ORIG_MNC                             "orig_mnc"

#define PDP_TYPE_IPV4    0
#define PDP_TYPE_IPV6    1
#define PDP_TYPE_IPV4V6  2
#define PDP_TYPE_MAX_LISTE 3
lv_obj_t * pdp_type_liste_img[PDP_TYPE_MAX_LISTE];
lv_obj_t * pdp_type_liste[PDP_TYPE_MAX_LISTE];
static int pdp_type = PDP_TYPE_IPV4V6;
lv_obj_t * select_pdp_type_page_win;
int pdp_type_map[3] = { ID_IPV4, ID_IPV6, ID_IPV4V6 };
char all_profile_data[APN_MAX_LENGTH];

struct pro_mgt {
    char profile_name[PROFILE_NAME_MAX_LENGTH + 1];
    char apn[PROFILE_APN_MAX_LENGTH + 1];
    char user_name[PROFILE_USER_NAME_MAX_LENGTH + 1];
    char password[PROFILE_PW_MAX_LENGTH + 1];
    char pdp_type[PROFILE_PDP_TYPE_MAX_LENGTH];
    char mcc[PROFILE_MCC_MAX_LENGTH];
    char mnc[PROFILE_MNC_MAX_LENGTH];
};
struct pro_mgt profile_mgt;
#define MAX_MATCH_APN_NUM  40
#define MCC_MNC_BUFF_LEN 4
#define MCC_AND_MNC_BUFF_LEN 7
#define MAX_APN_RETRY  20
#define MAX_MCCMNC_KEY_LEN  14
char mcc[MCC_MNC_BUFF_LEN];
char mnc[MCC_MNC_BUFF_LEN];
char mcc_mnc[MCC_AND_MNC_BUFF_LEN];
char mccmnc_key[MAX_MCCMNC_KEY_LEN];
char orig_mcc[MCC_MNC_BUFF_LEN];
char orig_mnc[MCC_MNC_BUFF_LEN];
char orig_mcc_mnc[MCC_AND_MNC_BUFF_LEN];
PROFILE_DATA match_apn_info[MAX_MATCH_APN_NUM];
lv_task_t * lv_init_apn_task = NULL;
static int run_task_times = 0;
static int num_of_apn = 0;
static int match_apn_num = 0;
static bool init_apn_done = false;
lv_task_t * lv_init_apn_monitor_task = NULL;
static int prev_sim_state = UNKNOWN;

pthread_mutex_t mutex;
//for profile management style
lv_style_t style_profile_l;
lv_style_t style_profile_rtl;

static bool reestablish_data_enable = false;
pthread_mutex_t reconnect_mutex;
char* profile_name_value = NULL;
char* profile_name_default = NULL;
//char profile_name_default[PROFILE_NAME_MAX_LENGTH + 1];
char orig_default_profile_name[PROFILE_NAME_MAX_LENGTH + 1];
////////////////////////////////////////////////////////
void profile_name_close_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    //only for UI test
    uint8_t id = lv_obj_get_user_data(btn);

    close_kb();
    reset_profile_state_config();
    reset_profile_temp_save_config();
}

void profile_name_keyboard_action(int id, const char* profile_info) {
    //only for UI test
    log_d("profile_name_keyboard_action profile_info:%s", profile_info);
    log_d("profile_name_keyboard_action id:%d", id);
    if ((id == ID_PROFILE_CREATE) || (id == ID_PROFILE_EDIT)) {
        if (create_profile_name_done == false) {
            //temp save input profile name
            enable_tick_btn(false);
            memset(profile_mgt.profile_name, '\0', sizeof(profile_mgt.profile_name));
            strcpy(profile_mgt.profile_name, profile_info);//ex:profile_name AT&t
            //modify l_btn_for_profile_manage to ic_headline_back style
            l_btn_for_profile_manage = lv_get_child_by_index(en_kb_create_for_profile_manage_root_view, 3);
            lv_img_set_src(lv_obj_get_child(l_btn_for_profile_manage, NULL), &ic_headline_back);

            //show temp profile name
            headline_txt = lv_get_child_by_index(en_kb_create_for_profile_manage_root_view,2);
            style_profile_text_create();
            lv_obj_set_style(headline_txt, (is_ltr() ? &style_profile_l : &style_profile_rtl));
            lv_label_set_text(headline_txt, profile_mgt.profile_name);

            lv_obj_set_size(headline_txt, 220, 40);
            lv_obj_align(headline_txt, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

            en_kb_set_tip(get_string(ID_KB_APN_TIP));
            en_kb_set_lable_length(PROFILE_APN_MAX_LENGTH);//max input length 60
            create_profile_name_done = true;
        } else if (create_profile_name_done == true
                && create_apn_name_done == false) {
            //modify l_btn_for_profile_manage to ic_headline_back style
            l_btn_for_profile_manage = lv_get_child_by_index(en_kb_create_for_profile_manage_root_view, 3);
            lv_img_set_src(lv_obj_get_child(l_btn_for_profile_manage, NULL), &ic_headline_back);

            //show temp profile name
            headline_txt = lv_get_child_by_index(en_kb_create_for_profile_manage_root_view,2);
            lv_obj_set_style(headline_txt, (is_ltr() ? &style_profile_l : &style_profile_rtl));
            lv_label_set_text(headline_txt, profile_mgt.profile_name);
            lv_obj_set_size(headline_txt, 220, 40);
            lv_obj_align(headline_txt, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

            memset(profile_mgt.apn, '\0', sizeof(profile_mgt.apn));
            strcpy(profile_mgt.apn, profile_info);//apn
            en_kb_set_tip(get_string(ID_KB_USR_NAME_TIP));
            en_kb_set_lable_length(PROFILE_USER_NAME_MAX_LENGTH);//max input length 127
            //allow user name to be empty
            en_kb_input_allow_empty();
            create_profile_name_done = true;
            create_apn_name_done = true;
        } else if (create_profile_name_done == true
                && create_apn_name_done == true
                && create_user_name_done == false) {
            //user name empty case
            if (get_tip_mode() != 0) {
                profile_info = "";
            }
            //modify l_btn_for_profile_manage to ic_headline_back style
            l_btn_for_profile_manage = lv_get_child_by_index(en_kb_create_for_profile_manage_root_view, 3);
            lv_img_set_src(lv_obj_get_child(l_btn_for_profile_manage, NULL), &ic_headline_back);

            //show temp profile name
            headline_txt = lv_get_child_by_index(en_kb_create_for_profile_manage_root_view,2);
            lv_obj_set_style(headline_txt, (is_ltr() ? &style_profile_l : &style_profile_rtl));
            lv_label_set_text(headline_txt, profile_mgt.profile_name);
            lv_obj_set_size(headline_txt, 220, 40);
            lv_obj_align(headline_txt, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

            memset(profile_mgt.user_name, '\0', sizeof(profile_mgt.user_name));
            strcpy(profile_mgt.user_name, profile_info);//user_name
            en_kb_set_tip(get_string(ID_KB_PWD_TIP));
            en_kb_set_lable_length(PROFILE_PW_MAX_LENGTH);//max input length 127
            //allow password to be empty
            en_kb_input_allow_empty();
            create_profile_name_done = true;
            create_apn_name_done = true;
            create_user_name_done = true;
        } else if (create_profile_name_done == true
                && create_apn_name_done == true && create_user_name_done == true
                && create_password_done == false) {
            //password empty case
            if (get_tip_mode() != 0) {
                profile_info = "";
            }
            memset(profile_mgt.password, '\0', sizeof(profile_mgt.password));
            strcpy(profile_mgt.password, profile_info);//password
            create_password_done = true;
            ds_set_int(DS_KEY_TMP_SAVE_PROFILE_ACTION,id);
            select_pdp_type_page();
            en_kb_set_profile_font_style(false);
        }
    }
}

void profile_management_loading(void) {
    //only for UI test
    if (profile_management_loading_task_loading <= 100) {
        update_loading_bar(profile_management_loading_task_mbox, profile_management_loading_task_loading);
        profile_management_loading_task_loading = profile_management_loading_task_loading + 3;
    } else {
        PROFILE_DATA profile1;
        memset(&profile1, '\0', sizeof(PROFILE_DATA));
        profile1.profile_name  = profile_mgt.profile_name;
        profile1.apn    = profile_mgt.apn;
        profile1.user_name = profile_mgt.user_name;
        profile1.password = profile_mgt.password;
        profile1.pdp_type = profile_mgt.pdp_type;
        profile1.sim_mcc = mcc;
        profile1.sim_mnc = mnc;
        log_d("profile_management_loading profile1.profile_name:%s", profile1.profile_name);
        log_d("profile_management_loading profile1.apn:%s", profile1.apn);
        log_d("profile_management_loading profile1.user_name:%s", profile1.user_name);
        log_d("profile_management_loading profile1.password:%s", profile1.password);
        log_d("profile_management_loading profile1.pdp_type:%s", profile1.pdp_type);
        log_d("profile_management_loading profile1.sim_mcc:%s", profile1.sim_mcc);
        log_d("profile_management_loading profile1.sim_mnc:%s", profile1.sim_mnc);
        if (search_apn_by_mcc_mnc_profilename(profile1.profile_name,profile1.sim_mcc, profile1.sim_mnc) == true) {
            log_d("profile_management_loading update_profile_name_node");
            update_profile_name_node(profile1.profile_name, profile1,profile1.sim_mcc,profile1.sim_mnc);
        } else {
            if ((*((int *)profile_management_loading_task->user_data)) == ID_PROFILE_CREATE) {
                //ID_PROFILE_CREATE
                //1.update apn data in data_storage.xml when find the related apn info by profile name,mcc,mnc
                //2.otherwise to write new one
                log_d("profile_management_loading ID_PROFILE_CREATE");
                write_new_profile(profile1);
            } else {
                //ID_PROFILE_EDIT
                //1.update apn data in data_storage.xml when find the related apn info by profile name,mcc,mnc
                //2.otherwise to update default apn profile name
                //update default apn profile name
                //= delete original default profile + add one profile that user modified
                log_d("profile_management_loading ID_PROFILE_EDIT");
                delete_profile_name_node(get_mcc_mnc_default_profile_name(mcc_mnc),
                        profile1.sim_mcc, profile1.sim_mnc);
                write_new_profile(profile1);
            }
        }
        //update mccmnc_xxxxxx default profile_name
        //ex:<mccmnc_46692>CHT</mccmnc_46692>
        set_mcc_mnc_default_profile_name(mcc_mnc, profile1.profile_name);
        //insert to modem
        int res = modify_modem_apn(profile_mgt.apn, profile_mgt.user_name,
                profile_mgt.password, profile_mgt.pdp_type);
        log_d("profile_management_loading res:%d", res);
        if (res == RIL_SUCCESS) {
            reestablish_data_task();
        }

        //close task and popup when finish
        close_popup();
        /*if (info_page_create_confirm_profile_info_win != NULL) {
            lv_obj_del(info_page_create_confirm_profile_info_win);
        }*/
        close_kb();
        /*if (profile_management_page_win != NULL) {
            lv_obj_del(profile_management_page_win);
        }*/
        //re-query
        if(profile_management_page_win != NULL) {
            query_profile_management_page();
        }
    }
}

void profile_management_loading_task_animation(int *btn_type) {
    profile_management_loading_task_loading = 0;
    profile_management_loading_task_mbox = popup_anim_loading_create(get_string(ID_PROF_MGMT), get_string(ID_LOADING));
    //*btn_type == 0(ID_PROFILE_CREATE)
    //*btn_type == 3(ID_PROFILE_EDIT)
    profile_management_loading_task = popup_loading_task_create(profile_management_loading, 50, LV_TASK_PRIO_MID, btn_type);
}

void profile_loading_action_for_create(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    if (get_reestablish_data_state() == true) {
        warning_popup(ID_PROF_MGMT_RECONNECT_MSG);
        return;
    }
    //only for UI test
    uint8_t id = lv_obj_get_user_data(btn);

    if (id == INDEX_TICK_BTN) {
        //do animation and insert profile name
        btn_type = ID_PROFILE_CREATE;
        profile_management_loading_task_animation(&btn_type);

    } else if (id == INDEX_BACK_BTN){

        /*if (info_page_create_confirm_profile_info_win != NULL) {
            lv_obj_del(info_page_create_confirm_profile_info_win);
        }*/
        close_kb();
        lv_obj_t * win = lv_win_get_from_btn(btn);
        lv_obj_del(win);
        //clear to default
        reset_profile_state_config();
        reset_profile_temp_save_config();
    }

}

void profile_loading_action_for_edit(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    if (get_reestablish_data_state() == true) {
        warning_popup(ID_PROF_MGMT_RECONNECT_MSG);
        return;
    }
    //only for UI test
    uint8_t id = lv_obj_get_user_data(btn);

    if (id == INDEX_TICK_BTN) {
        //do animation and update profile name
        btn_type = ID_PROFILE_EDIT;
        profile_management_loading_task_animation(&btn_type);

    } else if (id == INDEX_BACK_BTN){

        /*if (info_page_create_confirm_profile_info_win != NULL) {
            lv_obj_del(info_page_create_confirm_profile_info_win);
        }*/
        close_kb();
        lv_obj_t * win = lv_win_get_from_btn(btn);
        lv_obj_del(win);
        //clear to default
        reset_profile_state_config();
        reset_profile_temp_save_config();
    }
}

void select_pdp_type_close_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    log_d("select_pdp_type_close_action");
    /*if (info_page_create_confirm_profile_info_win != NULL) {
        lv_obj_del(info_page_create_confirm_profile_info_win);
    }*/
    close_kb();
    //lv_win_close_event_cb(btn, LV_EVENT_RELEASED);
    //clear to default
    reset_profile_state_config();
    reset_profile_temp_save_config();
}

void select_pdp_type_confirm_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    //ex:ds_get_int(DS_KEY_TMP_SAVE_PROFILE_ACTION) =>id
    confirm_profile_info(ds_get_int(DS_KEY_TMP_SAVE_PROFILE_ACTION));
    /*if (select_pdp_type_page_win != NULL) {
        lv_obj_del(select_pdp_type_page_win);
    }*/
}

void pdp_type_btn_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    //only for UI test
    int i;
    int item_id = lv_obj_get_user_data(btn);

    for (i = 0; i < PDP_TYPE_MAX_LISTE; i++){
        lv_img_set_src(pdp_type_liste_img[i], &btn_list_radio_n);
    }
    lv_img_set_src(pdp_type_liste_img[item_id], &btn_list_radio_p);
    pdp_type = item_id;//PDP_TYPE_IPV4,PDP_TYPE_IPV6 or PDP_TYPE_IPV4V6
    memset(profile_mgt.pdp_type, '\0', sizeof(profile_mgt.pdp_type));
    strcpy(profile_mgt.pdp_type, get_pdp_type(pdp_type));
    log_d("pdp_type_btn_action profile_mgt.pdp_type:%s", profile_mgt.pdp_type);
}

void select_pdp_type_page() {
    //only for UI test
    int i;
    liste_style_create();

    select_pdp_type_page_win = modify_list_header(lv_scr_act(), get_string(ID_PROF_MGMT_PDPTYPE),
            select_pdp_type_confirm_action, select_pdp_type_close_action);
    lv_obj_t * list = lv_list_create(select_pdp_type_page_win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    //add list element in order
    memset(profile_mgt.pdp_type, '\0', sizeof(profile_mgt.pdp_type));
    if (ds_get_int(DS_KEY_TMP_SAVE_PROFILE_ACTION) == ID_PROFILE_CREATE) {
        //ID_PROFILE_CREATE
        for (i = 0; i < PDP_TYPE_MAX_LISTE; i++) {
            //PDP_TYPE_IPV4V6 is default in ID_PROFILE_CREATE case
            pdp_type_liste[i] = lv_liste_profile_w_cbox(list, get_string(pdp_type_map[i]),
                    ((!strcmp(get_string(pdp_type_map[i]),get_string(pdp_type_map[PDP_TYPE_IPV4V6]))) ? true : false), pdp_type_btn_action, i);
            pdp_type_liste_img[i] = lv_obj_get_child(pdp_type_liste[i], NULL);
        }
        strcpy(profile_mgt.pdp_type, get_pdp_type(PDP_TYPE_IPV4V6));//IPV4V6
    } else {
        get_mcc_mnc_info();
        char* default_pdptype = get_profile_pdptype(get_mcc_mnc_default_profile_name(mcc_mnc), mcc, mnc);
        log_d("select_pdp_type_page default_pdptype:%s", default_pdptype);
        //ID_PROFILE_EDIT
        for (i = 0; i < PDP_TYPE_MAX_LISTE; i++) {
            pdp_type_liste[i] = lv_liste_profile_w_cbox(list, get_string(pdp_type_map[i]),
                    ((!strcmp(get_string(pdp_type_map[i]),pdp_type_str(default_pdptype))) ? true : false), pdp_type_btn_action, i);
            pdp_type_liste_img[i] = lv_obj_get_child(pdp_type_liste[i], NULL);
        }
        strcpy(profile_mgt.pdp_type, default_pdptype);//pdp
    }
    log_d("select_pdp_type_page profile_mgt.pdp_type:%s", profile_mgt.pdp_type);
}

void confirm_profile_info(int id) {
    log_d("confirm_profile_info id:%d", id);
    //only for UI test
    const char *confirm_profile_name_title = get_string(ID_PROF_MGMT_PROFILE_NAME);
    const char *confirm_apn_title = get_string(ID_PROF_MGMT_APN);
    const char *confirm_user_name_title = get_string(ID_PROF_MGMT_USER_NAME);
    const char *confirm_pw_title = get_string(ID_PASSWORD);
    const char *confirm_pdp_type_title = get_string(ID_PROF_MGMT_PDPTYPE);
    //workaround for AR in apn and pdp type
    int len = strlen(confirm_profile_name_title) + strlen(":") + strlen(profile_mgt.profile_name) + strlen("\n")
            + strlen((is_ltr() ? confirm_apn_title : profile_mgt.apn)) + strlen(":") + strlen((is_ltr() ? profile_mgt.apn : confirm_apn_title))+ strlen("\n")
            + strlen(confirm_user_name_title)+ strlen(":") + strlen(profile_mgt.user_name)+ strlen("\n")
            + strlen(confirm_pw_title)+ strlen(":") + strlen(profile_mgt.password)+ strlen("\n")
            + strlen((is_ltr() ? confirm_pdp_type_title : pdp_type_str(profile_mgt.pdp_type))) + strlen(":")
            + strlen((is_ltr() ? pdp_type_str(profile_mgt.pdp_type) : confirm_pdp_type_title));
    log_d("confirm_profile_info len:%d", len);
    char confirmInfoPageStr[len * 3 + 1];//x3 for special char support
    memset(confirmInfoPageStr, 0, sizeof(confirmInfoPageStr));
    char* format = "%s:%s\n%s:%s\n%s:%s\n%s:%s\n%s:%s";
    //workaround for AR in apn and pdp type
    snprintf(confirmInfoPageStr, sizeof(confirmInfoPageStr),
            format,
            confirm_profile_name_title, profile_mgt.profile_name,
            (is_ltr() ? confirm_apn_title : profile_mgt.apn),(is_ltr() ? profile_mgt.apn : confirm_apn_title),
            confirm_user_name_title, profile_mgt.user_name,
            confirm_pw_title, profile_mgt.password,
            (is_ltr() ? confirm_pdp_type_title : pdp_type_str(profile_mgt.pdp_type)),
            (is_ltr() ? pdp_type_str(profile_mgt.pdp_type) : confirm_pdp_type_title)
            );

    if (id == ID_PROFILE_CREATE) {
        info_page_create_confirm_profile_info_win = info_page_create_confirm_profile_info(lv_scr_act(), get_string(ID_PROF_MGMT_CONFIRM),
                confirmInfoPageStr, profile_loading_action_for_create);
    } else {
        //id == ID_PROFILE_EDIT
        info_page_create_confirm_profile_info_win = info_page_create_confirm_profile_info(lv_scr_act(), get_string(ID_PROF_MGMT_CONFIRM),
                confirmInfoPageStr, profile_loading_action_for_edit);
    }

    /*
    if (select_pdp_type_page_win != NULL) {
        lv_obj_del(select_pdp_type_page_win);
    }
    */
    close_kb();
}

void profile_management_page_action(lv_obj_t * btnm, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(btnm);
    //only for UI test
    log_d("profile_management_page_action txt:%s", txt);
    if (strcmp(txt, get_string(ID_CREATE)) == 0) {
        if (ds_get_bool(DS_KEY_BOOT_APN_LOADDING_DONE) == false) {
            warning_popup(ID_PROF_MGMT_APN_LOAD_WARNING);
            return;
        }
        //1.check whether reach MAX_LISTE limit first
        if ((strcmp(mcc, "") == 0) || (strcmp(mnc, "") == 0)) {
            get_mcc_mnc_info();
        }
        if (search_apn_xml_num_by_mcc_mnc(mcc, mnc,
                DEFAULT_DATA_STORE_FILE) >= MAX_LISTE) {
            static const char *btns[2];
            btns[1] = "";
            btns[0] = get_string(ID_OK);
            popup_anim_not_create(get_string(ID_PROF_MGMT_REACH_LIMIT_MSG), btns, profiles_reached_limit_close_action, NULL);
        } else {
            //do Create
            en_kb_set_profile_font_style(true);
            en_kb_create_for_profile_manage_root_view =
                    en_kb_reuse_create(get_string(ID_PROF_MGMT_PROFILE_NAME),
                            ID_PROFILE_CREATE, profile_name_keyboard_action,
                            profile_name_close_action);
                    en_kb_set_tip(get_string(ID_KB_PROF_NAME_TIP));
                    //limit PROFILE_NAME_MAX_LENGTH/2 when in AR
                    en_kb_set_lable_length((is_ltr() ? PROFILE_NAME_MAX_LENGTH : (PROFILE_NAME_MAX_LENGTH/2)));
        }
    }
    if (strcmp(txt, get_string(ID_SELECT)) == 0) {
        if (ds_get_bool(DS_KEY_BOOT_APN_LOADDING_DONE) == false) {
            warning_popup(ID_PROF_MGMT_APN_LOAD_WARNING);
            return;
        }
        //do select
        select_profile_name_page();
    }
    if (strcmp(txt, get_string(ID_DELETE)) == 0) {
        if (ds_get_bool(DS_KEY_BOOT_APN_LOADDING_DONE) == false) {
            warning_popup(ID_PROF_MGMT_APN_LOAD_WARNING);
            return;
        }
        //do Delete
        delete_profile_name_page();
    }
}

void init_profile_management_loading(void) {
    //only for UI test
    if (init_profile_management_loading_task_loading <= 100) {
        update_loading_bar(init_profile_management_loading_task_mbox, init_profile_management_loading_task_loading);
        init_profile_management_loading_task_loading = init_profile_management_loading_task_loading + 3;
    } else {
        //close task and popup when finish
        close_popup();

        //do something
        //only for UI test
        init_profile_management();
    }
}

void init_profile_management_loading_task_animation(void) {
    int sim_state = get_sim_state();
    log_d("init_profile_management_loading_task_animation sim_state:%d", sim_state);
    if (is_mccmnc_available(sim_state) == true) {
        log_d("init_profile_management_loading_task_animation ds_get_bool(DS_KEY_BOOT_APN_LOADDING_DONE):%d",
                ds_get_bool(DS_KEY_BOOT_APN_LOADDING_DONE));
        if (ds_get_bool(DS_KEY_BOOT_APN_LOADDING_DONE) == false) {
            warning_popup(ID_PROF_MGMT_APN_LOAD_WARNING);
            return;
        }
        init_profile_management_loading_task_loading = 0;
        init_profile_management_loading_task_mbox = popup_anim_loading_create(get_string(ID_PROF_MGMT), get_string(ID_LOADING));
        init_profile_management_loading_task = popup_loading_task_create(init_profile_management_loading, 50, LV_TASK_PRIO_MID, NULL);
    } else {
        info_page_create_label_align_center(lv_scr_act(), get_string(ID_PROF_MGMT),
                get_string(ID_PROF_MGMT_SIM_NOT_READY));
    }
}

void init_profile_management(void) {
        get_mcc_mnc_info();
        //only for UI test
        log_d("init_profile_management get_mcc_mnc_default_profile_name(mcc_mnc):%s", get_mcc_mnc_default_profile_name(mcc_mnc));
        if(!strcmp(get_mcc_mnc_default_profile_name(mcc_mnc), "")) {
            get_profile_settings profile_settings;
            memset(&profile_settings, '\0', sizeof(get_profile_settings));
            get_apn_profile(&profile_settings);
            //set one default profile if no any profile_name
            set_mcc_mnc_default_profile_name(mcc_mnc, "Predefined");
            PROFILE_DATA profile;
            memset(&profile, '\0', sizeof(PROFILE_DATA));
            profile.profile_name  = "Predefined";
            #if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
            profile.apn    = (profile_settings.apn_name==NULL)?"":profile_settings.apn_name;
            profile.user_name = (profile_settings.username==NULL)?"":profile_settings.username;
            profile.password = (profile_settings.password==NULL)?"":profile_settings.password;
            profile.pdp_type = (profile_settings.pdp_type==NULL)?"":profile_settings.pdp_type;
            profile.sim_mcc  = mcc;
            profile.sim_mnc  = mnc;
            #else
            profile.apn    = "internet";
            profile.user_name = "";
            profile.password = "";
            profile.pdp_type = "IPV4V6";
            profile.sim_mcc  = mcc;
            profile.sim_mnc  = mnc;
            #endif
            log_d("init_profile_management profile.apn:%s",profile.apn);
            log_d("init_profile_management profile.user_name:%s",profile.user_name);
            log_d("init_profile_management profile.password:%s",profile.password);
            log_d("init_profile_management profile.pdp_type:%s",profile.pdp_type);
            log_d("init_profile_management profile.sim_mcc:%s",profile.sim_mcc);
            log_d("init_profile_management profile.sim_mnc:%s",profile.sim_mnc);
            write_new_profile(profile);
        }
        query_profile_management_page();
}

/**
 * Profile Mgm main page back and home btn callback action
 */
void profile_management_page_close_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    if (profile_management_page_win != NULL) {
        close_kb();
        profile_management_page_win = NULL;
    }
    lv_win_close_event_cb(btn, LV_EVENT_RELEASED);
}

void query_profile_management_page(void) {
    close_all_pages();//close all page
    reset_profile_temp_save_config();
    reset_profile_state_config();
    get_mcc_mnc_info();
    //only for UI test
    const char *profile_name_title_str = get_string(ID_PROF_MGMT_PROFILE_NAME);
    const char *apn_title_str = get_string(ID_PROF_MGMT_APN);
    const char *username_title_str = get_string(ID_PROF_MGMT_USER_NAME);
    const char *pw_title_str = get_string(ID_PASSWORD);
    const char *pdp_type_title_str = get_string(ID_PROF_MGMT_PDPTYPE);
    char *profile_name = get_mcc_mnc_default_profile_name(mcc_mnc);
    char *apn = getApnName();
    char *user_name = getUserName();
    char *pw = getPassword();
    char *pdp_type = getPdpType();
    //workaround for AR in apn and pdp type
    int len = strlen(profile_name_title_str) + strlen(":") + strlen(profile_name) + strlen("\n")
            + strlen((is_ltr() ? apn_title_str : apn)) + strlen(":") + strlen((is_ltr() ? apn : apn_title_str))+ strlen("\n")
            + strlen(username_title_str)+ strlen(":") + strlen(user_name)+ strlen("\n")
            + strlen(pw_title_str)+ strlen(":") + strlen(pw)+ strlen("\n")
            + strlen((is_ltr() ? pdp_type_title_str : pdp_type_str(pdp_type)))+ strlen(":")
            + strlen((is_ltr() ? pdp_type_str(pdp_type) : pdp_type_title_str));
    log_d("query_profile_management_page len:%d", len);
    char profileInfoPageStr[len * 3 + 1];//x3 for special char support
    memset(profileInfoPageStr, 0, sizeof(profileInfoPageStr));
    char* format = "%s:%s\n%s:%s\n%s:%s\n%s:%s\n%s:%s";
    //workaround for AR in apn and pdp type
    snprintf(profileInfoPageStr, sizeof(profileInfoPageStr),
            format,
            profile_name_title_str, profile_name,
            (is_ltr() ? apn_title_str : apn),(is_ltr() ? apn : apn_title_str),
            username_title_str, user_name,
            pw_title_str, pw,
            (is_ltr() ? pdp_type_title_str : pdp_type_str(pdp_type)),
            (is_ltr() ? pdp_type_str(pdp_type) : pdp_type_title_str)
            );
    static const char * btns[4];
    btns[0] = get_string(ID_CREATE);
    btns[1] = get_string(ID_SELECT);
    btns[2] = get_string(ID_DELETE);
    btns[3] = "";
    profile_management_page_win = info_page_create_btmn_impl_profile_with_edit(
            lv_scr_act(), get_string(ID_PROF_MGMT), profileInfoPageStr, btns,
            profile_management_page_close_action, profile_management_page_action,
            profile_edit_action);
}

char* getApnName() {
    //TODO
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    get_profile_settings profile_settings;
    memset(&profile_settings, '\0', sizeof(get_profile_settings));
    get_apn_profile(&profile_settings);
    log_d("getApnName profile_settings.apn_name:%s", profile_settings.apn_name);
    return profile_settings.apn_name==NULL?"":profile_settings.apn_name;
#else
    char* apn_name = get_profile_apn(get_mcc_mnc_default_profile_name(mcc_mnc),mcc,mnc);
    log_d("getApnName apn_name:%s", apn_name);
    return apn_name;
#endif
}

char* getUserName() {
    //TODO
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    get_profile_settings profile_settings;
    memset(&profile_settings, '\0', sizeof(get_profile_settings));
    get_apn_profile(&profile_settings);
    log_d("getUserName profile_settings.username:%s", profile_settings.username);
    return profile_settings.username==NULL?"":profile_settings.username;
#else
    char* user_name = get_profile_user_name(get_mcc_mnc_default_profile_name(mcc_mnc),mcc,mnc);
    log_d("getUserName user_name:%s", user_name);
    return user_name;
#endif
}

char* getPassword() {
    //TODO
    //temp solution to show "********" for user
    //password = "********";
    //return password;
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    get_profile_settings profile_settings;
    memset(&profile_settings, '\0', sizeof(get_profile_settings));
    get_apn_profile(&profile_settings);
    log_d("getPassword profile_settings.password:%s", profile_settings.password);
    return profile_settings.password==NULL?"":profile_settings.password;
#else
    char* password = get_profile_password(get_mcc_mnc_default_profile_name(mcc_mnc),mcc,mnc);
    log_d("getPassword password:%s", password);
    return password;
#endif
}

char* getPdpType() {
    //TODO
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    get_profile_settings profile_settings;
    memset(&profile_settings, '\0', sizeof(get_profile_settings));
    get_apn_profile(&profile_settings);
    log_d("getPdpType profile_settings.pdp_type:%s", profile_settings.pdp_type);
    return profile_settings.pdp_type==NULL?"":profile_settings.pdp_type;
#else
    char* pdptype = get_profile_pdptype(get_mcc_mnc_default_profile_name(mcc_mnc),mcc,mnc);
    log_d("getPdpType pdptype:%s", pdptype);
    return pdptype;
#endif
}

void profile_name_data_confirm_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    if (get_reestablish_data_state() == true) {
        warning_popup(ID_PROF_MGMT_RECONNECT_MSG);
        return;
    }
    //only for UI test
    int item_id = lv_obj_get_user_data(btn);
    //do something to modem
    //To show select profile data
    //set the default_profile_name
    get_mcc_mnc_info();
    set_mcc_mnc_default_profile_name(mcc_mnc, select_profile_name_info);//
    //ex:<mccmnc_46692>pp</mccmnc_46692>

    //insert to modem
    char* confirm_profile_name = get_mcc_mnc_default_profile_name(mcc_mnc);
    log_d("profile_name_data_confirm_action confirm_profile_name:%s", confirm_profile_name);
    int res = modify_modem_apn(get_profile_apn(confirm_profile_name, mcc, mnc),
            get_profile_user_name(confirm_profile_name, mcc, mnc),
            get_profile_password(confirm_profile_name, mcc, mnc),
            get_profile_pdptype(confirm_profile_name, mcc, mnc));
    log_d("profile_name_data_confirm_action modify_modem_apn res:%d", res);
    if (res == RIL_SUCCESS) {
        reestablish_data_task();
    }

    if(profile_management_page_win != NULL) {
        query_profile_management_page();
    }

    /*if (select_profile_name_page_win != NULL) {
        lv_obj_del(select_profile_name_page_win);
    }*/
}

void profile_btn_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    //only for UI test
    int i;
    int item_id = lv_obj_get_user_data(btn);

    for (i = 0; i < search_apn_xml_num_by_mcc_mnc(mcc, mnc, DEFAULT_DATA_STORE_FILE); i++){
        lv_img_set_src(profile_liste_img[i], &btn_list_radio_n);

    }
    lv_img_set_src(profile_liste_img[item_id], &btn_list_radio_p);
    //copy the profile_map[item_id] content to select_profile_name_info
    memset(select_profile_name_info, '\0', sizeof(select_profile_name_info));
    strcpy(select_profile_name_info, profile_map[item_id]);
    log_d("profile_btn_action select_profile_name_info:%s", select_profile_name_info);
}

void select_profile_name_page(void) {
    //only for UI test
    //copy the get_mcc_mnc_default_profile_name(mcc_mnc) content to select_profile_name_info
    memset(select_profile_name_info, '\0', sizeof(select_profile_name_info));
    get_mcc_mnc_info();
    strcpy(select_profile_name_info, get_mcc_mnc_default_profile_name(mcc_mnc));

    liste_style_create();

    select_profile_name_page_win = modify_list_header(lv_scr_act(), get_string(ID_PROF_MGMT_SELECT_PROFILE),
            profile_name_data_confirm_action, lv_win_close_event_cb);
    lv_obj_t * list = lv_list_create(select_profile_name_page_win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);
    log_d("select_profile_name_page select_profile_name_info:%s", select_profile_name_info);
    //add list element in order
    xmlDocPtr doc = xmlReadFile(DEFAULT_DATA_STORE_FILE, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        printf("Failed to read %s", DEFAULT_DATA_STORE_FILE);
    }
    xmlNodePtr  curNode;
    curNode = xmlDocGetRootElement(doc);
    curNode = curNode->xmlChildrenNode;
    int i = 0;
    while (curNode != NULL) {
        //check HEADER profile and mcc,mnc match case
        if (!xmlStrcmp(curNode->name, BAD_CAST (const xmlChar *)PROFILE_HEADER)) {
            if (strcmp(((char *)xmlGetProp(curNode,PROFILE_MCC)), mcc) == 0 &&
                    strcmp(((char *)xmlGetProp(curNode,PROFILE_MNC)), mnc) == 0) {
                profile_map[i] = (char *)xmlGetProp(curNode,PROFILE_NAME);
                profile_liste[i] = lv_liste_profile_w_cbox(list, profile_map[i],((!strcmp(profile_map[i],select_profile_name_info))?true:false), profile_btn_action, i);
                profile_liste_img[i] = lv_obj_get_child(profile_liste[i], NULL);
                i++;
            }
        }
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);
}

void delete_selected_items_action(lv_obj_t * btnm, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    if (get_reestablish_data_state() == true) {
        warning_popup(ID_PROF_MGMT_RECONNECT_MSG);
        return;
    }
    get_mcc_mnc_info();
    const char * txt = lv_btnm_get_active_btn_text(btnm);
    log_d("delete_selected_items_action txt:%s", txt);
    //only for UI test
    if (strcmp(txt, get_string(ID_OK)) == 0) {
            log_d("delete_selected_items_action delete_profile_name_info:%s", delete_profile_name_info);
            //ex:delete profile_name="TWM1" <apn profile_name="TWM1" apn="internet" user_name="test" password="aaaaaa"/>
            delete_profile_name_node(delete_profile_name_info,mcc,mnc);
            //if delete finish then only one profile_name ,then re-query
            int profile_num = search_apn_xml_num_by_mcc_mnc(mcc,mnc,DEFAULT_DATA_STORE_FILE);
            log_d("delete_selected_items_action_profile_num:%d", profile_num);
            if (profile_num == 0) {
                //set one default profile if no any profile_name after delete completely

                get_profile_settings profile_settings;
                memset(&profile_settings, '\0', sizeof(get_profile_settings));
                get_apn_profile(&profile_settings);
                //set one default profile if no any profile_name
                set_mcc_mnc_default_profile_name(mcc_mnc, "Predefined");

            //#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
                 //insert to modem
                int res = modify_modem_apn("internet", "", "", "IPV4V6");
                if (res == RIL_SUCCESS) {
                    reestablish_data_task();
                }
            //#else
                 //PROFILE_CONTENT profile;
                 PROFILE_DATA profile;
                 memset(&profile, '\0', sizeof(PROFILE_DATA));
                 profile.profile_name  = "Predefined";
                 profile.apn       = "internet";
                 profile.user_name = "";
                 profile.password  = "";
                 profile.pdp_type  = "IPV4V6";
                 profile.sim_mcc = mcc;
                 profile.sim_mnc = mnc;
            //#endif
                 write_new_profile(profile);
            } else if (profile_num >= 1) {
                //no need to call modify_apn_profile if delete profile name and get_mcc_mnc_default_profile_name(mcc_mnc) are not the same.
                if (strcmp(delete_profile_name_info, get_mcc_mnc_default_profile_name(mcc_mnc)) == 0) {
                    //if profile_name >1 after delete complete ,then re-query the first profile_name
                    xmlDocPtr doc = xmlReadFile(DEFAULT_DATA_STORE_FILE, NULL, XML_PARSE_RECOVER);
                    if (doc == NULL) {
                        printf("Failed to read %s", DEFAULT_DATA_STORE_FILE);
                    }
                    xmlNodePtr  curNode;
                    curNode = xmlDocGetRootElement(doc);
                    curNode = curNode->xmlChildrenNode;
                    char* profile_name;
                    while (curNode != NULL) {
                        //check HEADER profile and mcc,mnc match case
                        if (!xmlStrcmp(curNode->name, BAD_CAST (const xmlChar *)PROFILE_HEADER)) {
                            if (strcmp(((char *)xmlGetProp(curNode,PROFILE_MCC)), mcc) == 0 &&
                                    strcmp(((char *)xmlGetProp(curNode,PROFILE_MNC)), mnc) == 0) {
                                profile_name = (char *)xmlGetProp(curNode,PROFILE_NAME);
                                //after delete complete ,then re-set the another profile_name as default
                                set_mcc_mnc_default_profile_name(mcc_mnc, profile_name);//
                                break;
                            }
                        }
                        curNode = curNode->next;
                    }
                    xmlFreeDoc(doc);
                    //insert to modem
                    char* profileName = get_mcc_mnc_default_profile_name(mcc_mnc);
                    log_d("delete_selected_items_action profileName:%s", profileName);
                    int res = modify_modem_apn(get_profile_apn(profileName, mcc, mnc),
                            get_profile_user_name(profileName, mcc, mnc),
                            get_profile_password(profileName, mcc, mnc),
                            get_profile_pdptype(profileName, mcc, mnc));
                    log_d("delete_selected_items_action res:%d", res);
                    if (res == RIL_SUCCESS) {
                        reestablish_data_task();
                    }
                }
            }
            if(profile_management_page_win != NULL) {
                query_profile_management_page();
            }
            /*if (delete_profile_name_page_win != NULL) {
                lv_obj_del(delete_profile_name_page_win);
            }*/
            close_all_lists(1);
    }
    close_popup();
}

void delete_profile_name_data_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    if(lv_btn_get_state(btn) == LV_BTN_STATE_INA) return;

    int item_id = lv_obj_get_user_data(btn);
    static const char * btns[3];
    btns[0] = get_string(ID_CANCEL);
    btns[1] = get_string(ID_OK);
    btns[2] = "";
    popup_anim_que_create(get_string(ID_PROF_MGMT_DEL_ITEM_CONFIRM_PROMPT), btns, delete_selected_items_action, NULL);
}

void delete_profile_btn_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    //only for UI test
    int i;
    int item_id = lv_obj_get_user_data(btn);

    for (i = 0; i < search_apn_xml_num_by_mcc_mnc(mcc, mnc, DEFAULT_DATA_STORE_FILE); i++) {
        lv_img_set_src(delete_profile_liste_img[i], &ic_list_checkbox);
    }
    lv_img_set_src(delete_profile_liste_img[item_id], &ic_list_checkbox_selected);

    //copy the delete_profile_map[item_id] content to delete_profile_name_info
    memset(delete_profile_name_info, '\0', sizeof(delete_profile_name_info));
    strcpy(delete_profile_name_info, delete_profile_map[item_id]);
    log_d("delete_profile_btn_action delete_profile_name_info:%s", delete_profile_name_info);
}

void delete_profile_name_page(void) {
    //only for UI test
    memset(delete_profile_name_info, '\0', sizeof(delete_profile_name_info));
    get_mcc_mnc_info();
    strcpy(delete_profile_name_info, get_mcc_mnc_default_profile_name(mcc_mnc));
    log_d("delete_profile_name_page delete_profile_name_info:%s", delete_profile_name_info);
    liste_style_create();

    delete_profile_name_page_win = delete_profile_list_header_create(lv_scr_act(), get_string(ID_PROF_MGMT_DELETE_PROFILE),
            delete_profile_name_data_action, lv_win_close_event_cb);
    //TODO need to set correct garbage btn state here
    int profile_num = search_apn_xml_num_by_mcc_mnc(mcc, mnc, DEFAULT_DATA_STORE_FILE);
    lv_win_ext_t * ext = lv_obj_get_ext_attr(delete_profile_name_page_win);
    lv_obj_t * gar_btn = lv_obj_get_child(ext->header, NULL);
    lv_obj_t * img = lv_obj_get_child(gar_btn, NULL);
    if (profile_num == 1) {
        lv_img_set_src(img, &ic_headline_delete_dis);
        lv_btn_set_state(gar_btn, LV_BTN_STATE_INA);
    } else {
        lv_img_set_src(img, &ic_headline_delete);
        lv_btn_set_state(gar_btn, LV_BTN_STYLE_REL);
    }
    lv_obj_t * list = lv_list_create(delete_profile_name_page_win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    //add list element in order
    xmlDocPtr doc = xmlReadFile(DEFAULT_DATA_STORE_FILE, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        printf("Failed to read %s", DEFAULT_DATA_STORE_FILE);
    }
    xmlNodePtr  curNode;
    curNode = xmlDocGetRootElement(doc);
    curNode = curNode->xmlChildrenNode;
    int i = 0;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST (const xmlChar *)PROFILE_HEADER)) {
            if (strcmp(((char *)xmlGetProp(curNode,"mcc")),mcc) == 0 &&
                    strcmp(((char *)xmlGetProp(curNode,"mnc")),mnc) == 0) {
                delete_profile_map[i] = (char *)xmlGetProp(curNode,"profile_name");
                delete_profile_liste[i] = lv_liste_profile_w_checkbox(list, delete_profile_map[i],((!strcmp(delete_profile_map[i],delete_profile_name_info))?true:false), delete_profile_btn_action, i);
                delete_profile_liste_img[i] = lv_get_child_by_index(delete_profile_liste[i], 1);
                i++;
            }
        }
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);
}

void profile_edit_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    if (ds_get_bool(DS_KEY_BOOT_APN_LOADDING_DONE) == false) {
        warning_popup(ID_PROF_MGMT_APN_LOAD_WARNING);
        return;
    }
    //only for UI test
    reset_profile_state_config();
    en_kb_set_profile_font_style(true);
    en_kb_create_for_profile_manage_root_view =
            en_kb_reuse_create(get_string(ID_PROF_MGMT_PROFILE_NAME),
                    ID_PROFILE_EDIT, profile_name_keyboard_action,
                    profile_name_close_action);
    en_kb_set_tip(get_string(ID_KB_PROF_NAME_TIP));
    en_kb_set_lable_length(PROFILE_NAME_MAX_LENGTH);
    get_mcc_mnc_info();
    en_kb_set_lable(get_mcc_mnc_default_profile_name(mcc_mnc));
}

char* get_all_profile_data() {
    get_mcc_mnc_info();
    memset(all_profile_data, '\0', sizeof(all_profile_data));
    snprintf(all_profile_data, sizeof(all_profile_data), "%s%s%s%s%s%s%s%s%s",
            get_mcc_mnc_default_profile_name(mcc_mnc),profile_info_delim, getApnName(),profile_info_delim, getUserName(),profile_info_delim,getPassword(),profile_info_delim,getPdpType());
    //ex:profile_name,apn,user_name,password,pdpType
    log_d("get_all_profile_data all_profile_data:%s", all_profile_data);
    return all_profile_data;
}

//reset all state for profile create
void reset_profile_state_config() {
    create_profile_name_done = false;
    create_apn_name_done = false;
    create_user_name_done = false;
    create_password_done = false;
    en_kb_set_profile_font_style(false);
}

//reset all profile temp save value
void reset_profile_temp_save_config() {
    ds_set_value(DS_KEY_TMP_SAVE_PROFILE_ACTION,"");
}

char* get_pdp_type(int type) {
    char* res;
    switch (type) {
    case PDP_TYPE_IPV4:
        res = "IPV4";
        break;
    case PDP_TYPE_IPV6:
        res = "IPV6";
        break;
    case PDP_TYPE_IPV4V6:
        res = "IPV4V6";
        break;
    }
    return res;
}

char* pdp_type_str(char* pdp_type) {
    char* res = "";
    if (pdp_type != NULL) {
        if (strcmp(pdp_type, "IPV4") == 0) {
            res = get_string(pdp_type_map[PDP_TYPE_IPV4]);
        } else if (strcmp(pdp_type, "IPV6") == 0) {
            res = get_string(pdp_type_map[PDP_TYPE_IPV6]);
        } else if (strcmp(pdp_type, "IPV4V6") == 0) {
            res = get_string(pdp_type_map[PDP_TYPE_IPV4V6]);
        }
    }
    return res;
}

void init_default_apn_profile() {
    ds_set_bool(DS_KEY_BOOT_APN_LOADDING_DONE, false);
    get_mcc_mnc_info();
    char* orig_mcc = get_orig_mcc();
    char* orig_mnc = get_orig_mnc();
    log_d("init_default_apn_profile orig_mcc:%s", orig_mcc);
    log_d("init_default_apn_profile orig_mnc:%s", orig_mnc);
    char* default_profile_name;
    #ifdef CUST_ZX73_NEC
    //NEC use the last default apn and need to set modem again
    if (strcmp(orig_mcc, "") != 0 && strcmp(orig_mnc, "") != 0) {
        memset(orig_mcc_mnc, '\0', sizeof(orig_mcc_mnc));
        snprintf(orig_mcc_mnc, sizeof(orig_mcc_mnc), "%s%s", orig_mcc, orig_mnc);
        default_profile_name = get_mcc_mnc_default_profile_name(orig_mcc_mnc);
        set_mccmnc_default_apn_to_modem(default_profile_name , orig_mcc, orig_mnc);
    } else {
        default_profile_name = get_mcc_mnc_default_profile_name(mcc_mnc);
        set_mccmnc_default_apn_to_modem(default_profile_name , mcc, mnc);
    }
    reestablish_data_connection();
    #else
    //default_profile_name = get_mcc_mnc_default_profile_name(mcc_mnc);
    profile_name_default = get_mcc_mnc_default_profile_name(mcc_mnc);
    set_mccmnc_default_apn_to_modem(default_profile_name, mcc, mnc);
    reestablish_data_connection();
    #endif
    PROFILE_DATA profile;
    memset(&profile, '\0', sizeof(PROFILE_DATA));
    get_profile_settings profile_settings;
    memset(&profile_settings, '\0', sizeof(get_profile_settings));
    get_apn_profile(&profile_settings);
    profile.profile_name = default_profile_name;
    #if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
        profile.apn    = (profile_settings.apn_name==NULL)?"":profile_settings.apn_name;
        profile.user_name = (profile_settings.username==NULL)?"":profile_settings.username;
        profile.password = (profile_settings.password==NULL)?"":profile_settings.password;
        profile.pdp_type = (profile_settings.pdp_type==NULL)?"":profile_settings.pdp_type;
        profile.sim_mcc = mcc;
        profile.sim_mnc = mnc;
    #else
    char* profile_apn = get_profile_apn(default_profile_name, mcc, mnc);
    profile.apn = (strcmp(profile_apn, "") == 0) ? "internet" : profile_apn;
    profile.user_name = get_profile_user_name(default_profile_name, mcc, mnc);
    profile.password = get_profile_password(default_profile_name, mcc, mnc);
    char* profile_pdptype = get_profile_pdptype(default_profile_name, mcc, mnc);
    profile.pdp_type = (strcmp(profile_pdptype, "") == 0) ? "IPV4V6" : profile_pdptype;
    profile.sim_mcc = "466";
    profile.sim_mnc = "92";
    #endif
    log_d("init_default_apn_profile default_profile_name:%s",default_profile_name);
    log_d("init_default_apn_profile profile.profile_name:%s",profile.profile_name);
    log_d("init_default_apn_profile profile.apn:%s",profile.apn);
    log_d("init_default_apn_profile profile.user_name:%s",profile.user_name);
    log_d("init_default_apn_profile profile.password:%s",profile.password);
    log_d("init_default_apn_profile profile.pdp_type:%s",profile.pdp_type);
    log_d("init_default_apn_profile profile.sim_mcc:%s",profile.sim_mcc);
    log_d("init_default_apn_profile profile.sim_mnc:%s",profile.sim_mnc);
    if (search_apn_by_mcc_mnc_profilename(default_profile_name, profile.sim_mcc, profile.sim_mnc) == true) {
        //update <apn profile_name="default_profile_name" apn="xxx" user_name="xxx" password="xxx" pdp_type="xxx" mcc="xxx" mnc="yyy" />
        //in data_storage.xml,and corresponding profile data from current modem
        log_d("init_default_apn_profile update_profile_name_node");
        update_profile_name_node(default_profile_name, profile, profile.sim_mcc, profile.sim_mnc);
    } else {
        //<mccmnc_xxxyy>default_profile_name</mccmnc_xxxyy>
        //and related <apn profile_name="xxx" .../> Does Not Exist CASE
        //set default profile data to data_storage.xml from current modem
        //ex:<mccmnc_46692>default_profile_name</mccmnc_46692>
        //ex:<apn profile_name="default_profile_name" apn="xxx" user_name="xxx" password="xxx" pdp_type="xxx" mcc="xxx" mnc="yyy" />
        log_d("init_default_apn_profile write_new_profile");
        write_new_profile(profile);
    }
    //set <mccmnc_46692>default_profile_name</mccmnc_46692> to data_storage.xml when every boot
    set_mcc_mnc_default_profile_name(mcc_mnc, default_profile_name);
    //save now mcc nnc info
    set_orig_mcc(profile.sim_mcc);
    set_orig_mnc(profile.sim_mnc);
    ds_set_bool(DS_KEY_BOOT_APN_LOADDING_DONE, true);
    log_d("init_default_apn_profile final default_profile_name:%s", get_mcc_mnc_default_profile_name(mcc_mnc));
}

void init_apn_info() {
    ds_set_bool(DS_KEY_BOOT_APN_LOADDING_DONE, false);
    init_apn_done = false;
    get_mcc_mnc_info();
    profile_name_default = get_mcc_mnc_default_profile_name(mcc_mnc);
    log_d("init_apn_info profile_name_default:%s", profile_name_default);
    //case1 check DEFAULT_DATA_STORE_FILE if match default_profile_name,mcc,mnc and put in the first
    if (search_apn_by_mcc_mnc_profilename(profile_name_default, mcc, mnc) == true) {
        memset(&match_apn_info[0], '\0', sizeof(PROFILE_DATA));
        match_apn_info[0].sim_mcc = mcc;
        match_apn_info[0].sim_mnc = mnc;
        match_apn_info[0].profile_name = profile_name_default;
        match_apn_info[0].apn = get_profile_apn(profile_name_default,mcc,mnc);
        match_apn_info[0].user_name = get_profile_user_name(profile_name_default,mcc,mnc);
        match_apn_info[0].password = get_profile_password(profile_name_default,mcc,mnc);
        match_apn_info[0].pdp_type = get_profile_pdptype(profile_name_default,mcc,mnc);
        match_apn_num = 1;
    }

    //case2 check DEFAULT_DATA_STORE_FILE if match mcc,mnc but except case1
    if (access( DEFAULT_DATA_STORE_FILE, F_OK) != -1) {
        // file exists
        xmlDocPtr data_store_doc = xmlReadFile(DEFAULT_DATA_STORE_FILE, NULL, XML_PARSE_RECOVER);
        if (data_store_doc != NULL) {
            xmlNodePtr  data_store_curNode;
            data_store_curNode = xmlDocGetRootElement(data_store_doc);
            data_store_curNode = data_store_curNode->xmlChildrenNode;
            while (data_store_curNode != NULL) {
                if (!xmlStrcmp(data_store_curNode->name, BAD_CAST (const xmlChar *)PROFILE_HEADER)) {
                    if (strcmp(((char *)xmlGetProp(data_store_curNode,PROFILE_MCC)), mcc) == 0 &&
                        strcmp(((char *)xmlGetProp(data_store_curNode,PROFILE_MNC)), mnc) == 0 &&
                            strcmp(((char *) xmlGetProp(data_store_curNode, PROFILE_NAME)),profile_name_default) != 0) {
                        memset(&match_apn_info[match_apn_num], '\0', sizeof(PROFILE_DATA));
                        match_apn_info[match_apn_num].sim_mcc = ((char *)xmlGetProp(data_store_curNode,PROFILE_MCC));
                        match_apn_info[match_apn_num].sim_mnc = ((char *)xmlGetProp(data_store_curNode,PROFILE_MNC));
                        match_apn_info[match_apn_num].profile_name = ((char *)xmlGetProp(data_store_curNode,PROFILE_NAME));
                        match_apn_info[match_apn_num].apn = ((char *)xmlGetProp(data_store_curNode,PROFILE_APN));
                        match_apn_info[match_apn_num].user_name = ((char *)xmlGetProp(data_store_curNode,PROFILE_USER_NAME));
                        match_apn_info[match_apn_num].password = ((char *)xmlGetProp(data_store_curNode,PROFILE_PASSWORD));
                        match_apn_info[match_apn_num].pdp_type = ((char *)xmlGetProp(data_store_curNode,PROFILE_PDPTYPE));
                        match_apn_num++;
                    }
                }
                data_store_curNode = data_store_curNode->next;
            }
            xmlFreeDoc(data_store_doc);
        }
    }

    //case3 check DEFAULT_APN_STORE_FILE if match mcc,mnc
    if (access( DEFAULT_APN_STORE_FILE, F_OK) != -1) {
        // file exists
        xmlDocPtr apn_store_doc = xmlReadFile(DEFAULT_APN_STORE_FILE, NULL, XML_PARSE_RECOVER);
        if (apn_store_doc != NULL) {
            xmlNodePtr  curNode;
            curNode = xmlDocGetRootElement(apn_store_doc);
            curNode = curNode->xmlChildrenNode;
            while (curNode != NULL) {
                if (!xmlStrcmp(curNode->name, BAD_CAST (const xmlChar *)PROFILE_HEADER)) {
                    if (strcmp(((char *)xmlGetProp(curNode,PROFILE_MCC)), mcc) == 0 &&
                            strcmp(((char *)xmlGetProp(curNode,PROFILE_MNC)), mnc) == 0) {
                        memset(&match_apn_info[match_apn_num], '\0', sizeof(PROFILE_DATA));
                        match_apn_info[match_apn_num].sim_mcc = ((char *)xmlGetProp(curNode,PROFILE_MCC));
                        match_apn_info[match_apn_num].sim_mnc = ((char *)xmlGetProp(curNode,PROFILE_MNC));
                        match_apn_info[match_apn_num].profile_name = ((char *)xmlGetProp(curNode,PROFILE_NAME));
                        match_apn_info[match_apn_num].apn = ((char *)xmlGetProp(curNode,PROFILE_APN));
                        match_apn_info[match_apn_num].user_name = ((char *)xmlGetProp(curNode,PROFILE_USER_NAME));
                        match_apn_info[match_apn_num].password = ((char *)xmlGetProp(curNode,PROFILE_PASSWORD));
                        match_apn_info[match_apn_num].pdp_type = ((char *)xmlGetProp(curNode,PROFILE_PDPTYPE));
                        match_apn_num++;
                    }
                }
                curNode = curNode->next;
            }
            xmlFreeDoc(apn_store_doc);
        }
    }

    log_d("init_apn_info match_apn_num:%d",match_apn_num);
    if (match_apn_num != 0) {
        //save orig or first default profile name
        memset(orig_default_profile_name, '\0', sizeof(orig_default_profile_name));
        strncpy(orig_default_profile_name, match_apn_info[0].profile_name,
                PROFILE_NAME_MAX_LENGTH);
        log_d("init_apn_info orig_default_profile_name:%s",orig_default_profile_name);
        //check the apn when data connect is workable
        //(RIL_QCMAP_WWAN_V4_STATUS_CONNECTED =3 or RIL_QCMAP_WWAN_V6_STATUS_CONNECTED =9)
        check_wwan_status_task();
    } else {
        //if no match customer xml,using Qualcomm by modem
        init_default_apn_profile();
    }
}

void close_wwan_check_task() {
    //case1 (init_apn_done == true) init apn success =>using the success apn to update data_storage.xml
    //case2 try all apn fail =>using the user default or the first custom apn to default
    log_d("close_wwan_check_task init_apn_done:%d,num_of_apn:%d,match_apn_num:%d", init_apn_done,num_of_apn,match_apn_num);
    if ((init_apn_done == true)
            || (init_apn_done == false && (num_of_apn == match_apn_num))
            || (num_of_apn == match_apn_num)) {
        PROFILE_DATA profile_info;
        memset(&profile_info, '\0', sizeof(PROFILE_DATA));
        //init apn success =>using the current success apn
        //try all apn fail =>using the user default or the first custom apn to default
        int numOfApn = (init_apn_done == true) ? num_of_apn : 0;
        log_d("close_wwan_check_task numOfApn:%d", numOfApn);
        get_mcc_mnc_info();
        log_d("close_wwan_check_task orig_default_profile_name:%s",orig_default_profile_name);
        profile_info.profile_name =((!init_apn_done) ? orig_default_profile_name : get_mcc_mnc_default_profile_name(mcc_mnc));
        profile_info.apn    = match_apn_info[numOfApn].apn;
        profile_info.user_name = match_apn_info[numOfApn].user_name;
        profile_info.password = match_apn_info[numOfApn].password;
        profile_info.pdp_type = match_apn_info[numOfApn].pdp_type;
        profile_info.sim_mcc = match_apn_info[numOfApn].sim_mcc;
        profile_info.sim_mnc = match_apn_info[numOfApn].sim_mnc;
        //using the user default or the first custom apn to default when try all apn fail
        if (!init_apn_done) {
            set_mcc_mnc_default_profile_name(mcc_mnc, orig_default_profile_name);
            int res = modify_modem_apn(profile_info.apn, profile_info.user_name,
                    profile_info.password, profile_info.pdp_type);
            log_d("close_wwan_check_task res:%d",res);
            if (res == RIL_SUCCESS) {
                reestablish_data_connection();
            }
        }
        log_d("close_wwan_check_task profile_info.profile_name:%s", profile_info.profile_name);
        log_d("close_wwan_check_task profile_info.apn:%s", profile_info.apn);
        log_d("close_wwan_check_task profile_info.user_name:%s", profile_info.user_name);
        log_d("close_wwan_check_task profile_info.password:%s", profile_info.password);
        log_d("close_wwan_check_task profile_info.pdp_type:%s", profile_info.pdp_type);
        log_d("close_wwan_check_task profile_info.sim_mcc:%s", profile_info.sim_mcc);
        log_d("close_wwan_check_task profile_info.sim_mnc:%s", profile_info.sim_mnc);
        if (search_apn_by_mcc_mnc_profilename(profile_info.profile_name,profile_info.sim_mcc,profile_info.sim_mnc) == true) {
            log_d("close_wwan_check_task update_profile_name_node");
            update_profile_name_node(profile_info.profile_name, profile_info,profile_info.sim_mcc,profile_info.sim_mnc);
        } else {
            log_d("close_wwan_check_task write_new_profile");
            write_new_profile(profile_info);
        }
        log_d("close_wwan_check_task default_profile_name profile_info.profile_name:%s",profile_info.profile_name);
        get_profile_settings profile_settings;
        memset(&profile_settings, '\0', sizeof(get_profile_settings));
        get_apn_profile(&profile_settings);
        //add log to check now modem get apn info
        #if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
        log_d("modem get profile_settings.apn_name:%s", profile_settings.apn_name);
        log_d("modem get profile_settings.username:%s", profile_settings.username);
        log_d("modem get profile_settings.password:%s", profile_settings.password);
        log_d("modem get profile_settings.pdp_type:%s", profile_settings.pdp_type);
        #endif
        //reset all config
        init_apn_done = false;
        match_apn_num = 0;
        num_of_apn = 0;
        ds_set_bool(DS_KEY_BOOT_APN_LOADDING_DONE, true);
    }
    log_d("close_wwan_check_task before run_task_times:%d",run_task_times);
    run_task_times = 0;
}

//check data connect is workable or not if set the apns
//(RIL_QCMAP_WWAN_V4_STATUS_CONNECTED =3 or
// RIL_QCMAP_WWAN_V6_STATUS_CONNECTED =9)
void check_wwan_status() {
    pthread_mutex_lock(&mutex);
    log_d("check_wwan_status run_task_times:%d", run_task_times);
    //insert modem related apn info only when run_task_times == 0
    if (run_task_times == 0) {
        log_d("num_of_apn:%d",num_of_apn);
        get_mcc_mnc_info();
        set_mcc_mnc_default_profile_name(mcc_mnc, match_apn_info[num_of_apn].profile_name);
        int res = modify_modem_apn(match_apn_info[num_of_apn].apn,
                match_apn_info[num_of_apn].user_name,
                match_apn_info[num_of_apn].password,
                match_apn_info[num_of_apn].pdp_type);
        log_d("check_wwan_status res:%d",res);
        if (res == RIL_SUCCESS) {
            reestablish_data_connection();
        }
        run_task_times++;
        //return to the next run and wait 1 sec then call get_wwan_status
        //to check data connect status after call modify_apn_profile
        //to avoid getting the before connection
        return;
    }
    log_d("check_wwan_status num_of_apn:%d,match_apn_num:%d", num_of_apn, match_apn_num);
    //check whether the num of apn < all match apn num or not
    if (num_of_apn < match_apn_num) {
        //check whether run_task_times <= MAX_APN_RETRY
        if (run_task_times <= MAX_APN_RETRY) {
            log_d("check_wwan_status run_task_times <= MAX_APN_RETRY run_task_times:%d",run_task_times);
            //check the apn when data connect is workable or not
            ril_qcmap_wwan_status_enum_t v4_status,v6_status;
            memset(&v4_status, '\0', sizeof(ril_qcmap_wwan_status_enum_t));
            memset(&v6_status, '\0', sizeof(ril_qcmap_wwan_status_enum_t));
            ril_error_type ret = get_wwan_status(&v4_status, &v6_status);
            if (ret == RIL_SUCCESS) {
                log_d("check_wwan_status v4_status:%d",v4_status);
                log_d("check_wwan_status v6_status:%d",v6_status);
                if ((v4_status == RIL_QCMAP_WWAN_V4_STATUS_CONNECTED)
                        || (v6_status == RIL_QCMAP_WWAN_V6_STATUS_CONNECTED)) {
                    log_d("check_wwan_status init_apn_done");
                    init_apn_done = true;
                    close_wwan_check_task();
                    return;
                }
            }
            run_task_times++;
        } else if (run_task_times > MAX_APN_RETRY) {
            //retry max is 20 times
            log_d("check_wwan_status run_task_times MAX_APN_RETRY");
            //change to next apn
            num_of_apn++;
            close_wwan_check_task();
        }
    } else {
        close_wwan_check_task();
    }
    pthread_mutex_unlock(&mutex);
}

int check_wwan_Thread() {
    while (1) {
        check_wwan_status();
        log_d("check_wwan_Thread ds_get_bool(DS_KEY_BOOT_APN_LOADDING_DONE):%d", ds_get_bool(DS_KEY_BOOT_APN_LOADDING_DONE));
        if (ds_get_bool(DS_KEY_BOOT_APN_LOADDING_DONE) == true) {
            pthread_exit(NULL);
            break;
        }
        usleep(1000000);
    }
    return 0;
}

void check_wwan_status_task() {
    pthread_t checkwwanThread;
    pthread_mutex_init(&mutex,NULL);
    int res = pthread_create(&checkwwanThread, NULL, &check_wwan_Thread, NULL);
    if (res) log_e("create check_wwan_Thread fail\n");
    pthread_mutex_destroy(&mutex);
}

void init_apn_monitor_task() {
    int state = get_sim_state();
    if (is_mccmnc_available(prev_sim_state) == false
            && is_mccmnc_available(state) == true) {
        log_d("init_apn_monitor_task start");
        if (access( DEFAULT_APN_STORE_FILE, F_OK) == 0) {
            // /oem/data/apn.xml file exists
            init_apn_info();
        } else {
            init_default_apn_profile();
        }
    }
    prev_sim_state = state;
}

//sync data_storage.xml and modem current apn
//due to modem apn can be modified by webui
void sync_data_storage_apn() {
    //get mc mnc info
    get_mcc_mnc_info();
    if (strcmp(mcc, "") == 0 || strcmp(mnc, "") == 0) {
        //do not update when no mcc or mnc
        log_d("sync_data_storage_apn when no mcc or mnc");
        return;
    }
    int sim_state = get_sim_state();
    if (is_mccmnc_available(sim_state) == true
            && ds_get_bool(DS_KEY_BOOT_APN_LOADDING_DONE) == false) {
        log_d("sync_data_storage_apn boot_apn_loading_done config is false");
        return;
    }
    PROFILE_DATA sync_profile;
    memset(&sync_profile, '\0', sizeof(PROFILE_DATA));
    memset(mccmnc_key, '\0', sizeof(mccmnc_key));
    snprintf(mccmnc_key, sizeof(mccmnc_key), "%s", "mccmnc_");
    strcat(mccmnc_key, mcc_mnc);
    //ex:<mccmnc_46692>XXX</mccmnc_46692>
    //mccmnc_key is mccmnc_46692
    log_d("sync_data_storage_apn mccmnc_key:%s", mccmnc_key);
    int res = search_keyword_in_xml_header_num(mccmnc_key);
    log_d("sync_data_storage_apn res:%d", res);
    if (res == -1 || res == 0) {
        //res == -1 ==> data_storage.xml does not exist
        //res == 0 ==> <mccmnc_46692>XXX</mccmnc_46692> info not ready so wait for
        //init_apn_info() or init_default_apn_profile() finish
        log_d("sync_data_storage_apn return");
        return;
    }
    //modem now get info
    sync_profile.apn = getApnName();
    sync_profile.user_name = getUserName();
    sync_profile.password = getPassword();
    sync_profile.pdp_type = getPdpType();
    sync_profile.sim_mcc = mcc;
    sync_profile.sim_mnc = mnc;
    sync_profile.profile_name = get_mcc_mnc_default_profile_name(mcc_mnc);//ex:CHT
    log_d("sync_data_storage_apn profile_name:%s",sync_profile.profile_name);
    log_d("sync_data_storage_apn apn:%s",sync_profile.apn);
    log_d("sync_data_storage_apn user_name:%s",sync_profile.user_name);
    log_d("sync_data_storage_apn password:%s",sync_profile.password);
    log_d("sync_data_storage_apn pdp_type:%s",sync_profile.pdp_type);
    log_d("sync_data_storage_apn sim_mcc:%s",sync_profile.sim_mcc);
    log_d("sync_data_storage_apn sim_mnc:%s",sync_profile.sim_mnc);
    if ((strcmp(get_profile_apn(sync_profile.profile_name, mcc,mnc),getApnName()) != 0)||
        (strcmp(get_profile_user_name(sync_profile.profile_name, mcc,mnc),getUserName()) != 0)||
        (strcmp(get_profile_password(sync_profile.profile_name, mcc,mnc),getPassword()) != 0)||
        (strcmp(get_profile_pdptype(sync_profile.profile_name, mcc,mnc),getPdpType()) != 0)) {
            //note:update data_storage.xml apn and related <mccmnc_xxxxx>AAA</mccmnc_xxxxx> data
            //ex:<mccmnc_46692>CHT</mccmnc_46692>
            //if modem get apn,username,pw,pdptype are different from data_storage.xml
        if (search_apn_by_mcc_mnc_profilename(sync_profile.profile_name, mcc, mnc) == true) {
            log_d("sync_data_storage_apn update_profile_name_node");
            update_profile_name_node(sync_profile.profile_name, sync_profile, mcc, mnc);
        }
    }
}

void get_mcc_mnc_info() {
    memset(mcc, '\0', sizeof(mcc));
    memset(mnc, '\0', sizeof(mnc));
    memset(mcc_mnc, '\0', sizeof(mcc_mnc));
    #if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
        get_sim_mcc_mnc(mcc, MCC_MNC_BUFF_LEN, mnc, MCC_MNC_BUFF_LEN);
    #else
        //only for UI test
        strcpy(mcc,"466");
        strcpy(mnc,"92");
    #endif
    snprintf(mcc_mnc, sizeof(mcc_mnc), "%s%s", mcc, mnc);
}

void set_mcc_mnc_default_profile_name(char* mcc_mnc, char* profile_name) {
    log_d("set_mcc_mnc_default_profile_name profile_name:%s", profile_name);
    memset(mccmnc_key, '\0', sizeof(mccmnc_key));
    snprintf(mccmnc_key, sizeof(mccmnc_key), "%s", "mccmnc_");
    strcat(mccmnc_key, mcc_mnc);
    //ex:mccmnc_key ==>mccmnc_46692 value ==>profile_name
    //<mccmnc_46692>CHT</mccmnc_46692>
    int escapechar_num = check_contain_escapechar_num(profile_name);//check contain & number
    char outstr[(strlen(profile_name) + 4 * escapechar_num) + 1];//& need to change from & to &amp;
    memset(outstr, '\0', sizeof(outstr));
    //do below convert if profile_name contain &
    //ex:<mccmnc_46692>at&amp;t</mccmnc_46692>
    //i=0 ==>at&t compare a vs & => copy a =>a
    //i=1 ==>t&t compare t vs & => copy t =>at
    //i=2 ==>&t compare & vs & => copy &amp; =>at&amp;
    //i=3 ==>t compare t vs & => copy t =>at&amp;t
    if (escapechar_num > 0) {
        int i = 0;
        for (i = 0; i < strlen(profile_name); i++) {
            if (!strncmp((profile_name + i), "&", strlen("&"))) {
                strcat(outstr, "&amp;");
                i += strlen("&") - 1;
            } else {
                strncat(outstr, profile_name + i, 1);
            }
        }
    }
    log_d("set_mcc_mnc_default_profile_name escapechar_num:%d", escapechar_num);
    log_d("set_mcc_mnc_default_profile_name outstr:%s", outstr);
    log_d("set_mcc_mnc_default_profile_name profile_name:%s", profile_name);
    ds_set_value(mccmnc_key, (escapechar_num > 0 ? outstr : profile_name));
}

char* get_mcc_mnc_default_profile_name(char* mcc_mnc) {
    memset(mccmnc_key, '\0', sizeof(mccmnc_key));
    snprintf(mccmnc_key, sizeof(mccmnc_key), "%s", "mccmnc_");
    strcat(mccmnc_key, mcc_mnc);
    //ex:mccmnc_key ==>mccmnc_46692 value ==>CHT
    //<mccmnc_46692>CHT</mccmnc_46692>
    //return profile_name is CHT
    profile_name_value = ds_get_value(mccmnc_key);
    log_d("get_mcc_mnc_default_profile_name Start profile_name_value:%s", profile_name_value);
    if (!strcmp(profile_name_value, "")) {
        set_mcc_mnc_default_profile_name(mcc_mnc, "Predefined");
        profile_name_value = ds_get_value(mccmnc_key);
    }
    log_d("get_mcc_mnc_default_profile_name End profile_name_value:%s", profile_name_value);
    return profile_name_value;
}

//modify modem apn by profile name,mcc,mnc
int modify_modem_apn(char *apn, char *username, char *password, char *pdp_type) {
    //insert to modem
    modify_profile_settings modify_profile;
    memset(&modify_profile, '\0', sizeof(modify_profile_settings));
    modify_profile.apn_name = apn;
    modify_profile.username = username;
    modify_profile.password = password;
    modify_profile.pdp_type = pdp_type;
    log_d("modify_modem_apn modify_profile.apn_name:%s", modify_profile.apn_name);
    log_d("modify_modem_apn modify_profile.username:%s", modify_profile.username);
    log_d("modify_modem_apn modify_profile.password:%s", modify_profile.password);
    log_d("modify_modem_apn modify_profile.pdp_type:%s", modify_profile.pdp_type);
    return modify_apn_profile(&modify_profile);
}

void profiles_reached_limit_close_action(lv_obj_t * mbox, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;
    close_popup();
}

void style_profile_text_create(void) {
    lv_style_copy(&style_profile_l, &lv_style_plain);
    style_profile_l.text.font = get_locale_font_cust(font_w_bold, font_h_22);
    style_profile_l.text.letter_space = 1;
    lv_style_copy(&style_profile_rtl, &lv_style_plain);
    style_profile_rtl.text.font = get_locale_font(AR, font_w_bold, font_h_22);
}

//compute the escape & number
int check_contain_escapechar_num(const char * input) {
    bool contain_escapechar = false;
    int num = 0;
    uint32_t length = strlen(input);
    uint32_t i = 0;
    uint32_t letter;
    while (i < length) {
        letter = lv_txt_encoded_next(input, &i);
        if (letter == 0x26) {//contain &
            num++;
        }
    }
    return num;
}

void warning_popup_close(lv_obj_t * mbox, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;
    close_popup();
}

void warning_popup(int id) {
    static const char *btns[2];
    btns[0] = get_string(ID_OK);
    btns[1] = "";
    popup_anim_not_create(get_string(id), btns, warning_popup_close, NULL);
}

void set_orig_mcc(char* mcc) {
    log_d("set_orig_mcc mcc:%s", mcc);
    ds_set_value(DS_KEY_ORIG_MCC, mcc);
}

char* get_orig_mcc() {
    memset(orig_mcc, '\0', sizeof(orig_mcc));
    strncpy(orig_mcc, ds_get_value(DS_KEY_ORIG_MCC), 3);
    return orig_mcc;
}

void set_orig_mnc(char* mnc) {
    log_d("set_orig_mnc mnc:%s", mnc);
    ds_set_value(DS_KEY_ORIG_MNC, mnc);
}

char* get_orig_mnc() {
    memset(orig_mnc, '\0', sizeof(orig_mnc));
    strncpy(orig_mnc, ds_get_value(DS_KEY_ORIG_MNC), 3);
    return orig_mnc;
}

void set_mccmnc_default_apn_to_modem(char* profile_name, char* mcc, char* mnc) {
    log_d("set_mccmnc_default_apn_to_modem profile_name:%s", profile_name);
    log_d("set_mccmnc_default_apn_to_modem mcc:%s", mcc);
    log_d("set_mccmnc_default_apn_to_modem mnc:%s", mnc);
    if (search_apn_by_mcc_mnc_profilename(profile_name, mcc, mnc) == true) {
        modify_modem_apn(get_profile_apn(profile_name, mcc, mnc),
                get_profile_user_name(profile_name, mcc, mnc),
                get_profile_password(profile_name, mcc, mnc),
                get_profile_pdptype(profile_name, mcc, mnc));
    }
}

void reestablish_data(void *arg) {
    pthread_mutex_lock(&reconnect_mutex);
    set_reestablish_data_state(true);
    reestablish_data_connection();
    set_reestablish_data_state(false);
    pthread_mutex_unlock(&reconnect_mutex);
    pthread_exit(NULL);
}

bool set_reestablish_data_state(bool state) {
    log_d("set_reestablish_data_state state:%d",state);
    reestablish_data_enable = state;
}

bool get_reestablish_data_state() {
    return reestablish_data_enable;
}

void reestablish_data_task() {
    pthread_t reestablishdataThread;
    pthread_mutex_init(&reconnect_mutex,NULL);
    int res = pthread_create(&reestablishdataThread, NULL, reestablish_data, NULL);
    if (res) log_e("create reestablish_data fail\n");
    pthread_mutex_destroy(&reconnect_mutex);
}
