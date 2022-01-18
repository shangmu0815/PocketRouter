
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>

#include "lv_pocket_router/src/about/device_information.h"
#include "lv_pocket_router/src/display/display.h"
#include "lv_pocket_router/src/util/convert_string.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/info_page.h"
#include "lv_pocket_router/src/util/list_action.h"
#include "lv_pocket_router/src/util/popup_box.h"
#include "lv_pocket_router/src/settings/update.h"
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/liste_style.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/res/values/string_value.h"

lv_obj_t * win;
#define FILE_BUF_SIZE                   1024

#ifdef CUST_DLINK
#ifdef FEATURE_ROUTER
#define DLINK_UPDATE_RES_FILE           "/data/misc/dfota/var/dfota/new_fw_info.json"
#else
#define DLINK_UPDATE_RES_FILE           "Data_Store/new_fw_info.json"
#endif

#define DLINK_VALUE_LEN                 80
#define DLINK_INFO_LEN                  120

bool update_available = false;
int update_size;
int status_code;
char update_version[DLINK_VALUE_LEN];
char update_info[DLINK_INFO_LEN * 3];
void dlink_update_create(bool reset);
void dlink_parse_update_info(bool parse_all);
void dlink_check_update_action(lv_obj_t * btn, lv_event_t event);
char* dlink_get_info();

#endif /* CUST_DLINK */
static lv_obj_t * check_for_update_btn;
static lv_style_t style_btn_rel;
static lv_style_t style_btn_pr;
static lv_style_t style_font;

lv_obj_t * label;
lv_obj_t * mbox;
lv_obj_t * update_sw;
lv_task_t * fotaclient_task;

bool downloading;
float loading;
uint32_t dl_start_timestamp;

pthread_t timer_conf_thread;

#define ANIM_INTERVAL                   50
#define LOADING_COUNT                   (20000/ANIM_INTERVAL)
#define WAIT_COUNT                      (1500/ANIM_INTERVAL)

#ifdef FEATURE_ROUTER
#define FC_NEED_UPGRADE_FLAG_FILEPATH   "/data/misc/fota_client/need_upgrade.flag"
#define FC_NEED_UPDATE_VERSION_FLAG_FILEPATH "/data/misc/fota_client/need_update_version.flag"
#define FC_DATA_STORE_FILE              "/data/misc/fota_client/fota_data.xml"
#define FC_UPDATE_TIME                  "/data/misc/fota_client/update_time.txt"
#define FC_UPGRADE_INFO                 "/data/misc/fota_upgrade_info.xml"
#else
#define FC_NEED_UPGRADE_FLAG_FILEPATH   "Data_Store/need_upgrade.flag"
#define FC_NEED_UPDATE_VERSION_FLAG_FILEPATH "Data_Store/need_update_version.flag"
#define FC_DATA_STORE_FILE              "Data_Store/fota_data.xml"
#define FC_UPDATE_TIME                  "Data_Store/update_time.txt"
#define FC_UPGRADE_INFO                 "Data_Store/fota_upgrade_info.xml"
#endif

bool meet_upgrade_precondition() {
    if (!is_charging() && get_battery_info() < 50) {
        return false;
    } else {
        return true;
    }
}

bool get_available_updates_state() {
#ifdef CUST_DLINK
    dlink_parse_update_info(false);
    return update_available;
#else
    return (meet_upgrade_precondition() && fota_need_upgrade());
#endif
}

int get_fota_info_result(char* buf, int res_len) {
    FILE *fp = NULL;
    int rlen;

    fp = fopen(FC_UPDATE_TIME, "r");
    if (fp == NULL) {
        log_e("open fota update time file failed!");
        return -1;
    }
    rlen = fread(buf, 1, res_len, fp);
    buf[rlen - 1] = '\0';
    fclose(fp);
    fp = NULL;

    char cmd[80];
    snprintf(cmd, 80, "grep -r %s.*success %s", buf, FC_UPGRADE_INFO);
    fp = popen(cmd, "r");
    if (fp == NULL) {
        log_e("look up fota upgrade info failed!");
        return -1;
    }
    memset(buf, 0, sizeof(buf));
    rlen = fread(buf, 1, res_len, fp);
    pclose(fp);
    fp = NULL;

    return rlen;
}

