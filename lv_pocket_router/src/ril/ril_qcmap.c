#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>

#include "ril.h"
#include "lv_pocket_router/src/util/debug_log.h"
#define QCMAP_CM_SUCCESS               0         /* Successful operation   */
#define QCMAP_QMI_SERVER_INSTANCE_ID_0       0x0
#define QCMAP_QMI_SERVER_INSTANCE_ID_1       0x1
#define QCMAP_QMI_SERVER_POLLING_TIMEOUT     5000

#define QCMAP_REINIT_STATUS_NORMAL_INIT 0
#define QCMAP_REINIT_STATUS_SERVICE_ERR 1
#define QCMAP_REINIT_STATUS_REINITIALIZING 2
#define QCMAP_REINIT_STATUS_INIT_DONE 3

extern int qcmap_ind_wwan_v4_status;
extern int qcmap_ind_wwan_v6_status;

#ifdef FEATURE_ROUTER
qmi_cci_os_signal_type qmi_ip_qcmap_msgr_os_params;
qmi_idl_service_object_type qmi_ip_qcmap_msgr_service_object;
#endif
pthread_mutex_t qcmap_init_status_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t qcmap_reinit_mutex;
pthread_cond_t qcmap_clnt_init_cond;
pthread_mutex_t qcmap_clnt_init_mutex;
int g_qcmap_reinit_status = 0;

void qcmap_client_init()
{
#ifdef FEATURE_ROUTER
    qcmap_msgr_wwan_status_enum_v01 v4_status, v6_status;
    int retry_count =1;

    pthread_cond_init(&qcmap_clnt_init_cond, NULL);
    pthread_mutex_init(&qcmap_clnt_init_mutex, NULL);
    pthread_mutex_init(&qcmap_reinit_mutex, NULL);

    if(qmi_qcmap_init() == QMI_ERROR)
    {
        log_e("QCMAP unable to initialize, exiting");
    }else{
        while(retry_count < 10)//max retry
        {
            if(EnableMobileAP()== true)
            {
                log_e("Enable MobileAP OK!!!\n");
                GetWWANStatus(&v4_status, &v6_status);
                qcmap_ind_wwan_v4_status = v4_status;
                qcmap_ind_wwan_v6_status = v6_status;
                set_init_status(QCMAP_REINIT_STATUS_INIT_DONE);
                break;
            }else{
                log_e("QCMAP restart EnableMobileAP() count :%d\n", retry_count);
            }
                retry_count++;
        }
    }
#endif
}

void ril_qcmap_deinit(void)
{
#ifdef FEATURE_ROUTER
    qmi_client_error_type qmi_error;
    int ret = 0;

    set_init_status(QCMAP_REINIT_STATUS_NORMAL_INIT);
    pthread_mutex_destroy(&qcmap_init_status_mutex);

    pthread_cond_signal(&qcmap_clnt_init_cond);
    pthread_mutex_destroy(&qcmap_clnt_init_mutex);
    pthread_cond_destroy(&qcmap_clnt_init_cond);

    qmi_error = qmi_client_release(qmi_ip_qcmap_msgr_notifier);
    qmi_ip_qcmap_msgr_notifier = NULL;

    if (qmi_error != QMI_NO_ERR)
    {
        log_e("Can not release client qcmap notifier %d", qmi_error);
    }
    ret = DisableMobileAP();
    log_d("disable mobile ap. ret=%d", ret);

    qmi_error = qmi_client_release(qmi_qcmap_msgr_handle);
    qmi_qcmap_msgr_handle = NULL;

    if (qmi_error != QMI_NO_ERR)
    {
        log_e("Can not release client qcmap handle %d", qmi_error);
    }
#endif
}

#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
void set_init_status(int status)
{
    pthread_mutex_lock(&qcmap_init_status_mutex);
    g_qcmap_reinit_status = status;
    pthread_mutex_unlock(&qcmap_init_status_mutex);
}

int get_init_status(void)
{
    int ret;

    pthread_mutex_lock(&qcmap_init_status_mutex);
    ret = g_qcmap_reinit_status;
    pthread_mutex_unlock(&qcmap_init_status_mutex);

    return ret;
}

void qmi_qcmap_ServerService_ErrCB(qmi_client_type user_handle, qmi_client_error_type error, void *err_cb_data)
{
    pthread_t qcmap_reinit_thread;

    if(user_handle == qmi_qcmap_msgr_handle)
    {
        log_e("qcmap service error=%d, release user handle", error);
        qmi_qcmap_msgr_handle = NULL;
        set_init_status(QCMAP_REINIT_STATUS_SERVICE_ERR);
    }
}

void *qcmap_reinit_routine(void *data)
{
    qmi_client_error_type qmi_error;
    int retry_count = 0;

    qmi_error = qmi_client_init_instance(qmi_ip_qcmap_msgr_service_object,
                                         QCMAP_QMI_SERVER_INSTANCE_ID_0,
                                         qmi_qcmap_msgr_ind_cb,
                                         NULL,
                                         &qmi_ip_qcmap_msgr_os_params,
                                         QCMAP_QMI_SERVER_POLLING_TIMEOUT,
                                         &qmi_qcmap_msgr_handle);

    log_e("qmi_client_init_instance: %d, user_handle=%p", qmi_error, qmi_qcmap_msgr_handle);

    qmi_error = qmi_client_register_error_cb(qmi_qcmap_msgr_handle,
                                             qmi_qcmap_ServerService_ErrCB,
                                             NULL);
    log_e("qmi_client_register_error_cb qmi_error=%d", qmi_error);

    while (retry_count < 10) //max retry
    {
        if (EnableMobileAP() == TRUE)
        {
            log_d("Enable MobileAP OK!!!");
            set_init_status(QCMAP_REINIT_STATUS_INIT_DONE);
            return NULL;
        }
        else
        {
            log_d("QCMAP restart EnableMobileAP() count :%d", retry_count);
        }
        retry_count++;
    }
    return NULL;
}

void *qcmap_normal_routine(void *data)
{
    qmi_client_error_type qmi_error;

    qmi_error = qmi_client_init_instance(qmi_ip_qcmap_msgr_service_object,
                                         QCMAP_QMI_SERVER_INSTANCE_ID_0,
                                         qmi_qcmap_msgr_ind_cb,
                                         NULL,
                                         &qmi_ip_qcmap_msgr_os_params,
                                         QCMAP_QMI_SERVER_POLLING_TIMEOUT,
                                         &qmi_qcmap_msgr_handle);

    log_e("qmi_client_init_instance: %d, user_handle=%p", qmi_error, qmi_qcmap_msgr_handle);

    qmi_error = qmi_client_register_error_cb(qmi_qcmap_msgr_handle,
                                             qmi_qcmap_ServerService_ErrCB,
                                             NULL);
    log_e("qmi_client_register_error_cb qmi_error=%d", qmi_error);

    pthread_mutex_lock(&qcmap_clnt_init_mutex);
    pthread_cond_signal(&qcmap_clnt_init_cond);
    log_e("signaling qcmap_clnt_init_cond");
    pthread_mutex_unlock(&qcmap_clnt_init_mutex);
    return NULL;
}

void qmi_qcmap_ServiceAvailableCB
(
    qmi_client_type                user_handle,
    qmi_idl_service_object_type    service_obj,
    qmi_client_notify_event_type   service_event,
    void                          *notify_cb_data
)
{
    pthread_t qcmap_reg_service_thread;
    int init_status;

    init_status = get_init_status();

    // TODO:  make sure only one init process or init_status would be incorrect
    log_e("init_status=%d", init_status);
    if( init_status == QCMAP_REINIT_STATUS_NORMAL_INIT )
    {
        if (0 != pthread_create(&qcmap_reg_service_thread,
                                NULL,
                                qcmap_normal_routine,
                                NULL))
        {
            log_e("Failed to create qcmap_normal_routine!");
        }
    }else if( init_status == QCMAP_REINIT_STATUS_SERVICE_ERR )
    {
        if (0 != pthread_create(&qcmap_reg_service_thread,
                                NULL,
                                qcmap_reinit_routine,
                                NULL))
        {
            log_e("Failed to create qcmap_reinit_routine!");
        }
    }
}

