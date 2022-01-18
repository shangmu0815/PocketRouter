#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "ril.h"
#include "lv_pocket_router/src/util/debug_log.h"
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
#include "data_system_determination_v01.h"
#include "wireless_data_service_v01.h"

#define NAS_SET_BIT( flag_variable, value)              flag_variable |= (1<<value)
#define NAS_IS_BIT_SET(flag_variable, value)           ((flag_variable & (1<<value))?TRUE:FALSE)

extern qmi_client_type nas_user_handle;
extern qmi_client_type dsd_user_handle;
extern ril_nw_resp_cb rilNwCb;

extern nas_perform_network_scan_resp_msg_v01* tmp_resp_ptr;
extern pthread_mutex_t async_resp;
extern pthread_cond_t async_resp_cond;
extern int nw_scan_rec_flag;
extern int g_nw_scan_time_out_flag;

extern ril_nw_network_time_info_type ril_nw_network_time_info;
extern ril_nw_lte_sib16_network_time_info_type ril_nw_lte_sib16_network_time_info;

data_rat_enum dsi_translate_qmi_to_dsi_bearer_tech_ex(wds_bearer_tech_info_type_v01  *qmi_bearer_tech_ex)
{
  data_rat_enum bearer_tech = DATA_BEARER_TECH_TYPE_UNKNOWN;

  if (!qmi_bearer_tech_ex)
  {
    log_e( "dsi_translate_qmi_to_dsi_bearer_tech_ex: bad param" );
    goto bail;
  }

  if (WDS_BEARER_TECH_NETWORK_3GPP_V01 == qmi_bearer_tech_ex->technology)
  {
    switch (qmi_bearer_tech_ex->rat_value)
    {
      case WDS_BEARER_TECH_RAT_EX_3GPP_WCDMA_V01:
        if (QMI_WDS_3GPP_SO_MASK_HSDPAPLUS_V01 & qmi_bearer_tech_ex->so_mask)
        {
          bearer_tech = DATA_BEARER_TECH_TYPE_HSDPA_PLUS;
        }
        else if (QMI_WDS_3GPP_SO_MASK_DC_HSDPAPLUS_V01 & qmi_bearer_tech_ex->so_mask)
        {
          bearer_tech = DATA_BEARER_TECH_TYPE_DC_HSDPA_PLUS;
        }
        else if (QMI_WDS_3GPP_SO_MASK_64_QAM_V01 & qmi_bearer_tech_ex->so_mask)
        {
          bearer_tech = DATA_BEARER_TECH_TYPE_64_QAM;
        }
        else if ((QMI_WDS_3GPP_SO_MASK_HSPA_V01 & qmi_bearer_tech_ex->so_mask) ||
                 ((QMI_WDS_3GPP_SO_MASK_HSUPA_V01 & qmi_bearer_tech_ex->so_mask) &&
                  (QMI_WDS_3GPP_SO_MASK_HSDPA_V01 & qmi_bearer_tech_ex->so_mask)))
        {
          bearer_tech = DATA_BEARER_TECH_TYPE_HSPA;
        }
        else if(QMI_WDS_3GPP_SO_MASK_HSUPA_V01 & qmi_bearer_tech_ex->so_mask)
        {
          bearer_tech = DATA_BEARER_TECH_TYPE_HSUPA;
        }
        else if(QMI_WDS_3GPP_SO_MASK_HSDPA_V01 & qmi_bearer_tech_ex->so_mask)
        {
          bearer_tech = DATA_BEARER_TECH_TYPE_HSDPA;
        }
        else
        {
          bearer_tech = DATA_BEARER_TECH_TYPE_WCDMA;
        }
        break;

      case WDS_BEARER_TECH_RAT_EX_3GPP_GERAN_V01:
        if (QMI_WDS_3GPP_SO_MASK_EDGE_V01 & qmi_bearer_tech_ex->so_mask)
        {
          bearer_tech = DATA_BEARER_TECH_TYPE_EDGE;
        }
        else if (QMI_WDS_3GPP_SO_MASK_GPRS_V01 & qmi_bearer_tech_ex->so_mask)
        {
          bearer_tech = DATA_BEARER_TECH_TYPE_GPRS;
        }
        else if (QMI_WDS_3GPP_SO_MASK_GSM_V01 & qmi_bearer_tech_ex->so_mask)
        {
          bearer_tech = DATA_BEARER_TECH_TYPE_GSM;
        }
        break;

      case WDS_BEARER_TECH_RAT_EX_3GPP_LTE_V01:
        bearer_tech = DATA_BEARER_TECH_TYPE_LTE;
        break;

      case WDS_BEARER_TECH_RAT_EX_3GPP_TDSCDMA_V01:
        bearer_tech = DATA_BEARER_TECH_TYPE_TDSCDMA;
        break;

      case WDS_BEARER_TECH_RAT_EX_3GPP_WLAN_V01:
        bearer_tech = DATA_BEARER_TECH_TYPE_3GPP_WLAN;
        break;

      case WDS_BEARER_TECH_RAT_EX_3GPP_5G_V01:
        bearer_tech = DATA_BEARER_TECH_TYPE_5G;
        break;

      default:
        log_e( "dsi_translate_qmi_to_dsi_bearer_tech_ex: unknown 3gpp_rat_mask=0x%x",
                       qmi_bearer_tech_ex->rat_value );
        break;
    }
  }
  else if (WDS_BEARER_TECH_NETWORK_3GPP2_V01 == qmi_bearer_tech_ex->technology)
  {
    switch (qmi_bearer_tech_ex->rat_value)
    {
      case WDS_BEARER_TECH_RAT_EX_3GPP2_1X_V01:
        bearer_tech = DATA_BEARER_TECH_TYPE_CDMA_1X;
        break;

      case WDS_BEARER_TECH_RAT_EX_3GPP2_HRPD_V01:
        bearer_tech = DATA_BEARER_TECH_TYPE_HRPD;
        break;

      case WDS_BEARER_TECH_RAT_EX_3GPP2_EHRPD_V01:
        bearer_tech = DATA_BEARER_TECH_TYPE_EHRPD;
        break;

      case WDS_BEARER_TECH_RAT_EX_3GPP2_WLAN_V01:
        bearer_tech = DATA_BEARER_TECH_TYPE_3GPP2_WLAN;
        break;

      default:
        log_e( "dsi_translate_qmi_to_dsi_bearer_tech_ex: unknown 3gpp2_rat_mask=0x%x",
                       qmi_bearer_tech_ex->rat_value );
        break;
    }
  }

bail:
  log_e( "dsi_translate_qmi_to_dsi_bearer_tech_ex: EXIT" );
  return bearer_tech;
}

