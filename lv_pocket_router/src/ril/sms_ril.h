#include <stdbool.h>
#include "ril_util.h"

#define RIL_SMS_MAX_ADDR_LENGTH 252
#define RIL_SMS_MAX_TIMESTAMP_LENGTH 14
#define RIL_SMS_MAX_MT_MSG_LENGTH 160

typedef enum {
  RIL_SMS_MSG_FORMAT_TEXT_GW_NOTSUPPORT,
  RIL_SMS_MSG_FORMAT_TEXT_GW_PP,    /**<  Message format GW_PP text. */
  RIL_SMS_MSG_FORMAT_TEXT_GW_BC     /**<  Message format GW_BC text. */
}ril_sms_msg_format_t;

typedef enum {

  RIL_SMS_MESSAGE_CLASS_0 = 0, /**<  Class 0. */
  RIL_SMS_MESSAGE_CLASS_1 = 1, /**<  Class 1. */
  RIL_SMS_MESSAGE_CLASS_2 = 2, /**<  Class 2. */
  RIL_SMS_MESSAGE_CLASS_3 = 3, /**<  Class 3. */
  RIL_SMS_MESSAGE_CLASS_NONE = 4 /**<  None. */

}ril_sms_message_class_t;


typedef enum {

  RIL_SMS_MESSAGE_STATUS_UNKNOW = 0,    //Empty record
  RIL_SMS_MESSAGE_STATUS_READ = 1,  //MT message
  RIL_SMS_MESSAGE_STATUS_UNREAD = 2,   //MT message
  RIL_SMS_MESSAGE_STATUS_SENT = 3   //MO message

}ril_sms_message_status_t;

typedef enum {

  RIL_SMS_STORAGE_NONE = 0, //MT SMS notify
  RIL_SMS_STORAGE_UIM = 1  //SIM message

}ril_sms_message_storage_t;

typedef enum {

  RIL_SMS_ENCODING_UNKNOWN = 0, 
  RIL_SMS_ENCODING_7BIT = 1, 
  RIL_SMS_ENCODING_8BIT = 2, 
  RIL_SMS_ENCODING_16BIT = 3

}ril_sms_message_encoding_t;

#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
/** @addtogroup mcm_sms_messages
    @{
  */
/** Indication message; Point-to-point message indication. */
typedef struct {

  /* Mandatory */
  ril_sms_msg_format_t message_format;
  /**<   Message format. */

  /* Mandatory */
  char message_content[(RIL_SMS_MAX_MT_MSG_LENGTH) * 3 + 1];  //x3 for UCS2 char support.
  /**<   Message content. */

  /* Mandatory */
  char source_address[RIL_SMS_MAX_ADDR_LENGTH + 1];
  /**<   Source address. */

  /* Mandatory */
  int64_t message_id;
  /**<   Message ID. */

  /* Optional */
  ril_sms_message_class_t message_class;
  /**<   Message class. */

  /* Optional */
  uint32_t message_content_length;
  /**<   Message Content Length. */

  ril_sms_message_encoding_t encoding_type;

  ril_sms_message_status_t message_status;
  ril_sms_message_storage_t message_storage;
  uint8_t record_num;   //record number on SIM;
  char timestamp[RIL_SMS_MAX_TIMESTAMP_LENGTH + 1];
  
}ril_sms_pp_ind_msg;  /* Message */

typedef struct {

  ril_sms_message_storage_t message_storage;
  
}ril_sms_full_ind_msg;

typedef void (*ril_new_sms_ind) (ril_sms_pp_ind_msg *ind_data);
typedef void (*ril_delete_sms_resp) (ril_result resp);
typedef void (*ril_mem_full_ind) (ril_sms_message_storage_t mem_storage);

typedef struct {
    ril_new_sms_ind *sms_ind_cb;    
    ril_mem_full_ind *sms_full_ind;
}ril_sms_init_data;

typedef struct {
    int sms_free_slot;    
    int sms_capacity;
}ril_sms_get_capacity;


bool rilSmsDeleteMessageFromSim(int rec_num, ril_delete_sms_resp *resp_cb);
void rilSmsInit(ril_sms_init_data *init_data);
void rilSmsHandleSmsInd(void *ind_data, uint32_t ind_len);
void rilSmsRefershSIMSMS();
void rilSmsGetCapacity(ril_sms_get_capacity *capacity);
void rilSmsSetReadStatus(int rec_num, ril_sms_message_status_t status);
#endif