//QCMAP init
int qmi_qcmap_init()
{
    uint32_t                                               num_services = 0, num_entries = 0;
    qmi_service_info                                       info[10];
    qmi_client_error_type                                  qmi_error, qmi_err_code = QMI_NO_ERR;
    int                                                    retry_count = 0;
    //qmi_client_type wds_handle;
    qcmap_msgr_enable = false;
    mobile_ap_handle = 0;
    qmi_ip_qcmap_msgr_notifier = NULL;
    memset(&qmi_qcmap_msgr_handle, 0, sizeof(qmi_client_type));
    log_e("qmi_qcmap_init()\n");
    qmi_ip_qcmap_msgr_service_object = qcmap_msgr_get_service_object_v01();
    if (qmi_ip_qcmap_msgr_service_object == NULL)
    {
        log_e("qmi QCMAP messenger service object not available\n");
        return QMI_ERROR;
    }

    qmi_error = qmi_client_notifier_init(qmi_ip_qcmap_msgr_service_object,
                                       &qmi_ip_qcmap_msgr_os_params,
                                       &qmi_ip_qcmap_msgr_notifier);
    if (qmi_error < 0)
    {
        log_e("qmi:qmi_client_notifier_init(qcmap_msgr) returned %d\n",
                   qmi_error);
       return QMI_ERROR;
    }

    while(retry_count < 10)//max retry
    {
        qmi_error = qmi_client_get_service_list(qmi_ip_qcmap_msgr_service_object,
                                            NULL,
                                            NULL,
                                            &num_services);
        log_e(" qmi: qmi_client_get_service_list: %d\n", qmi_error);

        if(qmi_error == QMI_NO_ERR)
            break;

        QMI_CCI_OS_SIGNAL_WAIT(&qmi_ip_qcmap_msgr_os_params, 500);//max timeout
        QMI_CCI_OS_SIGNAL_CLEAR(&qmi_ip_qcmap_msgr_os_params);
        log_e("Returned from os signal wait\n");
        retry_count++;
    }

    if(retry_count == 10 )//max retry
    {
        qmi_client_release(qmi_ip_qcmap_msgr_notifier);
        qmi_ip_qcmap_msgr_notifier = NULL;
        log_e("Reached maximum retry attempts %d\n", retry_count);
        return QMI_ERROR;
    }

    num_entries = num_services;

    log_e(" qmi: qmi_client_get_service_list: num_e %d num_s %d\n",
                num_entries, num_services);

    qmi_error = qmi_client_get_service_list(qmi_ip_qcmap_msgr_service_object,
                                          info,
                                          &num_entries,
                                          &num_services);

    log_e("qmi_client_get_service_list: num_e %d num_s %d error %d\n",
                num_entries, num_services, qmi_error);

    if (qmi_error != QMI_NO_ERR)
    {
        qmi_client_release(qmi_ip_qcmap_msgr_notifier);
        qmi_ip_qcmap_msgr_notifier = NULL;
        log_e("Can not get qcmap_msgr service list %d\n",
                  qmi_error);
        return QMI_ERROR;
    }
/**
    qmi_error = qmi_client_init(&info[0],
                              qmi_ip_qcmap_msgr_service_object,
                              qmi_qcmap_msgr_ind_cb,
                              NULL,
                              NULL,
                              &qmi_qcmap_msgr_handle);
**/
    /* Register for Service Available CB */
    qmi_error = qmi_client_register_notify_cb(qmi_ip_qcmap_msgr_notifier,
                                              qmi_qcmap_ServiceAvailableCB,
                                              NULL);

    if (qmi_error != QMI_NO_ERR)
    {
        qmi_client_release(qmi_ip_qcmap_msgr_notifier);
        qmi_ip_qcmap_msgr_notifier = NULL;
        log_e("Can not init qcmap_msgr client %d\n", qmi_error);
        return QMI_ERROR;
    }

    // waiting here for client handle ready then we can start to enable mobileap
    pthread_mutex_lock(&qcmap_clnt_init_mutex);
    log_e("pthread_cond_wait waiting.... ");
    pthread_cond_wait(&qcmap_clnt_init_cond, &qcmap_clnt_init_mutex);
    pthread_mutex_unlock(&qcmap_clnt_init_mutex);
    log_e("waiting done.");

    return QMI_SUCCESS;
}

/*===========================================================================
  FUNCTION  qcmap_msgr_qmi_qcmap_ind
  ===========================================================================*/
/*!
  @brief
  Processes an incoming QMI QCMAP Indication.

  @return
  void

  @note

  - Dependencies
  - None

  - Side Effects
  - None
 */
/*=========================================================================*/

void qmi_qcmap_msgr_ind_cb
(
 qmi_client_type user_handle,                    /* QMI user handle       */
 unsigned int    msg_id,                         /* Indicator message ID  */
 void           *ind_buf,                        /* Raw indication data   */
 unsigned int    ind_buf_len,                    /* Raw data length       */
 void           *ind_cb_data                     /* User call back handle */
 )
{
  qmi_client_error_type qmi_error;

  printf("qcmap_msgr_qmi_qcmap_ind: user_handle %X msg_id %d ind_buf_len %d.\n",
      user_handle, msg_id, ind_buf_len);

  switch (msg_id)
  {
    case QMI_QCMAP_MSGR_BRING_UP_WWAN_IND_V01:
    {
      qcmap_msgr_bring_up_wwan_ind_msg_v01 ind_data;

      qmi_error = qmi_client_message_decode(user_handle,
                                            QMI_IDL_INDICATION,
                                            msg_id,
                                            ind_buf,
                                            ind_buf_len,
                                            &ind_data,
                                            sizeof(qcmap_msgr_bring_up_wwan_ind_msg_v01));
      if (qmi_error != QMI_NO_ERR)
      {
        log_e("qcmap_msgr_qmi_qcmap_ind: qmi_client_message_decode error %d\n",qmi_error);
        break;
      }


      /* Process packet service status indication for WWAN for QCMAP*/
	   log_e("ind_data.mobile_ap_handle: %d\n",ind_data.mobile_ap_handle);
      if (ind_data.conn_status == QCMAP_MSGR_WWAN_STATUS_CONNECTED_V01)
      {
        if (ind_data.mobile_ap_handle == qmi_ip_mobile_ap_handle)
        {
          log_e("qcmap_msgr_qmi_qcmap_ind: WWAN Connected\n");
          return;
        }
      }
      else if (ind_data.conn_status == QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL_V01)
      {
        if (ind_data.mobile_ap_handle == qmi_ip_mobile_ap_handle)
        {
          log_e("qcmap_msgr_qmi_qcmap_ind: WWAN Connecting Failed...\n");
          return;
        }
      }

      break;
    }
    case QMI_QCMAP_MSGR_TEAR_DOWN_WWAN_IND_V01:
    {
      qcmap_msgr_tear_down_wwan_ind_msg_v01 ind_data;

      qmi_error = qmi_client_message_decode(user_handle,
                                            QMI_IDL_INDICATION,
                                            msg_id,
                                            ind_buf,
                                            ind_buf_len,
                                            &ind_data,
                                            sizeof(qcmap_msgr_tear_down_wwan_ind_msg_v01));
      if (qmi_error != QMI_NO_ERR)
      {
        log_e("qcmap_msgr_qmi_qcmap_ind: qmi_client_message_decode error %d\n",
            qmi_error,0,0);
        break;
      }

      if (ind_data.conn_status == QCMAP_MSGR_WWAN_STATUS_DISCONNECTED_V01)
      {
        if (ind_data.mobile_ap_handle == qmi_ip_mobile_ap_handle)
        {
          log_e("qcmap_msgr_qmi_qcmap_ind: WWAN Disconnected\n");
          return;
        }
      }
      else if (ind_data.conn_status == QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_FAIL_V01)
      {
        if (ind_data.mobile_ap_handle == qmi_ip_mobile_ap_handle)
        {
          log_e("qcmap_msgr_qmi_qcmap_ind: WWAN Disconnecting Failed...\n");
          return;
        }
      }

      break;
    }
    case QMI_QCMAP_MSGR_WWAN_STATUS_IND_V01:
    {
      qcmap_msgr_wwan_status_ind_msg_v01 ind_data;

      qmi_error = qmi_client_message_decode(user_handle,
                                            QMI_IDL_INDICATION,
                                            msg_id,
                                            ind_buf,
                                            ind_buf_len,
                                            &ind_data,
                                            sizeof(qcmap_msgr_wwan_status_ind_msg_v01));
      if (qmi_error != QMI_NO_ERR)
      {
        log_e("qcmap_msgr_qmi_qcmap_ind: qmi_client_message_decode error %d\n",
            qmi_error,0,0);
        break;
      }

      switch(ind_data.wwan_status)
      {
          case QCMAP_MSGR_WWAN_STATUS_DISCONNECTED_V01:
              log_e("qcmap_msgr_qmi_qcmap_ind: WWAN Ipv4 Disconnected...\n");
              qcmap_ind_wwan_v4_status = ind_data.wwan_status;
              break;
          case QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_FAIL_V01:
              log_e("qcmap_msgr_qmi_qcmap_ind: WWAN Ipv4 Disconnecting Failed..\n");
              qcmap_ind_wwan_v4_status = ind_data.wwan_status;
              break;
          case QCMAP_MSGR_WWAN_STATUS_CONNECTED_V01:
              log_e("qcmap_msgr_qmi_qcmap_ind: WWAN Ipv4 Connected...\n");
              qcmap_ind_wwan_v4_status = ind_data.wwan_status;
              break;
          case QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL_V01:
              log_e("qcmap_msgr_qmi_qcmap_ind: WWAN Ipv4 Connecting Failed...\n");
              qcmap_ind_wwan_v4_status = ind_data.wwan_status;
              break;
          case QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED_V01:
              log_e("qcmap_msgr_qmi_qcmap_ind: WWAN Ipv6 Disconnected...\n");
              qcmap_ind_wwan_v6_status = ind_data.wwan_status;
              break;
          case QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING_FAIL_V01:
              log_e("qcmap_msgr_qmi_qcmap_ind: WWAN Ipv6 Disconnecting Failed..\n");
              qcmap_ind_wwan_v6_status = ind_data.wwan_status;
              break;
          case QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED_V01:
              log_e("qcmap_msgr_qmi_qcmap_ind: WWAN Ipv6 Connected...\n");
              qcmap_ind_wwan_v6_status = ind_data.wwan_status;
              break;
          case QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_FAIL_V01:
              log_e("qcmap_msgr_qmi_qcmap_ind: WWAN Ipv6 Connecting Failed...\n");
              qcmap_ind_wwan_v6_status = ind_data.wwan_status;
              break;
          default:
              log_e("qcmap_msgr_qmi_qcmap_ind: QMI_QCMAP_MSGR_WWAN_STATUS_IND_V01 unhandled status:%d\n", ind_data.wwan_status);
              break;
      }
      break;
    }
  case QMI_QCMAP_MSGR_MOBILE_AP_STATUS_IND_V01:
    {
      qcmap_msgr_mobile_ap_status_ind_msg_v01 ind_data;

      qmi_error = qmi_client_message_decode(user_handle,
                                            QMI_IDL_INDICATION,
                                            msg_id,
                                            ind_buf,
                                            ind_buf_len,
                                            &ind_data,
                                            sizeof(qcmap_msgr_mobile_ap_status_ind_msg_v01));
      if (qmi_error != QMI_NO_ERR)
      {
        log_e("qcmap_msgr_qmi_qcmap_ind: qmi_client_message_decode error %d\n",
            qmi_error,0,0);
        break;
      }

      if (ind_data.mobile_ap_status == QCMAP_MSGR_MOBILE_AP_STATUS_CONNECTED_V01)
      {
        log_e("qcmap_msgr_qmi_qcmap_ind: Mobile AP Connected...\n");
        return;
      }
      else if (ind_data.mobile_ap_status == QCMAP_MSGR_MOBILE_AP_STATUS_DISCONNECTED_V01)
      {
        log_e("qcmap_msgr_qmi_qcmap_ind: Mobile AP Disconnected...\n");
        return;
      }
      break;
    }

  default:
    break;
}

  return;
}

