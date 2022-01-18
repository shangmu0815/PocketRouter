#include "ril.h"
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <lv_pocket_router/src/launcher.h>
#include <lv_pocket_router/src/util/list_action.h>
#include <lv_pocket_router/src/util/convert_string.h>
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"
#ifdef FEATURE_ROUTER
#include "device_management_service_v01.h"
#include "data_system_determination_v01.h"
#endif

//#define USE_IFCONFIG_USAGE // to be remove when GetWWANStatistics ready

#if defined (ANDROID_PROPERTY)
#include <cutils/properties.h>
#endif

#define DATA_USAGE_DUMP_FILE    DEFAULT_DATA_STORE_PATH "/DataUsage.txt"
#ifdef FEATURE_ROUTER
#define DATA_USAGE_DUMP_CMD     "ifconfig rmnet_ipa0 | grep \"RX bytes\""
#define DATA_USAGE_DUMP         "ifconfig rmnet_ipa0 | grep \"RX bytes\" > " DATA_USAGE_DUMP_FILE
#else
#define DATA_USAGE_DUMP_CMD     "ifconfig eth0 | grep \"RX bytes\""
#define DATA_USAGE_DUMP         "ifconfig eth0 | grep \"RX bytes\" > " DATA_USAGE_DUMP_FILE
#endif

enum {
    DATA_USAGE_READ_FILE,
    DATA_USAGE_READ_DUMP
};

//static const char* WLAN = "wlan0";
static const char* RMNET_IPA = "rmnet_ipa0";
static const char* RX_BYTES = "RX bytes:";
static const char* TX_BYTES = " TX bytes:";

#ifdef FEATURE_ROUTER
#define WEBUI_DATA_USAGE_FILE   "/data/misc/data_usage"
#else
#define WEBUI_DATA_USAGE_FILE   "Data_Store/data_usage"
#endif

static bool enableIP = true;

lv_task_t * lv_task;

static bool set_nitz_b = false;
time_t next_reset_time = 0;
static uint16_t g_ril_pref_mode = 0;
pthread_mutex_t async_resp;
pthread_cond_t async_resp_cond;
pthread_t async_resp_handler;
int nw_scan_rec_flag = -1;
int nw_scan_err;
char g_sim_mcc_cache[4];
char g_sim_mnc_cache[4];

// Transmission data will be dump to DATA_USAGE_DUMP_FILE regularly
// in monitor task instead of recording the value to data store regularly.
// Only update value to data store in next device boot for all accumulated
// transmission bytes before next reset happens.
// bytes_accumulated is all transmission bytes prior to current bootup
long long int bytes_accumulated = 0;
// reset_accumulated is transmission bytes prior to reset,
// this should be clean up in next reboot
long long int reset_accumulated = 0;

long long int rx_bytes = 0;
long long int tx_bytes = 0;
bool downlading = false;
bool uploading = false;
//QCMAP
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)

#endif
uint32_t qmi_ip_mobile_ap_handle;
int  qcmap_msgr_enable;
uint32_t mobile_ap_handle;

//QCMAP
int qcmap_ind_wwan_v4_status = 0;
int qcmap_ind_wwan_v6_status = 0;

#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
static int g_reg_state = 0;
uint16_t g_mcc = 0, g_mnc = 0;
static ril_nw_roaming_state g_nw_roaming_state = 0;
ril_nw_resp_cb rilNwCb;
ril_sim_pin_req_cb rilSimPinCB;
// qmi globals
qmi_client_type dsd_user_handle, nas_user_handle, wds_user_handle, uim_client_handle;
qmi_idl_service_object_type dsd_service_obj, nas_serv_obj, wds_service_obj, uim_serv_obj;

// NAS
char nw_description[OPERATOR_NAME_MAX_LENGTH+1];
ril_nw_network_time_info_type ril_nw_network_time_info;
ril_nw_lte_sib16_network_time_info_type ril_nw_lte_sib16_network_time_info;
int g_nw_scan_time_out_flag = 0;

// UIM
ril_sim_info g_sim_info;

void ril_deinit(void)
{
    int ret;

    ril_qcmap_deinit();

    if (dsd_user_handle != NULL)
    {
        ret = qmi_client_release(dsd_user_handle);
        log_d("qmi dsd client release. ret=%d", ret);
    }

    if (nas_user_handle != NULL)
    {
        ret = qmi_client_release(nas_user_handle);
        log_d("qmi nas client release. ret=%d", ret);
    }

    if (wds_user_handle != NULL)
    {
        ret = qmi_client_release(wds_user_handle);
        log_d("qmi wds client release. ret=%d", ret);
    }

    if (uim_client_handle != NULL)
    {
        ret = qmi_client_release(uim_client_handle);
        log_d("qmi uim client release. ret=%d", ret);
    }
    pthread_mutex_destroy(&async_resp);
    pthread_cond_destroy(&async_resp_cond);
}

ril_error_type convertQMIErrCodeToRil(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
    ril_error_type ret = RIL_ERROR_UNKNOWN;
    if(result == QMI_RESULT_SUCCESS_V01)
    {
        ret =  RIL_SUCCESS;
    }else
    {
        switch(error)
        {
            case QMI_ERR_NONE_V01:
                ret = RIL_SUCCESS;
                break;
            case QMI_ERR_GENERAL_V01:
            case QMI_ERR_UNKNOWN_V01:
                ret = RIL_ERROR_UNKNOWN;
                break;
            case QMI_ERR_DEVICE_MEMORY_ERROR_V01:
            case QMI_ERR_NO_MEMORY_V01:
                ret = RIL_ERROR_MEMORY_ERROR;
                break;
            case QMI_ERR_INCORRECT_PIN_V01:
                ret = RIL_ERROR_SIM_PIN_INCORRECT;
                break;
            case QMI_ERR_PIN_BLOCKED_V01:
                ret = RIL_ERROR_SIM_PIN_BLOCKED;
                break;
            case QMI_ERR_PIN_PERM_BLOCKED_V01:
                ret = RIL_ERROR_SIM_PIN_PERM_BLOCKED;
                break;
            default:
                log_e("not defined error code. error=%d.", error);
                ret = RIL_ERROR_UNKNOWN;
                break;
        }
    }
    return ret;
}

void set_nw_cb(ril_nw_resp_cb cb)
{
    if( cb == NULL)
    {
        log_e("set_nw_cb cb == NULL");
        return ;
    }
    rilNwCb = cb;
}

void set_sim_pin_cb(ril_sim_pin_req_cb cb)
{
    if( cb == NULL)
    {
        log_e("ril_sim_event_register cb == NULL");
        return ;
    }
    rilSimPinCB = cb;
}

void ril_unsolicited_nas_ind_handler( qmi_client_type user_handle,
                                      unsigned int msg_id,
                                      void *ind_buf,
                                      unsigned int ind_buf_len,
                                      void *ind_cb_data )
{
    int status, idx;
    nas_serving_system_ind_msg_v01* serv_sys_ind_msg;
    uint32_t decoded_payload_len;
    void *decoded_payload;
    qmi_client_error_type err_code;
    nas_network_time_ind_msg_v01 *nw_time_ind_msg;
    ril_nw_nitz_time_info_t nitz_time = {0};

    log_d("ril_unsolicited_nas_ind_handler, msg %d\n", msg_id);

    if(ind_buf != NULL)
    {
        qmi_idl_get_message_c_struct_len( nas_serv_obj,
                                          QMI_IDL_INDICATION,
                                          msg_id,
                                          &decoded_payload_len
                                        );

        if(decoded_payload_len)
        {
            decoded_payload = malloc(decoded_payload_len);
        }

        if ( decoded_payload || !decoded_payload_len )
        {
            err_code = qmi_client_message_decode(
                                   nas_user_handle,
                                   QMI_IDL_INDICATION,
                                   msg_id,
                                   ind_buf,
                                   ind_buf_len,
                                   decoded_payload,
                                   decoded_payload_len);
            if (err_code != QMI_NO_ERR)
            {
                log_e("Error: Decoding unsolicited indication with id = %ld, returned in error = %d\n", msg_id, (int)err_code);
                if(decoded_payload != NULL)
                    free(decoded_payload);
                return ;
            }
        }else
        {
            log_e("Error: decoded_payload = NULL or decoded_payload_len=0.\n");
            return ;
        }
    }else
    {
        log_e("Error: ind_buf = NULL.\n");
        return ;
    }

    if(msg_id == QMI_NAS_SERVING_SYSTEM_IND_MSG_V01)
    {
        serv_sys_ind_msg = (nas_serving_system_ind_msg_v01 *)decoded_payload;

        g_reg_state = serv_sys_ind_msg->serving_system.registration_state;
        log_d("ril_unsolicited_nas_ind_handler, g_reg_state registration_state = %d\n", g_reg_state);
    }else if( msg_id == QMI_NAS_NETWORK_TIME_IND_MSG_V01 )
    {
        nw_time_ind_msg = (nas_network_time_ind_msg_v01 *)decoded_payload;
        if(nw_time_ind_msg)
        {
            // update cache
            memcpy(&ril_nw_network_time_info.universal_time, &nw_time_ind_msg->universal_time, sizeof(ril_nw_network_time_info.universal_time));
            ril_nw_network_time_info.time_zone_valid = TRUE;
            memcpy(&ril_nw_network_time_info.time_zone, &nw_time_ind_msg->time_zone, sizeof(ril_nw_network_time_info.time_zone));
            ril_nw_network_time_info.daylt_sav_adj_valid = TRUE;
            memcpy(&ril_nw_network_time_info.daylt_sav_adj, &nw_time_ind_msg->daylt_sav_adj, sizeof(ril_nw_network_time_info.daylt_sav_adj));
            ril_nw_network_time_info.radio_if_valid = TRUE; 
            memcpy(&ril_nw_network_time_info.radio_if, &nw_time_ind_msg->radio_if, sizeof(ril_nw_network_time_info.radio_if));

            // fill ret data
            ril_nw_fill_nitz_time_resp(&nitz_time.nitz_time_valid, &nitz_time.nitz_time, &nitz_time.abs_time_valid, &nitz_time.abs_time, &nitz_time.leap_sec_valid, &nitz_time.leap_sec);
            log_d("QMI_NAS_NETWORK_TIME_IND_MSG_V01 nitz_time = %s\n", nitz_time.nitz_time);
            (*rilNwCb)(RIL_NW_IND_NITZ_TIME, RIL_SUCCESS, sizeof(ril_nw_nitz_time_info_t), &nitz_time);
        }
    }
    if(decoded_payload != NULL)
        free(decoded_payload);
}

void ril_nas_ind_registration()
{
    qmi_client_error_type qmi_error;
    nas_indication_register_req_msg_v01  indication_req;
    nas_indication_register_resp_msg_v01 indication_resp_msg;

    memset(&indication_req, 0, sizeof(indication_req));
    memset(&indication_resp_msg, 0, sizeof(indication_resp_msg));

    indication_req.req_serving_system_valid = TRUE;
    indication_req.req_serving_system = TRUE;

    indication_req.sys_info_valid = TRUE;
    indication_req.sys_info = FALSE;

    indication_req.reg_managed_roaming_valid = TRUE;
    indication_req.reg_managed_roaming = FALSE;

    indication_req.reg_network_time_valid = TRUE;
    indication_req.reg_network_time = TRUE;

    qmi_error = qmi_client_send_msg_sync (nas_user_handle,
                                          QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                                          &indication_req,
                                          sizeof(indication_req),
                                          &indication_resp_msg,
                                          sizeof(indication_resp_msg),
                                          30000);

    log_d("ril_nas_ind_registration, qmi_error = %d", qmi_error);
}