bool need_to_show_fota_result() {
    char buf[FILE_BUF_SIZE] = {0};

    int rlen = get_fota_info_result(buf, FILE_BUF_SIZE);
    if (rlen == 0) {
        // for WebUI workaround
        struct stat buffer;
        int exist = stat("/data/www/access.log", &buffer);
        if (exist == 0) {
            systemCmd("rm /data/www/access.log");
            log_d("Remove access.log");
        }

        log_d("FOTA upgrade were performed in last boot, check upgrade result now");
        return true;
    } else {
        return false;
    }
}

void fota_result_prompt() {
    int res = systemCmd("fotaclient -r");
    log_d("update result check cmd ret:%d", res);

    char buf[FILE_BUF_SIZE] = {0};
    int rlen = get_fota_info_result(buf, FILE_BUF_SIZE);

    if (rlen > 0) {
        char *res = strstr(buf, "success=\"");
        if (res != NULL) {
            res += strlen("success=\"");
            if (strncmp(res, "true", strlen("true")) == 0) {
                update_result_popup(ID_UPDATE, ID_UPDATE_FW_UPGRADE_SUCCESS);
            } else {
                update_result_popup(ID_UPDATE, ID_UPDATE_FW_UPGRADE_FAILED);
            }
        }
    }
}

#ifdef CUST_DLINK
void trim_update_info(char *s){
    bool pasring = false;
    int j, i, n = strlen(s);
    for (i=j=0; i<n; i++){
        if (!pasring && (s[i] == ' ')){
            //skip space before real parsing begin
            continue;
        }
        if ((s[i] != ',') && (s[i] != '"') && (s[i] != '\n') && (s[i] != '	')){
            s[j++] = s[i];
            pasring = true;
        }
    }
    s[j] = '\0';
}

void dlink_update_ret_msg(int res){
    switch(res)
    {
        case UPDATE_INTERNAL_ERR:
            log_d("[update] Internal Error");
            break;
        case UPDATE_AVAILABLE_BIN:
            log_d("[update] Has new available FW binary.");
            break;
        case UPDATE_NO_BIN:
            log_d("[update] No FW binary");
            break;
        case UPDATE_ERR_2ND_XML:
            log_d("[update] Error to access the 2 nd XML file in the remote server");
            break;
        case UPDATE_ERR_SKU_CHECKING:
            log_d("[update] Error to the device SKU checking");
            break;
        case UPDATE_ERR_XML_SCHEME:
            log_d("[update] Error in XML scheme");
            break;
        case UPDATE_ERR_UPDATE_INFO_SEC:
            log_d("[update] Error in the UpdateInfo section");
            break;
        case UPDATE_ERR_UPDATE_SEQ_SEC:
            log_d("[update] Error in the UpdateSequence section");
            break;
        case UPDATE_ERR_COPONENT_COUNT:
            log_d("[update] Error in update components count.");
            break;
        case UPDATE_ERR_UPDATE_SRC_SEC:
            log_d("[update] Error in UpdateSource section");
            break;
        case UPDATE_ERR_STORE_LOC:
            log_d("[update] Error for nUpdateStoreLocationError");
            break;
        case UPDATE_ERR_STORE_LOC_SPACE:
            log_d("[update] Error for nUpdateStoreLocationSpaceError.");
            break;
        case UPDATE_ERR_1ST_XML:
            log_d("[update] Error to access the 1 st XML file");
            break;
        case UPDATE_UPGRADE_SUCCESS:
            log_d("[update] Upgrade successful");
            break;
        case UPDATE_ERR_DOWNLOAD_FWFILE:
            log_d("[update] Error to download the FW file");
            break;
        case UPDATE_ERR_UNZIP_FWFILE:
            log_d("[update] Error to unzip the FW file");
            break;
        case UPDATE_ERR_APPLY_FWBIN:
            log_d("[update] Error to apply the new FW binary");
            break;
        default:
            log_d("[update] Unknown error");
            break;
    }
}