//QCMAP init end

int DisableMobileAP(void)
{
    qcmap_msgr_mobile_ap_disable_req_msg_v01 qcmap_disable_req_msg_v01;
    qcmap_msgr_mobile_ap_disable_resp_msg_v01 qcmap_disable_resp_msg_v01;
    qmi_client_error_type qmi_error = QMI_NO_ERR;

    memset(&qcmap_disable_req_msg_v01, 0, sizeof(qcmap_msgr_mobile_ap_disable_req_msg_v01));
    memset(&qcmap_disable_resp_msg_v01, 0, sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01));

    if (mobile_ap_handle <= 0)
    {
        /* QCMAP is not enabled */
        log_e("QCMAP not enabled\n");
        return false;
    }

    qcmap_disable_req_msg_v01.mobile_ap_handle = mobile_ap_handle;
    qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_MOBILE_AP_DISABLE_REQ_V01,
                                       &qcmap_disable_req_msg_v01,
                                       sizeof(qcmap_msgr_mobile_ap_disable_req_msg_v01),
                                       &qcmap_disable_resp_msg_v01,
                                       sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01),
                                       QCMAP_MSGR_QMI_TIMEOUT_VALUE);

    if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
         ( qmi_error != QMI_NO_ERR ) ||
         ( (qcmap_disable_resp_msg_v01.resp.error != QMI_ERR_NO_EFFECT_V01 &&
            qcmap_disable_resp_msg_v01.resp.error != QMI_ERR_NONE_V01)) ||
         ( qcmap_disable_resp_msg_v01.resp.result != QMI_NO_ERR ))
    {
        log_e( "Can not disable qcmap %d : %d", qmi_error, qcmap_disable_resp_msg_v01.resp.error);
        return false;
    }

    mobile_ap_handle = 0;
    return true;
}

//Enable mobile ap
int EnableMobileAP(void)
{
    //enable mobile ap
    log_e("EnableMobileAP\n");
    qmi_client_error_type qmi_error, qmi_err_code = QMI_NO_ERR;
    qcmap_msgr_mobile_ap_enable_resp_msg_v01 qcmap_enable_resp_msg_v01;
    qcmap_msgr_indication_register_req_msg_v01 qcmap_ind_reg;
    qcmap_msgr_indication_register_resp_msg_v01 qcmap_ind_rsp;
    uint64_t ind_reg_mask = 0xFFFF;


    memset(&qcmap_enable_resp_msg_v01, 0, sizeof(qcmap_msgr_mobile_ap_enable_resp_msg_v01));

    /*Register for Indication Register */

    memset(&qcmap_ind_reg, 0,sizeof(qcmap_msgr_indication_register_req_msg_v01));
    memset(&qcmap_ind_rsp, 0,sizeof(qcmap_msgr_indication_register_resp_msg_v01));
    /*Check if atlease one indication is registered or not*/
    if(ind_reg_mask > 0)
    {
        /*Check which ind regsitraion is enabled and include that TLV here*/
        if(ind_reg_mask | BACKHAUL_STATUS_IND)
        {
            /*Register for Backhaul status Indication*/
            qcmap_ind_reg.backhaul_status_valid = TRUE;
            qcmap_ind_reg.backhaul_status = TRUE;
        }

        if(ind_reg_mask | WWAN_ROAMING_STATUS_IND)
        {
            /*Register for WWAN Roaming status Indication*/
            qcmap_ind_reg.wwan_roaming_valid = TRUE;
            qcmap_ind_reg.wwan_roaming = TRUE;
        }

        if(ind_reg_mask | WWAN_STATUS_IND)
        {
            /*Register for Backhaul status Indication*/
            qcmap_ind_reg.wwan_status_valid = TRUE;
            qcmap_ind_reg.wwan_status = TRUE;
		}

        if(ind_reg_mask | MOBILE_AP_STATUS_IND)
        {
            /*Register for WWAN Roaming status Indication*/
            qcmap_ind_reg.mobile_ap_status_valid = TRUE;
            qcmap_ind_reg.mobile_ap_status = TRUE;
        }

        if(ind_reg_mask | STATION_MODE_STATUS_IND)
        {
            /*Register for Backhaul status Indication*/
            qcmap_ind_reg.station_mode_status_valid = TRUE;
            qcmap_ind_reg.station_mode_status = TRUE;
        }

        if(ind_reg_mask | CRADLE_MODE_STATUS_IND)
        {
            /*Register for WWAN Roaming status Indication*/
            qcmap_ind_reg.cradle_mode_status_valid = TRUE;
            qcmap_ind_reg.cradle_mode_status = TRUE;
        }

        if(ind_reg_mask | ETHERNET_MODE_STATUS_IND)
        {
            /*Register for Backhaul status Indication*/
            qcmap_ind_reg.ethernet_mode_status_valid = TRUE;
            qcmap_ind_reg.ethernet_mode_status = TRUE;
        }

        if(ind_reg_mask | BT_TETHERING_STATUS_IND)
        {
            /*Register for WWAN Roaming status Indication*/
            qcmap_ind_reg.bt_tethering_status_valid = TRUE;
            qcmap_ind_reg.bt_tethering_status = TRUE;
        }

        if(ind_reg_mask | BT_TETHERING_WAN_IND)
        {
            /*Register for WWAN Roaming status Indication*/
            qcmap_ind_reg.bt_tethering_wan_valid = TRUE;
            qcmap_ind_reg.bt_tethering_wan = TRUE;
        }

        qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                     QMI_QCMAP_MSGR_INDICATION_REGISTER_REQ_V01,
                     (void*)&qcmap_ind_reg,
                     sizeof(qcmap_msgr_indication_register_req_msg_v01),
                     (void*)&qcmap_ind_rsp,
                     sizeof(qcmap_msgr_indication_register_resp_msg_v01),
                     QCMAP_MSGR_QMI_TIMEOUT_VALUE);

        log_e("qmi_client_send_msg_sync: error %d result %d\n", qmi_error, qcmap_ind_rsp.resp.result);
        if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
             ( qmi_error != QMI_NO_ERR ) ||
             ( qcmap_ind_rsp.resp.result != QMI_NO_ERR ))
        {
            log_e("Can not register for indications %d : %d\n",qmi_error, qcmap_ind_rsp.resp.error);
            //*qmi_err_num = qcmap_ind_rsp.resp.error;
            return false;
        }
            log_e("Registered for Indications\n");
    }
    /*End Register for Indication Register*/

    qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                        QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ_V01,
                                        NULL,
                                        0,
                                        (void*)&qcmap_enable_resp_msg_v01,
                                        sizeof(qcmap_msgr_mobile_ap_enable_resp_msg_v01),
                                        QCMAP_MSGR_QMI_TIMEOUT_VALUE);
    log_e("qmi_client_send_msg_sync: error %d result %d valid %d\n",
                    qmi_error, qcmap_enable_resp_msg_v01.resp.result, qcmap_enable_resp_msg_v01.mobile_ap_handle_valid);
    if (( qmi_error == QMI_TIMEOUT_ERR ) ||
          ( qmi_error != QMI_NO_ERR ) ||
          ( qcmap_enable_resp_msg_v01.resp.result != QMI_NO_ERR) ||
          ( qcmap_enable_resp_msg_v01.mobile_ap_handle_valid != TRUE ))
    {
        log_e("Can not enable qcmap %d : %d\n",qmi_error, qcmap_enable_resp_msg_v01.resp.error);
        //*qmi_err_num = qcmap_enable_resp_msg_v01.resp.error;
        return false;
    }

    if( qcmap_enable_resp_msg_v01.mobile_ap_handle > 0 )
    {
        mobile_ap_handle = qcmap_enable_resp_msg_v01.mobile_ap_handle;
        qcmap_msgr_enable = true;
        log_e("qcmap_enable_resp_msg_v01.mobile_ap_handle :%d\n",qcmap_enable_resp_msg_v01.mobile_ap_handle);
        log_e("QCMAP Enabled\n");
        return true;
    }
    else
    {
        log_e("QCMAP Enable Failure\n");
    }

    return false;

}

