#include <stdbool.h>
#include <stdint.h>
#include "ril_utils.h"
#include <pthread.h>
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
#include "network_access_service_v01.h"
#endif

#define RIL_NW_MCC_MAX_LENGTH 3
#define RIL_NW_MNC_MAX_LENGTH 3
#define RIL_NW_OPERATOR_NAME_MAX_LENGTH 128
#define RIL_NW_SCAN_LIST_MAX_LENGTH 40
#define RIL_NW_RESP_CB_MAX_LENGTH 2      /** WebUI & DeviceUI **/
#define RIL_NW_NITZ_TIME_STR_BUF_MAX_V01 30
#define RIL_NW_MAX_SCELL_LIST_LEN_V01 4

#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
//typedef uint64_t ril_nw_mode_type;
typedef uint16_t ril_nw_mode_type;
#else
typedef unsigned long ril_nw_mode_type;
#endif
#define RIL_NW_MODE_GSM       (ril_nw_mode_type) 0x01ull   /**<  Include GSM networks. */
#define RIL_NW_MODE_WCDMA     (ril_nw_mode_type) 0x02ull   /**<  Include WCDMA networks. */
#define RIL_NW_MODE_CDMA      (ril_nw_mode_type) 0x04ull   /**<  Include CDMA networks. */
#define RIL_NW_MODE_EVDO      (ril_nw_mode_type) 0x08ull   /**<  Include EVDO networks. */
#define RIL_NW_MODE_LTE       (ril_nw_mode_type) 0x10ull   /**<  Include LTE networks. */
#define RIL_NW_MODE_TDSCDMA   (ril_nw_mode_type) 0x20ull   /**<  Include TDSCDMA networks. */
#define RIL_NW_MODE_NR5G      (ril_nw_mode_type) 0x40ull   /**<  Include NR5G networks. */

#define RIL_QMI_NW_SCAN_RES_ENTRY_PREFERRED              ( (uint8_t) 1 << 6 )
#define RIL_QMI_NW_SCAN_RES_ENTRY_NOT_PREFERRED          ( (uint8_t) 1 << 7 )
#define RIL_QMI_NW_SCAN_RES_ENTRY_FORBIDDEN              ( (uint8_t) 1 << 4 )
#define RIL_QMI_NW_SCAN_RES_ENTRY_NOT_FORBIDDEN          ( (uint8_t) 1 << 5 )
#define RIL_QMI_NW_SCAN_RES_ENTRY_HOME                   ( (uint8_t) 1 << 2 )
#define RIL_QMI_NW_SCAN_RES_ENTRY_ROAM                   ( (uint8_t) 1 << 3 )
#define RIL_QMI_NW_SCAN_RES_ENTRY_CUR_SERVING            ( (uint8_t) 1 << 0 )
#define RIL_QMI_NW_SCAN_RES_ENTRY_AVAILABLE              ( (uint8_t) 1 << 1 )

#define NAS_RADIO_IF_NONE           0x00 // - None (no service)
#define NAS_RADIO_IF_CDMA2000       0x01 // - cdma2000 1X
#define NAS_RADIO_IF_CDMA2000_HRPD  0x02 // - cdma2000 HRPD (1xEV-DO)
#define NAS_RADIO_IF_AMPS           0x03 // - AMPS
#define NAS_RADIO_IF_GSM            0x04 // - GSM
#define NAS_RADIO_IF_UMTS           0x05 // - UMTS
#define NAS_RADIO_IF_WLAN           0x06 // - WLAN
#define NAS_RADIO_IF_GPS            0x07 // - GPS
#define NAS_RADIO_IF_LTE            0x08 // - LTE
#define NAS_RADIO_IF_TDSCDMA        0x09 // - TDSCDMA
#define NAS_RADIO_IF_NR5G           0x0C // - NR5G

#define MESSAGE_RECEIVED 0
#define MESSAGE_NOT_RECEIVED 1
#define DECODE_ERROR 2
#define MESSAGE_RECEIVED_TRANSPORT_ERROR 3
#define MESSAGE_SENT_WAITING_CB 4

#define NETWORK_SCAN_ASYNC_TIMEOUT_SEC 118   // Less than UI 120 sec