//only parse new firmware available if parse_all set to false
void dlink_parse_update_info(bool parse_all){
    char buffer[FILE_BUF_SIZE] = {0};
    char found[DLINK_VALUE_LEN] = {0};
    char * ptr = NULL;
    char * end_ptr = NULL;
    int  i, shift, rlen;
    int len = parse_all ? update_list_len : 1;
    FILE *fp = fopen(DLINK_UPDATE_RES_FILE, "r");
    if (fp == NULL) {
        log_e("[update] %s popen failed", DLINK_UPDATE_RES_FILE);
        return;
    }
    rlen = fread(buffer, 1, FILE_BUF_SIZE, fp);
    buffer[rlen-1] = '\0';

    for(i = 0; i < len; i++){
        ptr = strstr(buffer, parse_all ?
                update_list_map[i] : update_list_map[NEW_FIRMWARE_AVAILABLE]);
        if (ptr == NULL || strlen(ptr) == 0) {
            log_e("[update] skipped %s, field missing", update_list_map[i]);
            continue;
        }
        end_ptr = strchr(ptr, '\n');
        if (end_ptr == NULL || strlen(end_ptr) == 0) {
            log_e("[update] skipped %s, missing end line", update_list_map[i]);
            continue;
        }

        shift = (strcspn(ptr, ":") + 1);
        memset(found, 0, sizeof(found));
        strncpy(found, ptr+shift, strlen(ptr)-strlen(end_ptr)-shift);
        trim_update_info(found);

        if(!parse_all || (i == NEW_FIRMWARE_AVAILABLE)){
            if(strcmp(found, "true") == 0){
                update_available = true;
            }else{
                update_available = false;
            }
            log_d("[update] update available: %s", found);

        }else if(i == LAST_CHECK_TIME){
            ds_set_value(DS_KEY_UPDATE_LAST_CHECK_TIME, found);
            log_d("[update] last check time: %s", found);

        }else if(i == VERSION){
            strcpy(update_version, found);
            log_d("[update] update version: %s", update_version);

        }else if(i == FILE_SIZE){
            update_size = (atoi(found) / (1024*1024));
            log_d("[update] update size: %dMB", update_size);

        }else if(i == STATUS_CODE){
            status_code = atoi(found);
            log_d("[update] status code: %d", status_code);
            if(status_code > 0){
                //print update return message log
                dlink_update_ret_msg(status_code);
            }
        }
    }
    fclose(fp);
}

char* dlink_get_info(){
    char ver_info[DLINK_INFO_LEN]={'\0'};
    char new_ver_info[DLINK_INFO_LEN]={'\0'};
    char last_checked_info[DLINK_INFO_LEN]={'\0'};
    char extra_info[DLINK_INFO_LEN]={'\0'};
    char * last_checked = ds_get_value(DS_KEY_UPDATE_LAST_CHECK_TIME);
    bool last_check_exist = strlen(last_checked) > 0 ? true:false;

    if(last_check_exist){
        snprintf(last_checked_info, sizeof(last_checked_info), "%s:\n    %s",
            get_string(ID_UPDATE_LAST_CHECKED), last_checked);
    }

    snprintf(ver_info, sizeof(ver_info), "%s:\n    %s\n    %s",
            get_string(ID_UPDATE_CURRENT_VERSION) ,
            getSwVersion(), getModuleSwVersion());

    if(strlen(update_version) > 0){
        snprintf(new_ver_info, sizeof(new_ver_info), "%s (%dMB)\n    %s",
                get_string(ID_UPDATE_NEW_VERSION_DLINK), update_size, update_version);
    }

    if ((status_code == UPDATE_AVAILABLE_BIN) && update_available) {
        strcpy(extra_info, new_ver_info);
    } else {
        snprintf(extra_info, sizeof(extra_info), "%s (%d)",
                get_string(ID_UPDATE_UP_TO_DATE), status_code);
    }

    if(strlen(extra_info) > 0){
        snprintf(update_info, sizeof(update_info), "%s\n%s\n%s",
                extra_info, last_checked_info, ver_info);
    } else if(last_check_exist){
        snprintf(update_info, sizeof(update_info), "%s\n%s",
                last_checked_info, ver_info);
    }else{
        snprintf(update_info, sizeof(update_info), "%s", ver_info);
    }
    return update_info;
}