//getwwanStatistics
int GetWWANStatistics
(
    qcmap_msgr_ip_family_enum_v01		 ip_family,
    qcmap_msgr_wwan_statistics_type_v01 *wwan_stats
)
{
    qcmap_msgr_get_wwan_stats_req_msg_v01 get_wwan_stats_req_msg;
    qcmap_msgr_get_wwan_stats_resp_msg_v01 get_wwan_stats_resp_msg;
    qmi_client_error_type qmi_error;
    //log_e("GetWWANStatistics...\n");
    memset(&get_wwan_stats_resp_msg,0,sizeof(qcmap_msgr_get_wwan_stats_resp_msg_v01));
    memset(&get_wwan_stats_req_msg,0,sizeof(qcmap_msgr_get_wwan_stats_req_msg_v01));

    get_wwan_stats_req_msg.mobile_ap_handle = mobile_ap_handle;
    //log_e("GetWWANStatistics mobile_ap_handle :%d\n",mobile_ap_handle);
    get_wwan_stats_req_msg.ip_family = ip_family;

    qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                          QMI_QCMAP_MSGR_GET_WWAN_STATS_REQ_V01,
                                          &get_wwan_stats_req_msg,
                                          sizeof(qcmap_msgr_get_wwan_stats_req_msg_v01),
                                          &get_wwan_stats_resp_msg,
                                          sizeof(qcmap_msgr_get_wwan_stats_resp_msg_v01),
                                          QCMAP_MSGR_QMI_TIMEOUT_VALUE);

    if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
              ( qmi_error != QMI_NO_ERR ) ||
              ( get_wwan_stats_resp_msg.resp.result != QMI_NO_ERR ) )
    {
        log_e("Can not get v4/v6wwan stats %d : %d\n",
              qmi_error, get_wwan_stats_resp_msg.resp.error);
        //*qmi_err_num = get_wwan_stats_resp_msg.resp.error;
        return false;
    }

    wwan_stats->bytes_rx = get_wwan_stats_resp_msg.wwan_stats.bytes_rx;
    wwan_stats->bytes_tx = get_wwan_stats_resp_msg.wwan_stats.bytes_tx;
    wwan_stats->pkts_rx = get_wwan_stats_resp_msg.wwan_stats.pkts_rx;
    wwan_stats->pkts_tx = get_wwan_stats_resp_msg.wwan_stats.pkts_tx;
    wwan_stats->pkts_dropped_rx = get_wwan_stats_resp_msg.wwan_stats.pkts_dropped_rx;
	wwan_stats->pkts_dropped_tx = get_wwan_stats_resp_msg.wwan_stats.pkts_dropped_tx;
    /*
    log_e("\nWWAN Stats Fetched.\n");
    log_e("\nbytes_rx: %llu",get_wwan_stats_resp_msg.wwan_stats.bytes_rx);
    log_e("\nbytes_tx: %llu",get_wwan_stats_resp_msg.wwan_stats.bytes_tx);
    log_e("\n(lu)bytes_rx: %lu",get_wwan_stats_resp_msg.wwan_stats.bytes_rx);
    log_e("\n(lu)bytes_tx: %lu",get_wwan_stats_resp_msg.wwan_stats.bytes_tx);
    log_e("\npkts_rx: %lu",get_wwan_stats_resp_msg.wwan_stats.pkts_rx);
    log_e("\npkts_tx: %lu",get_wwan_stats_resp_msg.wwan_stats.pkts_tx);
    log_e("\npkts_dropped_rx: %lu",get_wwan_stats_resp_msg.wwan_stats.pkts_dropped_rx);
    log_e("\npkts_dropped_tx: %lu",get_wwan_stats_resp_msg.wwan_stats.pkts_dropped_tx);
    */
    log_e("Get v%d WWAN Stats succeeded...\n",ip_family);
    return true;
}

//reset wwanStatostics
int ResetWWANStatistics(qcmap_msgr_ip_family_enum_v01 ip_family)
{
    log_e("ResetWWANStatistics...\n");
    qcmap_msgr_reset_wwan_stats_req_msg_v01 reset_wwan_stats_req_msg;
    qcmap_msgr_reset_wwan_stats_resp_msg_v01 reset_wwan_stats_resp_msg;
    qmi_client_error_type qmi_error;

    memset(&reset_wwan_stats_resp_msg,0,sizeof(qcmap_msgr_reset_wwan_stats_resp_msg_v01));
    memset(&reset_wwan_stats_req_msg,0,sizeof(qcmap_msgr_reset_wwan_stats_req_msg_v01));

    reset_wwan_stats_req_msg.mobile_ap_handle =  mobile_ap_handle;
    reset_wwan_stats_req_msg.ip_family = ip_family;

    qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_RESET_WWAN_STATS_REQ_V01,
                                       &reset_wwan_stats_req_msg,
                                       sizeof(qcmap_msgr_reset_wwan_stats_req_msg_v01),
                                       &reset_wwan_stats_resp_msg,
                                       sizeof(qcmap_msgr_reset_wwan_stats_resp_msg_v01),
                                       QCMAP_MSGR_QMI_TIMEOUT_VALUE);

    if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
         ( qmi_error != QMI_NO_ERR ) ||
         ( reset_wwan_stats_resp_msg.resp.result != QMI_NO_ERR ) )
    {
        log_e("Can not reset wwan stats %d : %d",qmi_error, reset_wwan_stats_resp_msg.resp.error);
        return false;
    }

    log_e("Reset WWAN Stats succeeded...");
    return true;
}

/*===========================================================================
  FUNCTION Get WAN status
  ===========================================================================*/
/*!
  @brief
    Gets WAN status

  @return
    true  - on Success
    false - on Failure

  @note

  - Dependencies
    - None

  - Side Effects
    - None
 */
/*=========================================================================*/

int GetWWANStatus
(
  qcmap_msgr_wwan_status_enum_v01 *v4_status,
  qcmap_msgr_wwan_status_enum_v01 *v6_status
  )
{
  log_e("GetWWANStatus...\n");
  qmi_client_error_type qmi_error, qmi_err_code = QMI_NO_ERR;
  qcmap_msgr_wwan_status_req_msg_v01 wan_status_req;
  qcmap_msgr_wwan_status_resp_msg_v01 wan_status_resp;

  memset(&wan_status_resp, 0, sizeof(qcmap_msgr_wwan_status_resp_msg_v01));
  wan_status_req.mobile_ap_handle = mobile_ap_handle;
  wan_status_req.call_type_valid = 1;
  wan_status_req.call_type = QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01;

  qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_WWAN_STATUS_REQ_V01,
                                       &wan_status_req,
                                       sizeof(qcmap_msgr_wwan_status_req_msg_v01),
                                       (void*)&wan_status_resp,
                                       sizeof(qcmap_msgr_wwan_status_resp_msg_v01),
                                       QCMAP_MSGR_QMI_TIMEOUT_VALUE);

  log_e("qmi_client_send_msg_sync(enable): error %d result %d",
                    qmi_error, wan_status_resp.resp.result);

  if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
       ( qmi_error != QMI_NO_ERR ) ||
       ( wan_status_resp.resp.result != QMI_NO_ERR ) )
  {
    log_e("Can not get IPV4 WAN status  %d : %d",
        qmi_error, wan_status_resp.resp.error);
    //*qmi_err_num = wan_status_resp.resp.error;
    return false;
  }

  if(wan_status_resp.conn_status_valid == 1)
  {
    *v4_status=wan_status_resp.conn_status;
    if(wan_status_resp.conn_status == QCMAP_MSGR_WWAN_STATUS_CONNECTING_V01)
    {
      log_e(" IPV4 WWAN is Connecting \n");
    }
    else if(wan_status_resp.conn_status == QCMAP_MSGR_WWAN_STATUS_CONNECTED_V01)
    {
      log_e(" IPV4 WWAN is connected \n");
    }
    else if(wan_status_resp.conn_status == QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_V01)
    {
      log_e(" IPV4 WWAN is Disconnecting \n");
    }
    else if(wan_status_resp.conn_status == QCMAP_MSGR_WWAN_STATUS_DISCONNECTED_V01)
    {
      log_e(" IPV4 WWAN is Disconnected \n");
    }
  }

  memset(&wan_status_resp, 0, sizeof(qcmap_msgr_wwan_status_resp_msg_v01));
  wan_status_req.mobile_ap_handle = mobile_ap_handle;
  wan_status_req.call_type_valid = 1;
  wan_status_req.call_type = QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01;

  qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_WWAN_STATUS_REQ_V01,
                                       &wan_status_req,
                                       sizeof(qcmap_msgr_wwan_status_req_msg_v01),
                                       (void*)&wan_status_resp,
                                       sizeof(qcmap_msgr_wwan_status_resp_msg_v01),
                                       QCMAP_MSGR_QMI_TIMEOUT_VALUE);

  log_e("qmi_client_send_msg_sync(enable): error %d result %d",
      qmi_error, wan_status_resp.resp.result);

  if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
       ( qmi_error != QMI_NO_ERR ) ||
       ( wan_status_resp.resp.result != QMI_NO_ERR ) )
  {
    log_e("Can not get IPV6 WAN status %d : %d",
        qmi_error, wan_status_resp.resp.error);
    //*qmi_err_num = wan_status_resp.resp.error;
    return false;
  }

  if(wan_status_resp.conn_status_valid == 1)
  {
    *v6_status=wan_status_resp.conn_status;
    if(wan_status_resp.conn_status == QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_V01)
    {
      log_e(" IPV6 WWAN is Connecting \n");
    }
    else if(wan_status_resp.conn_status == QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED_V01)
    {
      log_e(" IPV6 WWAN is connected \n");
    }
    else if(wan_status_resp.conn_status == QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING_V01)
    {
      log_e(" IPV6 WWAN is Disconnecting \n");
    }
    else if(wan_status_resp.conn_status == QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED_V01)
    {
      log_e(" IPV6 WWAN is Disconnected \n");
    }
  }

  return true;
}

