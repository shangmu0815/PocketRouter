#include <stdbool.h>
#include <netinet/in.h>

#ifdef FEATURE_ROUTER
#include "wireless_data_service_v01.h"
#include "qmi_client.h"
#include "qmi.h"
#include "qualcomm_mobile_access_point_msgr_v01.h"
#endif

//QCMAP
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
qmi_client_type   qmi_qcmap_msgr_handle;
qmi_client_type   qmi_ip_qcmap_msgr_notifier;
uint32_t qmi_ip_mobile_ap_handle;
int  qcmap_msgr_enable;
uint32_t mobile_ap_handle;
#endif

int qmi_qcmap_init();
void qcmap_client_init();
void ril_qcmap_deinit(void);
/*=============================================================================
                                FUNCTION FORWARD DECLARATION
==============================================================================*/
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
void qmi_qcmap_msgr_ind_cb
(
 qmi_client_type user_handle,                    /* QMI user handle       */
 unsigned int    msg_id,                         /* Indicator message ID  */
 void           *ind_buf,                        /* Raw indication data   */
 unsigned int    ind_buf_len,                    /* Raw data length       */
 void           *ind_cb_data                     /* User call back handle */
 );
#endif
int EnableMobileAP(void);
int DisableMobileAP(void);
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
int GetWWANStatistics
(
    qcmap_msgr_ip_family_enum_v01		 ip_family,
    qcmap_msgr_wwan_statistics_type_v01 *wwan_stats
);
int ResetWWANStatistics(qcmap_msgr_ip_family_enum_v01 ip_family);
int GetWWANStatus
(
  qcmap_msgr_wwan_status_enum_v01 *v4_status,
  qcmap_msgr_wwan_status_enum_v01 *v6_status
);

int ConnectBackHaul(qcmap_msgr_wwan_call_type_v01 call_type);
int DisconnectBackHaul(qcmap_msgr_wwan_call_type_v01 call_type);
//for enable, disable, restart WLAN
bool EnableWLAN();
bool DisableWLAN();

int GetQCMAPBootupCfg
(
qcmap_msgr_bootup_flag_v01 *mobileap_enable,
qcmap_msgr_bootup_flag_v01 *wlan_enable, 
qmi_error_type_v01 *qmi_err_num
);
int SetRoaming(int enable);
int GetRoaming(int *enable);
int EnableIPV4();
int DisableIPV4();
int EnableIPV6();
int DisableIPV6();
#endif

#define QCMAP_MSGR_MAX_IP_ADDR_LEN 45
#define INET_ADDRSTRLEN        16
#define INET6_ADDRSTRLEN       46

/** Data type for IPv4 configuration. */
typedef struct
{
  struct in_addr public_ip;       /**< Public IP address. */
  struct in_addr primary_dns;     /**< Primary domain name service (DNS) IP address. */
  struct in_addr secondary_dns;   /**< Secondary DNS IP
                                address. */
}v4_conf_t;

/** Data type for IPv6 configuration. */
typedef struct
{
  struct in6_addr public_ip_v6;      /**< Public IPv6 address. */
  struct in6_addr primary_dns_v6;    /**< Primary domain name service (DNS) IPv6 address. */
  struct in6_addr secondary_dns_v6;  /**< Secondary DNS
                                          IPv6 address. @newpagetable */
}v6_conf_t;

/** Data type for network configuration. */
typedef union
{
  v4_conf_t v4_conf;      /**< IPv4 configuration. */
  v6_conf_t v6_conf;      /**< IPv6 configuration. */
}qcmap_nw_params_t;

typedef struct {
    int error_v4;
    char v4_public_ip[QCMAP_MSGR_MAX_IP_ADDR_LEN+1];
    char v4_dns_pri[QCMAP_MSGR_MAX_IP_ADDR_LEN+1];
    char v4_dns_sec[QCMAP_MSGR_MAX_IP_ADDR_LEN+1];

    int error_v6;
    char v6_public_ip[QCMAP_MSGR_MAX_IP_ADDR_LEN+1];
    char v6_dns_pri[QCMAP_MSGR_MAX_IP_ADDR_LEN+1];
    char v6_dns_sec[QCMAP_MSGR_MAX_IP_ADDR_LEN+1];
} ril_get_network_config_resp;

int ril_get_nw_configuration(ril_get_network_config_resp *nw_config);
//QCMAP
