#include <stdio.h>
#include <stdbool.h>

#include <libxml/tree.h>
#include <libxml/xmlsave.h>
#include <libxml/xmlwriter.h>

#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/util/page_anim.h"
#include "lv_pocket_router/src/launcher.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/sms/sms_store.h"
#include "lv_pocket_router/src/sms/sms.h"

#define MAX_SMS_LISTE           60
#define MAX_UNREAD_SMS          99
#define MAX_UNREAD_SMS_LEN      4
#define SMS_RELOAD_INTERVAL     5000
#define SMS_RELOAD_TIMEOUT      5000

lv_obj_t * sms_win;
lv_obj_t * sms_list;
lv_obj_t * no_msg_label;


typedef struct {
    int id;
    int sim_id;
    lv_obj_t * obj;
} SMS;

static SMS sms_threads[MAX_SMS_LISTE];
int select_open_index = -1;

int sms_checked_map[MAX_SMS_LISTE];
int SMS_CNT;
int SMS_SELECT_CNT;
bool SMS_SELECT_MODE;
bool sms_back_key_b = false;

lv_obj_t * gar_btn;
lv_obj_t * win = NULL;
char unread_num[MAX_UNREAD_SMS_LEN];

//for sms reload function
static bool sms_init_done = false;
static bool sms_reload_enable = false;
static bool sms_reload_popup_enable = false;
lv_task_t * sms_reload_task;

//return sms unread num for launcher to show
char * get_sms_unread_num(){
    memset(unread_num, '\0', sizeof(unread_num));
    int unread = get_sms_unread();
    if(unread > MAX_UNREAD_SMS){
        //show 99+ if has over 99 unread sms
        char limit[MAX_UNREAD_SMS_LEN];
        memset(limit, '\0', sizeof(limit));
        snprintf(limit, sizeof(limit), "%d%s", MAX_UNREAD_SMS, "+");
        sprintf(unread_num, "%s", limit);
    }else{
        sprintf(unread_num, "%d", unread);
    }
    return unread_num;
}

//to uodate sms list title while in select mode
void update_sms_list_title(){
    char* sms_str_s = "(";
    char* sms_str_m = "/";
    char* sms_str_e = ")";
    const char* sms_str = get_string(ID_LAUNCHER_SMS);
    int len = strlen(sms_str) + 10;
    char title[len];
    memset(title, '\0', sizeof(title));
    if (SMS_SELECT_CNT == 0) {
        sprintf(title, "%s", sms_str);
    } else {
        sprintf(title, "%s%s%d%s%d%s", sms_str, sms_str_s,
                SMS_SELECT_CNT, sms_str_m, SMS_CNT, sms_str_e);
    }
    lv_btn_set_state(gar_btn, (SMS_CNT > 0) ? LV_BTN_STATE_REL : LV_BTN_STATE_INA);

    lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
    lv_label_set_text(ext->title, title);
    lv_obj_align(ext->title, ext->header, LV_ALIGN_CENTER, 0, 0);
}

//show the popup for delete action that triggered from opened sms
void sms_del_action(lv_obj_t * list_btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    static const char *btns[3];
    btns[0] = get_string(ID_CANCEL);
    btns[1] = get_string(ID_OK);
    btns[2] = "";

    popup_anim_que_create(get_string(ID_SMS_DEL_ONE_MSG), btns,
            sms_pop_del_comfirm_action, NULL);
}

//show the popup for delete action that triggered from sms list
void sms_list_del_action(lv_obj_t * list_btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    //return if sms list empty
    if(lv_btn_get_state(gar_btn) == LV_BTN_STATE_INA)return;

    static const char *btns[3];
    btns[0] = get_string(ID_CANCEL);
    btns[1] = get_string(ID_OK);
    btns[2] = "";

    int len = 0;
    int del_one_strlen = strlen(get_string(ID_SMS_DEL_ONE_MSG));
    int del_all_strlen = strlen(get_string(ID_SMS_DEL_ALL_MSG));

    if(del_one_strlen > del_all_strlen){
        len = del_one_strlen;
    }else{
        len = del_all_strlen;
    }
    char str[len];
    memset(str, '\0', sizeof(str));
    if (!SMS_SELECT_MODE) {
        sprintf(str, "%s", get_string(ID_SMS_DEL_ALL_MSG));
    } else {
        sprintf(str, "%s", get_string(ID_SMS_DEL_ONE_MSG));
    }
    popup_anim_que_create(str, btns, sms_pop_del_action, NULL);

}