/*===========================================================================
  FUNCTION ConnectBackHaul
  ===========================================================================*/
/*!
  @brief
  Brings up the WWAN interface up

  @return
  true  - on Success
  false - on Failure

  @note

  - Dependencies
  - None

  - Side Effects
  - None
 */
/*=========================================================================*/

int ConnectBackHaul
(
  qcmap_msgr_wwan_call_type_v01 call_type
)
{
  int qcmap_msgr_errno;
  //int ret = QCMAP_CM_SUCCESS;
  qcmap_msgr_bring_up_wwan_req_msg_v01  qcmap_bring_up_wwan_req_msg;
  qcmap_msgr_bring_up_wwan_resp_msg_v01 qcmap_bring_up_wwan_resp_msg;
  qmi_client_error_type qmi_error;

  //*qmi_err_num = QMI_ERR_NONE_V01;
  log_e("ConnectBackHaul...\n");
  memset(&qcmap_bring_up_wwan_req_msg, 0, sizeof(qcmap_msgr_bring_up_wwan_req_msg_v01));
  memset(&qcmap_bring_up_wwan_resp_msg, 0, sizeof(qcmap_msgr_bring_up_wwan_resp_msg_v01));

  /* Bring up the data call. */
  log_e("Bring up wwan, call_type=%d", call_type);
  qcmap_bring_up_wwan_req_msg.mobile_ap_handle = mobile_ap_handle;
  qcmap_bring_up_wwan_req_msg.call_type_valid = 1;

  qcmap_bring_up_wwan_req_msg.call_type = call_type;

  qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_BRING_UP_WWAN_REQ_V01,
                                       &qcmap_bring_up_wwan_req_msg,
                                       sizeof(qcmap_msgr_bring_up_wwan_req_msg_v01),
                                       &qcmap_bring_up_wwan_resp_msg,
                                       sizeof(qcmap_msgr_bring_up_wwan_resp_msg_v01),
                                       QCMAP_MSGR_QMI_TIMEOUT_VALUE);

  if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
       ( qmi_error != QMI_NO_ERR ) ||
       ( qcmap_bring_up_wwan_resp_msg.resp.result != QMI_NO_ERR ) )
  {
    log_e("Can not bring up wwan qcmap %d : %d",
        qmi_error, qcmap_bring_up_wwan_resp_msg.resp.error);
    //*qmi_err_num = qcmap_bring_up_wwan_resp_msg.resp.error;
    return false;
  }

/*
   If WWAN is already enabled, and we are trying to enable again from a different client,
   set error number to QMI_ERR_NO_EFFECT_V01, so that the correspondingclient can be
   informed. We hit this scenario in the following case:
   1. Start QCMAP_CLI and enable Backhaul.
   2. Start MCM_MOBILEAP_CLI and try enabling backhaul again.
  */
  if (call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01 &&
      qcmap_bring_up_wwan_resp_msg.conn_status ==
      QCMAP_MSGR_WWAN_STATUS_CONNECTED_V01)
  {
    log_e("WWAN is already enabled.");
    //*qmi_err_num = QMI_ERR_NO_EFFECT_V01;
  }
  else if (call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01 &&
      qcmap_bring_up_wwan_resp_msg.conn_status ==
      QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED_V01)
  {
    log_e("IPv6 WWAN is already enabled.");
    //*qmi_err_num = QMI_ERR_NO_EFFECT_V01;
  }
  else
    log_e("Bringing up wwan...");
  return true;
}

int DisconnectBackHaul
(
    qcmap_msgr_wwan_call_type_v01 call_type
)
{
    qcmap_msgr_tear_down_wwan_req_msg_v01 qcmap_tear_down_wwan_req_msg;
    qcmap_msgr_tear_down_wwan_resp_msg_v01 qcmap_tear_down_wwan_resp_msg;
    qmi_client_error_type qmi_error;

    log_e("DisconnectBackHaul...\n");

    memset(&qcmap_tear_down_wwan_req_msg, 0, sizeof(qcmap_msgr_tear_down_wwan_req_msg_v01));
    memset(&qcmap_tear_down_wwan_resp_msg, 0, sizeof(qcmap_msgr_tear_down_wwan_resp_msg_v01));

    qcmap_tear_down_wwan_req_msg.mobile_ap_handle = mobile_ap_handle;

    qcmap_tear_down_wwan_req_msg.call_type_valid = TRUE;
    qcmap_tear_down_wwan_req_msg.call_type = call_type;

    qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                         QMI_QCMAP_MSGR_TEAR_DOWN_WWAN_REQ_V01,
                                         &qcmap_tear_down_wwan_req_msg,
                                         sizeof(qcmap_msgr_tear_down_wwan_req_msg_v01),
                                         &qcmap_tear_down_wwan_resp_msg,
                                         sizeof(qcmap_msgr_tear_down_wwan_resp_msg_v01),
                                        QCMAP_MSGR_QMI_TIMEOUT_VALUE);

    if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
         ( qmi_error != QMI_NO_ERR) ||
         ( qcmap_tear_down_wwan_resp_msg.resp.result != QMI_NO_ERR ) )
    {
        log_e("Can not tear down wwan qcmap %d : %d\n",
            qmi_error, qcmap_tear_down_wwan_resp_msg.resp.error);
        return false;
    }

    /*
       If WWAN is already disabled, and we are trying to disable again from a different client,
       set error number to QMI_ERR_NO_EFFECT_V01, so that the correspondingclient can be
       informed. We hit this scenario in the following case:
       1. Start QCMAP_CLI and enable Backhaul.
       2. Start MCM_MOBILEAP_CLI and try enabling backhaul again.
       3. Disable backhaul from the 1st client.
       4. Now from the 2nd client.
    */
    if (call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01 &&
        qcmap_tear_down_wwan_resp_msg.conn_status ==
        QCMAP_MSGR_WWAN_STATUS_DISCONNECTED_V01)
    {
        log_e("WWAN is already disabled.\n");
    }
    else if (call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01 &&
        qcmap_tear_down_wwan_resp_msg.conn_status ==
        QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED_V01)
    {
        log_e("IPv6 WWAN is already disabled.\n");
    }
    else
        log_e("Tearing down wwan...\n");
    return true;
}

/*===========================================================================
  FUNCTION EnableWLAN
  ===========================================================================*/
/*!
  @brief
  Brings up the WLAN interface

  @return
  true  - on Success
  false - on Failure

  @note

  - Dependencies
  - None

  - Side Effects
  - None
 */
/*=========================================================================*/
bool EnableWLAN()
{

  log_e("EnableWLAN...\n");
  qmi_client_error_type qmi_error, qmi_err_code = QMI_NO_ERR;
  qcmap_msgr_enable_wlan_req_msg_v01 enable_wlan_req_msg_v01;
  qcmap_msgr_enable_wlan_resp_msg_v01 enable_wlan_resp_msg_v01;

  memset(&enable_wlan_req_msg_v01, 0, sizeof(qcmap_msgr_enable_wlan_req_msg_v01));
  memset(&enable_wlan_resp_msg_v01, 0, sizeof(qcmap_msgr_enable_wlan_resp_msg_v01));

  enable_wlan_req_msg_v01.mobile_ap_handle = mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_ENABLE_WLAN_REQ_V01,
                                       &enable_wlan_req_msg_v01,
                                       sizeof(qcmap_msgr_enable_wlan_req_msg_v01),
                                       (void*)&enable_wlan_resp_msg_v01,
                                       sizeof(qcmap_msgr_enable_wlan_resp_msg_v01),
                                       QCMAP_MSGR_QMI_TIMEOUT_VALUE);

  log_e("qmi_client_send_msg_sync(enable): error %d result %d",
      qmi_error, enable_wlan_resp_msg_v01.resp.result);

  if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
       ( qmi_error != QMI_NO_ERR ) ||
       ( enable_wlan_resp_msg_v01.resp.result != QMI_NO_ERR ) )
  {
    log_e("Can not enable wlan %d : %d",
        qmi_error, enable_wlan_resp_msg_v01.resp.error,0);
    //*qmi_err_num = enable_wlan_resp_msg_v01.resp.error;
    return false;
  }
  return true;
}