typedef enum {
    DATA_BEARER_TECH_TYPE_UNKNOWN = 0,
    DATA_BEARER_TECH_TYPE_CDMA_1X = 1,
    DATA_BEARER_TECH_TYPE_EVDO_REV0 = 2,
    DATA_BEARER_TECH_TYPE_EVDO_REVA = 3,
    DATA_BEARER_TECH_TYPE_EVDO_REVB = 4,
    DATA_BEARER_TECH_TYPE_EHRPD = 5,
    DATA_BEARER_TECH_TYPE_FMC = 6,
    DATA_BEARER_TECH_TYPE_HRPD = 7,
    DATA_BEARER_TECH_TYPE_3GPP2_WLAN = 8,
    DATA_BEARER_TECH_TYPE_WCDMA = 9,
    DATA_BEARER_TECH_TYPE_GPRS = 10,
    DATA_BEARER_TECH_TYPE_HSDPA = 11,
    DATA_BEARER_TECH_TYPE_HSUPA = 12,
    DATA_BEARER_TECH_TYPE_EDGE = 13,
    DATA_BEARER_TECH_TYPE_LTE = 14,
    DATA_BEARER_TECH_TYPE_HSDPA_PLUS = 15,
    DATA_BEARER_TECH_TYPE_DC_HSDPA_PLUS = 16,
    DATA_BEARER_TECH_TYPE_HSPA = 17,
    DATA_BEARER_TECH_TYPE_64_QAM = 18,
    DATA_BEARER_TECH_TYPE_TDSCDMA = 19,
    DATA_BEARER_TECH_TYPE_GSM = 20,
    DATA_BEARER_TECH_TYPE_3GPP_WLAN = 21,
    DATA_BEARER_TECH_TYPE_5G = 22
}data_rat_enum;

typedef enum {
    NW_ROAMING_STATE_OFF = 0,
    NW_ROAMING_STATE_ON = 1
}ril_nw_roaming_state_enum;


typedef struct {
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    //uint64_t pref_nw_mode;                    /** bit mask of RIL_NW_MODE_* ***/
    uint16_t pref_nw_mode;                    /** bit mask of RIL_NW_MODE_* ***/
#else
    unsigned long pref_nw_mode;
#endif
    ril_nw_roaming_state_enum pref_roaming;
}ril_nw_config;

typedef enum {
    RIL_NW_NETWORK_STATUS_T_MIN_ENUM_VAL_V01 = -2147483647, /* To force a 32 bit signed enum.  Do not change or use*/
    RIL_NW_NETWORK_STATUS_NONE_V01 = 0x0000,                /**<  Network status not available. */
    RIL_NW_NETWORK_STATUS_CURRENT_SERVING_V01 = 0x0001,     /**<  Current serving network. */
    RIL_NW_NETWORK_STATUS_PREFERRED_V01 = 0x0002,           /**<  Preferred network. */
    RIL_NW_NETWORK_STATUS_NOT_PREFERRED_V01 = 0x0003,       /**<  Not the preferred network. */
    RIL_NW_NETWORK_STATUS_AVAILABLE_V01 = 0x0004,           /**<  Service available. */
    RIL_NW_NETWORK_STATUS_FORBIDDEN_V01 = 0x0005,           /**<  Forbidden service. */
    RIL_NW_NETWORK_STATUS_T_MAX_ENUM_VAL_V01 = 2147483647   /* To force a 32 bit signed enum.  Do not change or use*/
}ril_nw_network_status_t;

typedef struct {
    char long_name[RIL_NW_OPERATOR_NAME_MAX_LENGTH + 1];
    char short_name[RIL_NW_OPERATOR_NAME_MAX_LENGTH + 1];
    char mcc[RIL_NW_MCC_MAX_LENGTH + 1];
    char mnc[RIL_NW_MNC_MAX_LENGTH + 1];
}ril_nw_operator_name_t;

typedef enum {
  RIL_NW_RADIO_TECH_T_MIN_ENUM_VAL_V01 = -2147483647, /* To force a 32 bit signed enum.  Do not change or use*/
  RIL_NW_RADIO_TECH_TD_SCDMA_V01 = 1,
  RIL_NW_RADIO_TECH_GSM_V01 = 2,     /**<  GSM; only supports voice. */
  RIL_NW_RADIO_TECH_HSPAP_V01 = 3,   /**<  HSPA+. */
  RIL_NW_RADIO_TECH_LTE_V01 = 4,     /**<  LTE. */
  RIL_NW_RADIO_TECH_EHRPD_V01 = 5,   /**<  EHRPD. */
  RIL_NW_RADIO_TECH_EVDO_B_V01 = 6,  /**<  EVDO B. */
  RIL_NW_RADIO_TECH_HSPA_V01 = 7,    /**<  HSPA. */
  RIL_NW_RADIO_TECH_HSUPA_V01 = 8,   /**<  HSUPA. */
  RIL_NW_RADIO_TECH_HSDPA_V01 = 9,   /**<  HSDPA. */
  RIL_NW_RADIO_TECH_EVDO_A_V01 = 10, /**<  EVDO A. */
  RIL_NW_RADIO_TECH_EVDO_0_V01 = 11, /**<  EVDO 0. */
  RIL_NW_RADIO_TECH_1xRTT_V01 = 12,  /**<  1xRTT. */
  RIL_NW_RADIO_TECH_IS95B_V01 = 13,  /**<  IS95B. */
  RIL_NW_RADIO_TECH_IS95A_V01 = 14,  /**<  IS95A. */
  RIL_NW_RADIO_TECH_UMTS_V01 = 15,   /**<  UMTS. */
  RIL_NW_RADIO_TECH_EDGE_V01 = 16,   /**<  EDGE. */
  RIL_NW_RADIO_TECH_GPRS_V01 = 17,   /**<  GPRS. */
  RIL_NW_RADIO_TECH_NR5G_V01 = 18,   /**<  5GNR. */
  RIL_NW_RADIO_TECH_NONE_V01 = 19,   /**<  No technology selected. */
  RIL_NW_RADIO_TECH_T_MAX_ENUM_VAL_V01 = 2147483647 /* To force a 32 bit signed enum.  Do not change or use*/
}ril_nw_radio_tech_t;