int ril_qmi_init()
{
    qmi_client_os_params  os_params;
    int time_out = 4;
    qmi_client_error_type rc;
    int ret = RIL_ERROR_INIT_FAIL;

    if( dsd_user_handle == NULL )
    {
		// dsd init
		dsd_service_obj = dsd_get_service_object_v01();
		rc = qmi_client_init_instance( dsd_service_obj,
                                       QMI_CLIENT_INSTANCE_ANY,
                                       NULL,
                                       NULL,
                                       &os_params,
                                       time_out,
                                       &dsd_user_handle );

		if (rc != QMI_NO_ERR )
		{
			log_e("Error: QMI_DSD connection not Initialized...Error Code:%d\n", rc);
			return ret;
		}else
		{
			log_d("DSD Initialized...dsd_user_handle:%p\n", dsd_user_handle);
		}
    }else
    {
        log_d("dsd_user_handle not NULL:%p\n", dsd_user_handle);
    }

    if( nas_user_handle == NULL )
    {
        // nas init
        nas_serv_obj = nas_get_service_object_v01();
        rc = qmi_client_init_instance( nas_serv_obj,
                                       QMI_CLIENT_INSTANCE_ANY,
                                       ril_unsolicited_nas_ind_handler,
                                       NULL,
                                       &os_params,
                                       time_out,
                                       &nas_user_handle );

        if (rc != QMI_NO_ERR )
        {
            log_e("Error: QMI_NAS connection not Initialized...Error Code:%d\n", rc);
            return ret;
        }else
        {
            log_d("NAS Initialized...nas_user_handle:%p\n", nas_user_handle);
            ril_nas_ind_registration();
        }
    }else
    {
        log_d("nas_user_handle not NULL:%p\n", nas_user_handle);
    }

    if( wds_user_handle == NULL )
    {
        // wds init
        memset(&os_params, 0, sizeof(os_params));
        wds_service_obj = wds_get_service_object_v01();
        rc = qmi_client_init_instance( wds_service_obj,
                                       QMI_CLIENT_INSTANCE_ANY,
                                       NULL,
                                       NULL,
                                       &os_params,
                                       time_out,
                                       &wds_user_handle );

        if (rc != QMI_NO_ERR )
        {
            log_e("Error: QMI_WDS connection not Initialized...Error Code:%d\n", rc);
            return ret;
        }else
        {
            log_d("WDS Initialized...wds_user_handle:%p\n", wds_user_handle);
        }
    }else
    {
        log_d("wds_user_handle not NULL:%p\n", wds_user_handle);
    }

    if( uim_client_handle == NULL )
    {
        // uim init
        uim_serv_obj = uim_get_service_object_v01();
        rc = qmi_client_init_instance( uim_serv_obj,
                                       QMI_CLIENT_INSTANCE_ANY,
                                       ril_unsolicited_uim_ind_handler,
                                       NULL,
                                       &os_params,
                                       time_out,
                                       &uim_client_handle );

        if (rc != QMI_NO_ERR )
        {
            log_e("Error: QMI_UIM connection not Initialized...Error Code:%d\n", rc);
            return ret;
        }else
        {
            log_d("UIM Initialized...uim_client_handle:%p\n", uim_client_handle);
            ril_sim_event_register();
        }
    }else
    {
        log_d("uim_client_handle not NULL:%p\n", uim_client_handle);
    }

    return RIL_SUCCESS;
}

int ril_init(ril_init_data *init_data){
    int ret = -1;
    volatile int i = 0;

    memset(&g_sim_info, 0, sizeof(ril_sim_info));

    // starting QMI init clients
    for(i=0; i<RIL_QMI_INIT_RETRY_COUNT_MAX; i++)
    {
        if( ril_qmi_init() == RIL_SUCCESS )
        {
            log_d("ril_qmi_init successfully.\n");
            break;
        }
        else
        {
            log_d("ril_qmi_init fail. wait 10 sec retry.\n");
            usleep(10000000);
        }
    }

    if(init_data != NULL)
    {
        rilSmsInit(&(init_data->sms_init));
        set_sim_pin_cb(init_data->sim_pin_cb);
        set_nw_cb(init_data->nw_cb);
    }

    pthread_mutex_init(&async_resp, NULL);
    pthread_cond_init (&async_resp_cond, NULL);
    return RIL_SUCCESS;
}
#endif  /* defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD) */

long long int data_usage_overflow_check(long long int value) {
    if(value >= LLONG_MAX || value < 0) {
        log_d("data usage value overflow, reset it.");
        return 0;
    }

    return value;
}

void data_usage_reset_check() {
    time_t now = time(NULL);
    if (next_reset_time < now) {
        log_d("Going to perform data usage reset! Scheduled reset time %d, now %d", next_reset_time, now);
        reset_data_usage(true);
        reset_date_usage_start_date();
    }
}

void reset_data_usage(bool reset_time_b) {
    if (reset_time_b) {
        time_t t = time(NULL);
        struct tm *timeinfo = localtime(&t);
        char current_time[12];
        strftime(current_time, sizeof(current_time), "%Y-%m-%d", timeinfo);
        ds_set_value(DS_KEY_DATA_USAGE_RESET_TIME, current_time);
    }

    //reset data usage
    long long int total = rx_bytes + tx_bytes;
#if defined (USE_IFCONFIG_USAGE) || !defined(FEATURE_ROUTER)
    ds_set_long_long_int(DS_KEY_DATA_USAGE_RESET_ACCUMULATED, total);
    reset_accumulated = total;
#else
    ResetWWANStatistics(QCMAP_MSGR_IP_FAMILY_V4_V01);
    ResetWWANStatistics(QCMAP_MSGR_IP_FAMILY_V6_V01);
#endif
    ds_set_long_long_int(DS_KEY_DATA_USAGE_ACCUMULATED, 0);
    bytes_accumulated = 0;
    rx_bytes = 0;
    tx_bytes = 0;

    dump_webui_data_usage();
}

//ben
int get_apn_profile(get_profile_settings *getprofile){
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    log_e("get_apn_profile\n");
    int i=0;
    qmi_client_error_type qmi_err;
    int time_out = 4;
    char *ip_version;

    //get apn  list
    wds_get_profile_list_req_msg_v01   pl_req;
    wds_get_profile_list_resp_msg_v01  pl_resp;
    memset(&pl_req, 0, sizeof(pl_req));
    memset(&pl_resp, 0, sizeof(pl_resp));

    pl_req.profile_type_valid = true;
    pl_req.profile_type = WDS_PROFILE_TYPE_3GPP_V01;
    if (QMI_NO_ERR != qmi_client_send_msg_sync(
                     wds_user_handle,
                     QMI_WDS_GET_PROFILE_LIST_REQ_V01,
                     &pl_req,
                     sizeof(pl_req),
                     &pl_resp,
                     sizeof(pl_resp),
                     5000))
    {
        log_e("get_profile_list failed with error [%d] " \
                         "qmi_err_code [%d]", pl_resp.resp.result, pl_resp.resp.error);
    }

    log_e("get_profile_list for profile_tech [%d] "         \
                          "returned [%d] profile ids",
                          pl_req.profile_type, pl_resp.profile_list_len);
    //get apn info
    wds_get_profile_settings_req_msg_v01   ps_req;
    wds_get_profile_settings_resp_msg_v01  ps_resp;
    memset(&ps_req, 0, sizeof(ps_req));
    memset(&ps_resp, 0, sizeof(ps_resp));
    ps_req.profile.profile_type = WDS_PROFILE_TYPE_3GPP_V01;
    ps_req.profile.profile_index = 1;

    log_e("qmi_client_send_msg_sync-- QMI_WDS_GET_PROFILE_SETTINGS_REQ_V01\n");
    qmi_err = qmi_client_send_msg_sync(
                     wds_user_handle,
                     QMI_WDS_GET_PROFILE_SETTINGS_REQ_V01,
                     &ps_req,
                     sizeof(ps_req),
                     &ps_resp,
                     sizeof(ps_resp),
                     5000);
    log_e("qmi_err : %d\n" , qmi_err);
    if(QMI_NO_ERR == qmi_err)
    {
        log_e("apn class :%d\n",ps_resp.apn_class);
        switch (ps_resp.pdp_type)
        {
            case WDS_PDP_TYPE_PDP_IPV4_V01:
                log_e("GET ipversion :ipv4\n");
                ip_version="IPV4";
                break;

            case WDS_PDP_TYPE_PDP_IPV6_V01:
                log_e("GET ipversion :ipv6\n");
                ip_version="IPV6";
                break;

            case WDS_PDP_TYPE_PDP_IPV4V6_V01:
                log_e("GET ipversion :ipv4v6\n");
                ip_version="IPV4V6";
                break;

            default:
                log_e("unknown cdma pdp_type=%d\n",ps_resp.pdp_type);
                break;
        }

        //return apn name
        //strcpy(Buffer->data,ps_resp.apn_name);
        getprofile->apn_name = ps_resp.apn_name;
        getprofile->pdp_type = ip_version;
        getprofile->username = ps_resp.username;
        getprofile->password = ps_resp.password;
        log_e("apn username :%s\n",getprofile->username);
        log_e("apn password :%s\n",getprofile->password);
        log_e("[%d] has name [%s]\n",ps_req.profile.profile_index,getprofile->apn_name);
        log_e("apn pdp_type :%s\n",getprofile->pdp_type);
    } else {
        log_e("get apn name fail\n");
    }
    return 1;
#else
    return 1;
#endif
}

#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
void wait_for_disconnected_result(qcmap_msgr_wwan_call_type_v01 call_type)
{
    qcmap_msgr_wwan_status_enum_v01 v4status;
    qcmap_msgr_wwan_status_enum_v01 v6status;
    if( call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01 ){
        // wait for 5 sec
        for(int i=0; i<20; i++)
        {
            usleep(250000);
            if( qcmap_ind_wwan_v4_status == QCMAP_MSGR_WWAN_STATUS_DISCONNECTED_V01 )
                return ;
        }
    }
    else if( call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01 ){
        // wait for 5 sec
        for(int i=0; i<20; i++)
        {
            usleep(250000);
            if( qcmap_ind_wwan_v6_status == QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED_V01 )
                return ;
        }
    }
    else{
        log_e("unknown call type..\n");
        return ;
    }

    // not received status from indication
    log_e("wait for disconnected result timeout. type:%d.\n", call_type);
}