void dlink_battery_warning_close(lv_obj_t * mbox, lv_event_cb_t event_cb)
{
    if (event_cb != LV_EVENT_CLICKED) return;
    close_popup();
}

void dlink_updating() {
    // check status from log file to see if should turn off backlight before enter recovery mode
    FILE *file_p = popen("tail -n 5 /data/misc/dfota/var/dfota/dfota.log | grep \"Start to upgrade firmware\"", "r");
    if (file_p != NULL) {
        char buffer[100];
        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, sizeof(buffer), file_p);
        if (strlen(buffer) != 0) {
            log_d("dlink dfota going to Start to upgrade firmware");
            backlight_off();
        }
        pclose(file_p);
    }

    reset_screen_timeout();

    FILE *fp = popen("ps | grep \"[d]fota -u\"", "r");
    if (fp == NULL) {
        log_e("grep dfota process failed");
    }

    char buffer[MAX_DATA_LENGTH];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    if (strlen(buffer) == 0) {
        log_d("dlink dfota upgrade finish");
        update_result_popup(ID_UPDATE_CHECKING_FOR_UPDATES, ID_UPDATE_CHECK_FAILED);
    }
    pclose(fp);
}

void dlink_upgrade_static_cb(){
    lv_obj_t * mbox = popup_anim_loading_create(get_string(ID_UPDATE_NEW_VERSION_DLINK), get_string(ID_UPDATE_UPDATING));
    // issue 8. FOTA: warning wording font size in large
    update_anim_loading_msg_font(mbox, font_w_bold, font_h_24);
    popup_loading_task_create(dlink_updating, 1000, LV_TASK_PRIO_MID, NULL);
    hide_loading_bar(mbox);
    set_static_popup(true);
}

void dlink_upgrade_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    if (!meet_upgrade_precondition()) {
        static const char * btns[2];
        btns[0] = get_string(ID_OK);
        btns[1] = "";

        popup_scrl_create(get_string(ID_UPDATE), get_string(ID_UPDATE_BATTERY_WARNING),
              btns, dlink_battery_warning_close);
        return;
    }

    int res = systemCmd("dfota -u &");
    log_d("[update] dlink download cmd ret:%d", res);

#if (0)
    //close current page
    info_page_close_win(win);

    //TODO temp UI, will change later
    win = info_page_create_btmn(lv_scr_act(), get_string(ID_UPDATE),
            "Image downloading...", NULL, NULL);
#else
    create_static_popup(dlink_upgrade_static_cb);
#endif
}

void dlink_check_update_action(lv_obj_t * btn, lv_event_t event){
    if (event != LV_EVENT_CLICKED) return;

    int res = systemCmd("dfota -c");
    log_d("[update] dlink update cmd ret:%d", res);

    //parse json file
    dlink_parse_update_info(true);

    //close current page
    info_page_close_win(win);

    dlink_update_create(false);
}

void dlink_update_create(bool reset){
    int id = ID_UPDATE_CHECKING_FOR_UPDATES;
    lv_event_cb_t action = dlink_check_update_action;

    if(reset){
        update_available = false;
        update_size = 0;
        status_code = -1;
        memset(update_version, 0, sizeof(update_version));
        memset(update_info, 0, sizeof(update_info));
        // parse previous info status
        dlink_parse_update_info(true);
    }

    if (update_available) {
        id = ID_UPDATE_UPGRADE_IMAGE;
        action = dlink_upgrade_action;
    }

    static const char *btns[2];
    btns[0] = get_string(id);
    btns[1] = "";

    win = info_page_create_btmn(lv_scr_act(), get_string(ID_UPDATE),
            dlink_get_info(), btns, action);
}

#endif /* CUST_DLINK */

bool get_fotaclient_timer_state(void) {
    bool enabled = false;
    FILE *fp = NULL;
#ifdef FEATURE_ROUTER
    fp = popen("systemctl is-enabled fotaclient.timer", "r");
#endif
    if (fp == NULL) {
        return enabled;
    }

    char buffer[MAX_DATA_LENGTH];
    fgets(buffer, sizeof(buffer), fp);
    enabled = !strncmp(buffer, "enabled", sizeof("enabled") - 1);
    pclose(fp);

    log_d("fota client timer service status is %d", enabled);
    return enabled;
}

