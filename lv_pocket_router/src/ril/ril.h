#include <stdbool.h>

#ifdef FEATURE_ROUTER
#include "wireless_data_service_v01.h"
#include "qmi_client.h"
#include "qmi.h"
#include "qualcomm_mobile_access_point_msgr_v01.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#include "sms_ril.h"
#include "ril_sim.h"
#include "ril_nw.h"
#include "ril_qcmap.h"

#define RIL_DEBUG 0

#define SIM_UNKNOWN        0
#define SIM_ABSENT         1
#define SIM_PIN_REQUEST    2
#define SIM_PUK_REQUEST    3
#define SIM_NETWORK_LOCKED 4
#define SIM_READY          5
#define QMI_SUCCESS 0
#define QMI_ERROR   -1
#define QCMAP_MSGR_QMI_TIMEOUT_VALUE     90000
#define DATA_USAGE_UNIT_GB      0
#define DATA_USAGE_UNIT_MB      1

#define OPERATOR_NAME_MAX_LENGTH 128
#define APN_MAX_LENGTH 200
#define RIL_QMI_INIT_RETRY_COUNT_MAX 30

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

void monitor_task();
int get_radio_tech();
int get_strength_state(ril_nw_signal_rat_t *signal_source);
void get_operator_name(char *oper_name_buff, int buffer_len);
void get_imei(char *imei_buff, int buff_len);
void get_phone_num(char *num_buff, int buff_len);
void ril_deinit(void);

void startRilDataMonitor();
void reset_data_usage(bool reset_time_b);
double get_data_usage();
bool is_data_downloading();
bool is_data_upgrading();
void dump_data_usage();
void data_usage_check();

typedef struct {
  int profile_index;
  char *apn_name;
  char *pdp_type;
  char *username;
  char *password;
}get_profile_settings,modify_profile_settings;

/** Data type for the register indications. */
typedef enum
{
  WWAN_ROAMING_STATUS_IND = 0x0001,
  BACKHAUL_STATUS_IND = 0x0002,
  WWAN_STATUS_IND = 0x0004,
  MOBILE_AP_STATUS_IND = 0x0008,
  STATION_MODE_STATUS_IND = 0x0010,
  CRADLE_MODE_STATUS_IND = 0x0020,
  ETHERNET_MODE_STATUS_IND = 0x0040,
  BT_TETHERING_STATUS_IND = 0x0080,
  BT_TETHERING_WAN_IND = 0x0100,
  WLAN_STATUS_IND = 0x0200,
  PACKET_STATS_STATUS_IND = 0x0800
}qcmap_register_indications_t;

typedef enum {
  RIL_QCMAP_WWAN_STATUS_UNKNOWN = 0,
  RIL_QCMAP_WWAN_V4_STATUS_CONNECTING = 1,
  RIL_QCMAP_WWAN_V4_STATUS_CONNECTING_FAIL = 2,
  RIL_QCMAP_WWAN_V4_STATUS_CONNECTED = 3,
  RIL_QCMAP_WWAN_V4_STATUS_DISCONNECTING = 4,
  RIL_QCMAP_WWAN_V4_STATUS_DISCONNECTING_FAIL = 5,
  RIL_QCMAP_WWAN_V4_STATUS_DISCONNECTED = 6,
  RIL_QCMAP_WWAN_V6_STATUS_CONNECTING = 7,
  RIL_QCMAP_WWAN_V6_STATUS_CONNECTING_FAIL = 8,
  RIL_QCMAP_WWAN_V6_STATUS_CONNECTED = 9,
  RIL_QCMAP_WWAN_V6_STATUS_DISCONNECTING = 10,
  RIL_QCMAP_WWAN_V6_STATUS_DISCONNECTING_FAIL = 11,
 RIL_QCMAP_WWAN_V6_STATUS_DISCONNECTED = 12
} ril_qcmap_wwan_status_enum_t;