void close_sms(bool launch_home)
{
    if (SMS_SELECT_MODE) {
        SMS_SELECT_MODE = false;
    }
    //close_all_lists(0);
    if (!sms_back_key_b) {
        sms_back_key_b = false;
        if(launch_home){
            launch_home_behaviour();
        }
    }
}

void sms_list_home_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    close_sms(true);
}

//to handle sms back key, if in select mode, change to showing read status
//in first time pressing back, if not, close sms main windows
void sms_list_back_action(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    if(SMS_SELECT_MODE){
        SMS_SELECT_MODE = false;
        SMS_SELECT_CNT = 0;
        refresh_sms_list();
        update_sms_list_title();
        // workaround backdoor solution, add btn back to list action cache list
        list_action_add_back_cache(btn, sms_list_back_action);
    } else {
        sms_back_key_b = true;
        lv_win_close_event_cb(btn, LV_EVENT_RELEASED);
        sms_back_key_b = false;
        update_sms_num();
        close_all_lists(0);
        win = NULL;
        sms_init_done = false;
        if (sms_reload_task != NULL) {
            lv_task_del(sms_reload_task);
            sms_reload_task = NULL;
        }
    }
}

//we check if user short/long press here
void sms_open_action(lv_obj_t * btn, lv_event_t event){
    if (event == LV_EVENT_SHORT_CLICKED){
        sms_short_press_action(btn, event);
    }else if(event == LV_EVENT_LONG_PRESSED){
        sms_long_press_action(btn, event);
    }
}

//handle long press action for sms list element
void sms_long_press_action(lv_obj_t * btn, lv_event_t event)
{
    int i = 0;
    int index = lv_obj_get_user_data(btn);

    xmlDocPtr doc = xmlReadFile(SMS_XML_PATH, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        log_e("Failed to read %s", SMS_XML_PATH);
        return;
    }
    xmlDocPtr cur = xmlDocGetRootElement(doc);
    xmlNodePtr curNode = cur->xmlChildrenNode;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST SMS_STORE_VERSION_KEY)) {
            curNode = curNode->next;
            continue;
        }
        if (xmlStrcmp(curNode->name, BAD_CAST SMS_HEADER)) {
            curNode = curNode->next;
            continue;
        }
        //skip if id=0, it means sms_threads[i].obj has been deleted, so go find next
        if (sms_threads[i].id == 0) {
            i++;
            continue ;
        }
        if (i == index) {
            lv_obj_t * img = lv_get_child_by_index(sms_threads[i].obj, 1);
            lv_img_set_src(img, &ic_list_checkbox_selected);
            SMS_SELECT_CNT = 1;
            SMS_SELECT_MODE = true;
            sms_checked_map[i] = CHECK;
        } else {
            lv_obj_t * img = lv_get_child_by_index(sms_threads[i].obj, 1);
            lv_img_set_src(img, &ic_list_checkbox);
            sms_checked_map[i] = UNCHECK;
        }
        curNode = curNode->next;
        i++;
    }
    xmlFreeDoc(doc);
    update_sms_list_title();
}

/*handle sms list element click action
 *if in select mode, it will check/uncheck the sms, support multi-select
 *if not in select mode, open the sms using info_page
 */