data_rat_enum ril_nw_get_rat_tech(void){
    dsd_get_system_status_resp_msg_v01 resp = {0};
    qmi_error_type_v01 qmi_error;
    data_rat_enum ret = DATA_BEARER_TECH_TYPE_UNKNOWN;

    qmi_error = qmi_client_send_msg_sync(dsd_user_handle,
                                         QMI_DSD_GET_SYSTEM_STATUS_REQ_V01,
                                         NULL,
                                         0,
                                         &resp,
                                         sizeof(resp),
                                         5000);

    if ( resp.avail_sys_valid )
    {
        /* Null bearer system RAT implies modem is out of service */
        if ( DSD_SYS_RAT_EX_NULL_BEARER_V01 == resp.avail_sys[0].rat_value )
        {
            log_e("Modem is Out of Service.\n");
            ret = DATA_BEARER_TECH_TYPE_UNKNOWN;
        }
        else
        {
           ret = (data_rat_enum) dsi_translate_qmi_to_dsi_bearer_tech_ex((wds_bearer_tech_info_type_v01 *)&resp.avail_sys[0]);
           log_e("Rat type = %d.\n", ret);
        }
    }else
    {
        log_e("Invalid Reg status received.\n");
        ret = DATA_BEARER_TECH_TYPE_UNKNOWN;
    }

    return ret;
}

/**
+uint32_t ril_nw_convert_ril_prefmode_to_qmi(uint32_t ril_pref_mode)
+{
+
+    int qmi_pref_mode = 0;
+
+    if ( NAS_IS_RAT_SET(ril_pref_mode,RIL_NW_MODE_CDMA))
+    {
+        NAS_SET_BIT(qmi_pref_mode,QMI_NAS_RAT_MODE_PREF_CDMA2000_1X_BIT_V01);
+    }
+    if ( NAS_IS_RAT_SET(ril_pref_mode,RIL_NW_MODE_EVDO))
+    {
+        NAS_SET_BIT(qmi_pref_mode,QMI_NAS_RAT_MODE_PREF_CDMA2000_HRPD_BIT_V01);
+    }
+    if ( NAS_IS_RAT_SET(ril_pref_mode,RIL_NW_MODE_GSM))
+    {
+        NAS_SET_BIT(qmi_pref_mode,QMI_NAS_RAT_MODE_PREF_GSM_BIT_V01);
+    }
+    if ( NAS_IS_RAT_SET(ril_pref_mode,RIL_NW_MODE_WCDMA))
+    {
+        NAS_SET_BIT(qmi_pref_mode,QMI_NAS_RAT_MODE_PREF_UMTS_BIT_V01);
+    }
+    if ( NAS_IS_RAT_SET(ril_pref_mode,RIL_NW_MODE_LTE))
+    {
+        NAS_SET_BIT(qmi_pref_mode,CRI_NAS_RAT_LTE_BIT);
+    }
+    if ( NAS_IS_RAT_SET(ril_pref_mode,MCM_MODE_TDSCDMA_V01))
+    {
+        NAS_SET_BIT(qmi_pref_mode,CRI_NAS_RAT_TDSCDMA_BIT);
+    }
+    if ( NAS_IS_RAT_SET(ril_pref_mode,MCM_MODE_TDSCDMA_V01))
+    {
+        NAS_SET_BIT(qmi_pref_mode,CRI_NAS_RAT_TDSCDMA_BIT);
+    }
+    if ( NAS_IS_RAT_SET(ril_pref_mode,MCM_MODE_NR5G_V01))
+    {
+        NAS_SET_BIT(qmi_pref_mode,CRI_NAS_RAT_NR5G_BIT);
+    }
+    if ( NAS_IS_RAT_SET(ril_pref_mode,MCM_MODE_PRL_V01))
+    {
+        NAS_SET_BIT(qmi_pref_mode,CRI_NAS_RAT_PRL_BIT);
+    }
+
+    return qmi_pref_mode;
+}
+**/

int ril_nw_get_config_qmi(ril_nw_config *output_nw_config)
{
    qmi_client_error_type qmi_err;
    nas_get_system_selection_preference_resp_msg_v01 resp_msg;

    memset(&resp_msg, 0, sizeof(resp_msg));
    qmi_err = qmi_client_send_msg_sync(  nas_user_handle,
                                         QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                         NULL,
                                         0,
                                         &resp_msg,
                                         sizeof(nas_get_system_selection_preference_resp_msg_v01),
                                         5000);

    if(QMI_NO_ERR == qmi_err)
    {
        log_e("ril_nw_get_config_qmi ENTER \n");

        if(resp_msg.roam_pref_valid == true)
        {
            if(resp_msg.roam_pref == NAS_ROAMING_PREF_OFF_V01)
            {
                output_nw_config->pref_roaming = NW_ROAMING_STATE_OFF;
            }else if(resp_msg.roam_pref == NAS_ROAMING_PREF_ANY_V01)
            {
                output_nw_config->pref_roaming = NW_ROAMING_STATE_ON;
            }else
            {
                log_e("ril_nw_get_config_qmi resp_msg.roam_pref = %d. assume OFF \n", resp_msg.roam_pref);
                output_nw_config->pref_roaming = NW_ROAMING_STATE_OFF;
            }
        }else
        {
            log_e("ril_nw_get_config_qmi resp_msg.roam_pref_valid = FALSE. assume OFF \n");
            output_nw_config->pref_roaming = NW_ROAMING_STATE_OFF;
        }

        if(resp_msg.mode_pref_valid == true)
        {
            output_nw_config->pref_nw_mode = (uint16_t)resp_msg.mode_pref;
        }else
        {
            log_e("ril_nw_get_config_qmi resp_msg.mode_pref_valid = FALSE. assume 0 \n");
            output_nw_config->pref_nw_mode = 0;
        }
        return RIL_SUCCESS;
    }else
    {
        log_e("ril_nw_get_config_qmi qmi_err = %d, result=%d, error=%d.\n", qmi_err, resp_msg.resp.result, resp_msg.resp.error);
        return RIL_ERROR_UNKNOWN;
    }
}