bool set_fotaclient_timer_status(bool state) {
    int res = -1;
#ifdef FEATURE_ROUTER
    if (state) {
        res = systemCmd("ln -sf /vendor/etc/fotaclient.timer /data/misc/fotaclient.timer");
        log_d("link fota client timer service res %d", res);
    } else {
        res = systemCmd("unlink /data/misc/fotaclient.timer");
        log_d("unlink fota client timer service res %d", res);
    }

    if (res == 0) {
        res = reload_systemctl_daemon();
    }
#endif
    return (res == 0);
}

int reload_systemctl_daemon() {
    int res = -1;
    res = systemCmd("systemctl daemon-reload");
    log_d("systemctl daemon-reload res %d", res);
    if (res == 0) {
        res = systemCmd("systemctl restart fotaclient.timer");
        log_d("fota client timer service restart res %d", res);
    }
    return res;
}

void wait_loading() {
    if (loading <= WAIT_COUNT) {
        update_loading_bar(mbox, loading/WAIT_COUNT*100);
        loading++;
    } else {
        close_static_popup();
    }
}

void set_status_thread(void *arg)
{
    bool * enable = (bool *)arg;

    if (!set_fotaclient_timer_status(*enable)) {
        if (get_fotaclient_timer_state()) {
            lv_sw_on(update_sw, LV_ANIM_OFF);
        } else {
            lv_sw_off(update_sw, LV_ANIM_OFF);
        }
    }
    pthread_exit(NULL);
}

void update_sw_release_action(lv_obj_t * sw, lv_event_cb_t event_cb)
{
    if (event_cb != LV_EVENT_CLICKED) return;

    loading = 0;
    mbox = popup_anim_loading_create("", get_string(ID_LOADING));
    popup_loading_task_create(wait_loading, ANIM_INTERVAL, LV_TASK_PRIO_MID, NULL);

    static bool status_enable = false;

    if (lv_sw_get_state(sw)) {
        //ds_set_bool(DS_KEY_AUTO_CHECK_FOR_UPDATE, true);
        status_enable = true;
        lv_sw_on(update_sw, LV_ANIM_OFF);
    } else {
        //ds_set_bool(DS_KEY_AUTO_CHECK_FOR_UPDATE, false);
        status_enable = false;
        lv_sw_off(update_sw, LV_ANIM_OFF);
    }
#ifdef FEATURE_ROUTER
    memset(&timer_conf_thread, 0, sizeof(pthread_t));
    pthread_create(&timer_conf_thread, NULL, set_status_thread, (void*)&status_enable);
#endif
    //disable or enable auto check for updated and to do something
}

bool is_fotaclient_running() {
    bool ret = false;
    FILE *fp = popen("ps | grep \"[f]otaclient -u\"", "r");
    if (fp == NULL) {
        log_e("grep fotaclient process failed");
    }

    char buffer[MAX_DATA_LENGTH];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    if (strlen(buffer) != 0) {
        ret = true;
    }
    fclose(fp);
    fp = NULL;

    return ret;
}

void update_and_check() {
    //log_d("update_and_check %.0f", loading);
    if (loading <= LOADING_COUNT || (downloading && is_fotaclient_running())) {
        update_loading_bar(mbox, (int)(loading/LOADING_COUNT*100));
        loading++;
    } else {
        //updated_ok_popup
        struct stat buffer;
        int exist = stat(FC_NEED_UPDATE_VERSION_FLAG_FILEPATH, &buffer);
        if (exist == 0) {
            log_d("fota complete");
        }
    }
}

void startFotaclient() {
#ifdef FEATURE_ROUTER
    int res = systemCmd("systemctl start fotaclient@-u.service");
    log_d("start fota client service res: %d", res);
#endif
}

void close_progress_popup() {
    if(fotaclient_task != NULL){
        lv_task_del(fotaclient_task);
        fotaclient_task = NULL;
    }
    close_static_popup();
}

bool fota_need_upgrade() {
    bool ret = false;
    int rlen;
    FILE *fp = NULL;
    char buf[FILE_BUF_SIZE] = {0};

    fp = fopen(FC_NEED_UPGRADE_FLAG_FILEPATH, "r");
    if(fp != NULL)
    {
        rlen = fread(buf, 1, FILE_BUF_SIZE, fp);
        buf[rlen - 1] = '\0';
        fclose(fp);
        fp = NULL;
    }

    if(!strncmp(buf, "1", strlen("1"))) {
        ret = true;
    }
    return ret;
}