void sms_short_press_action(lv_obj_t * btn, lv_event_t event)
{
    int index = lv_obj_get_user_data(btn);
    xmlDocPtr doc = xmlReadFile(SMS_XML_PATH, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        log_e("Failed to read %s", SMS_XML_PATH);
        return;
    }
    select_open_index = -1;//reset
    xmlDocPtr cur = xmlDocGetRootElement(doc);
    xmlNodePtr curNode = cur->xmlChildrenNode;

    //open sms using info_page
    if (SMS_SELECT_CNT == 0 && !SMS_SELECT_MODE){
        select_open_index = index;
        while (curNode != NULL) {
            if (!xmlStrcmp(curNode->name, BAD_CAST SMS_STORE_VERSION_KEY)) {
                curNode = curNode->next;
                continue ;
            }
            if (xmlStrcmp(curNode->name, BAD_CAST SMS_HEADER)) {
                curNode = curNode->next;
                continue;
            }
            if (sms_threads[index].id == 0) {
                index++;
                continue ;
            }
            char* str_id = xmlGetProp(curNode,BAD_CAST SMS_ID);
            if (str_id != NULL && sms_threads[index].id == atoi(str_id)) {
                select_open_index = index;
                char* number = xmlGetProp(curNode,BAD_CAST SMS_PHONE_NUMBER);
                char* date = xmlGetProp(curNode,BAD_CAST SMS_DATE);
                char* content = xmlGetProp(curNode,BAD_CAST SMS_CONTENT);
                char* encoding = xmlGetProp(curNode,BAD_CAST SMS_ENCODING_TYPE);

                int encode = RIL_SMS_ENCODING_7BIT;
                if(encoding != NULL)
                    encode = atoi(encoding);
                int len = 0;
                if(date != NULL){
                    len = strlen(date);
                }
                if(content != NULL){
                    len += strlen(content);
                }
                char txt[len+2];
                sprintf(txt, "%s\n%s", date, content);
                sms_win = info_page_sms_create(lv_scr_act(), number, txt, sms_del_action, encode);
                lv_obj_t * img = lv_get_child_by_index(sms_threads[index].obj, 1);
                lv_img_set_src(img, &ic_list_sms_read);
                set_sms_read(sms_threads[index].id);

                xmlFree(number);
                xmlFree(date);
                xmlFree(content);
                xmlFree(str_id);
                xmlFree(encoding);
                break;
            }
            xmlFree(str_id);
            curNode = curNode->next;
        }
        xmlFreeDoc(doc);

    } else {
        //check or uncheck sms while in select mode
        lv_obj_t * img = lv_get_child_by_index(sms_threads[index].obj, 1);
        if (sms_checked_map[index] == UNCHECK){
            sms_checked_map[index] = CHECK;
            lv_img_set_src(img, &ic_list_checkbox_selected);
            SMS_SELECT_CNT++;
        } else {
            sms_checked_map[index] = UNCHECK;
            lv_img_set_src(img, &ic_list_checkbox);
            SMS_SELECT_CNT--;
        }
        update_sms_list_title();
    }
}

//refresh sms list to read/unread state
void refresh_sms_list(){
    int i = 0;
    xmlDocPtr doc = xmlReadFile(SMS_XML_PATH, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        log_e("Failed to read %s", SMS_XML_PATH);
        return;
    }
    xmlDocPtr cur = xmlDocGetRootElement(doc);
    xmlNodePtr curNode = cur->xmlChildrenNode;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST SMS_STORE_VERSION_KEY)) {
            curNode = curNode->next;
            continue ;
        }
        if (xmlStrcmp(curNode->name, BAD_CAST SMS_HEADER)) {
            curNode = curNode->next;
            continue;
        }
        if (sms_threads[i].id == 0) {
            i++;
            continue ;
        }
        char* read = xmlGetProp(curNode,BAD_CAST SMS_READ);
        lv_obj_t * img = lv_get_child_by_index(sms_threads[i].obj, 1);
        if (read != NULL && strcmp(read, "true") == 0){
            lv_img_set_src(img, &ic_list_sms_read);
        } else {
            lv_img_set_src(img, &ic_list_sms);
        }

        xmlFree(read);
        curNode = curNode->next;
        i++;
    }
    //disable garbage btn if all sms been deleted
    if(SMS_CNT == 0) {
        lv_btn_set_state(gar_btn, LV_BTN_STATE_INA);
        lv_obj_set_hidden(no_msg_label, false);
    }
    xmlFreeDoc(doc);
}