/*===========================================================================
  FUNCTION DisableWLAN
  ===========================================================================*/
/*!
  @brief
  Brings the WLAN interface down

  @return
  true  - on Success
  false - on Failure

  @note

  - Dependencies
  - None

  - Side Effects
  - None
 */
/*=========================================================================*/
bool DisableWLAN()
{
  log_e("DisableWLAN...\n");  
  qmi_client_error_type qmi_error, qmi_err_code = QMI_NO_ERR;
  qcmap_msgr_disable_wlan_req_msg_v01  disable_wlan_req_msg_v01;
  qcmap_msgr_disable_wlan_resp_msg_v01 disable_wlan_resp_msg_v01;

  memset(&disable_wlan_req_msg_v01, 0, sizeof(qcmap_msgr_disable_wlan_req_msg_v01));
  memset(&disable_wlan_resp_msg_v01, 0, sizeof(qcmap_msgr_disable_wlan_resp_msg_v01));

  disable_wlan_req_msg_v01.mobile_ap_handle = mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_DISABLE_WLAN_REQ_V01,
                                       &disable_wlan_req_msg_v01,
                                       sizeof(qcmap_msgr_disable_wlan_req_msg_v01),
                                       (void*)&disable_wlan_resp_msg_v01,
                                       sizeof(qcmap_msgr_disable_wlan_resp_msg_v01),
                                       QCMAP_MSGR_QMI_TIMEOUT_VALUE);

  log_e("qmi_client_send_msg_sync(enable): error %d result %d",
      qmi_error, disable_wlan_resp_msg_v01.resp.result);

  if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
       ( qmi_error != QMI_NO_ERR ) ||
       ( disable_wlan_resp_msg_v01.resp.result != QMI_NO_ERR ) )
  {
    log_e("Can not disable wlan %d : %d",
        qmi_error, disable_wlan_resp_msg_v01.resp.error);
    return false;
  }
  return true;  
}

/*===========================================================================
  FUNCTION GetQCMAPBootupCfg
  ===========================================================================*/
/*!
  @brief
  Get QCMAP bootup configuration for MobileAP and WLAN

  @return
  true  - on Success
  false - on Failure

  @note

  - Dependencies
  - None

  - Side Effects
  - None
 */
/*=========================================================================*/

int GetQCMAPBootupCfg(qcmap_msgr_bootup_flag_v01 *mobileap_enable, qcmap_msgr_bootup_flag_v01 *wlan_enable, qmi_error_type_v01 *qmi_err_num)
{
  qmi_client_error_type qmi_error, qmi_err_code = QMI_NO_ERR;
  qcmap_msgr_get_qcmap_bootup_cfg_resp_msg_v01   qcmap_bootup_cfg_resp_msg;

  log_e("GetQCMAPBootupCfg...\n");

  memset(&qcmap_bootup_cfg_resp_msg, 0, sizeof(qcmap_msgr_get_qcmap_bootup_cfg_resp_msg_v01));

  qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_GET_QCMAP_BOOTUP_CFG_REQ_V01,
                                       NULL,
                                       0,
                                       (void*)&qcmap_bootup_cfg_resp_msg,
                                       sizeof(qcmap_msgr_get_qcmap_bootup_cfg_resp_msg_v01),
                                       QCMAP_MSGR_QMI_TIMEOUT_VALUE);

  log_e("qmi_client_send_msg_sync(enable): error %d result %d",
      qmi_error,qcmap_bootup_cfg_resp_msg.resp.result);

  if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
       ( qmi_error != QMI_NO_ERR ) ||
       ( qcmap_bootup_cfg_resp_msg.resp.result != QMI_NO_ERR ) )
  {
    log_e("Cannot Set Bootup Configuration of QCMAP Components %d : %d",
        qmi_error, qcmap_bootup_cfg_resp_msg.resp.error);
    *qmi_err_num = qcmap_bootup_cfg_resp_msg.resp.error;
    return false;
  }

  *mobileap_enable = qcmap_bootup_cfg_resp_msg.mobileap_bootup_flag;
  *wlan_enable = qcmap_bootup_cfg_resp_msg.wlan_bootup_flag;

  return true;
}

/*===========================================================================
  FUNCTION SetRoaming
  ===========================================================================*/
/*!
  @brief
  Enables the Roaming feature

  @return
  true  - on Success
  false - on Failure

  @note

  - Dependencies
  - None

  - Side Effects
  - None
 */
/*=========================================================================*/

int SetRoaming(int enable)
{
  log_e("SetRoaming...\n");
  qcmap_msgr_set_roaming_pref_req_msg_v01 set_roaming_req_msg;
  qcmap_msgr_set_roaming_pref_resp_msg_v01 set_roaming_resp_msg;
  qmi_client_error_type qmi_error;

  memset(&set_roaming_resp_msg,0,sizeof(qcmap_msgr_set_roaming_pref_resp_msg_v01));
  memset(&set_roaming_req_msg,0,sizeof(qcmap_msgr_set_roaming_pref_req_msg_v01));

  set_roaming_req_msg.mobile_ap_handle = mobile_ap_handle;
  set_roaming_req_msg.allow_wwan_calls_while_roaming = enable;

  qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_SET_ROAMING_PREF_REQ_V01,
                                       &set_roaming_req_msg,
                                       sizeof(qcmap_msgr_set_roaming_pref_req_msg_v01),
                                       &set_roaming_resp_msg,
                                       sizeof(qcmap_msgr_set_roaming_pref_resp_msg_v01),
                                       QCMAP_MSGR_QMI_TIMEOUT_VALUE);
  if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
       ( qmi_error != QMI_NO_ERR ) ||
       ( set_roaming_resp_msg.resp.result != QMI_NO_ERR ) )
  {
    log_e("Can not set auto connect flag %d : %d",
        qmi_error, set_roaming_resp_msg.resp.error);
    //*qmi_err_num = set_roaming_resp_msg.resp.error;
    return false;
  }

  log_e("Roaming is Set succesfully...\n");
  return true;
}

/*===========================================================================
  FUNCTION GetRoaming
  ===========================================================================*/
/*!
  @brief
  Enables the Roaming feature

  @return
  true  - on Success
  false - on Failure

  @note

  - Dependencies
  - None

  - Side Effects
  - None
 */
/*=========================================================================*/

int GetRoaming(int *enable)
{
  log_e("GetRoaming...\n");
  qcmap_msgr_get_roaming_pref_req_msg_v01 get_roaming_req_msg;
  qcmap_msgr_get_roaming_pref_resp_msg_v01 get_roaming_resp_msg;
  qmi_client_error_type qmi_error;

  memset(&get_roaming_resp_msg,0,sizeof(qcmap_msgr_get_roaming_pref_resp_msg_v01));
  memset(&get_roaming_req_msg,0,sizeof(qcmap_msgr_get_roaming_pref_req_msg_v01));

  get_roaming_req_msg.mobile_ap_handle = mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_GET_ROAMING_PREF_REQ_V01,
                                       &get_roaming_req_msg,
                                       sizeof(qcmap_msgr_get_roaming_pref_req_msg_v01),
                                       &get_roaming_resp_msg,
                                       sizeof(qcmap_msgr_get_roaming_pref_resp_msg_v01),
                                       QCMAP_MSGR_QMI_TIMEOUT_VALUE);
  if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
       ( qmi_error != QMI_NO_ERR ) ||
       ( get_roaming_resp_msg.resp.result != QMI_NO_ERR ) )
  {
    log_e("Can not set auto connect flag %d : %d",
        qmi_error, get_roaming_resp_msg.resp.error);
    //*qmi_err_num = get_roaming_resp_msg.resp.error;
    return false;
  }

  if(get_roaming_resp_msg.allow_wwan_calls_while_roaming_valid)
  {
    *enable = get_roaming_resp_msg.allow_wwan_calls_while_roaming;
  }
  return true;
}

/*===========================================================================
  FUNCTION EnableIPV4
  ===========================================================================*/
/*!
  @brief
  Enables IPV4 Functionality.

  @return
  true  - on Success
  false - on Failure

  @note

  - Dependencies
  - None

  - Side Effects
  - None
 */
/*=========================================================================*/