int ril_nw_set_config_qmi(ril_nw_config *input_nw_config)
{
    qmi_client_error_type qmi_err;
    nas_set_system_selection_preference_req_msg_v01 req_msg;
    nas_set_system_selection_preference_resp_msg_v01 resp_msg;

    memset(&req_msg, 0, sizeof(nas_set_system_selection_preference_req_msg_v01));
    memset(&resp_msg, 0, sizeof(nas_set_system_selection_preference_resp_msg_v01));

    log_e("ril_nw_set_config_qmi ENTER \n");

    req_msg.mode_pref_valid = TRUE;
    req_msg.mode_pref = input_nw_config->pref_nw_mode;

    qmi_err = qmi_client_send_msg_sync(  nas_user_handle,
                                         QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                         &req_msg,
                                         sizeof(nas_set_system_selection_preference_req_msg_v01),
                                         &resp_msg,
                                         sizeof(nas_set_system_selection_preference_resp_msg_v01),
                                         5000);

    if(QMI_NO_ERR == qmi_err)
    {
        log_e("ril_nw_set_config_qmi SUCCESS \n");
        return RIL_SUCCESS;
    }else
    {
        log_e("ril_nw_set_config_qmi qmi_err = %d, result=%d, error=%d.\n", qmi_err, resp_msg.resp.result, resp_msg.resp.error);
        return RIL_ERROR_UNKNOWN;
    }
}

int ril_nw_set_ps_attach(void)
{
    qmi_client_error_type qmi_err;
    nas_set_system_selection_preference_req_msg_v01 req_msg;
    nas_set_system_selection_preference_resp_msg_v01 resp_msg;

    memset(&req_msg, 0, sizeof(nas_set_system_selection_preference_req_msg_v01));
    memset(&resp_msg, 0, sizeof(nas_set_system_selection_preference_resp_msg_v01));

    log_e("ril_nw_set_ps_attach ENTER \n");

    req_msg.change_duration_valid = TRUE;
    req_msg.change_duration = NAS_POWER_CYCLE_V01;

    req_msg.srv_domain_pref_valid = TRUE;
    req_msg.srv_domain_pref = QMI_SRV_DOMAIN_PREF_PS_ATTACH_V01;

    qmi_err = qmi_client_send_msg_sync(  nas_user_handle,
                                         QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                         &req_msg,
                                         sizeof(nas_set_system_selection_preference_req_msg_v01),
                                         &resp_msg,
                                         sizeof(nas_set_system_selection_preference_resp_msg_v01),
                                         5000);

    if(QMI_NO_ERR == qmi_err)
    {
        log_e("ril_nw_set_ps_attach SUCCESS \n");
        return RIL_SUCCESS;
    }else
    {
        log_e("ril_nw_set_ps_attach qmi_err = %d, result=%d, error=%d.\n", qmi_err, resp_msg.resp.result, resp_msg.resp.error);
        return RIL_ERROR_UNKNOWN;
    }
}

int ril_nw_set_ps_detach(void)
{
    qmi_client_error_type qmi_err;
    nas_set_system_selection_preference_req_msg_v01 req_msg;
    nas_set_system_selection_preference_resp_msg_v01 resp_msg;

    memset(&req_msg, 0, sizeof(nas_set_system_selection_preference_req_msg_v01));
    memset(&resp_msg, 0, sizeof(nas_set_system_selection_preference_resp_msg_v01));

    log_e("ril_nw_set_ps_detach ENTER \n");

    req_msg.change_duration_valid = TRUE;
    req_msg.change_duration = NAS_POWER_CYCLE_V01;

    req_msg.srv_domain_pref_valid = TRUE;
    req_msg.srv_domain_pref = QMI_SRV_DOMAIN_PREF_PS_DETACH_V01;

    qmi_err = qmi_client_send_msg_sync(  nas_user_handle,
                                         QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                         &req_msg,
                                         sizeof(nas_set_system_selection_preference_req_msg_v01),
                                         &resp_msg,
                                         sizeof(nas_set_system_selection_preference_resp_msg_v01),
                                         5000);

    if(QMI_NO_ERR == qmi_err)
    {
        log_e("ril_nw_set_ps_detach SUCCESS \n");
        return RIL_SUCCESS;
    }else
    {
        log_e("ril_nw_set_ps_detach qmi_err = %d, result=%d, error=%d.\n", qmi_err, resp_msg.resp.result, resp_msg.resp.error);
        return RIL_ERROR_UNKNOWN;
    }
}

int ril_nas_check_centralized_eons_support_status()
{
    qmi_client_error_type qmi_err;
    nas_get_centralized_eons_support_status_resp_msg_v01 resp_msg;

    memset(&resp_msg, 0, sizeof(resp_msg));

    qmi_err = qmi_client_send_msg_sync(  nas_user_handle,
                                         QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_REQ_MSG_V01,
                                         NULL,
                                         0,
                                         &resp_msg,
                                         sizeof(resp_msg),
                                         5000);

    if( TRUE == resp_msg.centralized_eons_supported_valid && TRUE == resp_msg.centralized_eons_supported )
    {
        log_d("centralized eons supported\n");
        return TRUE;
    }
    else
    {
        log_d("centralized eons not supported\n");
        return FALSE;
    }
}

int is_3gpp(uint32_t rat)
{
    int ret = FALSE;
    if ( rat == NAS_RADIO_IF_GSM_V01
            || rat == NAS_RADIO_IF_UMTS_V01
            || rat == NAS_RADIO_IF_LTE_V01
            || rat == NAS_RADIO_IF_TDSCDMA_V01
            || rat == NAS_RADIO_IF_NR5G_V01 )
    {
        ret = TRUE;
    }
    return ret;
}