//handle the delete action from sms list, if
//i.e. in select mode or delete all from sms main page
void sms_pop_del_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(mbox);
    if (strcmp(txt, get_string(ID_OK)) == 0) {
        int i = 0;
        xmlDocPtr doc = xmlReadFile(SMS_XML_PATH, NULL, XML_PARSE_RECOVER);
        if (doc == NULL) {
            log_e("Failed to read %s", SMS_XML_PATH);
            return;
        }
        xmlDocPtr cur = xmlDocGetRootElement(doc);
        xmlNodePtr curNode = cur->xmlChildrenNode;
        while (curNode != NULL) {
            if (!xmlStrcmp(curNode->name, BAD_CAST SMS_STORE_VERSION_KEY)) {
                curNode = curNode->next;
                continue ;
            }
            if (xmlStrcmp(curNode->name, BAD_CAST SMS_HEADER)) {
                curNode = curNode->next;
                continue;
            }
            if (sms_threads[i].id == 0) {
                i++;
                continue ;
            }
            if (!SMS_SELECT_MODE || (SMS_SELECT_MODE && sms_checked_map[i] == CHECK)) {
                delete_sms(sms_threads[i].id, sms_threads[i].sim_id);
                lv_obj_del(sms_threads[i].obj);
                sms_threads[i].id = 0;
                sms_threads[i].sim_id = 0;
                sms_threads[i].obj = NULL;
                SMS_CNT--;
            }

            curNode = curNode->next;
            i++;
        }
        xmlFreeDoc(doc);

        if (!SMS_SELECT_MODE){
            lv_btn_set_state(gar_btn, LV_BTN_STATE_INA);
            lv_obj_set_hidden(no_msg_label, false);
            SMS_CNT = 0;
        } else {
            SMS_SELECT_MODE = false;
            SMS_SELECT_CNT = 0;
        }
        update_sms_list_title();
        refresh_sms_list();
    }
    close_popup();
}

//handle the delete action from opened sms
void sms_pop_del_comfirm_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(mbox);
    if (strcmp(txt, get_string(ID_OK)) == 0) {

        if (select_open_index != -1) {
            delete_sms(sms_threads[select_open_index].id,
                    sms_threads[select_open_index].sim_id);
            lv_obj_del(sms_threads[select_open_index].obj);
            sms_threads[select_open_index].id = 0;
            sms_threads[select_open_index].sim_id = 0;
            sms_threads[select_open_index].obj = NULL;
            select_open_index = -1;
            SMS_CNT--;
            refresh_sms_list();
        }
    }
    close_popup();
}

void sms_popup_trim(lv_obj_t * btn, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;
    close_popup();
}

//return how many need to be trimmed in sms xml
int get_trim_sms_cnt()
{
    int cnt = 0;
    xmlDocPtr doc = xmlReadFile(SMS_XML_PATH, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        log_e("Failed to read %s", SMS_XML_PATH);
        return 0;
    }
    xmlDocPtr cur = xmlDocGetRootElement(doc);
    xmlNodePtr curNode = cur->xmlChildrenNode;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST SMS_STORE_VERSION_KEY)) {
            curNode = curNode->next;
            continue;
        }
        if (xmlStrcmp(curNode->name, BAD_CAST SMS_HEADER)) {
            curNode = curNode->next;
            continue;
        }
        cnt++;
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);
    return (cnt > MAX_SMS_LISTE) ? (cnt - MAX_SMS_LISTE) : 0;
}