int EnableIPV4()
{
  int qcmap_msgr_errno;
  int ret = QCMAP_CM_SUCCESS;
  qcmap_msgr_enable_ipv4_req_msg_v01 qcmap_enable_ipv4_req_msg;
  qcmap_msgr_enable_ipv4_resp_msg_v01 qcmap_enable_ipv4_resp_msg;
  qmi_client_error_type qmi_error;

  memset(&qcmap_enable_ipv4_resp_msg, 0, sizeof(qcmap_msgr_enable_ipv4_resp_msg_v01));
  memset(&qcmap_enable_ipv4_req_msg, 0, sizeof(qcmap_msgr_enable_ipv4_req_msg_v01));

  /* Enable IPV4. */
  log_e("Enable IPV4 \n");
  qcmap_enable_ipv4_req_msg.mobile_ap_handle = mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_ENABLE_IPV4_REQ_V01,
                                       &qcmap_enable_ipv4_req_msg,
                                       sizeof(qcmap_msgr_enable_ipv4_req_msg_v01),
                                       &qcmap_enable_ipv4_resp_msg,
                                       sizeof(qcmap_msgr_enable_ipv4_resp_msg_v01),
                                       QCMAP_MSGR_QMI_TIMEOUT_VALUE);

  if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
       ( qmi_error != QMI_NO_ERR ) ||
       ( qcmap_enable_ipv4_resp_msg.resp.result != QMI_NO_ERR ))
  {
    log_e("Can not enable ipv4 %d : %d\n",
        qmi_error, qcmap_enable_ipv4_resp_msg.resp.error);
    return false;
  }

  log_e("Enabled IPV4...\n");
  return true;
}

/*===========================================================================
  FUNCTION DisableIPV4
  ===========================================================================*/
/*!
  @brief
  Enables IPV4 Functionality.

  @return
  true  - on Success
  false - on Failure

  @note

  - Dependencies
  - None

  - Side Effects
  - None
 */
/*=========================================================================*/

int DisableIPV4()
{
  int qcmap_msgr_errno;
  int ret = QCMAP_CM_SUCCESS;
  qcmap_msgr_disable_ipv4_req_msg_v01 qcmap_disable_ipv4_req_msg;
  qcmap_msgr_disable_ipv4_resp_msg_v01 qcmap_disable_ipv4_resp_msg;
  qmi_client_error_type qmi_error;

  memset(&qcmap_disable_ipv4_resp_msg, 0, sizeof(qcmap_msgr_disable_ipv4_resp_msg_v01));
  memset(&qcmap_disable_ipv4_req_msg, 0, sizeof(qcmap_msgr_disable_ipv4_req_msg_v01));

  /* Disable IPV4. */
  log_e("Disable IPV4\n");
  qcmap_disable_ipv4_req_msg.mobile_ap_handle = mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_DISABLE_IPV4_REQ_V01,
                                       &qcmap_disable_ipv4_req_msg,
                                       sizeof(qcmap_msgr_disable_ipv4_req_msg_v01),
                                       &qcmap_disable_ipv4_resp_msg,
                                       sizeof(qcmap_msgr_disable_ipv4_resp_msg_v01),
                                       QCMAP_MSGR_QMI_TIMEOUT_VALUE);

  if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
       ( qmi_error != QMI_NO_ERR ) ||
       ( qcmap_disable_ipv4_resp_msg.resp.result != QMI_NO_ERR ) )
  {
    log_e("Can not disable ipv4 %d : %d",
        qmi_error, qcmap_disable_ipv4_resp_msg.resp.error);
    return false;
  }

  log_e("Disabled IPV4...\n");
  return true;
}

/*===========================================================================
  FUNCTION EnableIPV6
  ===========================================================================*/
/*!
  @brief
  Enables IPV6 Functionality.

  @return
  true  - on Success
  false - on Failure

  @note

  - Dependencies
  - None

  - Side Effects
  - None
 */
/*=========================================================================*/

int EnableIPV6()
{
  int qcmap_msgr_errno;
  int ret = QCMAP_CM_SUCCESS;
  qcmap_msgr_enable_ipv6_req_msg_v01 qcmap_enable_ipv6_req_msg;
  qcmap_msgr_enable_ipv6_resp_msg_v01 qcmap_enable_ipv6_resp_msg;
  qmi_client_error_type qmi_error;

  memset(&qcmap_enable_ipv6_resp_msg, 0, sizeof(qcmap_msgr_enable_ipv6_resp_msg_v01));
  memset(&qcmap_enable_ipv6_req_msg, 0, sizeof(qcmap_msgr_enable_ipv6_req_msg_v01));

  /* Enable IPV6. */
  log_e("Enable IPV6\n");
  qcmap_enable_ipv6_req_msg.mobile_ap_handle = mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_ENABLE_IPV6_REQ_V01,
                                       &qcmap_enable_ipv6_req_msg,
                                       sizeof(qcmap_msgr_enable_ipv6_req_msg_v01),
                                       &qcmap_enable_ipv6_resp_msg,
                                       sizeof(qcmap_msgr_enable_ipv6_resp_msg_v01),
                                       QCMAP_MSGR_QMI_TIMEOUT_VALUE);

  if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
       ( qmi_error != QMI_NO_ERR ) ||
       ( qcmap_enable_ipv6_resp_msg.resp.result != QMI_NO_ERR ))
  {
    log_e("Can not enable ipv6 %d : %d",
        qmi_error, qcmap_enable_ipv6_resp_msg.resp.error);
   // *qmi_err_num = qcmap_enable_ipv6_resp_msg.resp.error;
    return false;
  }

  log_e("Enabled IPV6...\n");
  return true;
}

/*===========================================================================
  FUNCTION DisableIPV6
  ===========================================================================*/
/*!
  @brief
  Enables IPV6 Functionality.

  @return
  true  - on Success
  false - on Failure

  @note

  - Dependencies
  - None

  - Side Effects
  - None
 */
/*=========================================================================*/

int DisableIPV6()
{
  int qcmap_msgr_errno;
  int ret = QCMAP_CM_SUCCESS;
  qcmap_msgr_disable_ipv6_req_msg_v01 qcmap_disable_ipv6_req_msg;
  qcmap_msgr_disable_ipv6_resp_msg_v01 qcmap_disable_ipv6_resp_msg;
  qmi_client_error_type qmi_error;

  memset(&qcmap_disable_ipv6_resp_msg, 0, sizeof(qcmap_msgr_disable_ipv6_resp_msg_v01));
  memset(&qcmap_disable_ipv6_req_msg, 0, sizeof(qcmap_msgr_disable_ipv6_req_msg_v01));

  /* Enable IPV6. */
  log_e("Disable IPV6\n");
  qcmap_disable_ipv6_req_msg.mobile_ap_handle = mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_DISABLE_IPV6_REQ_V01,
                                       &qcmap_disable_ipv6_req_msg,
                                       sizeof(qcmap_msgr_disable_ipv6_req_msg_v01),
                                       &qcmap_disable_ipv6_resp_msg,
                                       sizeof(qcmap_msgr_disable_ipv6_resp_msg_v01),
                                       QCMAP_MSGR_QMI_TIMEOUT_VALUE);

  if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
       ( qmi_error != QMI_NO_ERR ) ||
       ( qcmap_disable_ipv6_resp_msg.resp.result != QMI_NO_ERR ) )
  {
    log_e("Can not disable ipv6 %d : %d",
        qmi_error, qcmap_disable_ipv6_resp_msg.resp.error);
   // *qmi_err_num = qcmap_disable_ipv6_resp_msg.resp.error;
    return false;
  }

  log_e("Disabled IPV6...\n");
  return true;
}

static int system_ip_to_readable_ip
(
  int domain,
  uint32_t *addr,
  char *str
)
{
  if((addr!=NULL) && (str!=NULL))
  {
    if (inet_ntop(domain, (const char *)addr, str, INET6_ADDRSTRLEN) == NULL)
    {
      log_e("Not in presentation format!!  error :%d \n", errno);
      return RIL_ERROR_UNKNOWN;
    }
    else
      return RIL_SUCCESS;
  }
  return RIL_ERROR_UNKNOWN;
}

static void copy_ip_addr_reverse(char *dest, int src_len, char *src)
{
    char *str_arr[4];
    char *s = strtok(src, ".");
    int i=0;

    while(s != NULL)
    {
        str_arr[i] = s;
        // log_d("copy_ip_addr_reverse. str_arr[%d]=%s", i, str_arr[i]);
        i++;
        s = strtok(NULL, ".");
        if(i==4)
            break;
    }

    snprintf(dest, src_len,"%s.%s.%s.%s", str_arr[3], str_arr[2], str_arr[1], str_arr[0]);
}