ril_error_type ril_nas_get_plmn_name_from_modem
(
    uint16_t mcc,
    uint16_t mnc,
    char *short_eons,
    char *long_eons,
    uint32_t plmn_rat
)
{
    int ret_val = RIL_ERROR_UNKNOWN;
    uint32_t is_3gpp_rat;
    uint32_t qmi_plmn_rat;

    int is_spn_present;
    int is_plmn_name_present;
    int prefer_spn;

    nas_get_plmn_name_req_msg_v01  get_plmn_req;
    nas_get_plmn_name_resp_msg_v01 get_plmn_resp;

    nas_get_3gpp2_subscription_info_req_msg_v01 cdma_subscription_info_req;
    nas_get_3gpp2_subscription_info_resp_msg_v01 cdma_subscription_info_resp;

    qmi_client_error_type qmi_client_error = QMI_NO_ERR;

    // TODO: check with Sai on how to deal with spn or plmn name.
    prefer_spn = FALSE;

    if( !mcc || !short_eons || !long_eons )
    {
        log_d( "Invalid args. NULL ptr passed.");
        return RIL_ERROR_INVALID_INPUT;
    }

    memset(&get_plmn_req,0,sizeof(get_plmn_req));
    memset(&get_plmn_resp,0,sizeof(get_plmn_resp));

    memset(&cdma_subscription_info_req, 0, sizeof(cdma_subscription_info_req));
    memset(&cdma_subscription_info_resp, 0, sizeof(cdma_subscription_info_resp));

    memset(short_eons, 0, RIL_NW_OPERATOR_NAME_MAX_LENGTH);
    memset(long_eons, 0, RIL_NW_OPERATOR_NAME_MAX_LENGTH);

    //is_3gpp_rat = is_3gpp(plmn_rat);
    qmi_plmn_rat = plmn_rat;

    if ( true )
    { // 3gpp

        get_plmn_req.plmn.mcc = mcc;
        get_plmn_req.plmn.mnc = mnc;

        // TODO: 3 digits mnc, start with 0. Get mcc,mnc from other QMI req
        if( mnc > 99 )
        {
            get_plmn_req.mnc_includes_pcs_digit_valid = TRUE;
            get_plmn_req.mnc_includes_pcs_digit = TRUE;
        }

        get_plmn_req.always_send_plmn_name_valid = TRUE;
        get_plmn_req.always_send_plmn_name = TRUE;

        get_plmn_req.suppress_sim_error_valid = TRUE;
        get_plmn_req.suppress_sim_error = TRUE;

        // Pass the RAT from network scan response if found valid.
        if(qmi_plmn_rat != 0 )
        {
            get_plmn_req.rat_valid = TRUE;
            get_plmn_req.rat = qmi_plmn_rat;
        }
        else
        {
            log_d("Unknown RAT");
            get_plmn_req.rat_valid = FALSE;
        }

    qmi_client_error = qmi_client_send_msg_sync( nas_user_handle,
                                                QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01,
                                                (void*) &get_plmn_req,
                                                sizeof( get_plmn_req ),
                                                (void*) &get_plmn_resp,
                                                sizeof( get_plmn_resp ),
                                                5000);

    if(RIL_DEBUG)
        log_d("qmi_client_error: %d",(int)qmi_client_error );

    if( QMI_ERR_NONE_V01 == qmi_client_error )
    {
        if( TRUE == get_plmn_resp.eons_plmn_name_3gpp_valid )
        {
            is_spn_present = (get_plmn_resp.eons_plmn_name_3gpp.spn_len > 0);
            is_plmn_name_present = ((get_plmn_resp.eons_plmn_name_3gpp.plmn_short_name_len > 0) || (get_plmn_resp.eons_plmn_name_3gpp.plmn_long_name_len > 0));

            log_d("get_plmn_resp.eons_plmn_name_3gpp.spn: %s",get_plmn_resp.eons_plmn_name_3gpp.spn );

            if ( is_plmn_name_present == FALSE && is_spn_present == TRUE )
            {
                prefer_spn = TRUE;
            }

            if( TRUE == prefer_spn )
            {
                if( NAS_CODING_SCHEME_CELL_BROADCAST_GSM_V01 == get_plmn_resp.eons_plmn_name_3gpp.spn_enc )
                {
                ril_nas_convert_gsm8bit_alpha_string_to_utf8( (const char*)get_plmn_resp.eons_plmn_name_3gpp.spn,
                                                                get_plmn_resp.eons_plmn_name_3gpp.spn_len,
                                                                short_eons,
                                                                RIL_NW_OPERATOR_NAME_MAX_LENGTH);
                ril_nas_convert_gsm8bit_alpha_string_to_utf8( (const char*)get_plmn_resp.eons_plmn_name_3gpp.spn,
                                                                get_plmn_resp.eons_plmn_name_3gpp.spn_len,
                                                                long_eons,
                                                                RIL_NW_OPERATOR_NAME_MAX_LENGTH);
                //log_d("spn is 7-bit Unpacked data");
                }
            }
            else
            {
                if( NAS_CODING_SCHEME_CELL_BROADCAST_GSM_V01 == get_plmn_resp.eons_plmn_name_3gpp.plmn_short_name_enc )
                {
                    ril_nas_convert_gsm8bit_alpha_string_to_utf8( (const char*)get_plmn_resp.eons_plmn_name_3gpp.plmn_short_name,
                                                                get_plmn_resp.eons_plmn_name_3gpp.plmn_short_name_len,
                                                                short_eons,
                                                                RIL_NW_OPERATOR_NAME_MAX_LENGTH);
                    //log_d("plmn short name is 7-bit Unpacked data");
                }
                else
                {
                    ril_nas_decode_operator_name_in_little_endian(short_eons,
                                                                RIL_NW_OPERATOR_NAME_MAX_LENGTH,
                                                                get_plmn_resp.eons_plmn_name_3gpp.plmn_short_name_enc,
                                                                (unsigned char*)get_plmn_resp.eons_plmn_name_3gpp.plmn_short_name,
                                                                get_plmn_resp.eons_plmn_name_3gpp.plmn_short_name_len);
                    //log_d("short eons derived from plmn %s",short_eons);
                }

                    if( NAS_CODING_SCHEME_CELL_BROADCAST_GSM_V01 == get_plmn_resp.eons_plmn_name_3gpp.plmn_long_name_enc )
                    {
                        ril_nas_convert_gsm8bit_alpha_string_to_utf8( (const char*)get_plmn_resp.eons_plmn_name_3gpp.plmn_long_name,
                                                                    get_plmn_resp.eons_plmn_name_3gpp.plmn_long_name_len,
                                                                    long_eons,
                                                                    RIL_NW_OPERATOR_NAME_MAX_LENGTH);
                        //log_d("plmn long name is 7-bit Unpacked data");
                    }
                    else
                    {
                        ril_nas_decode_operator_name_in_little_endian(long_eons,
                                                                    RIL_NW_OPERATOR_NAME_MAX_LENGTH,
                                                                    get_plmn_resp.eons_plmn_name_3gpp.plmn_long_name_enc,
                                                                    (unsigned char*)get_plmn_resp.eons_plmn_name_3gpp.plmn_long_name,
                                                                    get_plmn_resp.eons_plmn_name_3gpp.plmn_long_name_len);
                        //log_d("long eons derived from plmn %s",long_eons);
                    }
                }
            }
        }

        log_d("get_plmn_resp.eons_plmn_name_3gpp_valid = %d",get_plmn_resp.eons_plmn_name_3gpp_valid);
    }
    else
    {
    // 3gpp2

        memset(&cdma_subscription_info_req,0,sizeof(cdma_subscription_info_req));

        cdma_subscription_info_req.nam_id = 0xFF; // current NAM
        cdma_subscription_info_req.get_3gpp2_info_mask_valid = TRUE;
        cdma_subscription_info_req.get_3gpp2_info_mask = QMI_NAS_GET_3GPP2_SUBS_INFO_NAM_NAME_V01;

        qmi_client_error = qmi_client_send_msg_sync( nas_user_handle,
                                                   QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01,
                                                   (void*) &cdma_subscription_info_req,
                                                   sizeof( cdma_subscription_info_req ),
                                                   (void*) &cdma_subscription_info_resp,
                                                   sizeof( cdma_subscription_info_resp ),
                                                   5000 );
        if ( qmi_client_error == QMI_ERR_NONE_V01 )
        {
            if ( cdma_subscription_info_resp.nam_name_valid == TRUE )
            {
                memcpy(long_eons,
                      cdma_subscription_info_resp.nam_name,
                      cdma_subscription_info_resp.nam_name_len);
                memcpy(short_eons,
                    cdma_subscription_info_resp.nam_name,
                    cdma_subscription_info_resp.nam_name_len);
            }
        }
    }
    return qmi_client_error;
}