//trim sms xml to MAX_SMS_LISTE if exceeds MAX allowed
bool trim_sms_xml(){
    bool trim_sms = false;
    int cnt = get_trim_sms_cnt();
    if(cnt == 0) return false;
    log_d("sms will trim %d oldest sms in xml", cnt);

    xmlDocPtr doc = xmlReadFile(SMS_XML_PATH, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        log_e("Failed to read %s", SMS_XML_PATH);
        return false;
    }
    xmlDocPtr cur = xmlDocGetRootElement(doc);
    xmlNodePtr curNode = cur->xmlChildrenNode;
    if (curNode != NULL) {
        int i = 0;
        while (curNode != NULL) {
            if (!xmlStrcmp(curNode->name, BAD_CAST SMS_STORE_VERSION_KEY)) {
                curNode = curNode->next;
                continue ;
            }
            if (xmlStrcmp(curNode->name, BAD_CAST SMS_HEADER)) {
                curNode = curNode->next;
                continue;
            }
            int del_id = 0;
            int del_sim_id = 0;
            char* id = xmlGetProp(curNode,BAD_CAST SMS_ID);
            char* sim_id = xmlGetProp(curNode,BAD_CAST SMS_REC_NUM);
            if(id != NULL) del_id = atoi(id);
            if(sim_id != NULL) del_sim_id = atoi(sim_id);

            //delete sms if over max allowed
            if (del_id > 0 && cnt > i) {
                delete_sms(del_id, del_sim_id);
                trim_sms = true;
            } else{
                xmlFree(id);
                xmlFree(sim_id);
                break;
            }
            xmlFree(id);
            xmlFree(sim_id);
            curNode = curNode->next;
            i++;
        }
    }
    xmlFreeDoc(doc);
    return trim_sms;
}

//sms reload static popup callback
void reload_popup_cb(){
    log_d("[sms reload] create reload popup");
    popup_anim_not_create(get_string(ID_SMS_RELOADING_MSG), NULL, NULL, NULL);
    set_static_popup(true);
    lv_task_t * task = lv_task_create(close_static_popup, SMS_RELOAD_TIMEOUT, LV_TASK_PRIO_MID, NULL);
    lv_task_once(task);
}

void enable_reload_static_popup(){
    if(win != NULL && sms_init_done){
        log_d("[sms reload] reload popup enabled, will create soon");
        sms_reload_popup_enable = true;
    }
}

//enable sms reload flag is sms opened, will reload every 5000ms
void enable_sms_reload(){
    if(win != NULL && sms_init_done){
        log_d("[sms reload] reload enabled, will start reload soon");
        sms_reload_enable = true;
    }
}

//for reload, we close sms and init it again
void sms_reload(){
    if(sms_reload_popup_enable){
        create_static_popup(reload_popup_cb);
    }
    if(!sms_reload_enable){
        return;
    }
    if(win == NULL || !sms_init_done){
        log_d("[sms reload] Skip sms reload due to sms not ready");
        return;
    }
    log_d("[sms reload] sms reload start");
    //close sms info page and popup if any
    SMS_CNT = 0;
    SMS_SELECT_CNT = 0;
    SMS_SELECT_MODE = false;
    sms_reload_enable = false;
    sms_reload_popup_enable = false;
    close_popup();
    close_all_pages();
    close_all_lists(0);
    win = NULL;
    if (sms_reload_task != NULL) {
        lv_task_del(sms_reload_task);
        sms_reload_task = NULL;
    }
    init_sms(false);
    move_static_popup_to_foreground();
    log_d("[sms reload] sms reload done, SMS_CNT: %d", SMS_CNT);
}