int GetIPv4NetworkConfiguration( in_addr_t   *public_ip,
                                 uint32_t    *primary_dns,
                                 in_addr_t   *secondary_dns,
                                 qmi_error_type_v01 *qmi_err_num)
{
    qcmap_msgr_get_wwan_config_req_msg_v01 get_wwan_config_req_msg;
    qcmap_msgr_get_wwan_config_resp_msg_v01 get_wwan_config_resp_msg;
    qmi_client_error_type qmi_error;

    memset(&get_wwan_config_resp_msg,0,sizeof(qcmap_msgr_get_wwan_config_resp_msg_v01));
    memset(&get_wwan_config_req_msg,0,sizeof(qcmap_msgr_get_wwan_config_req_msg_v01));

    get_wwan_config_req_msg.mobile_ap_handle = mobile_ap_handle;
    get_wwan_config_req_msg.addr_type_op = QCMAP_MSGR_MASK_V4_ADDR_V01 | QCMAP_MSGR_MASK_V4_DNS_ADDR_V01;

    log_d("GetIPv4NetworkConfiguration");

    qmi_error = qmi_client_send_msg_sync(qmi_qcmap_msgr_handle,
                                         QMI_QCMAP_MSGR_GET_WWAN_CONFIG_REQ_V01,
                                         &get_wwan_config_req_msg,
                                         sizeof(qcmap_msgr_get_wwan_config_req_msg_v01),
                                         &get_wwan_config_resp_msg,
                                         sizeof(qcmap_msgr_get_wwan_config_resp_msg_v01),
                                         QCMAP_MSGR_QMI_TIMEOUT_VALUE);

    if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
         ( qmi_error != QMI_NO_ERR ) ||
         ( get_wwan_config_resp_msg.resp.result != QMI_NO_ERR ) )
    {
        log_e("Can not get Network config %d : %d", qmi_error, get_wwan_config_resp_msg.resp.error);
        *qmi_err_num = get_wwan_config_resp_msg.resp.error;
        return false;
    }

    log_d("get_wwan_config_resp_msg.v4_addr_valid=%d", get_wwan_config_resp_msg.v4_addr_valid);

    if (get_wwan_config_resp_msg.v4_addr_valid)
        *public_ip = get_wwan_config_resp_msg.v4_addr;
    if (get_wwan_config_resp_msg.v4_prim_dns_addr_valid)
        *primary_dns = get_wwan_config_resp_msg.v4_prim_dns_addr;
    if (get_wwan_config_resp_msg.v4_sec_dns_addr_valid)
        *secondary_dns = get_wwan_config_resp_msg.v4_sec_dns_addr;
    log_d("Get Network Config succeeded...");
    return true;
}

int GetIPv6NetworkConfiguration( struct in6_addr    *public_ip,
                                 struct in6_addr    *primary_dns,
                                 struct in6_addr    *secondary_dns,
                                 qmi_error_type_v01 *qmi_err_num)
{
    qcmap_msgr_get_wwan_config_req_msg_v01 get_wwan_config_req_msg;
    qcmap_msgr_get_wwan_config_resp_msg_v01 get_wwan_config_resp_msg;
    qmi_client_error_type qmi_error;

    memset(&get_wwan_config_resp_msg, 0, sizeof(qcmap_msgr_get_wwan_config_resp_msg_v01));

    get_wwan_config_req_msg.mobile_ap_handle = mobile_ap_handle;
    get_wwan_config_req_msg.addr_type_op = QCMAP_MSGR_MASK_V6_ADDR_V01 | QCMAP_MSGR_MASK_V6_DNS_ADDR_V01;

    log_d("GetIPv6NetworkConfiguration");

    qmi_error = qmi_client_send_msg_sync( qmi_qcmap_msgr_handle,
                                          QMI_QCMAP_MSGR_GET_WWAN_CONFIG_REQ_V01,
                                          &get_wwan_config_req_msg,
                                          sizeof(qcmap_msgr_get_wwan_config_req_msg_v01),
                                          &get_wwan_config_resp_msg,
                                          sizeof(qcmap_msgr_get_wwan_config_resp_msg_v01),
                                          QCMAP_MSGR_QMI_TIMEOUT_VALUE);

    if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
         ( qmi_error != QMI_NO_ERR ) ||
         ( get_wwan_config_resp_msg.resp.result != QMI_NO_ERR ) )
    {
        log_e("Can not get network config %d : %d", qmi_error, get_wwan_config_resp_msg.resp.error);
        *qmi_err_num = get_wwan_config_resp_msg.resp.error;
        return false;
    }

    log_d("get_wwan_config_resp_msg.v6_addr_valid=%d", get_wwan_config_resp_msg.v6_addr_valid);

    if (get_wwan_config_resp_msg.v6_addr_valid)
        memcpy(&public_ip->s6_addr, &get_wwan_config_resp_msg.v6_addr, QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8_t));
    if (get_wwan_config_resp_msg.v6_prim_dns_addr_valid)
        memcpy(&primary_dns->s6_addr, &get_wwan_config_resp_msg.v6_prim_dns_addr, QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8_t));
    if (get_wwan_config_resp_msg.v6_sec_dns_addr_valid)
        memcpy(&secondary_dns->s6_addr, &get_wwan_config_resp_msg.v6_sec_dns_addr, QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8_t));

    log_d("Get network Config succeeded...");
    return true;
}


int ril_get_nw_configuration(ril_get_network_config_resp *nw_config)
{
    qcmap_nw_params_t qcmap_nw_params;
    qmi_error_type_v01 qmi_err_num = 0;
    int ret = 0;
    char str[QCMAP_MSGR_MAX_IP_ADDR_LEN+1];

    memset(&qcmap_nw_params, 0, sizeof(qcmap_nw_params_t));
    ret = GetIPv4NetworkConfiguration( &(qcmap_nw_params.v4_conf.public_ip.s_addr),
                                       &(qcmap_nw_params.v4_conf.primary_dns.s_addr),
                                       &(qcmap_nw_params.v4_conf.secondary_dns.s_addr),
                                       &qmi_err_num);
    log_d("ril_get_nw_configuration qmi_err_num: %d", qmi_err_num);
    if(ret == false)
    {
        nw_config->error_v4 = RIL_ERROR_INVALID_QMI_RESULT;
        snprintf(nw_config->v4_public_ip, sizeof("0.0.0.0"), "%s", "0.0.0.0");
        snprintf(nw_config->v4_dns_pri, sizeof("0.0.0.0"), "%s", "0.0.0.0");
        snprintf(nw_config->v4_dns_sec, sizeof("0.0.0.0"), "%s","0.0.0.0");
    }else
    {
        nw_config->error_v4 = RIL_SUCCESS;
        memset(str, 0, QCMAP_MSGR_MAX_IP_ADDR_LEN+1);
        system_ip_to_readable_ip(AF_INET,(uint32_t *)&qcmap_nw_params.v4_conf.public_ip.s_addr,(char *)&str);
        copy_ip_addr_reverse(nw_config->v4_public_ip, strlen(str)+1, str);

        memset(str, 0, QCMAP_MSGR_MAX_IP_ADDR_LEN+1);
        system_ip_to_readable_ip(AF_INET,(uint32_t *)&qcmap_nw_params.v4_conf.primary_dns.s_addr,(char *)&str);
        copy_ip_addr_reverse(nw_config->v4_dns_pri, strlen(str)+1, str);
        memset(str, 0, QCMAP_MSGR_MAX_IP_ADDR_LEN+1);
        system_ip_to_readable_ip(AF_INET,(uint32_t *)&qcmap_nw_params.v4_conf.secondary_dns.s_addr,(char *)&str);
        copy_ip_addr_reverse(nw_config->v4_dns_sec, strlen(str)+1, str);
    }

    memset(&qcmap_nw_params, 0, sizeof(qcmap_nw_params_t));
    qmi_err_num = 0;
    ret = GetIPv6NetworkConfiguration( &qcmap_nw_params.v6_conf.public_ip_v6,
                                       &qcmap_nw_params.v6_conf.primary_dns_v6,
                                       &qcmap_nw_params.v6_conf.secondary_dns_v6,
                                       &qmi_err_num);
    log_d("ril_get_nw_configuration qmi_err_num: %d", qmi_err_num);
    if(ret == false)
    {
        nw_config->error_v6 = RIL_ERROR_INVALID_QMI_RESULT;
        snprintf(nw_config->v6_public_ip, sizeof("0.0.0.0"), "%s", "0.0.0.0");
        snprintf(nw_config->v6_dns_pri, sizeof("0.0.0.0"), "%s", "0.0.0.0");
        snprintf(nw_config->v6_dns_sec, sizeof("0.0.0.0"), "%s", "0.0.0.0");
    }else
    {
        nw_config->error_v6 = RIL_SUCCESS;
        memset(str, 0, QCMAP_MSGR_MAX_IP_ADDR_LEN+1);
        system_ip_to_readable_ip(AF_INET6,(uint32_t *)&qcmap_nw_params.v6_conf.public_ip_v6,(char *)&str);
        snprintf(nw_config->v6_public_ip, sizeof(str), "%s", str);
        memset(str, 0, QCMAP_MSGR_MAX_IP_ADDR_LEN+1);
        system_ip_to_readable_ip(AF_INET6,(uint32_t *)&qcmap_nw_params.v6_conf.primary_dns_v6,(char *)&str);
        snprintf(nw_config->v6_dns_pri, sizeof(str), "%s", str);
        memset(str, 0, QCMAP_MSGR_MAX_IP_ADDR_LEN+1);
        system_ip_to_readable_ip(AF_INET6,(uint32_t*)&qcmap_nw_params.v6_conf.secondary_dns_v6,(char *)&str);
        snprintf(nw_config->v6_dns_sec, sizeof(str), "%s", str);
    }

    if(nw_config->error_v4 == QMI_SUCCESS || nw_config->error_v6 == QMI_SUCCESS)
        return RIL_SUCCESS;
    return RIL_ERROR_UNKNOWN;
}
#endif