typedef enum {
  RIL_NW_SIG_UNKNOWN = 0,
  RIL_NW_SIG_NR5G = 1,
  RIL_NW_SIG_LTE = 2,
  RIL_NW_SIG_WCDMA = 3,
  RIL_NW_SIG_GSM = 4
}ril_nw_signal_rat_t;

typedef enum {
    RIL_NW_RESP_UNKNOWN,
    RIL_NW_RESP_NW_SCAN,
    RIL_NW_RESP_NW_SELECTION,
    RIL_NW_IND_NITZ_TIME,
    RIL_NW_RESP_DATA_REG_STATE
}ril_nw_resp_enum;

typedef struct {
    ril_nw_network_status_t     network_status;
    ril_nw_operator_name_t      operator_name;
    ril_nw_radio_tech_t         rat;
    uint8_t                     is_home_nw;
}ril_nw_scan_entry_t;

typedef struct {
    int                   entry_len;
    ril_nw_scan_entry_t   entry[RIL_NW_SCAN_LIST_MAX_LENGTH];
}ril_nw_scan_result_t;

typedef struct {
    uint8_t               nitz_time_valid;
    char                  nitz_time[RIL_NW_NITZ_TIME_STR_BUF_MAX_V01 + 1];
    uint8_t               abs_time_valid;
    uint64_t              abs_time;           // Absolute time in milliseconds since Jan 6, 1980 00:00:00 hr.
    uint8_t               leap_sec_valid;
    int8_t                leap_sec;           // Leap Second
}ril_nw_nitz_time_info_t;

typedef struct {
    int             pcell_band;
    int             scell_info_list_len;
    int             scell_band[RIL_NW_MAX_SCELL_LIST_LEN_V01];
}ril_nw_lte_ca_info_t;

typedef void (*ril_nw_resp_cb)
(
    ril_nw_resp_enum nw_resp,
    int              error,
    unsigned int     resp_data_size,
    void             *resp_data
);

#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
typedef struct
{
    nas_julian_time_type_v01 universal_time;
    uint8_t time_zone_valid;
    int8_t time_zone;
    uint8_t daylt_sav_adj_valid;
    uint8_t daylt_sav_adj;
    uint8_t radio_if_valid;
    nas_radio_if_enum_v01 radio_if;
} ril_nw_network_time_info_type;

typedef struct
{
    uint8_t lte_sib16_acquired_valid;
    nas_tri_state_boolean_type_v01 lte_sib16_acquired;
    uint8_t universal_time_valid;
    nas_lte_sib16_julian_time_type_v01 universal_time;
    uint8_t abs_time_valid;
    uint64_t abs_time;
    uint8_t leap_sec_valid;
    int8_t leap_sec;
    uint8_t time_zone_valid;
    int8_t time_zone;
    uint8_t daylt_sav_adj_valid;
    uint8_t daylt_sav_adj;
} ril_nw_lte_sib16_network_time_info_type;

data_rat_enum ril_nw_get_rat_tech(void);
void ril_nw_get_operator_name(char* long_name, char* short_name);
int ril_nw_get_nitz_time(ril_nw_nitz_time_info_t *resp_buff);
int ril_nw_get_config_qmi(ril_nw_config *output_nw_config);
int ril_nw_set_config_qmi(ril_nw_config *input_nw_config);
int ril_nw_scan_network_qmi(uint16_t perf_mode);
int ril_nw_set_network_selection_auto_qmi(void);
int ril_nw_get_lte_ca_info(ril_nw_lte_ca_info_t *ca_info);
int ril_nw_set_ps_attach(void);
int ril_nw_set_ps_detach(void);

static void nas_cb_scan_network( qmi_client_type user_handle,
                          unsigned int msg_id,
                          void *resp_c_struct,
                          unsigned int resp_c_struct_len,
                          void *resp_cb_data,
                          int tranp_err);

#endif