int ril_nas_is_operator_name_empty_or_white_space ( char * str, int max_len)
{
    int is_empty_or_white_space = TRUE;
    int idx = 0;

    if ( str )
    {
        for ( idx = 0; idx < max_len && str[idx] ; idx++ )
        {
            if ( str[idx] != ' ' )
            {
                is_empty_or_white_space = FALSE;
                break;
            }
        }
    }
    return is_empty_or_white_space;
}

void ril_nas_handle_centralized_short_long_eons(uint16_t mcc, uint16_t mnc, char *short_eons, char *long_eons,uint32_t plmn_rat)
{
  char * internal_long_name;
  char * internal_short_name;

  if( mcc && short_eons && long_eons )
  {
      ril_nas_get_plmn_name_from_modem (mcc, mnc, short_eons, long_eons, plmn_rat);

      if( ril_nas_is_operator_name_empty_or_white_space( long_eons, RIL_NW_OPERATOR_NAME_MAX_LENGTH ) &&
          ril_nas_is_operator_name_empty_or_white_space( short_eons, RIL_NW_OPERATOR_NAME_MAX_LENGTH))
      {
        log_d("Received empty operator name\n");
      }
  }
  else
  {
    log_d("Null pointer passed\n");
  }

}

static void nas_cb_scan_network( qmi_client_type user_handle,
                          unsigned int msg_id,
                          void *resp_c_struct,
                          unsigned int resp_c_struct_len,
                          void *resp_cb_data,
                          int tranp_err)
{
    nas_perform_network_scan_resp_msg_v01* resp = (nas_perform_network_scan_resp_msg_v01*)resp_c_struct;

    // Received QMI async callback after time out.
    // We drop this callback.
    if(g_nw_scan_time_out_flag)
    {
        log_e("nas_cb_scan_network g_nw_scan_time_out_flag=TRUE. Skip this info. Do nothing here.\n");
        pthread_mutex_lock(&async_resp);
        g_nw_scan_time_out_flag = FALSE;
        nw_scan_rec_flag = MESSAGE_RECEIVED;
        pthread_mutex_unlock(&async_resp);
        return ;
    }

    if(resp == NULL)
    {
        log_e("nas_cb_scan_network resp NULL \n");
        pthread_mutex_lock(&async_resp);
        nw_scan_rec_flag = MESSAGE_RECEIVED_TRANSPORT_ERROR;
        pthread_cond_signal(&async_resp_cond);
        pthread_mutex_unlock(&async_resp);
        return ;
    }

    if(resp->resp.result != QMI_NO_ERR)
    {
        log_e("nas_cb_scan_network resp->resp.err = %d.\n", resp->resp.error);
        pthread_mutex_lock(&async_resp);
        nw_scan_rec_flag = MESSAGE_RECEIVED_TRANSPORT_ERROR;
        pthread_cond_signal(&async_resp_cond);
        pthread_mutex_unlock(&async_resp);
        return ;
    }

    log_d("nas_cb_scan_network alloc qmi struct lock\n");
    pthread_mutex_lock(&async_resp);
    if(tmp_resp_ptr == NULL)
    {
        log_e("tmp_resp_ptr == NULL");
        tmp_resp_ptr = (nas_perform_network_scan_resp_msg_v01 *)malloc(sizeof(nas_perform_network_scan_resp_msg_v01));
    }
    memcpy(tmp_resp_ptr, resp, sizeof(nas_perform_network_scan_resp_msg_v01));
    log_d("nas_cb_scan_network alloc qmi struct unlock\n");
    nw_scan_rec_flag = MESSAGE_RECEIVED;
    pthread_cond_signal(&async_resp_cond);
    pthread_mutex_unlock(&async_resp);
}