//create sms page
void init_sms(bool show_anim) {

    //start sms reload task to monitor if sms need reload
    sms_reload_task = lv_task_create(sms_reload, SMS_RELOAD_INTERVAL, LV_TASK_PRIO_MID, NULL);
    //clean sms storage xml every time sms init
    clean_sms_xml();

    liste_style_create();
    SMS_CNT = 0;
    SMS_SELECT_CNT = 0;
    SMS_SELECT_MODE = false;

    win = sms_list_header_create(lv_scr_act(), get_string(ID_LAUNCHER_SMS), sms_list_back_action, sms_list_del_action);
    lv_win_ext_t * ext = lv_obj_get_ext_attr(win);
    gar_btn = lv_obj_get_child(ext->header, NULL);

#if !defined (FEATURE_ROUTER)
    SMS_THREAD thread;
    thread.content= get_string(ID_WIFI_INFORMATION);
    thread.date="2019/7/8 14:23";
    thread.in_sim="true";
    thread.number="088888555";
    thread.rec_num="0";
    thread.encoding_type="2";
    thread.read="true";
    SMS_THREAD thread1;
    thread1.content=get_string(ID_CONN_GUIDE_MANUAL_INFO);
    thread1.date="2019/10/8 8:55";
    thread1.in_sim="false";
    thread1.number="0666688888";
    thread1.rec_num="0";
    thread1.encoding_type="2";
    thread1.read="false";
    write_new_sms(thread);
    write_new_sms(thread1);
#endif

    //trim xml if sms exceed MAX_SMS_LISTE
    if(trim_sms_xml() && !is_static_popup()){
        log_d("display popup to inform user we trim sms");
        static const char *btns[2];
        btns[1] = "";
        btns[0] = get_string(ID_OK);
        popup_anim_not_long_create(get_string(ID_SMS_TRIM_WARNING_MSG), btns,
                sms_popup_trim, NULL);
    }

    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;

    sms_list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(sms_list, LV_SB_MODE_OFF);
    lv_list_set_style(sms_list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(sms_list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);
    lv_obj_set_size(sms_list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(sms_list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(sms_list, LV_LAYOUT_OFF);

    static lv_style_t style_label;
    lv_style_copy(&style_label, &lv_style_plain);
    style_label.text.color = LV_COLOR_GREYISH_BROWN;
    style_label.text.font = get_font(font_w_bold, font_h_16);
    style_label.text.letter_space = 1;
    no_msg_label = lv_label_create(win, NULL);
    lv_label_set_text(no_msg_label, get_string(ID_SMS_NO_MESSAGE));
    lv_label_set_style(no_msg_label, LV_LABEL_STYLE_MAIN, &style_label);
    lv_obj_align(no_msg_label, win, LV_ALIGN_CENTER, 0, 15);
    lv_btn_set_state(gar_btn, LV_BTN_STATE_INA);

    xmlDocPtr doc = xmlReadFile(SMS_XML_PATH, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        log_e("Failed to read %s", SMS_XML_PATH);
        return ;
    }
    xmlDocPtr cur = xmlDocGetRootElement(doc);
    xmlNodePtr curNode = cur->xmlChildrenNode;

    if (curNode != NULL) {
        int i = 0;
        while (curNode != NULL) {
            if (!xmlStrcmp(curNode->name, BAD_CAST SMS_STORE_VERSION_KEY)) {
                curNode = curNode->next;
                continue ;
            }
            if (xmlStrcmp(curNode->name, BAD_CAST SMS_HEADER)) {
                curNode = curNode->next;
                continue;
            }
            char* id = xmlGetProp(curNode,BAD_CAST SMS_ID);
            char* date = xmlGetProp(curNode,BAD_CAST SMS_DATE);
            char* content = xmlGetProp(curNode,BAD_CAST SMS_CONTENT);
            char* read = xmlGetProp(curNode,BAD_CAST SMS_READ);
            char* sim_id = xmlGetProp(curNode,BAD_CAST SMS_REC_NUM);
            char* encoding = xmlGetProp(curNode,BAD_CAST SMS_ENCODING_TYPE);

            if(id != NULL)
                sms_threads[i].id = atoi(id);
            if(sim_id != NULL)
                sms_threads[i].sim_id = atoi(sim_id);

            int encode = RIL_SMS_ENCODING_7BIT;
            if(encoding != NULL)
                encode = atoi(encoding);

            int state = 0;
            if(read != NULL && (strcmp(read, "true") == 0)){
                state = 1;
            }

            sms_threads[i].obj = lv_liste_sms(sms_list, date, content, state, i, encode);
            lv_obj_set_event_cb(sms_threads[i].obj, sms_open_action);
            xmlFree(id);
            xmlFree(date);
            xmlFree(content);
            xmlFree(read);
            xmlFree(sim_id);
            xmlFree(encoding);

            i++;
            SMS_CNT++;
            curNode = curNode->next;
        }
    }
    xmlFreeDoc(doc);

    if(SMS_CNT > 0) {
        lv_btn_set_state(gar_btn, LV_BTN_STATE_REL);
        lv_obj_set_hidden(no_msg_label, true);
    }
    //show page to page exit part
    if(show_anim) {
        page_anim_exit();
    }
    //will do sms reload after sms init done
    sms_init_done = true;

}