void wait_for_connected_result(qcmap_msgr_wwan_call_type_v01 call_type)
{
    qcmap_msgr_wwan_status_enum_v01 v4status;
    qcmap_msgr_wwan_status_enum_v01 v6status;
    if( call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01 ){
        // wait for 5 sec
        for(int i=0; i<20; i++)
        {
            usleep(250000);
            if( qcmap_ind_wwan_v4_status == QCMAP_MSGR_WWAN_STATUS_CONNECTED_V01 )
                return ;
        }
    }
    else if( call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01 ){
        // wait for 5 sec
        for(int i=0; i<20; i++)
        {
            usleep(250000);
            if( qcmap_ind_wwan_v6_status == QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED_V01 )
                return ;
        }
    }
    else{
        log_e("unknown call type..\n");
        return ;
    }

    // not received status from indication
    log_e("wait for connected result timeout. type:%d.\n", call_type);
}
#endif

int modify_apn_profile(modify_profile_settings *modifyprofile){
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    log_e("modify_apn_profile\n");
    int time_out = 4;
    qmi_client_error_type qmi_err;

    //get list
    wds_get_profile_list_req_msg_v01   pl_req;
    wds_get_profile_list_resp_msg_v01  pl_resp;
    memset(&pl_req, 0, sizeof(pl_req));
    memset(&pl_resp, 0, sizeof(pl_resp));

    pl_req.profile_type_valid = true;
    pl_req.profile_type = WDS_PROFILE_TYPE_3GPP_V01;
    if (QMI_NO_ERR != qmi_client_send_msg_sync(
                    wds_user_handle,
                    QMI_WDS_GET_PROFILE_LIST_REQ_V01,
                    &pl_req,
                    sizeof(pl_req),
                    &pl_resp,
                    sizeof(pl_resp),
                    5000))
    {
        log_e("get_profile_list failed with error [%d] " \
                        "qmi_err_code [%d]", pl_resp.resp.result, pl_resp.resp.error);
    }

    log_e("get_profile_list for profile_tech [%d] "        \
                        "returned [%d] profile ids",
                         pl_req.profile_type, pl_resp.profile_list_len);

    //get list end
    wds_get_profile_settings_req_msg_v01        ps_req;
    wds_get_profile_settings_resp_msg_v01        ps_resp;
    memset(&ps_req, 0, sizeof(ps_req));
    memset(&ps_resp, 0, sizeof(ps_resp));

    ps_req.profile.profile_type = WDS_PROFILE_TYPE_3GPP_V01;
    ps_req.profile.profile_index = 1;

    log_e("qmi_client_send_msg_sync-- QMI_WDS_GET_PROFILE_SETTINGS_REQ_V01\n");
    qmi_err = qmi_client_send_msg_sync(
                     wds_user_handle,
                     QMI_WDS_GET_PROFILE_SETTINGS_REQ_V01,
                     &ps_req,
                     sizeof(ps_req),
                     &ps_resp,
                     sizeof(ps_resp),
                     5000);
    log_e("qmi_err : %d\n" , qmi_err);
    if(QMI_NO_ERR == qmi_err)
    {
        //set apn profile
        wds_modify_profile_settings_req_msg_v01   req;
        wds_modify_profile_settings_resp_msg_v01  resp;
        memset(&req, 0, sizeof(req));
        memset(&resp, 0, sizeof(resp));
        req.profile.profile_index = 1;
        req.profile.profile_type = WDS_PROFILE_TYPE_3GPP_V01;
        //set apn name
        req.apn_name_valid = 1;
        strncpy(req.apn_name, modifyprofile->apn_name, strlen(modifyprofile->apn_name));

        //set pdp type
        req.pdp_type_valid = 1;
        if(strcasecmp(modifyprofile->pdp_type,"IPV4")== 0) {
            req.pdp_type = WDS_PDP_TYPE_PDP_IPV4_V01;
        } else if (strcasecmp(modifyprofile->pdp_type,"IPV6")== 0){
            req.pdp_type = WDS_PDP_TYPE_PDP_IPV6_V01;
        } else if (strcasecmp(modifyprofile->pdp_type,"IPV4V6")== 0){
            req.pdp_type = WDS_PDP_TYPE_PDP_IPV4V6_V01;
        } else {
            log_e("set ip type fail. not match ip type: %s",modifyprofile->pdp_type);
            return RIL_ERROR_INVALID_INPUT;
        }

        //set user name
        req.username_valid = 1;
        strncpy(req.username, modifyprofile->username, strlen(modifyprofile->username));

        //set password
        req.password_valid = 1;
        strncpy(req.password , modifyprofile->password, strlen(modifyprofile->password));

        if(strlen(modifyprofile->username) != 0 && strlen(modifyprofile->password) != 0)
        {
            req.authentication_preference_valid = 1;
            req.authentication_preference = QMI_WDS_MASK_AUTH_PREF_PAP_V01 | QMI_WDS_MASK_AUTH_PREF_CHAP_V01;
        }else
        {
            req.authentication_preference_valid = 1;
            req.authentication_preference = 0;
        }

        qmi_err = qmi_client_send_msg_sync(
                     wds_user_handle,
                     QMI_WDS_MODIFY_PROFILE_SETTINGS_REQ_V01,
                     &req,
                     sizeof(req),
                     &resp,
                     sizeof(resp),
                     5000);
        log_e("set profile qmi_err : %d\n" , qmi_err);
    }
    else
    {
        log_e("QMI_WDS_GET_PROFILE_SETTINGS_REQ_V01 fail\n");
    }

    if( QMI_NO_ERR == qmi_err )
        return RIL_SUCCESS;
    else
        return RIL_ERROR_SEND_QMI_FAIL;
#else
    return 0;
#endif
}

void reestablish_data_connection(void)
{
#ifdef FEATURE_ROUTER
    qcmap_msgr_wwan_status_enum_v01 v4status;
    qcmap_msgr_wwan_status_enum_v01 v6status;
    int con_ret_code = 0;
    GetWWANStatus(&v4status, &v6status);
    //
    //Set_Attach_Req(wds_user_handle);
    //

    ril_nw_set_ps_detach();
    usleep(250000);
    ril_nw_set_ps_attach();

    qcmap_msgr_wwan_call_type_v01 call_t = QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01;
    if(v4status == QCMAP_MSGR_WWAN_STATUS_CONNECTED_V01)
    {
        call_t = QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01;
        con_ret_code = DisconnectBackHaul(call_t);
        log_d("DisconnectBackHaul v4, ret : %d\n", con_ret_code);
        wait_for_disconnected_result(call_t);

        call_t = QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01;
        con_ret_code = ConnectBackHaul(call_t);
        log_d("ConnectBackHaul v4, ret : %d \n", con_ret_code);
        wait_for_connected_result(call_t);
    }else if(v4status == QCMAP_MSGR_WWAN_STATUS_DISCONNECTED_V01)
    {
        call_t = QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01;
        con_ret_code = ConnectBackHaul(call_t);
        log_d("ConnectBackHaul v4, ret : %d \n", con_ret_code);
        wait_for_connected_result(call_t);
    }else
    {
        log_e("unhandeld v4status: %d \n", v4status);
    }

    if(v6status == QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED_V01)
    {
        call_t = QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01;
        con_ret_code = DisconnectBackHaul(call_t);
        log_d("DisconnectBackHaul v6, ret : %d \n", con_ret_code);
        wait_for_disconnected_result(call_t);

        call_t = QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01;
        con_ret_code = ConnectBackHaul(call_t);
        log_d("ConnectBackHaul v6, ret : %d \n", con_ret_code);
        wait_for_connected_result(call_t);
    }else if(v6status == QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED_V01)
    {
        call_t = QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01;
        con_ret_code = ConnectBackHaul(call_t);
        log_d("ConnectBackHaul v6, ret : %d \n", con_ret_code);
        wait_for_connected_result(call_t);
    }else
    {
        log_e("unhandeld v6status: %d \n", v6status);
    }
#endif
}

#ifdef FEATURE_ROUTER
int Set_Attach_Req
(
  qmi_client_type wds_handle
)
{
    log_e("Set_Attach_Req\n");
    qmi_client_error_type qmi_error;
    wds_set_lte_attach_pdn_list_req_msg_v01 attach_req;
    wds_set_lte_attach_pdn_list_resp_msg_v01 attach_resp;
    uint32_t len=1;

    memset(&attach_req,0,sizeof(wds_set_lte_attach_pdn_list_req_msg_v01));
    memset(&attach_resp,0,sizeof(wds_set_lte_attach_pdn_list_resp_msg_v01));
    attach_req.action_valid = true;
    attach_req.attach_pdn_list_len = len;
    attach_req.attach_pdn_list[0] = 1;
    attach_req.action = 0x02;
    qmi_error = qmi_client_send_msg_sync(wds_handle,
                                         QMI_WDS_SET_LTE_ATTACH_PDN_LIST_REQ_V01,
                                         &attach_req,
                                         sizeof(wds_set_lte_attach_pdn_list_req_msg_v01),
                                         &attach_resp,
                                         sizeof(wds_set_lte_attach_pdn_list_resp_msg_v01),
                                         5000);

    if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
         ( qmi_error != QMI_NO_ERR ) ||
         ( attach_resp.resp.result != QMI_NO_ERR ) )
    {
      log_e("Can not Set_Attach_Req %d\n : %d",qmi_error, attach_resp.resp.error);
      return false;
    }

    return true;
}
#endif

void get_operator_name(char *oper_name_buff, int buffer_len) {
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    char long_opr_name[OPERATOR_NAME_MAX_LENGTH] = {0};
    char short_opr_name[OPERATOR_NAME_MAX_LENGTH] = {0};

    if( g_reg_state != NAS_REGISTERED_V01 )
    {
        log_e("data reg state = %d. can not get operator name.\n", g_reg_state);
        return ;
    }

    // use serving system info first.
    if(strlen(nw_description) != 0)
    {
        if(strlen(nw_description) < buffer_len)
        {
            strncpy(oper_name_buff, nw_description, strlen(nw_description));
            return ;
        }else
        {
            log_e("Buffer length is too small: %d\n", buffer_len);
        }
    }

    ril_nas_handle_centralized_short_long_eons(g_mcc, g_mnc, short_opr_name, long_opr_name, 0);
    log_e("ril_nas_handle_centralized_short_long_eons: short name = %s\n", short_opr_name);

    if(strlen(short_opr_name) < buffer_len)
    {
        strncpy(oper_name_buff, short_opr_name, strlen(short_opr_name));
    }else
    {
        log_e("Buffer length is too small: %d\n", buffer_len);
    }

#elif defined (ANDROID_BUILD)
    //TS053 solution
    char prop[OPERATOR_NAME_MAX_LENGTH];
    int len = property_get("gsm.operator.alpha", prop, "");
    if (len > 0 && strcmp(prop, ",") != 0) {
        strncpy(oper_name_buff, prop, strlen(prop));
    }
#else
    strncpy(oper_name_buff, "AT&T", strlen("AT&T"));
#endif
}