int retrieve_scan_network_type(uint16_t perf_mode, uint8_t* scan_network_type)
{
    int ret = 0;

    if( perf_mode & QMI_NAS_RAT_MODE_PREF_GSM_V01 )
    {
        *scan_network_type |= NAS_NETWORK_TYPE_GSM_ONLY_V01;
    }
    if( perf_mode & QMI_NAS_RAT_MODE_PREF_UMTS_V01 )
    {
        *scan_network_type |= NAS_NETWORK_TYPE_WCDMA_ONLY_V01;
    }
    if( perf_mode & QMI_NAS_RAT_MODE_PREF_LTE_V01 )
    {
        *scan_network_type |= NAS_NETWORK_TYPE_LTE_ONLY_V01;
    }
    if( perf_mode & QMI_NAS_RAT_MODE_PREF_TDSCDMA_V01 )
    {
        *scan_network_type |= NAS_NETWORK_TYPE_TDSCDMA_ONLY_V01;
    }
/*** TODO:  check 5G supported ??
    if( perf_mode & QMI_NAS_RAT_MODE_PREF_NR5G_V01 )
    {
        *scan_network_type |= NAS_NETWORK_TYPE_NR5G_ONLY_V01;
    }
**/
    ret = TRUE;
    return ret;
}

int ril_nw_scan_network_qmi(uint16_t perf_mode)
{
    qmi_client_error_type qmi_err;
    nas_perform_network_scan_req_msg_v01 req_msg;
    nas_perform_network_scan_resp_msg_v01 resp_msg;
    qmi_txn_handle txn_handle;

    memset(&req_msg, 0, sizeof(req_msg));
    memset(&resp_msg, 0, sizeof(resp_msg));

    log_d("ril_nw_scan_network_qmi ENTER. perf_mode=%d \n", perf_mode);
    req_msg.network_type_valid = retrieve_scan_network_type( perf_mode, &req_msg.network_type );

    qmi_err = qmi_client_send_msg_async( nas_user_handle,
                                         QMI_NAS_PERFORM_NETWORK_SCAN_REQ_MSG_V01,
                                         &req_msg,
                                         sizeof(nas_perform_network_scan_req_msg_v01),
                                         &resp_msg,
                                         sizeof(nas_perform_network_scan_resp_msg_v01),
                                         &nas_cb_scan_network,
                                         NULL,
                                         &txn_handle);

    if(QMI_NO_ERR == qmi_err)
    {
        log_e("ril_nw_scan_network_qmi SUCCESS \n");
        return RIL_SUCCESS;
    }else
    {
        log_e("ril_nw_scan_network_qmi qmi_err = %d, result=%d, error=%d.\n", qmi_err, resp_msg.resp.result, resp_msg.resp.error);
        return RIL_ERROR_UNKNOWN;
    }

    return RIL_SUCCESS;
}

int ril_nw_set_network_selection_auto_qmi(void)
{
    qmi_client_error_type qmi_err;
    nas_set_system_selection_preference_req_msg_v01 req_msg;
    nas_set_system_selection_preference_resp_msg_v01 resp_msg;

    memset(&req_msg, 0, sizeof(req_msg));
    memset(&resp_msg, 0, sizeof(resp_msg));

    log_e("ril_nw_set_network_selection_auto ENTER \n");
    req_msg.net_sel_pref_valid = TRUE;
    req_msg.net_sel_pref.net_sel_pref = NAS_NET_SEL_PREF_AUTOMATIC_V01;

    qmi_err = qmi_client_send_msg_sync(  nas_user_handle,
                                         QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                         &req_msg,
                                         sizeof(nas_set_system_selection_preference_req_msg_v01),
                                         &resp_msg,
                                         sizeof(nas_set_system_selection_preference_resp_msg_v01),
                                         5000);

    if(QMI_NO_ERR == qmi_err)
    {
        log_e("ril_nw_set_network_selection_auto SUCCESS \n");
        return RIL_SUCCESS;
    }else
    {
        log_e("ril_nw_set_network_selection_auto qmi_err = %d, result=%d, error=%d.\n", qmi_err, resp_msg.resp.result, resp_msg.resp.error);
        return RIL_ERROR_UNKNOWN;
    }
}

uint32_t nas_convert_rat_to_qmi_rat(uint32_t rat)
{
    uint32_t qmi_rat;

    switch(rat)
    {
        case RIL_NW_RADIO_TECH_GSM_V01:
            qmi_rat = NAS_RADIO_IF_GSM_V01;
            break;

        case RIL_NW_RADIO_TECH_UMTS_V01:
            qmi_rat = NAS_RADIO_IF_UMTS_V01;

            break;

        case RIL_NW_RADIO_TECH_TD_SCDMA_V01:
            qmi_rat = NAS_RADIO_IF_TDSCDMA_V01;

            break;

        case RIL_NW_RADIO_TECH_LTE_V01:
            qmi_rat = NAS_RADIO_IF_LTE_V01;
            break;
/**
        case CRI_NAS_RTE_CDMA:
            qmi_rat = NAS_RADIO_IF_CDMA_1X_V01;
            break;

        case CRI_NAS_RTE_HDR:
            qmi_rat = NAS_RADIO_IF_CDMA_1XEVDO_V01;
            break;
*/
        case RIL_NW_RADIO_TECH_NR5G_V01:
            qmi_rat = NAS_RADIO_IF_NR5G_V01;
            break;

        default:
            qmi_rat = NAS_RADIO_IF_NO_SVC_V01;
            break;
    }
    return qmi_rat;
}

int ril_nw_set_network_selection_manual_qmi(ril_nw_scan_entry_t *entry)
{
    qmi_client_error_type qmi_err;
    nas_set_system_selection_preference_req_msg_v01 req_msg;
    nas_set_system_selection_preference_resp_msg_v01 resp_msg;
    uint32_t rat;
    uint16_t mcc;
    uint16_t mnc;

    memset(&req_msg, 0, sizeof(req_msg));
    memset(&resp_msg, 0, sizeof(resp_msg));

    log_e("ril_nw_set_network_selection_manual START \n");
    rat = entry->rat;
    mcc = (uint16_t) atoi(entry->operator_name.mcc);
    mnc = (uint16_t) atoi(entry->operator_name.mnc);

    req_msg.net_sel_pref_valid = TRUE;
    req_msg.net_sel_pref.net_sel_pref = NAS_NET_SEL_PREF_MANUAL_V01;
    req_msg.net_sel_pref.mcc = mcc;
    req_msg.net_sel_pref.mnc = mnc;
    req_msg.mnc_includes_pcs_digit_valid = TRUE;
    req_msg.mnc_includes_pcs_digit = (mnc > 99 ) ? TRUE : FALSE;
    if( rat != 0 )
    {
        req_msg.rat_valid = TRUE;
        req_msg.rat = nas_convert_rat_to_qmi_rat(rat);
    }

    qmi_err = qmi_client_send_msg_sync(  nas_user_handle,
                                         QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                         &req_msg,
                                         sizeof(nas_set_system_selection_preference_req_msg_v01),
                                         &resp_msg,
                                         sizeof(nas_set_system_selection_preference_resp_msg_v01),
                                         5000);

    if(QMI_NO_ERR == qmi_err)
    {
        log_e("ril_nw_set_network_selection_manual SUCCESS \n");
        return RIL_SUCCESS;
    }else
    {
        log_e("ril_nw_set_network_selection_manual qmi_err = %d, result=%d, error=%d.\n", qmi_err, resp_msg.resp.result, resp_msg.resp.error);
        return RIL_ERROR_UNKNOWN;
    }
}

