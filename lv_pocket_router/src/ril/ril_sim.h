#include <stdbool.h>
#ifdef FEATURE_ROUTER
#include "user_identity_module_v01.h"
#endif

typedef enum {
    UNKNOWN,
    ABSENT,
    PIN_REQUIRED,
    PUK_REQUIRED,
    NETWORK_LOCKED,
    READY,
    NOT_READY,
    PERM_DISABLED,
    CARD_IO_ERROR,
    CARD_RESTRICTED,
    ILLEGAL
}sim_state_enum;

typedef enum {
    PIN_UNKNOWN,
    PIN_ENABLED_NOT_VERIFIED,
    PIN_ENABLED_VERIFIED,
    PIN_DISABLED,
    PIN_BLOCKED,
    PIN_PERMANENTLY_BLOCKED
}sim_pin_state_enum;

typedef enum {
    CARD_TYPE_UNKNOWN, /**<  Unidentified card type.  */
    CARD_TYPE_ICC, /**<  Card of SIM or RUIM type.  */
    CARD_TYPE_UICC /**<  Card of USIM or CSIM type.  */
}sim_type_enum;

typedef enum {
    SIM_PIN_REQ_UNKNOWN,
    SIM_PIN_REQ_VERIFY_PIN,
    SIM_PIN_REQ_CHANGE_PIN,
    SIM_PIN_REQ_ENABLE_PIN,
    SIM_PIN_REQ_DISABLE_PIN,
    SIM_PIN_REQ_VERIFY_PUK
}sim_pin_req_enum;

typedef struct {
    sim_state_enum sim_state;
    sim_type_enum sim_type;
    sim_pin_state_enum sim_pin1_state;
    sim_pin_state_enum sim_pin2_state;
    int pin1_retries;
    int puk1_retries;
    int sim_state_valid;
}ril_sim_info;

typedef void (*ril_sim_pin_req_cb)
(
    sim_pin_req_enum  sim_pin_req,
    int  error,
    int  retry_left
);

#define SIM_PIN1_MIN_LENGTH 4
#define SIM_PIN1_MAX_LENGTH 8
#define SIM_PUK1_MAX_LENGTH 8
#define SIM_IMSI_LENGTH 16
#define SIM_MSISDN_MIN_LEN 14
#define SIM_PHONE_NUMBER_MAX 82

#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
int ril_sim_get_card_status(ril_sim_info *sim_info);
void ril_sim_get_phone_num(char *num_buff, int buff_len);
int ril_sim_change_pin(char *old_pin, char *new_pin);
int ril_sim_enable_pin(char *pin1);
int ril_sim_disable_pin(char *pin1);
int ril_sim_verify_pin(char *pin1);
int ril_sim_verify_puk(char *puk1, char *new_pin1);
void ril_sim_event_register(void);
void ril_sim_change_pin_resp(uim_change_pin_resp_msg_v01 *resp_ptr);
void ril_sim_enable_pin_resp(uim_set_pin_protection_resp_msg_v01 *resp_ptr);
void ril_sim_disable_pin_resp(uim_set_pin_protection_resp_msg_v01 *resp_ptr);
void ril_sim_verify_pin_resp(uim_verify_pin_resp_msg_v01 *resp_ptr);
void ril_sim_verify_puk_resp(uim_unblock_pin_resp_msg_v01 *resp_ptr);
int ril_sim_get_imsi(char *imsi, int imsi_buff_len);
int ril_sim_get_mnc_length(sim_type_enum sim_type, int *mnc_len);

void ril_unsolicited_uim_ind_handler( qmi_client_type user_handle, unsigned int msg_id, void *ind_buf, unsigned int ind_buf_len, void *ind_cb_data );
#endif

static const char *MCCMNC_CODES_HAVING_3DIGITS_MNC[] = {
        "302370", "302720", "310260",
        "405025", "405026", "405027", "405028", "405029", "405030", "405031", "405032",
        "405033", "405034", "405035", "405036", "405037", "405038", "405039", "405040",
        "405041", "405042", "405043", "405044", "405045", "405046", "405047", "405750",
        "405751", "405752", "405753", "405754", "405755", "405756", "405799", "405800",
        "405801", "405802", "405803", "405804", "405805", "405806", "405807", "405808",
        "405809", "405810", "405811", "405812", "405813", "405814", "405815", "405816",
        "405817", "405818", "405819", "405820", "405821", "405822", "405823", "405824",
        "405825", "405826", "405827", "405828", "405829", "405830", "405831", "405832",
        "405833", "405834", "405835", "405836", "405837", "405838", "405839", "405840",
        "405841", "405842", "405843", "405844", "405845", "405846", "405847", "405848",
        "405849", "405850", "405851", "405852", "405853", "405854", "405855", "405856",
        "405857", "405858", "405859", "405860", "405861", "405862", "405863", "405864",
        "405865", "405866", "405867", "405868", "405869", "405870", "405871", "405872",
        "405873", "405874", "405875", "405876", "405877", "405878", "405879", "405880",
        "405881", "405882", "405883", "405884", "405885", "405886", "405908", "405909",
        "405910", "405911", "405912", "405913", "405914", "405915", "405916", "405917",
        "405918", "405919", "405920", "405921", "405922", "405923", "405924", "405925",
        "405926", "405927", "405928", "405929", "405930", "405931", "405932", "502142",
        "502143", "502145", "502146", "502147", "502148"
};