bool fota_client_alive() {
    FILE *fp = popen("ps | grep [f]otaclient", "r");
    if (fp == NULL) {
        log_e("grep fotaclient process failed");
        return true;
    }

    char buffer[60];
    memset(buffer, 0, sizeof(buffer));
    fgets(buffer, sizeof(buffer), fp);
    pclose(fp);

    if (strlen(buffer) == 0) {
        log_d("[update] fota client not alive");
        return false;
    } else {
        return true;
    }
}

void fota_update_status() {

    reset_screen_timeout(); // reset screen off timeout to keep screen on

    if (fota_need_upgrade())
    {
        log_d("Update downloading");
        if(!downloading) {
            log_d("Check update failed due to fota client and UI unsync");
            update_result_popup(ID_UPDATE_CHECKING_FOR_UPDATES, ID_UPDATE_CHECK_FAILED);
        }
        if(downloading) {
            uint32_t t = lv_tick_elaps(dl_start_timestamp);
            // wait 10 sec before starting to check if fota client might have crashed
            // to handle this case by showing download failed
            if((t > 10000) && !fota_client_alive()) {
                log_d("Download Image failed due to fota client died");
                update_result_popup(ID_UPDATE, ID_UPDATE_DOWNLOAD_FAILED);
            }
        }
    }
    else
    {
        downloading = false;

        // don't check fota status until after UI half way through loading progress
        if (loading < LOADING_COUNT/2) return;

        char cValue[5];
        memset(cValue, 0, sizeof(cValue));
        get_ds_data(FC_DATA_STORE_FILE, "LastFOTAStatus", &cValue);
        int value = atoi(cValue);

        switch(value) {
            case 10:
                log_d("No new firmware: %d", value);
                update_result_popup(ID_UPDATE_UPDATED, ID_UPDATE_VERSION_IS_UP_TO_DATE);
                break;
            case 12:
                log_d("Download Image failed: %d", value);
                update_result_popup(ID_UPDATE, ID_UPDATE_DOWNLOAD_FAILED);
                break;
            case 14:
                memset(cValue, 0, sizeof(cValue));
                get_ds_data(FC_DATA_STORE_FILE, "ConnectStatus", &cValue);
                int response = atoi(cValue);
                if (response == 0) {
                    log_d("Download meta failed: %d", value);
                    update_result_popup(ID_UPDATE, ID_UPDATE_CONNECT_FAILED);
                    break;
                }
                // continue to show check fail if not connection issue
            case 16:
            case 18:
                log_d("Check update failed: %d", value);
                update_result_popup(ID_UPDATE_CHECKING_FOR_UPDATES, ID_UPDATE_CHECK_FAILED);
                break;
            default:
                log_d("[update] Unhandled fota status: %d", value);
                if(!fota_client_alive()) {
                    log_d("Check update failed, unexpected status");
                    update_result_popup(ID_UPDATE_CHECKING_FOR_UPDATES, ID_UPDATE_CHECK_FAILED);
                }
                break;
        }

    }
}

static int update_content;
void update_check_anim_static_cb(){
    loading = 0;
    if (update_content == ID_UPDATE_UPDATING) {
        // issue 8. FOTA: warning wording font size in large
        // and change popup title when downloading
        mbox = popup_anim_loading_create(get_string(ID_UPDATE_DOWNLOADING), get_string(update_content));
        update_anim_loading_msg_font(mbox, font_w_bold, font_h_24);
    } else {
        mbox = popup_anim_loading_create(get_string(ID_UPDATE), get_string(update_content));
    }
    popup_loading_task_create(update_and_check, ANIM_INTERVAL, LV_TASK_PRIO_MID, NULL);
    set_static_popup(true);
}

void update_and_check_animation(int content) {
    update_content = content;
    create_static_popup(update_check_anim_static_cb);
}

void battery_warning_close(lv_obj_t * mbox, lv_event_cb_t event_cb)
{
    if (event_cb != LV_EVENT_CLICKED) return;
    close_popup();
}