void copy_nw_time()
{
}

qmi_error_type_v01 ril_nw_get_lte_sib16_nitz_time_info()
{
    qmi_error_type_v01 err_code;
    nas_get_lte_sib16_network_time_resp_msg_v01 resp = {0};

    err_code = QMI_ERR_INTERNAL_V01;
    err_code = qmi_client_send_msg_sync( nas_user_handle,
                                         QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_REQ_MSG_V01,
                                         NULL,
                                         0,
                                         &resp,
                                         sizeof(resp),
                                         5000);

    if(QMI_ERR_NONE_V01 == err_code)
    {
        memset(&ril_nw_lte_sib16_network_time_info, 0, sizeof(ril_nw_lte_sib16_network_time_info_type));
        memcpy(&ril_nw_lte_sib16_network_time_info.lte_sib16_acquired, &resp.lte_sib16_acquired, sizeof(ril_nw_lte_sib16_network_time_info.lte_sib16_acquired));
        memcpy(&ril_nw_lte_sib16_network_time_info.universal_time, &resp.universal_time, sizeof(ril_nw_lte_sib16_network_time_info.universal_time));
        memcpy(&ril_nw_lte_sib16_network_time_info.abs_time, &resp.abs_time, sizeof(ril_nw_lte_sib16_network_time_info.abs_time));
        memcpy(&ril_nw_lte_sib16_network_time_info.leap_sec, &resp.leap_sec, sizeof(ril_nw_lte_sib16_network_time_info.leap_sec));
        memcpy(&ril_nw_lte_sib16_network_time_info.time_zone, &resp.time_zone, sizeof(ril_nw_lte_sib16_network_time_info.time_zone));
        memcpy(&ril_nw_lte_sib16_network_time_info.daylt_sav_adj, &resp.daylt_sav_adj, sizeof(ril_nw_lte_sib16_network_time_info.daylt_sav_adj));
        err_code = RIL_SUCCESS;
    }else
    {
        err_code = RIL_ERROR_UNKNOWN;
    }
    return err_code;
}

qmi_error_type_v01 ril_nw_get_nitz_time_info()
{
    qmi_error_type_v01 err_code;
    nas_get_network_time_resp_msg_v01 resp = {0};

    err_code = QMI_ERR_INTERNAL_V01;    
    err_code = qmi_client_send_msg_sync( nas_user_handle,
                                         QMI_NAS_GET_NETWORK_TIME_REQ_MSG_V01,
                                         NULL,
                                         0,
                                         &resp,
                                         sizeof(resp),
                                         5000);

    if(QMI_ERR_NONE_V01 == err_code)
    {
        memset(&ril_nw_network_time_info, 0, sizeof(ril_nw_network_time_info));
        if(resp.nas_3gpp_time_valid)
        {
            memcpy(&ril_nw_network_time_info.universal_time, &resp.nas_3gpp_time.universal_time, sizeof(ril_nw_network_time_info.universal_time));
            ril_nw_network_time_info.time_zone_valid = TRUE;
            memcpy(&ril_nw_network_time_info.time_zone, &resp.nas_3gpp_time.time_zone, sizeof(ril_nw_network_time_info.time_zone));
            ril_nw_network_time_info.daylt_sav_adj_valid = TRUE;
            memcpy(&ril_nw_network_time_info.daylt_sav_adj, &resp.nas_3gpp_time.daylt_sav_adj, sizeof(ril_nw_network_time_info.daylt_sav_adj));
            ril_nw_network_time_info.radio_if_valid = TRUE; 
            memcpy(&ril_nw_network_time_info.radio_if, &resp.nas_3gpp_time.radio_if, sizeof(ril_nw_network_time_info.radio_if));
        }

        if(resp.nas_3gpp2_time_valid)
        {
            memcpy(&ril_nw_network_time_info.universal_time, &resp.nas_3gpp2_time.universal_time, sizeof(ril_nw_network_time_info.universal_time));
            ril_nw_network_time_info.time_zone_valid = TRUE;
            memcpy(&ril_nw_network_time_info.time_zone, &resp.nas_3gpp2_time.time_zone, sizeof(ril_nw_network_time_info.time_zone));
            ril_nw_network_time_info.daylt_sav_adj_valid = TRUE;
            memcpy(&ril_nw_network_time_info.daylt_sav_adj, &resp.nas_3gpp2_time.daylt_sav_adj, sizeof(ril_nw_network_time_info.daylt_sav_adj));
            ril_nw_network_time_info.radio_if_valid = TRUE;
            memcpy(&ril_nw_network_time_info.radio_if, &resp.nas_3gpp2_time.radio_if, sizeof(ril_nw_network_time_info.radio_if));
        }
        err_code = RIL_SUCCESS;
    }else
    {
        err_code = RIL_ERROR_UNKNOWN;
    }
    return err_code;
}

uint8_t ril_nw_sib16_acquired_status()
{
    uint8 sib16_acquired = FALSE;

    sib16_acquired = ril_nw_lte_sib16_network_time_info.lte_sib16_acquired_valid ?
                            ril_nw_lte_sib16_network_time_info.lte_sib16_acquired : FALSE;
    return sib16_acquired;
}

