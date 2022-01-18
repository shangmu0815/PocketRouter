
#ifndef LV_POCKET_ROUTER_SRC_SETTINGS_UPDATE_H_
#define LV_POCKET_ROUTER_SRC_SETTINGS_UPDATE_H_

enum update_list{
    STATUS_CODE,
    LAST_CHECK_TIME,
    NEW_FIRMWARE_AVAILABLE,
    COMPONENT_ID,
    VERSION,
    MODEM_ID,
    FWURL,
    NOTEURL,
    ZIP_SIZE,
    ZIPMD5,
    TOKENP,
    FILE_NAME,
    FILE_SIZE,
    MD5,
    SYSTEM_REBOOT,
    STORE_LOCATION,
};

static const char * update_list_map[] = {
        "status_code",
        "last_check_time",
        "new_firmware_available",
        "ComponentID",
        "Version",
        "ModemID",
        "FWURL",
        "NoteURL",
        "ZipSize",
        "ZipMD5",
        "TokenP",
        "FileName",
        "FileSize",
        "MD5",
        "SystemReboot",
        "StoreLocation"
};
static const int update_list_len = sizeof(update_list_map) / sizeof(char *);

enum update_err_map {
    UPDATE_INTERNAL_ERR = 0,
    UPDATE_AVAILABLE_BIN,
    UPDATE_NO_BIN,
    UPDATE_ERR_2ND_XML,
    UPDATE_ERR_SKU_CHECKING,
    UPDATE_ERR_XML_SCHEME,
    UPDATE_ERR_UPDATE_INFO_SEC,
    UPDATE_ERR_UPDATE_SEQ_SEC,
    UPDATE_ERR_COPONENT_COUNT,
    UPDATE_ERR_UPDATE_SRC_SEC,
    UPDATE_ERR_STORE_LOC,
    UPDATE_ERR_STORE_LOC_SPACE,
    UPDATE_ERR_1ST_XML,
    UPDATE_UPGRADE_SUCCESS = 100,
    UPDATE_ERR_DOWNLOAD_FWFILE,
    UPDATE_ERR_UNZIP_FWFILE,
    UPDATE_ERR_APPLY_FWBIN
};

/*
0: Internal Error
1: Has new available FW binary.
2: No FW binary
3: Error to access the 2 nd XML file in the remote server
4: Error to the device SKU checking
5: Error in XML scheme
6: Error in the UpdateInfo section
7: Error in the UpdateSequence section
8: Error in update components count.
9: Error in UpdateSource section
10: Error for nUpdateStoreLocationError
11: Error for nUpdateStoreLocationSpaceError.
12: Error to access the 1 st XML file
100: Upgrade successful
101: Error to download the FW file
102: Error to unzip the FW file
103: Error to apply the new FW binary
*/

bool get_available_updates_state();
void update_sw_release_action(lv_obj_t * sw, lv_event_cb_t event_cb);
void update_and_check();
void update_and_check_animation(int);
void update_ok_action(lv_obj_t * mbox, lv_event_cb_t event_cb);
void update_result_popup(int, int);
bool fota_need_upgrade();
void upgrade_available_check();
//static lv_action_t win_close_action(lv_obj_t * btn);
void init_update(void);
int reload_systemctl_daemon();

#endif /* LV_POCKET_ROUTER_SRC_SETTINGS_UPDATE_H_ */