typedef enum {
  RIL_NW_SERVICE_MIN_ENUM = -2147483647, /* To force a 32 bit signed enum.  Do not change or use*/
  RIL_NW_SERVICE_NONE     = 0x0000,    /**<  Not registered or no data. */
  RIL_NW_SERVICE_LIMITED  = 0x0001,    /**<  Registered; emergency service only. */
  RIL_NW_SERVICE_FULL     = 0x0002,    /**<  Registered, full service. */
  RIL_NW_SERVICE_MAX_ENUM = 2147483647 /* To force a 32 bit signed enum.  Do not change or use*/
}ril_nw_service_state;

typedef enum {
  RIL_NW_ROAMING_STATE_UNKNOWN = 0,
  RIL_NW_ROAMING_STATE_ROAMING_OFF = 1,
  RIL_NW_ROAMING_STATE_ROAMING_ON = 2
}ril_nw_roaming_state;

typedef enum {
    RIL_SUCCESS = 0,
    RIL_ERROR_UNKNOWN = 1,
    RIL_ERROR_INIT_FAIL = 2,
    RIL_ERROR_INVALID_INPUT = 3,
    RIL_ERROR_SIM_PIN_INCORRECT = 4,
    RIL_ERROR_MEMORY_ERROR = 5,
    RIL_ERROR_SIM_PIN_BLOCKED = 6,
    RIL_ERROR_SIM_PIN_PERM_BLOCKED = 7,
    RIL_ERROR_SEND_QMI_FAIL = 8,
    RIL_ERROR_INVALID_QMI_RESULT = 9,
    RIL_ERROR_NETWORK_BUSY = 10
}ril_error_type;

#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
typedef struct {
    ril_sms_init_data  sms_init;
    ril_sim_pin_req_cb sim_pin_cb;
    ril_nw_resp_cb     nw_cb;
}ril_init_data;
int ril_init(ril_init_data *init_data);
#endif

#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
ril_error_type convertQMIErrCodeToRil(qmi_result_type_v01 result, qmi_error_type_v01 error);
#endif
sim_type_enum get_sim_type();
int get_apn_profile(get_profile_settings *getprofile);
int modify_apn_profile(modify_profile_settings *modifyprofile);
ril_error_type get_wwan_status(ril_qcmap_wwan_status_enum_t *ipv4, ril_qcmap_wwan_status_enum_t *ipv6);
bool is_wwan_connected();
void reestablish_data_connection(void);

void refresh_data_usage_info();
void read_data_usage(int from, long long int * rx, long long int * tx);
void reset_date_usage_start_date();

int async_cb_thread();
// network
int is_4G_network_supported();
ril_error_type support_4G_network(int enable);
ril_error_type set_nw_selection_auto();
ril_error_type set_nw_selection_manual(ril_nw_scan_entry_t *entry);
ril_error_type scan_network();
ril_error_type get_nitz_time(ril_nw_nitz_time_info_t *resp_buff);
bool is_nr5g_icon_supported();
ril_nw_roaming_state get_roaming_status();
int get_lte_ca_activated(void);
ril_error_type get_network_config(ril_get_network_config_resp *nw_config);

// sim 
int get_sim_state();
ril_error_type change_sim_pin(char *new_pin, char *old_pin);
ril_error_type enable_disable_sim_pin(int enable, char *pin1);
ril_error_type verify_sim_pin(char *pin1);
ril_error_type verify_sim_puk(char *puk1, char *new_pin1);
sim_pin_state_enum get_sim_pin1_state(void);
sim_pin_state_enum get_sim_pin2_state(void);
int get_sim_pin1_retries();
int get_sim_puk1_retries();
ril_error_type get_sim_mcc_mnc(char* mcc, int mcc_buff_len, char* mnc, int mnc_buff_len);
#ifdef FEATURE_ROUTER
int Set_Attach_Req
(
  qmi_client_type wds_handle
);
#endif
bool is_mccmnc_available(int state);
//For cc_regdb, get mcc to wlan_country
void get_service_mcc(int *mcc_num);