void ril_nw_fill_nitz_time_resp(uint8_t *nitz_time_valid, char *nitz_time, uint8_t *abs_time_valid, uint64_t *abs_time, uint8_t *leap_sec_valid, int8_t *leap_sec)
{
    int time_zone=0, time_zone_west=FALSE, daylight=0;

    do
    {
        if((NULL == nitz_time_valid) ||(NULL == nitz_time) || (NULL == abs_time_valid) || (NULL == abs_time) || (NULL == leap_sec_valid) || (NULL == leap_sec))
            break;

        if(ril_nw_lte_sib16_network_time_info.universal_time_valid || ril_nw_lte_sib16_network_time_info.abs_time_valid)
        {
            log_e("LTE_SIB16 info available");
            if(ril_nw_lte_sib16_network_time_info.universal_time_valid)
            {
                log_e("LTE_SIB16 Universal time available");
                if(ril_nw_lte_sib16_network_time_info.time_zone_valid)
                {
                    time_zone = ril_nw_lte_sib16_network_time_info.time_zone;
                    if (0 > time_zone)
                    {
                        time_zone *= -1;
                        time_zone_west = TRUE;
                    }
                }

                if (ril_nw_lte_sib16_network_time_info.daylt_sav_adj_valid)
                {
                    daylight = ril_nw_lte_sib16_network_time_info.daylt_sav_adj;
                }

                *nitz_time_valid = TRUE;
                snprintf( nitz_time, RIL_NW_NITZ_TIME_STR_BUF_MAX_V01, "%02d/%02d/%02d,%02d:%02d:%02d%c%02d,%02d",
                        (int) ril_nw_lte_sib16_network_time_info.universal_time.year%100,
                        (int) ril_nw_lte_sib16_network_time_info.universal_time.month,
                        (int) ril_nw_lte_sib16_network_time_info.universal_time.day,
                        (int) ril_nw_lte_sib16_network_time_info.universal_time.hour,
                        (int) ril_nw_lte_sib16_network_time_info.universal_time.minute,
                        (int) ril_nw_lte_sib16_network_time_info.universal_time.second,
                        time_zone_west ? '-' : '+',
                        (int) time_zone,
                        (int) daylight);
            }

            if(ril_nw_lte_sib16_network_time_info.abs_time_valid)
            {
                log_e("LTE_SIB16 Absolute time available");
                *abs_time_valid = TRUE;
                *abs_time = ril_nw_lte_sib16_network_time_info.abs_time;
            }

            if(ril_nw_lte_sib16_network_time_info.leap_sec_valid)
            {
                log_e("LTE_SIB16 Leap sec available");
                *leap_sec_valid = TRUE;
                *leap_sec = ril_nw_lte_sib16_network_time_info.leap_sec;
            }
        }
        else if( NAS_TRI_TRUE_V01 != ril_nw_sib16_acquired_status())
        {
            log_e("Network Time info available");
            if(ril_nw_network_time_info.time_zone_valid)
            {
                time_zone = ril_nw_network_time_info.time_zone;
                if (0 > time_zone)
                {
                    time_zone *= -1;
                    time_zone_west = TRUE;
                }
            }

            if (ril_nw_network_time_info.daylt_sav_adj_valid)
            {
                daylight=ril_nw_network_time_info.daylt_sav_adj;
            }

            *nitz_time_valid = TRUE;
            snprintf( nitz_time, RIL_NW_NITZ_TIME_STR_BUF_MAX_V01, "%02d/%02d/%02d,%02d:%02d:%02d%c%02d,%02d",
                        (int) ril_nw_network_time_info.universal_time.year%100,
                        (int) ril_nw_network_time_info.universal_time.month,
                        (int) ril_nw_network_time_info.universal_time.day,
                        (int) ril_nw_network_time_info.universal_time.hour,
                        (int) ril_nw_network_time_info.universal_time.minute,
                        (int) ril_nw_network_time_info.universal_time.second,
                        time_zone_west ? '-' : '+',
                        (int) time_zone,
                        (int) daylight);
        }
    }while(FALSE);
}

int ril_nw_get_nitz_time(ril_nw_nitz_time_info_t *resp_buff)
{
    ril_nw_get_lte_sib16_nitz_time_info();
    ril_error_type error_type = ril_nw_get_nitz_time_info();
    ril_nw_fill_nitz_time_resp(&resp_buff->nitz_time_valid, &resp_buff->nitz_time, &resp_buff->abs_time_valid, &resp_buff->abs_time, &resp_buff->leap_sec_valid, &resp_buff->leap_sec);

    log_d("nw time = %s, error type = %d", resp_buff->nitz_time, error_type);
    return error_type;
}

int ril_nw_get_lte_ca_info(ril_nw_lte_ca_info_t *ca_info)
{
    nas_get_lte_cphy_ca_info_resp_msg_v01 qmi_nas_lte_ca_resp = {0};
    qmi_client_error_type qmi_error;

    if(ca_info == NULL)
    {
        log_e("ril_nw_get_lte_ca_info ca_info == NULL.");
        return RIL_ERROR_INVALID_INPUT;
    }

    qmi_error = qmi_client_send_msg_sync(nas_user_handle,
                                         QMI_NAS_GET_LTE_CPHY_CA_INFO_REQ_MSG_V01,
                                         NULL,
                                         0,
                                         &qmi_nas_lte_ca_resp,
                                         sizeof(nas_get_lte_cphy_ca_info_resp_msg_v01),
                                         5000);

    if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
         ( qmi_error != QMI_NO_ERR ) ||
         ( qmi_nas_lte_ca_resp.resp.result != QMI_NO_ERR ) )
    {
        log_e("Can not get LTE CA info %d : %d", qmi_error, qmi_nas_lte_ca_resp.resp.error);
        return RIL_ERROR_UNKNOWN;
    }

    if(qmi_nas_lte_ca_resp.pcell_info_valid)
    {
        ca_info->pcell_band = qmi_nas_lte_ca_resp.pcell_info.band;
        //log_e("pcell_band=%d", ca_info->pcell_band);
    }

    if( qmi_nas_lte_ca_resp.cphy_scell_info_list_valid )
    {
        ca_info->scell_info_list_len = qmi_nas_lte_ca_resp.cphy_scell_info_list_len;
        if(RIL_DEBUG)
            log_e("scell_info_list_len=%d", ca_info->scell_info_list_len);
        for(int i=0; i<RIL_NW_MAX_SCELL_LIST_LEN_V01; i++)
        {
            ca_info->scell_band[i] = qmi_nas_lte_ca_resp.cphy_scell_info_list[i].scell_info.band;
        }
    }
    return RIL_SUCCESS;
}
#endif