int ril_nw_get_signal_strength_qmi(ril_nw_signal_rat_t *signal_rat)
{
    int signal_strength = 0;
#ifdef FEATURE_ROUTER
    qmi_client_error_type qmi_err;
    nas_get_sig_info_resp_msg_v01 resp;

    if(signal_rat == NULL)
    {
        log_e("signal_rat == NULL\n");
        return signal_strength;
    }

    *signal_rat = RIL_NW_SIG_UNKNOWN;
    if( g_reg_state != NAS_REGISTERED_V01 )
    {
        log_e("data reg state = %d. can not get strength.\n", g_reg_state);
        return signal_strength;
    }
    memset(&resp,0,sizeof(nas_get_sig_info_resp_msg_v01));
    qmi_err = qmi_client_send_msg_sync( nas_user_handle,
                                         QMI_NAS_GET_SIG_INFO_REQ_MSG_V01,
                                         NULL,
                                         0,
                                         &resp,
                                         sizeof(nas_get_sig_info_resp_msg_v01),
                                         5000);
    if( (qmi_err != QMI_NO_ERR) || (resp.resp.result != QMI_NO_ERR) )
    {
        log_e("Can not get signal info %d : %d\n", qmi_err, resp.resp.error);
        //err = qmi_client_release(nas_user_handle);
        return signal_strength;
    }

    // NR5G signal
    if(resp.nr5g_sig_info_valid){

        if( resp.nr5g_sig_info.rsrp == -32768 ||
            resp.nr5g_sig_info.rsrp == -1 ||
            resp.nr5g_sig_info.rsrp == 0 ){
            if(RIL_DEBUG)
                log_e("[DEBUG][RESULT] 5g signal: NULL\n");
        }else
        {
            if(RIL_DEBUG)
                log_e("[DEBUG][RESULT] 5g signal rsrp: %ddBm\n", resp.nr5g_sig_info.rsrp);
            if(resp.nr5g_sig_info.rsrp > -44)
            {
                signal_strength = 0;
            }else if(resp.nr5g_sig_info.rsrp >= -85)
            {
                signal_strength = 5;
            }else if(resp.nr5g_sig_info.rsrp >= -95)
            {
                signal_strength = 4;
            }else if(resp.nr5g_sig_info.rsrp >= -105)
            {
                signal_strength = 3;
            }else if(resp.nr5g_sig_info.rsrp >= -115)
            {
                signal_strength = 2;
            }else if(resp.nr5g_sig_info.rsrp >= -140)
            {
                signal_strength = 1;
            }else
            {
                signal_strength = 0;
            }
            *signal_rat = RIL_NW_SIG_NR5G;
            return signal_strength;
        }
    }

    // LTE signal
    if(resp.lte_sig_info_valid){
        if(RIL_DEBUG)
        {
            log_d("[DEBUG][RESULT] 4g signal rsrp: %ddBm\n", resp.lte_sig_info.rsrp);
            log_d("[DEBUG][RESULT] 4g signal rsrq: %ddB\n", resp.lte_sig_info.rsrq);
            log_d("[DEBUG][RESULT] 4g signal rssi: %ddBm\n", resp.lte_sig_info.rssi);
        }
        if(resp.lte_sig_info.rsrp > -44)
        {
            signal_strength = 0;
        }else if(resp.lte_sig_info.rsrp >= -85)
        {
            signal_strength = 5;
        }else if(resp.lte_sig_info.rsrp >= -95)
        {
            signal_strength = 4;
        }else if(resp.lte_sig_info.rsrp >= -105)
        {
            signal_strength = 3;
        }else if(resp.lte_sig_info.rsrp >= -115)
        {
            signal_strength = 2;
        }else if(resp.lte_sig_info.rsrp >= -140)
        {
            signal_strength = 1;
        }else
        {
            signal_strength = 0;
        }
        *signal_rat = RIL_NW_SIG_LTE;
        return signal_strength;
    }

    // WCDMA signal
    if(resp.wcdma_sig_info_valid)
    {
        if(RIL_DEBUG)
            log_e("[DEBUG][RESULT] WCDMA signal rssi: %ddBm\n", resp.wcdma_sig_info.rssi);
        if(resp.wcdma_sig_info.rssi == 0)
        {
            signal_strength = 0;
        }else if(resp.wcdma_sig_info.rssi <= -113)
        {
            signal_strength = 0;
        }else if(resp.wcdma_sig_info.rssi <= -104)
        {
            signal_strength = 1;
        }else if(resp.wcdma_sig_info.rssi <= -98)
        {
            signal_strength = 2;
        }else if(resp.wcdma_sig_info.rssi <= -89)
        {
            signal_strength = 3;
        }else if(resp.wcdma_sig_info.rssi <= -80)
        {
            signal_strength = 4;
        }else
        {
            signal_strength = 5;
        }
        *signal_rat = RIL_NW_SIG_WCDMA;
        return signal_strength;
    }

    // GSM signal
    if(resp.gsm_sig_info_valid)
    {
        if(RIL_DEBUG)
            log_e("[DEBUG][RESULT] GSM signal rssi: %ddBm\n", resp.gsm_sig_info);
        if(resp.gsm_sig_info <= -113)
        {
            signal_strength = 0;
        }else if(resp.gsm_sig_info <= -104)
        {
            signal_strength = 1;
        }else if(resp.gsm_sig_info <= -98)
        {
            signal_strength = 2;
        }else if(resp.gsm_sig_info <= -89)
        {
            signal_strength = 3;
        }else if(resp.gsm_sig_info <= -80)
        {
            signal_strength = 4;
        }else if(resp.gsm_sig_info == 0)
        {
            signal_strength = 0;
        }else
        {
            signal_strength = 5;
        }
        *signal_rat = RIL_NW_SIG_GSM;
        return signal_strength;
    }

    //err = qmi_client_release(nas_user_handle);
#endif
    return signal_strength;
}

int get_strength_state(ril_nw_signal_rat_t *signal_rat) {
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    int signal_strength = 0;
    signal_strength = ril_nw_get_signal_strength_qmi(signal_rat);
    return signal_strength;
#else
    *signal_rat = (rand() % 5);
    return *signal_rat;
#endif
}

int get_sim_state() {
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    if( g_sim_info.sim_state_valid == FALSE )
    {
        log_d("sim state valid = false");
        ril_sim_get_card_status(&g_sim_info);
    }
    return g_sim_info.sim_state;
#else
    return READY;
#endif
}

sim_type_enum get_sim_type() {
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    if(g_sim_info.sim_type == CARD_TYPE_UNKNOWN) {
        ril_sim_get_card_status(&g_sim_info);
    }
    printf("return get_sim_type(): %d\n",g_sim_info.sim_type);
    return g_sim_info.sim_type;
#else
    return CARD_TYPE_UICC;
#endif
}

void check_sim_state() {
#if defined (ANDROID_PROPERTY)
    //for smx3, use to check/implement UI flow
    char prop[128];
    int len = property_get("gsm.sim.state", prop, "");
    if (len > 0 && strcmp(prop, "LOADED") == 0) {
        g_sim_info.sim_state = READY;
        update_sim_signal();
    }
#endif
    update_sim_signal();
}

int get_radio_tech() {
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    return ril_nw_get_rat_tech();
#else
    return DATA_BEARER_TECH_TYPE_5G;
#endif
}

void check_radio_tech() {
    update_radio_tech();
}

void read_data_usage(int from, long long int * rx, long long int * tx) {
    FILE *fp;
    if (from == DATA_USAGE_READ_FILE) {
        fp = fopen(DATA_USAGE_DUMP_FILE, "r+");
        if (fp == NULL) {
            log_e("Not found %s", DATA_USAGE_DUMP_FILE);
            return;
        }
    } else {
        fp = popen(DATA_USAGE_DUMP_CMD, "r");
        if (fp == NULL) {
            log_e("data usage dump popen failed");
            return;
        }
    }

    char buffer[150];
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        char *bytes = NULL;
        bytes = strstr(buffer, RX_BYTES);
        if (bytes != NULL) {
            bytes += (strlen(RX_BYTES));
            *rx = atoll(bytes);
            bytes = strstr(buffer, TX_BYTES);
            if (bytes != NULL) {
                bytes += (strlen(TX_BYTES));
                *tx = atoll(bytes);
            }
        }
    }
    if (from == DATA_USAGE_READ_FILE) {
        fclose(fp);
    } else {
        pclose(fp);
    }
}

void read_WWANStatistics(long long int * rx, long long int * tx) {
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    qcmap_msgr_wwan_statistics_type_v01 wwan_stats;

    memset(&wwan_stats, 0, sizeof(qcmap_msgr_wwan_statistics_type_v01));
    GetWWANStatistics(QCMAP_MSGR_IP_FAMILY_V4_V01, &wwan_stats);
    *rx = wwan_stats.bytes_rx;
    *tx = wwan_stats.bytes_tx;

    memset(&wwan_stats, 0, sizeof(qcmap_msgr_wwan_statistics_type_v01));
    GetWWANStatistics(QCMAP_MSGR_IP_FAMILY_V6_V01, &wwan_stats);
    *rx += wwan_stats.bytes_rx;
    *tx += wwan_stats.bytes_tx;
#endif
}

long long int get_intime_data_usage_bytes() {
    long long int rx, tx;
#if defined (USE_IFCONFIG_USAGE) || !defined(FEATURE_ROUTER)
    systemCmd(DATA_USAGE_DUMP);
    read_data_usage(DATA_USAGE_READ_FILE, &rx, &tx);
#else
    read_WWANStatistics(&rx, &tx);
#endif
    long long int value = data_usage_overflow_check(bytes_accumulated + rx + tx - reset_accumulated);
    return (value); //Bytes
}

double get_intime_data_usage() {
    return (get_intime_data_usage_bytes() / 1024.0 / 1024.0); //MB
}

long long int get_data_usage_bytes() {
    long long int value = data_usage_overflow_check(bytes_accumulated + rx_bytes + tx_bytes - reset_accumulated);
    if (value == 0 && bytes_accumulated != 0) {
        bytes_accumulated = 0;
        ds_set_long_long_int(DS_KEY_DATA_USAGE_ACCUMULATED, 0);
    }
    return (value); //Bytes
}

double get_data_usage() {
    return (get_data_usage_bytes() / 1024.0 / 1024.0); //MB
}

void dump_webui_data_usage() {
    char dumpCmd[100];

    if (ds_get_bool(DS_KEY_DATA_USAGE_MONITOR)) {
        sprintf(dumpCmd, "echo MaxDataUsage: %d > %s", ds_get_int(DS_KEY_MAX_DATA_USAGE), WEBUI_DATA_USAGE_FILE);
    } else {
        sprintf(dumpCmd, "echo MaxDataUsage: %d > %s", 0, WEBUI_DATA_USAGE_FILE);
    }
    systemCmd(dumpCmd);

    sprintf(dumpCmd, "echo DataUsage: %lld >> %s", get_data_usage_bytes(), WEBUI_DATA_USAGE_FILE);
    systemCmd(dumpCmd);
}