void upgrade_comfirm_action(lv_obj_t * mbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    const char * txt = lv_btnm_get_active_btn_text(mbox);

    set_upgrade_notified(); // set this flag so it won't prompt user again until next device boot up

    if (strcmp(txt, get_string(ID_CANCEL)) == 0) {
        close_popup();
    }
    if (strcmp(txt, get_string(ID_OK)) == 0) {
        close_static_popup();
        update_and_check_animation(ID_UPDATE_UPDATING);
        downloading = true;
        dl_start_timestamp = lv_tick_get();

        fota_update_status();
        startFotaclient();
        fotaclient_task = lv_task_create(fota_update_status, 2000, LV_TASK_PRIO_MID, NULL);
    }
}

void upgrade_available_check() {
    downloading = false;
    bool upgrade_available = fota_need_upgrade();
    log_d("[update] upgrade_available: %d", upgrade_available);
    if (upgrade_available && !meet_upgrade_precondition()) {
        close_progress_popup();
        static const char * btns[2];
        btns[0] = get_string(ID_OK);
        btns[1] = "";

        popup_scrl_create(get_string(ID_UPDATE), get_string(ID_UPDATE_BATTERY_WARNING),
              btns, battery_warning_close);
    } else if (upgrade_available) {
        close_progress_popup();
        static const char * btns[3];
        btns[0] = get_string(ID_CANCEL);
        btns[1] = get_string(ID_OK);
        btns[2] = "";
        popup_scrl_create_impl(get_string(ID_UPDATE), get_string(ID_UPDATE_NEW_VERSION),
                   btns, upgrade_comfirm_action, NULL);
    } else {
        // let fota_update_status to handle no update available cases and other possible error cases
        fotaclient_task = lv_task_create(fota_update_status, 2000, LV_TASK_PRIO_MID, NULL);
    }
}

void check_update_action(lv_obj_t * btnm, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED) return;

    fotaclient_task = lv_task_create(upgrade_available_check, 12000, LV_TASK_PRIO_MID, NULL);
    lv_task_once(fotaclient_task);

    update_and_check_animation(ID_UPDATE_CHECKING_FOR_UPDATES);

    if(!fota_client_alive()) {
        int res = systemCmd("fotaclient -c &");
        log_d("[update] update cmd ret:%d", res);
    } else {
        log_d("[update] fota client already running, skip fota start command");
    }
}

void update_ok_action(lv_obj_t * mbox, lv_event_cb_t event_cb)
{
    if (event_cb != LV_EVENT_CLICKED) return;
    close_popup();
}

void update_result_popup(int title, int content) {
    static const char * btns[2];
    btns[0] = get_string(ID_OK);
    btns[1] = "";

    if(fotaclient_task != NULL){
        lv_task_del(fotaclient_task);
        fotaclient_task = NULL;
    }
    close_static_popup();
    popup_scrl_create(get_string(title), get_string(content), btns,
            update_ok_action);
}

void win_close_cb(lv_obj_t * btn, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) return;

    // fix issue: if press pwr key during checking for updates,
    // task will be deleted thus always result in check failed
    // Remove below since its static popup now, we need the task
    // even when update win closed
    //if (fotaclient_task != NULL) {
    //    lv_task_del(fotaclient_task);
    //    fotaclient_task = NULL;
    //}
    close_popup();

    if (label != NULL) {
        lv_obj_del(label);
    }
}

void default_update_create(void) {
    downloading = false;

    liste_style_create();

    lv_obj_t * win = default_list_header(lv_scr_act(), get_string(ID_UPDATE), win_close_cb);
    lv_obj_t * list = lv_list_create(win, NULL);
    lv_list_set_sb_mode(list, LV_SB_MODE_OFF);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);

    //Set list object size
    lv_obj_set_size(list, LIST_OBJ_HEIGHT, LIST_OBJ_WIDTH);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_btn_set_layout(list, LV_LAYOUT_OFF);

    lv_obj_t * liste = lv_liste_w_switch(list, get_string(ID_UPDATE_AUTO_CHECK_FOR_UPDATED),
            update_sw_release_action);
    update_sw = lv_obj_get_child(liste, NULL);

    //if (ds_get_bool(DS_KEY_AUTO_CHECK_FOR_UPDATE)) {
    if (get_fotaclient_timer_state()) {
        lv_sw_on(update_sw, LV_ANIM_OFF);
    } else {
        lv_sw_off(update_sw, LV_ANIM_OFF);
    }

    //draw current version
    lv_style_copy(&style_font, &lv_style_plain);
    style_font.text.font = get_font(font_w_bold, font_h_16);
    style_font.text.color = LV_COLOR_GREYISH_BROWN;
    style_font.text.letter_space = 1;

    char soft_version_info[MAX_INFO_LENGTH];
    char *soft_version_info1 = get_string(ID_UPDATE_CURRENT_VERSION);