void dump_data_usage() {
/*
    FILE *fp = popen("ifconfig", "r");

    if (fp == NULL) {
        log_d("data_usage popen failed");
        return ;
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, RMNET_IPA)) {
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                char* rx_tag = strstr(buffer, RX_BYTES);
                if (rx_tag) {
                    char* tx_tag = strstr(rx_tag, TX_BYTES);
                    int rx = string_to_int_by_string(rx_tag, RX_BYTES, TX_BYTES);
                    int tx = string_to_int_by_index(tx_tag, strlen(TX_BYTES), strlen(tx_tag));
                    downlading = (rx != rx_bytes);
                    if (downlading) {
                        rx_bytes = rx;
                    }

                    uploading = (tx != tx_bytes);
                    if (uploading) {
                        tx_bytes = tx;
                    }
                    break;
                }
            }
            break;
        }
    }
    pclose(fp);
*/

    long long int rx, tx;
#if defined (USE_IFCONFIG_USAGE) || !defined(FEATURE_ROUTER)
    systemCmd(DATA_USAGE_DUMP);
    read_data_usage(DATA_USAGE_READ_FILE, &rx, &tx);

#else
    read_WWANStatistics(&rx, &tx);

    // error handling when usage value become < previous dump when internet PDN disconnected
    if (rx < rx_bytes || tx < tx_bytes) {
        log_e("Error: data usage value smaller than previous dump");
        bytes_accumulated += rx_bytes + tx_bytes - reset_accumulated;
        rx_bytes = rx;
        tx_bytes = tx;
        log_e("data usage accumulation updated for error handling: %lld", bytes_accumulated);
        bytes_accumulated = data_usage_overflow_check(bytes_accumulated);
        ds_set_long_long_int(DS_KEY_DATA_USAGE_ACCUMULATED, bytes_accumulated);
    }

    char cmd[100];
    sprintf(cmd, "echo %s%lld %s%lld > %s", RX_BYTES, rx, TX_BYTES, tx, DATA_USAGE_DUMP_FILE);
    systemCmd(cmd);
#endif

    downlading = (rx != rx_bytes);
    if (downlading) {
        rx_bytes = rx;
    }

    uploading = (tx != tx_bytes);
    if (uploading) {
        tx_bytes = tx;
    }

    //update_data_flow(); //move to monitor task to do it so can be done in main thread

    dump_webui_data_usage();

    if (!ds_get_bool(DS_KEY_DATA_USAGE_MONITOR) && enableIP) {
        // Still update data flow status icon even when monitor is off
        // but stop running below usage monitor related part
        return;
    }

    data_usage_reset_check();
    update_data_usage_bar();

    // check if should enable or disable data
    int max = ds_get_int(DS_KEY_MAX_DATA_USAGE);
    double usage = get_data_usage();
    if (usage >= max) {
        if (enableIP) {
            enableIP = false;
            enable_data(false);
        }
    } else {
        if (!enableIP) {
            enableIP = true;
            enable_data(true);
        }
    }
}

void data_usage_check() {
    if (ds_get_bool(DS_KEY_DATA_USAGE_MONITOR)) {
        int max = ds_get_int(DS_KEY_MAX_DATA_USAGE);
        double usage = get_intime_data_usage();
        if (usage >= max) {
            if (enableIP) {
                dump_data_usage();
            }
        } else {
            if (!enableIP) {
                dump_data_usage();
            }
        }
    }
}

void set_device_time(char * nitz_time) {
    if (nitz_time == NULL) return;
#ifdef FEATURE_ROUTER
    char value[3];
    memset(value, 0, sizeof(value));

    struct tm datetime;
    memset(&datetime, 0, sizeof(datetime));

    // year, month, day
    strncpy(value, nitz_time, 2);
    datetime.tm_year = atoi(value);
    strncpy(value, nitz_time + 3, 2);
    datetime.tm_mon = atoi(value) - 1;
    strncpy(value, nitz_time + 6, 2);
    datetime.tm_mday = atoi(value);

    // hour
    strncpy(value, nitz_time + 9, 2);
    datetime.tm_hour = atoi(value);

    // minutes with timezone
    strncpy(value, nitz_time + 12, 2);
    datetime.tm_min = atoi(value);
    strncpy(value, nitz_time + 18, 2);
    int tz_min_differ = atoi(value) * 15;
    if(!strncmp(nitz_time + 17, "+", strlen("+"))) {
        datetime.tm_min += tz_min_differ;
    } else {
        datetime.tm_min -= tz_min_differ;
    }

    // seconds
    strncpy(value, nitz_time + 15, 2);
    datetime.tm_sec = atoi(value);

    // daylight saving time
    strncpy(value, nitz_time + 21, 2);
    datetime.tm_isdst = atoi(value);
    // Per spec found by Modem team, "9.4.19.6 Network Daylight Saving Time,
    // This IE may be sent by the network.  If this IE is sent, the contents
    // of this IE indicates the value that has been used to adjust the local
    // time zone."  so remove adjust hour using dst
    //datetime.tm_hour += atoi(value);

    // no sim case nitz still return valid with 00 values, so use year
    // value to confirm if nitz time string valid or not
    if (datetime.tm_year != 0) {
        time_t t = mktime(&datetime);
        struct tm *local = localtime(&t);

        char cmd[50];
        //sprintf(cmd, "timedatectl set-time \"%02d-%02d-%02d %02d:%02d:%02d\"", local->tm_year,
        sprintf(cmd, "dci -t \"%02d-%02d-%02d %02d:%02d:%02d\"", local->tm_year,
                local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
        int res = systemCmd(cmd);
        log_d("%s result: %d", cmd, res);
    } else {
        log_d("time invalid, device date time not set.");
    }
#endif
}

void monitor_task() {
    //dump_data_usage(); // move to bg_thread
    update_data_flow();
    check_sim_state();
    check_radio_tech();
    //For cc_regdb, get mcc to wlan_country
    check_current_country();

    if (!set_nitz_b) {
        ril_nw_nitz_time_info_t nitz_time = {0};
        if (RIL_SUCCESS == get_nitz_time(&nitz_time)) {
            set_device_time(nitz_time.nitz_time);
            set_nitz_b = true;
#ifdef BATTERY_LOG
            init_battery_log();
#endif
        }
    }
}

void startRilDataMonitor() {
    if (lv_task == NULL) {
        lv_task = lv_task_create(monitor_task, 2000, LV_TASK_PRIO_LOW, NULL);
    }
}

void init_data_usage_info() {
    bytes_accumulated = ds_get_long_long_int(DS_KEY_DATA_USAGE_ACCUMULATED);
#if defined (USE_IFCONFIG_USAGE) || !defined(FEATURE_ROUTER)
    reset_accumulated = ds_get_long_long_int(DS_KEY_DATA_USAGE_RESET_ACCUMULATED);
#endif

    long long int file_rx = 0, file_tx = 0;
    read_data_usage(DATA_USAGE_READ_FILE, &file_rx, &file_tx);
#if defined (USE_IFCONFIG_USAGE) || !defined(FEATURE_ROUTER)
    read_data_usage(DATA_USAGE_READ_DUMP, &rx_bytes, &tx_bytes);
#else
    read_WWANStatistics(&rx_bytes, &tx_bytes);
#endif
    if (file_rx > rx_bytes) {
        // if accumulated value in file larger means there was a reboot
        // Update this value to data store
        bytes_accumulated += file_rx + file_tx - reset_accumulated;
        log_d("data usage accumulation updated to: %lld", bytes_accumulated);
        bytes_accumulated = data_usage_overflow_check(bytes_accumulated);
        ds_set_long_long_int(DS_KEY_DATA_USAGE_ACCUMULATED, bytes_accumulated);
    } else {
        // Might be first boot or just a restart not device reboot
        log_d("data usage accumulation no update needed");
    }

#if defined (USE_IFCONFIG_USAGE) || !defined(FEATURE_ROUTER)
    ds_set_long_int(DS_KEY_DATA_USAGE_RESET_ACCUMULATED, 0);
    reset_accumulated = 0;
#endif

    next_reset_time = ds_get_int(DS_KEY_DATA_USAGE_NEXT_RESET_TIME);
    if (next_reset_time == 0) {
        reset_date_usage_start_date();
    }
}

void reset_date_usage_start_date() {
    int day = atoi(ds_get_value(DS_KEY_DATA_USAGE_START_DATE));

    time_t t = time(NULL);
    struct tm next, *now = localtime(&t);
    memset(&next, 0, sizeof(next));
    next.tm_year = now->tm_year;

    if (day > now->tm_mday) {
        next.tm_mon = now->tm_mon;
    } else {
        next.tm_mon = now->tm_mon + 1;
    }

    // special handle for Feb 30th and 31st
    if (next.tm_mon == 2 && (day > 29)) {
        next.tm_mday = 29;
    } else {
        next.tm_mday = day;
    }
    next_reset_time = mktime(&next);
    ds_set_int(DS_KEY_DATA_USAGE_NEXT_RESET_TIME, next_reset_time);
    log_d("Next reset date: %d-%d-%d\n", next.tm_year + 1900, next.tm_mon + 1, next.tm_mday);
}

void enable_data(bool enable) {
#ifdef FEATURE_ROUTER
    if (enable) {
        log_d("data usage enable data");
        EnableIPV4();
        EnableIPV6();
    } else {
        log_d("data usage disable data");
        bytes_accumulated += rx_bytes + tx_bytes - reset_accumulated;
        rx_bytes = 0;
        tx_bytes = 0;
        log_d("data usage accumulation updated to before disable data: %lld", bytes_accumulated);
        bytes_accumulated = data_usage_overflow_check(bytes_accumulated);
        ds_set_long_long_int(DS_KEY_DATA_USAGE_ACCUMULATED, bytes_accumulated);

        DisableIPV4();
        DisableIPV6();
    }
#endif
}

bool is_data_downloading() {
    return downlading;
}

bool is_data_upgrading() {
    return uploading;
}

void get_imei(char *imei_buff, int buff_len){
#ifdef FEATURE_ROUTER
    qmi_client_error_type rc;
    qmi_idl_service_object_type dms_service_obj;
    int time_out = 4;
    qmi_client_os_params  os_params;
    qmi_client_type dms_user_handle;
    qmi_client_error_type qmi_err;
    dms_get_device_serial_numbers_req_msg_v01 get_device_serial_numbers_req_msg;
    dms_get_device_serial_numbers_resp_msg_v01 get_device_serial_numbers_resp_msg;

    if (NULL == imei_buff || 0 == buff_len){
        log_e("Buffer is empty.");
        return;
    }

    dms_service_obj = dms_get_service_object_v01();
    rc = qmi_client_init_instance( dms_service_obj,
                                   QMI_CLIENT_INSTANCE_ANY,
                                   NULL,
                                   NULL,
                                   &os_params,
                                   time_out,
                                   &dms_user_handle );
    if (rc != QMI_NO_ERR )
    {
         log_e("Error: connection not Initialized...Error Code:%d\n",rc);
         rc = qmi_client_release(dms_user_handle);
         if (rc < 0 )
         {
            log_e("Release not successful \n");
         }
         return ;
    }
    else
    {
        printf("Connection Initialized....User Handle:%p\n", dms_user_handle);
    }

    memset(&get_device_serial_numbers_req_msg, 0, sizeof(dms_get_device_serial_numbers_req_msg_v01));
    memset(&get_device_serial_numbers_resp_msg, 0, sizeof(dms_get_device_serial_numbers_resp_msg_v01));

    qmi_err = qmi_client_send_msg_sync(
                   dms_user_handle,
                   QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_REQ_V01,
                   (void*) &get_device_serial_numbers_req_msg,
                   sizeof(dms_get_device_serial_numbers_req_msg_v01),
                   (void*) &get_device_serial_numbers_resp_msg,
                   sizeof(dms_get_device_serial_numbers_resp_msg_v01),
                   5000);

    if(QMI_NO_ERR == qmi_err)
    {
        if(get_device_serial_numbers_resp_msg.resp.result == 0)
        {
            if(get_device_serial_numbers_resp_msg.imei_valid == 1){
                if (strlen(get_device_serial_numbers_resp_msg.imei) + 1 > buff_len){
                    log_e("Buffer overflow.");
                }else{
                    memcpy(imei_buff, get_device_serial_numbers_resp_msg.imei, strlen(get_device_serial_numbers_resp_msg.imei) + 1);
                }
            }else{
                log_e("device imei is invalid. imei_valid=%d\n",  get_device_serial_numbers_resp_msg.imei_valid);
            }
        }else
        {
            log_e("QMI get data settings resp error. error code = 0x%x\n", get_device_serial_numbers_resp_msg.resp.error);
        }
    }else{
        log_e("qmi_err = %d\n", qmi_err);
    }

    rc = qmi_client_release(dms_user_handle);
    if (rc < 0 )
    {
        log_e("Release not successful \n");
    }
    else
    {
        printf("QMI client release successful \n");
    }
#endif
}

void get_phone_num(char *num_buff, int buff_len){
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    ril_sim_get_phone_num(num_buff, buff_len);
#endif
}

ril_error_type change_sim_pin(char *new_pin, char *old_pin){
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    int err = RIL_ERROR_UNKNOWN;
    sim_pin_state_enum pin_state = PIN_UNKNOWN;

    pin_state = get_sim_pin1_state();
    // only change pin1 when sim pin enabled
    if(pin_state == PIN_ENABLED_VERIFIED)
        err = ril_sim_change_pin(new_pin, old_pin);
    store_sim_pin(new_pin);

    if( err != RIL_SUCCESS){
        log_e("ril_sim_change_pin error: %d.", err);
    }
    return err;
#endif
}

ril_error_type verify_sim_pin(char *pin1){
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    int err;
    err = ril_sim_verify_pin(pin1);
    store_sim_pin(pin1);
    if( err != RIL_SUCCESS){
        log_e("ril_sim_verify_pin error: %d.", err);
    }
    return err;
#endif
}

ril_error_type enable_disable_sim_pin(int enable, char *pin1){
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    int err = 0;
    sim_pin_state_enum pin_state = PIN_UNKNOWN;

    pin_state = get_sim_pin1_state();
    if(enable){
        // chcek pin1 state. if already enabled, ignore this request
        if( pin_state == PIN_DISABLED ) {
            err = ril_sim_enable_pin(pin1);
            store_sim_pin(pin1);
        }
    }else{
        if( pin_state == PIN_ENABLED_VERIFIED ) {
            err = ril_sim_disable_pin(pin1);
            store_sim_pin("");
        }
    }

    if( err != RIL_SUCCESS){
        log_e("ril_sim_enable_pin / ril_sim_disable_pin. is_enable = %d, error: %d.", enable, err);
    }
    return err;
#endif
}

sim_pin_state_enum get_sim_pin1_state(void)
{
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    if( g_sim_info.sim_pin1_state == PIN_UNKNOWN )
    {
        ril_sim_get_card_status(&g_sim_info);
    }
    log_d("g_sim_info.sim_pin1_state = %d", g_sim_info.sim_pin1_state);

    return g_sim_info.sim_pin1_state;
#endif
}

sim_pin_state_enum get_sim_pin2_state(void)
{
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    if( g_sim_info.sim_pin1_state == PIN_UNKNOWN )
    {
        ril_sim_get_card_status(&g_sim_info);
    }
    return g_sim_info.sim_pin2_state;
#endif
}

int is_4G_network_supported()
{
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    ril_nw_config curr_config = {0};
    ril_error_type ret = RIL_SUCCESS;

    ret = ril_nw_get_config_qmi(&curr_config);
    if(ret == RIL_SUCCESS)
    {
        log_d("curr_config.pref_nw_mode = %d", (int)curr_config.pref_nw_mode);
        //if(curr_config.pref_nw_mode & RIL_NW_MODE_LTE)
        g_ril_pref_mode = curr_config.pref_nw_mode;
        if(curr_config.pref_nw_mode & QMI_NAS_RAT_MODE_PREF_LTE_V01)
            return true;
        else
            return false;
    }else
    {
        log_e("ril_nw_get_config fail. ret=%d.", ret);
        return false;
    }
#else
    return true;
#endif
}

ril_error_type support_4G_network(int enable)
{
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    ril_nw_config curr_config = {0};
    ril_error_type ret = RIL_SUCCESS;

    ret = ril_nw_get_config_qmi(&curr_config);

    if(ret == RIL_SUCCESS)
    {
        if(enable)
        {
            //curr_config.pref_nw_mode = curr_config.pref_nw_mode | RIL_NW_MODE_LTE;
            curr_config.pref_nw_mode = curr_config.pref_nw_mode | QMI_NAS_RAT_MODE_PREF_LTE_V01;
        }else
        {
            //curr_config.pref_nw_mode = curr_config.pref_nw_mode & ~(RIL_NW_MODE_LTE);
            curr_config.pref_nw_mode = curr_config.pref_nw_mode & ~(QMI_NAS_RAT_MODE_PREF_LTE_V01);
        }
        ret = ril_nw_set_config_qmi(&curr_config);
        if( ret != RIL_SUCCESS )
            log_e("ril_nw_set_config fail. ret=%d.", ret);
    }else
    {
        log_e("ril_nw_get_config fail. ret=%d.", ret);
    }
    return ret;
#else
    return 0;
#endif
}

ril_error_type set_nw_selection_auto()
{
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    return ril_nw_set_network_selection_auto_qmi();
#else
    return 0;
#endif
}

ril_error_type set_nw_selection_manual(ril_nw_scan_entry_t *entry)
{
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    return ril_nw_set_network_selection_manual_qmi(entry);
#else
    return 0;
#endif
}

#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
nas_perform_network_scan_resp_msg_v01* tmp_resp_ptr;

void nw_scan_cb_handler(ril_nw_scan_result_t *ril_nw_scan_resp_data)
{
    int iter_i;
    char *qmi_nw_info_mcc_str;
    char *qmi_nw_info_mnc_str;
    char *long_eons;
    char *short_eons;
    int nw_cnt = 0;
    uint8_t nw_status;
    int rat = 0, modem_rat;
    uint8_t plmn_rat = 0;
    int network_status;

    if(tmp_resp_ptr == NULL)
    {
        log_e("tmp_resp_ptr == NULL");
        return ;
    }

    log_e("nw_scan_cb_handler tmp_resp_ptr->nas_3gpp_network_info_valid=%d\n", tmp_resp_ptr->nas_3gpp_network_info_valid);
    if(tmp_resp_ptr->nas_3gpp_network_info_valid)
    {
        log_e("nw_scan_cb_handler info_len=%d\n", (int)tmp_resp_ptr->nas_3gpp_network_info_len);
        memset(ril_nw_scan_resp_data, 0, sizeof(ril_nw_scan_result_t));
        for(iter_i = 0; iter_i < (int)tmp_resp_ptr->nas_3gpp_network_info_len; iter_i++)
        {
            qmi_nw_info_mcc_str = ril_nw_scan_resp_data->entry[iter_i].operator_name.mcc;
            qmi_nw_info_mnc_str = ril_nw_scan_resp_data->entry[iter_i].operator_name.mnc;
            long_eons = ril_nw_scan_resp_data->entry[iter_i].operator_name.long_name;
            short_eons = ril_nw_scan_resp_data->entry[iter_i].operator_name.short_name;

            snprintf( qmi_nw_info_mcc_str, RIL_NW_MCC_MAX_LENGTH+1, "%03d", (int)tmp_resp_ptr->nas_3gpp_network_info[iter_i].mobile_country_code );

            if ( (tmp_resp_ptr->nas_3gpp_network_info[iter_i].mobile_network_code > 99) ||
                 (tmp_resp_ptr->mnc_includes_pcs_digit_valid &&
                  tmp_resp_ptr->mnc_includes_pcs_digit[iter_i].mnc_includes_pcs_digit) )
            {
                snprintf( qmi_nw_info_mnc_str, NAS_MCC_MNC_MAX_V01+1, "%03d", (int)tmp_resp_ptr->nas_3gpp_network_info[iter_i].mobile_network_code );
            }
            else
            {
                snprintf( qmi_nw_info_mnc_str, NAS_MCC_MNC_MAX_V01+1, "%02d", (int)tmp_resp_ptr->nas_3gpp_network_info[iter_i].mobile_network_code );
            }

            if( strncmp(g_sim_mcc_cache, qmi_nw_info_mcc_str, strlen(qmi_nw_info_mcc_str)) == 0 )
            {
                if( strncmp(g_sim_mnc_cache, qmi_nw_info_mnc_str, strlen(qmi_nw_info_mnc_str)) == 0 )
                {
                    ril_nw_scan_resp_data->entry[iter_i].is_home_nw = 1;
                }
            }

            if( TRUE == ril_nas_check_centralized_eons_support_status() )
            {
                if ( tmp_resp_ptr->nas_network_radio_access_technology_valid )
                {
                    plmn_rat = tmp_resp_ptr->nas_network_radio_access_technology[iter_i].rat;
                }
                ril_nas_handle_centralized_short_long_eons(tmp_resp_ptr->nas_3gpp_network_info[iter_i].mobile_country_code, tmp_resp_ptr->nas_3gpp_network_info[iter_i].mobile_network_code, short_eons, long_eons, plmn_rat);
            }

            if ( *(long_eons) && !*(short_eons) )
            {
                log_d("Filling short eons with long eons");
                strncpy( short_eons, long_eons, sizeof(short_eons));
            }

            if ( *(short_eons) && !*(long_eons) )
            {
                log_d("Filling long eons with short eons");
                strncpy( long_eons, short_eons, sizeof(long_eons));
            }

            nw_status = tmp_resp_ptr->nas_3gpp_network_info[iter_i].network_status;
            if ( (nw_status & RIL_QMI_NW_SCAN_RES_ENTRY_CUR_SERVING) )
            {
                network_status = RIL_NW_NETWORK_STATUS_CURRENT_SERVING_V01;
            }
            else if ( nw_status & RIL_QMI_NW_SCAN_RES_ENTRY_FORBIDDEN )
            {
                network_status = RIL_NW_NETWORK_STATUS_FORBIDDEN_V01;
            }
            else if ( nw_status & RIL_QMI_NW_SCAN_RES_ENTRY_AVAILABLE )
            {
                network_status = RIL_NW_NETWORK_STATUS_AVAILABLE_V01;
            }
            else
            {
                log_e("Unknown network status: %d", tmp_resp_ptr->nas_3gpp_network_info[iter_i].network_status);
                network_status = 0;
            }

            if( tmp_resp_ptr->nas_network_radio_access_technology_valid )
            {
                modem_rat = tmp_resp_ptr->nas_network_radio_access_technology[nw_cnt++].rat;
                switch(modem_rat)
                {
                    case NAS_RADIO_IF_CDMA2000:
                        rat = RIL_NW_RADIO_TECH_1xRTT_V01;
                        break;
                    case NAS_RADIO_IF_CDMA2000_HRPD:
                        rat = RIL_NW_RADIO_TECH_EHRPD_V01;
                        break;
                    case NAS_RADIO_IF_GSM:
                        rat = RIL_NW_RADIO_TECH_GSM_V01;
                        break;
                    case NAS_RADIO_IF_UMTS:
                        rat = RIL_NW_RADIO_TECH_UMTS_V01;
                        break;
                    case NAS_RADIO_IF_LTE:
                        rat = RIL_NW_RADIO_TECH_LTE_V01;
                        break;
                    case NAS_RADIO_IF_TDSCDMA:
                        rat = RIL_NW_RADIO_TECH_TD_SCDMA_V01;
                        break;
                    case NAS_RADIO_IF_NR5G:
                        rat = RIL_NW_RADIO_TECH_NR5G_V01;
                        break;
                    default:
                        log_e("Unhandled modem rat: %d", modem_rat);
                        rat = 0;
                        break;
                }
            }

                log_d("Network #%d - %s/%s/%d/%d",
                                iter_i+1,
                                long_eons,
                                short_eons,
                                network_status,
                                rat);
                strncpy(ril_nw_scan_resp_data->entry[iter_i].operator_name.mcc, qmi_nw_info_mcc_str, RIL_NW_MCC_MAX_LENGTH+1);
                strncpy(ril_nw_scan_resp_data->entry[iter_i].operator_name.mnc, qmi_nw_info_mnc_str, RIL_NW_MNC_MAX_LENGTH+1);
                ril_nw_scan_resp_data->entry[iter_i].rat = rat;
                ril_nw_scan_resp_data->entry[iter_i].network_status = network_status;
                ril_nw_scan_resp_data->entry_len++;

                if(RIL_DEBUG)
                {
                    log_e("nw_scan_cb_handler. i=%d. mcc=%s, mnc=%s.\n", iter_i, ril_nw_scan_resp_data->entry[iter_i].operator_name.mcc, ril_nw_scan_resp_data->entry[iter_i].operator_name.mnc);
                    log_e("nw_scan_cb_handler. i=%d. rat=%d, network_status=%d.\n", iter_i, ril_nw_scan_resp_data->entry[iter_i].rat, ril_nw_scan_resp_data->entry[iter_i].network_status);
                    log_e("nw_scan_cb_handler. long_name = %s.\n", ril_nw_scan_resp_data->entry[iter_i].operator_name.long_name);
                    log_e("nw_scan_cb_handler. short_name = %s.\n", ril_nw_scan_resp_data->entry[iter_i].operator_name.short_name);
                    log_e("nw_scan_cb_handler. is_home = %d.\n", ril_nw_scan_resp_data->entry[iter_i].is_home_nw);
                }
        }
    }
}

static void scan_nw_resp_thread(void* user_handle)
{
    int rc;
    ril_nw_scan_result_t nw_scan_resp_data = {0};
    struct timeval now;
    struct timespec outtime;

    log_e("scan_nw_resp_thread ENTER\n");
    pthread_mutex_lock(&async_resp);
    gettimeofday(&now, NULL);
    outtime.tv_sec = now.tv_sec + NETWORK_SCAN_ASYNC_TIMEOUT_SEC;
    outtime.tv_nsec = now.tv_usec * 1000;

    rc = pthread_cond_timedwait(&async_resp_cond, &async_resp, &outtime);
    if( rc == 0 )
    {
        log_e("signaled. rc=%d\n", rc);
        if (nw_scan_rec_flag == MESSAGE_RECEIVED ){
            nw_scan_cb_handler(&nw_scan_resp_data);
            (*rilNwCb)(RIL_NW_RESP_NW_SCAN, RIL_SUCCESS, sizeof(nw_scan_resp_data), &nw_scan_resp_data);
        }
        else if (nw_scan_rec_flag == MESSAGE_RECEIVED_TRANSPORT_ERROR) {
            log_e(" Message received but there was a transport error \n");
            memset(&nw_scan_resp_data, 0, sizeof(nw_scan_resp_data));
            (*rilNwCb)(RIL_NW_RESP_NW_SCAN, RIL_ERROR_UNKNOWN, sizeof(nw_scan_resp_data), &nw_scan_resp_data);
        }
        else if (nw_scan_rec_flag == MESSAGE_NOT_RECEIVED) {
            log_e(" The asynchronous message sending failed...so no need to wait for reading it \n");
            memset(&nw_scan_resp_data, 0, sizeof(nw_scan_resp_data));
            (*rilNwCb)(RIL_NW_RESP_NW_SCAN, RIL_ERROR_UNKNOWN, sizeof(nw_scan_resp_data), &nw_scan_resp_data);
        }
    }else
    {
        log_e("timed out. rc=%d\n", rc);
        // TODO: if multiple clients cb. need to check which one is dropped.
        g_nw_scan_time_out_flag = TRUE;
    }

    if(tmp_resp_ptr)
    {
        free(tmp_resp_ptr);
        tmp_resp_ptr = NULL;
    }

    pthread_mutex_unlock(&async_resp);
    pthread_exit(NULL);
}

static void scan_nw_thread( void* user_handle )
{
    int thread_rc;
    int err;

    thread_rc = pthread_create(&async_resp_handler, NULL, scan_nw_resp_thread, NULL);

    if (thread_rc) {
        log_e("pthread_create failed \n");
        nw_scan_err = RIL_ERROR_UNKNOWN;
        pthread_exit(NULL);
    }

    err = get_sim_mcc_mnc(g_sim_mcc_cache, 4, g_sim_mnc_cache, 4);
    if( err != RIL_SUCCESS )
    {
        log_e("Can not get sim card mcc mnc. err=%d", err);
        memset(g_sim_mcc_cache, '\0', 4);
        memset(g_sim_mnc_cache, '\0', 4);
    }

    if( nw_scan_rec_flag == MESSAGE_SENT_WAITING_CB )
    {
        nw_scan_err = RIL_ERROR_NETWORK_BUSY;
        log_e("ERROR. nw_scan_rec_flag waiting callback");
        pthread_exit(NULL);
    }

    if( g_ril_pref_mode == 0 )
    {
        is_4G_network_supported();
    }

    nw_scan_err = ril_nw_scan_network_qmi(g_ril_pref_mode);

    if (nw_scan_err == RIL_SUCCESS ) {
        log_d("Starting network interface asyncronously...\n");
        pthread_mutex_lock(&async_resp);
        nw_scan_rec_flag = MESSAGE_SENT_WAITING_CB;
        pthread_mutex_unlock(&async_resp);
    }
    else {
        log_e("ERROR in sending the start n/w interface message, ERROR CODE:%d", nw_scan_err);
        pthread_mutex_lock(&async_resp);
        nw_scan_rec_flag = MESSAGE_NOT_RECEIVED;
        pthread_cond_signal(&async_resp_cond);
        pthread_mutex_unlock(&async_resp);
    }
    pthread_exit(NULL);
}
#endif

ril_error_type scan_network()
{
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    int rc;
    pthread_t l_thread_scan_nw;
    void *status;

    rc = pthread_create(&l_thread_scan_nw, NULL, scan_nw_thread, NULL);

    if (rc) {
        log_e("Error in creating the scan_nw_thread thread \n");
        return RIL_ERROR_UNKNOWN;
    }

    rc = pthread_join(l_thread_scan_nw, &status);
    if (rc == -1 ) {
        log_e("Error in pthread_join scan_nw_thread thread \n");
    }
    return nw_scan_err;
#else
    return 0;
#endif
}

int get_sim_pin1_retries()
{
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    if( g_sim_info.pin1_retries == -1 )
    {
        // sim maybe not ready, try once
        ril_sim_get_card_status(&g_sim_info);
    }
    return g_sim_info.pin1_retries;
#else
    return 0;
#endif
}

int get_sim_puk1_retries()
{
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    if( g_sim_info.puk1_retries == -1 )
    {
        // sim maybe not ready, try once
        ril_sim_get_card_status(&g_sim_info);
    }
    return g_sim_info.puk1_retries;
#else
    return 0;
#endif
}

ril_error_type verify_sim_puk(char *puk1, char *new_pin1){
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    int err;
    err = ril_sim_verify_puk(puk1, new_pin1);
    store_sim_pin(new_pin1);
    if( err != RIL_SUCCESS){
        log_e("verify_sim_puk error: %d.", err);
    }
    return err;
#endif
}

ril_error_type get_nitz_time(ril_nw_nitz_time_info_t *resp_buff)
{
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    return ril_nw_get_nitz_time(resp_buff);
#else
    return 0;
#endif
}

void store_sim_pin(char* pin) {
    ds_set_value(DS_KEY_SIM_PIN_VALUE, pin);
}

char* get_sim_pin() {
    return ds_get_value(DS_KEY_SIM_PIN_VALUE);
}

bool is_nr5g_icon_supported()
{
#ifdef FEATURE_ROUTER
    dsd_get_ui_info_resp_msg_v01 resp;
    qmi_client_error_type qmi_err;
    memset(&resp, 0, sizeof(dsd_get_ui_info_resp_msg_v01));

    if(dsd_user_handle <= 0)
    {
        log_e("dsd_user_handle error=%d.", dsd_user_handle);
        return false;
    }

    qmi_err = qmi_client_send_msg_sync(
                  dsd_user_handle,
                  QMI_DSD_GET_UI_INFO_REQ_V01,
                  NULL,
                  0,
                  &resp,
                  sizeof(dsd_get_ui_info_resp_msg_v01),
                  5000);

    if (QMI_NO_ERR == qmi_err)
    {
        if (resp.resp.result == 0)
        {
            if(resp.global_ui_info_valid = true)
            {
                if(0 != (QMI_DSD_UI_MASK_3GPP_5G_UWB_V01 & resp.global_ui_info.ui_mask))
                    return true;
                if(0 != (QMI_DSD_UI_MASK_3GPP_5G_BASIC_V01 & resp.global_ui_info.ui_mask))
                    return true;
            }else
            {
                log_e("is_nr5g_icon_supported resp.global_ui_info_valid = false.");
            }
        }else
        {
            log_e("is_nr5g_icon_supported resp.resp.error = %d.", resp.resp.error);
        }
    }else
    {
        log_e("is_nr5g_icon_supported qmi_err = %d.", qmi_err);
    }
#endif
    return false;
}

ril_nw_roaming_state get_roaming_status()
{
#ifdef FEATURE_ROUTER
    log_e("get_roaming_status. g_reg_state = %d, g_nw_roaming_state = %d", g_reg_state, g_nw_roaming_state);
    if( g_reg_state == NAS_REGISTERED_V01 )
    {
        return g_nw_roaming_state;
    }else
    {
        return RIL_NW_ROAMING_STATE_UNKNOWN;
    }
#endif
    return RIL_NW_ROAMING_STATE_UNKNOWN;
}

//For cc_regdb, get mcc to wlan_country
void get_service_mcc(int *mcc_num) {
#ifdef FEATURE_ROUTER
    int time_out = 4;
    qmi_client_error_type qmi_err;
    nas_get_serving_system_resp_msg_v01 nas_get_serving_system_resp_msg;

    memset(&nas_get_serving_system_resp_msg, 0, sizeof(nas_get_serving_system_resp_msg_v01));

    qmi_err = qmi_client_send_msg_sync(nas_user_handle,
                                         QMI_NAS_GET_SERVING_SYSTEM_REQ_MSG_V01,
                                         NULL,
                                         0,
                                         &nas_get_serving_system_resp_msg,
                                         sizeof(nas_get_serving_system_resp_msg_v01),
                                         5000);

    if(QMI_NO_ERR == qmi_err)
    {
          //log_e("[cc_regdb] resp.error: %d, resp.result: %d\n", nas_get_serving_system_resp_msg.resp.error, nas_get_serving_system_resp_msg.resp.result);
          //log_e("[cc_regdb] operator name: %s, sizeof: %d\n", nas_get_serving_system_resp_msg.current_plmn.network_description,
          //                                                  sizeof(nas_get_serving_system_resp_msg.current_plmn.network_description));
          log_e("[cc_regdb] mcc: %d, mnc: %d, g_reg_state: %d\n", nas_get_serving_system_resp_msg.current_plmn.mobile_country_code, nas_get_serving_system_resp_msg.current_plmn.mobile_network_code, g_reg_state);
          *mcc_num = nas_get_serving_system_resp_msg.current_plmn.mobile_country_code;
          //log_e("[cc_regdb] return mcc_num:%d\n",*mcc_num);
          //touch = true;
          g_mcc = nas_get_serving_system_resp_msg.current_plmn.mobile_country_code;
          g_mnc = nas_get_serving_system_resp_msg.current_plmn.mobile_network_code;
          g_reg_state = nas_get_serving_system_resp_msg.serving_system.registration_state;
          //log_e("[cc_regdb] g_reg_state:%d\n", g_reg_state);

          if(strlen(nas_get_serving_system_resp_msg.current_plmn.network_description) < OPERATOR_NAME_MAX_LENGTH)
          {
              memset(nw_description, '\0', sizeof(nw_description));
              strncpy(nw_description, nas_get_serving_system_resp_msg.current_plmn.network_description, strlen(nas_get_serving_system_resp_msg.current_plmn.network_description));
          }else
          {
              log_e("[cc_regdb] Not copied. operator name: %s.\n", nas_get_serving_system_resp_msg.current_plmn.network_description);
          }

          if(nas_get_serving_system_resp_msg.roaming_indicator_valid)
          {
              if( nas_get_serving_system_resp_msg.roaming_indicator == NAS_ROAMING_IND_OFF_V01 )
                  g_nw_roaming_state = RIL_NW_ROAMING_STATE_ROAMING_OFF;
              else
                  g_nw_roaming_state = RIL_NW_ROAMING_STATE_ROAMING_ON;
          }else
          {
              g_nw_roaming_state = RIL_NW_ROAMING_STATE_UNKNOWN;
          }
    }else{
        log_e("[cc_regdb] qmi_err = %d\n", qmi_err);
    }
#endif
}

ril_error_type get_sim_mcc_mnc(char* mcc, int mcc_buff_len, char* mnc, int mnc_buff_len)
{
#ifdef FEATURE_ROUTER
    char imsi[SIM_IMSI_LENGTH+1]={0};
    int ret = RIL_ERROR_UNKNOWN, mnc_length = 0;

    if(mcc == NULL || mnc == NULL)
    {
        log_e("get_sim_mcc_mnc, NULL pointer parameters.\n");
        return RIL_ERROR_INVALID_INPUT;
    }

    // mcc length is 3. mnc length is 2 or 3.
    if(mcc_buff_len < 4 || mnc_buff_len < 4)
    {
        log_e("get_sim_mcc_mnc, invalid string length.\n");
        return RIL_ERROR_INVALID_INPUT;
    }

    memset(mcc, '\0', mcc_buff_len);
    memset(mnc, '\0', mnc_buff_len);

    ret = ril_sim_get_imsi(imsi, SIM_IMSI_LENGTH+1);
    if( ret == RIL_SUCCESS )
    {
        // get EF_AD
        ret = ril_sim_get_mnc_length(get_sim_type(), &mnc_length);
        // check known 3-digit MNCs.
        if( mnc_length == 0 || ((mnc_length == 2)&&(strlen(imsi)>=6)) )
        {
            int arrsize = sizeof(MCCMNC_CODES_HAVING_3DIGITS_MNC)/sizeof(MCCMNC_CODES_HAVING_3DIGITS_MNC[0]);
            char mccmnc[7] = {'\0'};
            strncpy(mccmnc, imsi, 6);
            log_d("get_sim_mcc_mnc, arrsize=%d. mccmnc=%s\n", arrsize, mccmnc);
            int i;
            for(i=0; i<arrsize; i++)
            {
                //log_d("comparing mccmnc=%s. MCCMNC_table=%s\n", mccmnc, MCCMNC_CODES_HAVING_3DIGITS_MNC[i]);
                if(strncmp(mccmnc, MCCMNC_CODES_HAVING_3DIGITS_MNC[i], 6) == 0)
                {
                    // matched!
                    log_d("get_sim_mcc_mnc, mcc=%s, mcc=%s.\n", mcc, mnc);
                    strncpy(mcc, mccmnc, 3);
                    strncpy(mnc, mccmnc+3, 3);
                    return RIL_SUCCESS;
                }
            }
            // Not found
            log_d("No matched item.");
            if( mnc_length == 0 )
            {
                log_e("Unknown mnc length.");
                return RIL_ERROR_UNKNOWN;
            }
        }
        // mnc mcc
        strncpy(mcc, imsi, 3);
        strncpy(mnc, imsi+3, mnc_length);
        log_d("get_sim_mcc_mnc, mcc=%s, mcc=%s.\n", mcc, mnc);
    }
    return ret;
#else
    return RIL_ERROR_UNKNOWN;
#endif
}

#ifdef FEATURE_ROUTER
ril_qcmap_wwan_status_enum_t converWWANStatusToRIL(qcmap_msgr_wwan_status_enum_v01 qcmap_wwan_status)
{
    ril_qcmap_wwan_status_enum_t ret;
    switch(qcmap_wwan_status)
    {
        case QCMAP_MSGR_WWAN_STATUS_CONNECTING_V01:
            ret = RIL_QCMAP_WWAN_V4_STATUS_CONNECTING;
            break;
        case QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL_V01:
            ret = RIL_QCMAP_WWAN_V4_STATUS_CONNECTING_FAIL;
            break;
        case QCMAP_MSGR_WWAN_STATUS_CONNECTED_V01:
            ret = RIL_QCMAP_WWAN_V4_STATUS_CONNECTED;
            break;
        case QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_V01:
            ret = RIL_QCMAP_WWAN_V4_STATUS_DISCONNECTING;
            break;
        case QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_FAIL_V01:
            ret = RIL_QCMAP_WWAN_V4_STATUS_DISCONNECTING_FAIL;
            break;
        case QCMAP_MSGR_WWAN_STATUS_DISCONNECTED_V01:
            ret = RIL_QCMAP_WWAN_V4_STATUS_DISCONNECTED;
            break;
        case QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_V01:
            ret = RIL_QCMAP_WWAN_V6_STATUS_CONNECTING;
            break;
        case QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_FAIL_V01:
            ret = RIL_QCMAP_WWAN_V6_STATUS_CONNECTING_FAIL;
            break;
        case QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED_V01:
            ret = RIL_QCMAP_WWAN_V6_STATUS_CONNECTED;
            break;
        case QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING_V01:
            ret = RIL_QCMAP_WWAN_V6_STATUS_DISCONNECTING;
            break;
        case QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING_FAIL_V01:
            ret = RIL_QCMAP_WWAN_V6_STATUS_DISCONNECTING_FAIL;
            break;
        case QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED_V01:
            ret = RIL_QCMAP_WWAN_V6_STATUS_DISCONNECTED;
            break;
        default:
            ret = RIL_QCMAP_WWAN_STATUS_UNKNOWN;
            log_e("Unknown qcmap_wwan_status.");
            break;
    }
    return ret;
}
#endif

ril_error_type get_wwan_status(ril_qcmap_wwan_status_enum_t *ipv4, ril_qcmap_wwan_status_enum_t *ipv6)
{
#ifdef FEATURE_ROUTER
    qcmap_msgr_wwan_status_enum_v01 v4_status, v6_status;

    if(GetWWANStatus(&v4_status, &v6_status))
    {
        *ipv4 = converWWANStatusToRIL(v4_status);
        *ipv6 = converWWANStatusToRIL(v6_status);
    }else
    {
        log_e("GetWWANStatus fail.");
        return RIL_ERROR_UNKNOWN;
    }
    return RIL_SUCCESS;
#else
    return RIL_ERROR_UNKNOWN;
#endif
}

int get_lte_ca_activated(void)
{
    ril_nw_lte_ca_info_t ca_info={0};
    int ca_status = 0, err;
#ifdef FEATURE_ROUTER
    err = ril_nw_get_lte_ca_info(&ca_info);
    if(err == RIL_SUCCESS)
    {
        if(ca_info.scell_info_list_len > 0)
            ca_status = 1;
    }
#endif
    return ca_status;
}

bool is_wwan_connected()
{
#ifdef FEATURE_ROUTER
    ril_qcmap_wwan_status_enum_t ipv4, ipv6;
    bool ret = 0;

    if(get_wwan_status(&ipv4, &ipv6) != RIL_SUCCESS)
    {
        ret = 0;
    }else
    {
        if( ipv4 == RIL_QCMAP_WWAN_V4_STATUS_CONNECTED ||
            ipv6 == RIL_QCMAP_WWAN_V6_STATUS_CONNECTED )
            ret = 1;
    }
    return ret;
    //if(qcmap_ind_wwan_v4_status == QCMAP_MSGR_WWAN_STATUS_CONNECTED_V01 ||
       //qcmap_ind_wwan_v6_status == QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED_V01)
        //return 1;
#endif
    return 0;
}

ril_error_type get_network_config(ril_get_network_config_resp *nw_config)
{
#ifdef FEATURE_ROUTER
    if( !is_wwan_connected() )
    {
        log_e("get_network_config. wwan disconnected.");
        return RIL_ERROR_UNKNOWN;
    }
    return ril_get_nw_configuration(nw_config);
#endif
}

bool is_mccmnc_available(int state) {
    bool ret = false;
    switch (state)
    {
        case NETWORK_LOCKED:
        case READY:
        case ILLEGAL:
            ret = true;
            break;
        default:
            ret = false;
            break;
    }
    return ret;
}