#ifdef CUST_ZYXEL
    char *soft_version_info2 = getZyxelFwVersion();
#else
    char *soft_version_info2 = getSwVersion();
#endif
    snprintf(soft_version_info, sizeof(soft_version_info), "%s%s%s",
            soft_version_info1,"\n" ,soft_version_info2);
    label = lv_label_create(win, NULL);
    lv_label_set_text(label, soft_version_info);
    lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style_font);

    //re-align depend on is_ltr result
    if(is_ltr()){
        lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
        lv_obj_align(label, liste, LV_ALIGN_OUT_BOTTOM_LEFT, LISTE_SHIFT, HOR_SPACE);
    }else{
        lv_label_set_align(label, LV_LABEL_ALIGN_RIGHT);
        lv_obj_align(label, liste, LV_ALIGN_OUT_BOTTOM_RIGHT, -LISTE_SHIFT, HOR_SPACE);
    }

    //draw check_for_update button
    lv_style_copy(&style_btn_rel, &lv_style_btn_rel);
    style_btn_rel.body.main_color = LV_COLOR_WHITE;
    style_btn_rel.body.grad_color = LV_COLOR_WHITE;
    style_btn_rel.body.border.color = LV_COLOR_SILVER;
    style_btn_rel.body.border.width = 2;
    style_btn_rel.body.border.opa = LV_OPA_COVER;
    style_btn_rel.body.border.part = LV_BORDER_TOP;
    style_btn_rel.body.radius = 0;
    style_btn_rel.text.color = LV_COLOR_GREYISH_BROWN;
    style_btn_rel.text.font = get_font(font_w_bold, font_h_22);
    style_btn_rel.text.letter_space = 1;

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.body.main_color = LV_COLOR_WHITE;
    style_btn_pr.body.grad_color = LV_COLOR_WHITE;
    style_btn_pr.body.border.color = LV_COLOR_BASE;
    style_btn_pr.body.border.width = 3;
    style_btn_pr.text.color = LV_COLOR_BASE;

    check_for_update_btn = lv_btn_create(win, NULL);
    lv_obj_set_event_cb(check_for_update_btn, check_update_action);
    lv_obj_set_size(check_for_update_btn, 320 * LV_RES_OFFSET,
            50 * LV_RES_OFFSET);
    lv_btn_set_style(check_for_update_btn, LV_BTN_STYLE_REL, &style_btn_rel);
    lv_btn_set_style(check_for_update_btn, LV_BTN_STYLE_PR, &style_btn_pr);
    lv_obj_align(check_for_update_btn, win, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    lv_obj_t * btn_label = lv_label_create(check_for_update_btn, NULL);
    const char * btn_txt = get_string(ID_UPDATE_CHK_FOR_UPDATES);
    lv_label_set_text(btn_label, btn_txt);

    //if txt too long, set lable to scroll circle mode
    lv_coord_t txt_x = get_txt_x_size(btn_label, btn_txt);
    lv_coord_t total = txt_x + HOR_SPACE*4;
    if(total > LV_HOR_RES_MAX){
        lv_coord_t new_txt_x = LV_HOR_RES_MAX - HOR_SPACE*4;
        lv_label_set_long_mode(btn_label, LV_LABEL_LONG_SROLL_CIRC);
        lv_obj_set_width(btn_label, new_txt_x);
    }
}

void init_update(void) {

#ifdef CUST_DLINK
    dlink_update_create(true);
#else
    default_update_create();
#endif

}

