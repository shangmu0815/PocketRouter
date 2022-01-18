#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "ril.h"
#include "lv_pocket_router/src/util/debug_log.h"

#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
extern ril_sim_info g_sim_info;
extern ril_sim_pin_req_cb rilSimPinCB;
extern qmi_client_type uim_client_handle;
extern qmi_idl_service_object_type uim_serv_obj;

char uim_bin_to_hexchar(uint8_t bin);
void updateSimStateCache(uim_card_status_type_v01 *card_status);
int QMIcardStatusToRil(uim_card_status_type_v01 *uim_status, uint8_t slot, uint8_t app);
int QMIcardTypeToRil(uim_app_type_enum_v01 app_type);
int QMIpinStateToRil(uim_pin_state_enum_v01 pin_state);

void updateSimStateCache(uim_card_status_type_v01 *card_status)
{
    uint8_t primary_slot = 0;
    uint8_t primary_app = 0;
    uint8_t current_slot = 0;
    uint8_t current_app = 0;

    log_d("updateSimStateCache() Enter");
    if( card_status->index_gw_pri != 0xFFFF )
    {
        // MSB: slot, LSB: app.
        primary_slot = (uint8_t)((card_status->index_gw_pri >> 8)&0x00FF);
        primary_app = (uint8_t)card_status->index_gw_pri;
        log_d("primary_slot=%d, primary_app=%d", primary_slot, primary_app);

        if( card_status->card_info_len > primary_slot )
        {
            current_slot = primary_slot;
            log_d("current_slot = %d", current_slot);
        }
        else
        {
            log_e("invalid card_info_len=%d, primary_slot=%d.", card_status->card_info_len, primary_slot);
            current_slot = 0;
        }

        if( card_status->card_info[current_slot].app_info_len > primary_app)
        {
            current_app = primary_app;
            log_d("current_app = %d", current_app);
        }
        else
        {
            log_e("invalid app_info_len=%d, primary_app=%d.", card_status->card_info[current_slot].app_info_len, primary_app);
            current_app = 0;
        }
    }else
    {
        log_e("unassigned index_gw_pri. both use default value 0.");
    }
    g_sim_info.sim_state = QMIcardStatusToRil(card_status, current_slot, current_app);
    g_sim_info.sim_type = QMIcardTypeToRil(card_status->card_info[current_slot].app_info[current_app].app_type);
    g_sim_info.sim_pin1_state = QMIpinStateToRil(card_status->card_info[current_slot].app_info[current_app].pin1.pin_state);
    g_sim_info.sim_pin2_state = QMIpinStateToRil(card_status->card_info[current_slot].app_info[current_app].pin2.pin_state);
    g_sim_info.pin1_retries = card_status->card_info[current_slot].app_info[current_app].pin1.pin_retries;
    g_sim_info.puk1_retries = card_status->card_info[current_slot].app_info[current_app].pin1.puk_retries;
}

int QMIcardStatusToRil(uim_card_status_type_v01 *uim_status, uint8_t slot, uint8_t app)
{
    int sim_status = UNKNOWN;

    switch(uim_status->card_info[slot].card_state)
    {
        case UIM_CARD_STATE_ABSENT_V01:
            sim_status = ABSENT;
            break;
        case UIM_CARD_STATE_PRESENT_V01:
            {
                switch(uim_status->card_info[slot].app_info[app].app_state)
                {
                    case UIM_APP_STATE_DETECTED_V01:
                        sim_status = NOT_READY;
                        break;
                    case UIM_APP_STATE_PIN1_OR_UPIN_REQ_V01:
                        sim_status = PIN_REQUIRED;
                        break;
                    case UIM_APP_STATE_PUK1_OR_PUK_REQ_V01:
                        sim_status = PUK_REQUIRED;
                        break;
                    case UIM_APP_STATE_PERSON_CHECK_REQ_V01:
                        sim_status = NETWORK_LOCKED;
                        break;
                    case UIM_APP_STATE_PIN1_PERM_BLOCKED_V01:
                        sim_status = PERM_DISABLED;
                        break;
                    case UIM_APP_STATE_ILLEGAL_V01:
                        sim_status = ILLEGAL;
                        break;
                    case UIM_APP_STATE_READY_V01:
                        sim_status = READY;
                        break;
                    default:
                        sim_status = UNKNOWN;
                        break;
                }
            }
            break;
        default:
            sim_status = CARD_IO_ERROR;
            break;
    }
    return sim_status;
}

int QMIcardTypeToRil(uim_app_type_enum_v01 app_type)
{
    int card_type = CARD_TYPE_UNKNOWN;

    switch(app_type)
    {
        case UIM_APP_TYPE_SIM_V01:
        case UIM_APP_TYPE_RUIM_V01:
            card_type = CARD_TYPE_ICC;
            break;
        case UIM_APP_TYPE_USIM_V01:
        case UIM_APP_TYPE_CSIM_V01:
        case UIM_APP_TYPE_ISIM_V01:
            card_type = CARD_TYPE_UICC;
            break;
        default:
            break;
    }
    return card_type;
}

int QMIpinStateToRil(uim_pin_state_enum_v01 pin_state)
{
    int ret_state = PIN_UNKNOWN;

    switch(pin_state)
    {
       case UIM_PIN_STATE_ENABLED_NOT_VERIFIED_V01:
           ret_state = PIN_ENABLED_NOT_VERIFIED;
           break;
       case UIM_PIN_STATE_ENABLED_VERIFIED_V01:
           ret_state = PIN_ENABLED_VERIFIED;
           break;
       case UIM_PIN_STATE_DISABLED_V01:
           ret_state = PIN_DISABLED;
           break;
       case UIM_PIN_STATE_BLOCKED_V01:
           ret_state = PIN_BLOCKED;
           break;
       case UIM_PIN_STATE_PERMANENTLY_BLOCKED_V01:
           ret_state = PIN_PERMANENTLY_BLOCKED;
           break;
       default:
           break;
    }
    return ret_state;
}

void ril_sim_change_pin_resp(uim_change_pin_resp_msg_v01 *resp_ptr)
{
    ril_error_type err;

    if(RIL_DEBUG)
        log_d("ril_sim_change_pin_resp ENTER");

    if(resp_ptr->resp.result == QMI_RESULT_SUCCESS_V01)
    {
        log_d("ril_sim_change_pin_resp: SUCCESS");
        err = RIL_SUCCESS;
    }else
    {
        log_e("ril_sim_change_pin_resp: ERROR: 0x%x", resp_ptr->resp.error);
        err = convertQMIErrCodeToRil(resp_ptr->resp.result, resp_ptr->resp.error);
    }

    if(resp_ptr->retries_left_valid)
    {
        log_d("number of retires left: 0x%x", resp_ptr->retries_left.verify_left);
        g_sim_info.pin1_retries = (int) resp_ptr->retries_left.verify_left;
    }else
    {
        log_d("ril_sim_change_pin_resp: retries invalid");
    }

    (*rilSimPinCB)(SIM_PIN_REQ_CHANGE_PIN, err, resp_ptr->retries_left.verify_left);
}

void ril_sim_enable_pin_resp(uim_set_pin_protection_resp_msg_v01 *resp_ptr)
{
    ril_error_type err;

    if(RIL_DEBUG)
        log_d("ril_sim_enable_pin_resp ENTER");

    if(resp_ptr->resp.result == QMI_RESULT_SUCCESS_V01)
    {
        log_d("ril_sim_enable_pin_resp: SUCCESS");
        err = RIL_SUCCESS;
    }else
    {
        log_e("ril_sim_enable_pin_resp: ERROR: 0x%x", resp_ptr->resp.error);
        err = convertQMIErrCodeToRil(resp_ptr->resp.result, resp_ptr->resp.error);
    }

    if(resp_ptr->retries_left_valid)
    {
        log_d("number of retires left: 0x%x", resp_ptr->retries_left.verify_left);
        g_sim_info.pin1_retries = (int) resp_ptr->retries_left.verify_left;
    }else
    {
        log_d("ril_sim_enable_pin_resp: retries invalid");
    }

    (*rilSimPinCB)(SIM_PIN_REQ_ENABLE_PIN, err, resp_ptr->retries_left.verify_left);
}

void ril_sim_disable_pin_resp(uim_set_pin_protection_resp_msg_v01 *resp_ptr)
{
    ril_error_type err;

    if(RIL_DEBUG)
        log_d("ril_sim_disable_pin_resp ENTER");

    if(resp_ptr->resp.result == QMI_RESULT_SUCCESS_V01)
    {
        log_d("ril_sim_disable_pin_resp: SUCCESS");
        err = RIL_SUCCESS;
    }else
    {
        log_e("ril_sim_disable_pin_resp: ERROR: 0x%x", resp_ptr->resp.error);
        err = convertQMIErrCodeToRil(resp_ptr->resp.result, resp_ptr->resp.error);
    }

    if(resp_ptr->retries_left_valid)
    {
        log_d("number of retires left: 0x%x", resp_ptr->retries_left.verify_left);
        g_sim_info.pin1_retries = (int) resp_ptr->retries_left.verify_left;
    }else
    {
        log_d("ril_sim_disable_pin_resp: retries invalid");
    }

    (*rilSimPinCB)(SIM_PIN_REQ_DISABLE_PIN, err, resp_ptr->retries_left.verify_left);
}

void ril_sim_verify_pin_resp(uim_verify_pin_resp_msg_v01 *resp_ptr)
{
    ril_error_type err;

    if(RIL_DEBUG)
        log_d("ril_sim_verify_pin_resp ENTER");

    if(resp_ptr->resp.result == QMI_RESULT_SUCCESS_V01)
    {
        log_d("ril_sim_verify_pin_resp: SUCCESS");
        err = RIL_SUCCESS;
    }else
    {
        log_e("ril_sim_verify_pin_resp: ERROR: 0x%x", resp_ptr->resp.error);
        err = convertQMIErrCodeToRil(resp_ptr->resp.result, resp_ptr->resp.error);
    }

    if(resp_ptr->retries_left_valid)
    {
        log_d("number of retires left: 0x%x", resp_ptr->retries_left.verify_left);
        g_sim_info.pin1_retries = (int) resp_ptr->retries_left.verify_left;
    }else
    {
        log_d("ril_sim_verify_pin_resp: retries invalid");
    }

    (*rilSimPinCB)(SIM_PIN_REQ_VERIFY_PIN, err, resp_ptr->retries_left.verify_left);
}

void ril_sim_verify_puk_resp(uim_unblock_pin_resp_msg_v01 *resp_ptr)
{
    ril_error_type err;

    if(RIL_DEBUG)
        log_d("ril_sim_verify_puk_resp ENTER");

    if(resp_ptr->resp.result == QMI_RESULT_SUCCESS_V01)
    {
        log_d("ril_sim_verify_puk_resp: SUCCESS");
        err = RIL_SUCCESS;
    }else
    {
        log_e("ril_sim_verify_puk_resp: ERROR: 0x%x", resp_ptr->resp.error);
        err = convertQMIErrCodeToRil(resp_ptr->resp.result, resp_ptr->resp.error);
    }

    if(resp_ptr->retries_left_valid)
    {
        log_d("number of retires left: 0x%x", resp_ptr->retries_left.unblock_left);
        g_sim_info.puk1_retries = (int) resp_ptr->retries_left.unblock_left;
    }else
    {
        log_d("ril_sim_verify_puk_resp: retries invalid");
    }

    (*rilSimPinCB)(SIM_PIN_REQ_VERIFY_PUK, err, resp_ptr->retries_left.unblock_left);
}

int ril_sim_get_card_status(ril_sim_info *sim_info){
    uim_get_card_status_resp_msg_v01 resp = {0};
    qmi_error_type_v01 qmi_error;
    uint8_t primary_slot = 0;
    uint8_t primary_app = 0;
    uint8_t current_slot = 0;
    uint8_t current_app = 0;

    qmi_error = qmi_client_send_msg_sync(uim_client_handle,
                                         QMI_UIM_GET_CARD_STATUS_REQ_V01,
                                         NULL,
                                         0,
                                         &resp,
                                         sizeof(resp),
                                         5000);
    if(qmi_error == QMI_ERR_NONE_V01)
    {
        if(resp.resp.result == QMI_RESULT_SUCCESS_V01)
        {
            if(resp.card_status_valid == TRUE)
            {
                //memcpy(&uim_status_cache, &resp.card_status, sizeof(uim_card_status_type_v01));
                log_d("resp.card_status_valid = %d", resp.card_status_valid);

                if( resp.card_status.index_gw_pri != 0xFFFF )
                {
                    // MSB: slot, LSB: app.
                    primary_slot = (uint8_t)((resp.card_status.index_gw_pri >> 8)&0x00FF);
                    primary_app = (uint8_t)resp.card_status.index_gw_pri;
                    log_d("primary_slot=%d, primary_app=%d", primary_slot, primary_app);

                    if( resp.card_status.card_info_len > primary_slot )
                    {
                        current_slot = primary_slot;
                        log_d("current_slot = %d", current_slot);
                    }
                    else
                    {
                        log_e("invalid card_info_len=%d, primary_slot=%d.", resp.card_status.card_info_len, primary_slot);
                        current_slot = 0;
                    }

                    if( resp.card_status.card_info[current_slot].app_info_len > primary_app)
                    {
                        current_app = primary_app;
                        log_d("current_app = %d", current_app);
                    }
                    else
                    {
                        log_e("invalid app_info_len=%d, primary_app=%d.", resp.card_status.card_info[current_slot].app_info_len, primary_app);
                        current_app = 0;
                    }
                }else
                {
                    log_e("unassigned index_gw_pri. both use default value 0.");
                }

                sim_info->sim_state = QMIcardStatusToRil(&resp.card_status, current_slot, current_app);
                sim_info->sim_type = QMIcardTypeToRil(resp.card_status.card_info[current_slot].app_info[current_app].app_type);
                sim_info->sim_pin1_state = QMIpinStateToRil(resp.card_status.card_info[current_slot].app_info[current_app].pin1.pin_state);
                sim_info->sim_pin2_state = QMIpinStateToRil(resp.card_status.card_info[current_slot].app_info[current_app].pin2.pin_state);
                sim_info->pin1_retries = resp.card_status.card_info[current_slot].app_info[current_app].pin1.pin_retries;
                sim_info->puk1_retries = resp.card_status.card_info[current_slot].app_info[current_app].pin1.puk_retries;
                g_sim_info.sim_state_valid = TRUE;
                log_d("card info. state=%d, type=%d, pin1_state=%d, pin1_retry=%d, puk1_retry=%d", sim_info->sim_state, sim_info->sim_type, sim_info->sim_pin1_state, sim_info->pin1_retries, sim_info->puk1_retries);
            }else
            {
                log_e("resp.card_status_valid = %d", resp.card_status_valid);
                sim_info->sim_state = UNKNOWN;
                g_sim_info.sim_state_valid = FALSE;
            }
        }else
        {
            log_e("qmi resp err. error = %d", resp.resp.error);
            return RIL_ERROR_INVALID_QMI_RESULT;
        }
    }else
    {
        log_e("qmi send failed. qmi_error = %d", qmi_error);
        sim_info->sim_state = ABSENT;
        return RIL_ERROR_SEND_QMI_FAIL;
    }
    return RIL_SUCCESS;
}

void ril_sim_get_phone_num(char *num_buff, int buff_len){
    uim_read_record_req_msg_v01 read_req = {0};
    uim_read_record_resp_msg_v01 resp = {0};
    qmi_error_type_v01 qmi_error;
    uint8_t gsm_path[4] = {0x00, 0x3F, 0x20, 0x7F}; /* DF GSM */
    uint8_t uicc_path[4] = {0x00, 0x3F, 0xFF, 0x7F}; /* ADF USIM/CSIM/ISIM */
    int phone_number_len = 0, len_offset = 0, data_offset = 0, ret_num_len = 0;
    uint32_t src = 0, dst = 0;

    if( num_buff == NULL )
    {
        log_e("ril_sim_get_phone_num error: num_buff = NULL");
        return ;
    }
    memset(num_buff, '\0', sizeof(buff_len));

    read_req.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    if( g_sim_info.sim_type == CARD_TYPE_ICC )
    {
        read_req.file_id.path_len = sizeof(gsm_path);
        memcpy(read_req.file_id.path, gsm_path, read_req.file_id.path_len);
    }else if( g_sim_info.sim_type == CARD_TYPE_UICC )
    {
        read_req.file_id.path_len = sizeof(uicc_path);
        memcpy(read_req.file_id.path, uicc_path, read_req.file_id.path_len);
    }else
    {
        log_e("[ril_sim_get_imsi] unknown card app type.");
    }
    read_req.file_id.file_id = 0x6F40;

    read_req.read_record.record = 1;
    /* data length of 0 indicates read entire record */
    read_req.read_record.length = 0;

    qmi_error = qmi_client_send_msg_sync(uim_client_handle,
                                         QMI_UIM_READ_RECORD_REQ_V01,
                                         &read_req,
                                         sizeof(read_req),
                                         &resp,
                                         sizeof(resp),
                                         5000);
    if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
         ( qmi_error != QMI_NO_ERR ) ||
         ( resp.resp.result != QMI_NO_ERR ) )
    {
        log_e("Send fail. error=%d.", resp.resp.result);
        return ;
    }

    if(resp.read_result_valid)
    {
        if(resp.read_result.content_len > QMI_UIM_CONTENT_RECORD_MAX_V01)
        {
            log_e("Invalid DPN data length: 0x%x", resp.read_result.content_len);
            return ;
        }

        if(resp.read_result.content_len < SIM_MSISDN_MIN_LEN)
        {
            log_e("Invalid MSISDN data length: 0x%x", resp.read_result.content_len);
            return ;
        }

        len_offset = resp.read_result.content_len - SIM_MSISDN_MIN_LEN;
        phone_number_len = resp.read_result.content[len_offset] - 1;
        /* Offset from the length byte where phone number data begins */
        data_offset = len_offset + 2;

        if(phone_number_len == 0 ||
           (phone_number_len * 2) > SIM_PHONE_NUMBER_MAX ||
           (data_offset + phone_number_len) > QMI_UIM_CONTENT_RECORD_MAX_V01)
        {
            log_e("Invalid data; phone num len: 0x%x, offset: 0x%x", phone_number_len, data_offset);
            return ;
        }

        for(src = data_offset, dst = 0; src < (data_offset + phone_number_len) && dst < (SIM_PHONE_NUMBER_MAX - 1); src++)
        {
            num_buff[dst] = uim_bin_to_hexchar(resp.read_result.content[src] & 0x0F);
            dst++;
            num_buff[dst] = uim_bin_to_hexchar(resp.read_result.content[src] >> 4);
            dst++;
        }
        ret_num_len = dst;

        /* Check if the phone number is an odd number of digits.
           If so, then the last digit should not be a numeric ASCII
           digit and should be marked to be removed. */
        if(num_buff[ret_num_len-1] < '0' ||
           num_buff[ret_num_len-1] > '9')
        {
            num_buff[ret_num_len-1] = '\0';
            ret_num_len--;
        }
    }
}

void ril_sim_event_register(void)
{
    qmi_error_type_v01 err = QMI_ERR_NONE_V01;
    uim_event_reg_req_msg_v01 qmi_request;
    uim_event_reg_resp_msg_v01 qmi_response;

    log_d("register for card status indications START");

    memset(&qmi_request, 0x00, sizeof(uim_event_reg_req_msg_v01));
    memset(&qmi_response, 0x00, sizeof(uim_event_reg_resp_msg_v01));

    /* Bit 0 of event mask - Card status */
    qmi_request.event_mask = 1;

    err = qmi_client_send_msg_sync( uim_client_handle,
                                    QMI_UIM_EVENT_REG_REQ_V01,
                                    (void*) &qmi_request,
                                    sizeof(qmi_request),
                                    (void*) &qmi_response,
                                    sizeof(qmi_response),
                                    5000);

    if(err == QMI_ERR_NONE_V01)
    {
        if (qmi_response.resp.result == QMI_RESULT_SUCCESS_V01)
        {
            log_d("uim_card_status_event_reg: OK");
            return ;
        }else
        {
            log_e("uim_card_status_event_reg: ERROR: 0x%x", qmi_response.resp.error);
            return ;
        }
    }
    else
    {
        log_e("uim_card_status_event_reg: failed to send QMI message: 0x%x", err);
        return ;
    }
} 

int ril_sim_change_pin(char *old_pin, char *new_pin)
{
    uim_change_pin_req_msg_v01 req = {0};
    uim_change_pin_resp_msg_v01 resp = {0};
    qmi_error_type_v01 qmi_error;

    if(old_pin == NULL || new_pin == NULL){
        return RIL_ERROR_INVALID_INPUT;
    }

    if(strlen(old_pin) < SIM_PIN1_MIN_LENGTH ||
       strlen(old_pin) > SIM_PIN1_MAX_LENGTH ){
        return RIL_ERROR_INVALID_INPUT;
    }

    if(strlen(new_pin) < SIM_PIN1_MIN_LENGTH ||
       strlen(new_pin) > SIM_PIN1_MAX_LENGTH ){
        return RIL_ERROR_INVALID_INPUT;
    }

    req.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    req.change_pin.pin_id = UIM_PIN_ID_PIN_1_V01;
    req.change_pin.old_pin_value_len = strlen(old_pin);
    memcpy(req.change_pin.old_pin_value, old_pin, req.change_pin.old_pin_value_len);
    req.change_pin.new_pin_value_len = strlen(new_pin);
    memcpy(req.change_pin.new_pin_value, new_pin, req.change_pin.new_pin_value_len);

    qmi_error = qmi_client_send_msg_sync(uim_client_handle,
                                         QMI_UIM_CHANGE_PIN_REQ_V01,
                                         &req,
                                         sizeof(req),
                                         &resp,
                                         sizeof(resp),
                                         5000);
    if(qmi_error == QMI_ERR_NONE_V01)
    {
        if(resp.resp.result == QMI_RESULT_SUCCESS_V01)
        {
            ril_sim_change_pin_resp(&resp);
            return RIL_SUCCESS;
        }else
        {
            log_e("qmi resp err. error = %d", resp.resp.error);
            if(resp.resp.error == QMI_ERR_INCORRECT_PIN_V01 || 
               resp.resp.error == QMI_ERR_PIN_BLOCKED_V01 || 
               resp.resp.error == QMI_ERR_PIN_PERM_BLOCKED_V01){
                ril_sim_change_pin_resp(&resp);
                return RIL_SUCCESS;
            }
            return RIL_ERROR_INVALID_QMI_RESULT;
        }
    }else
    {
        log_e("qmi send failed. qmi_error = %d", qmi_error);
        return RIL_ERROR_SEND_QMI_FAIL;
    }
}

int ril_sim_enable_pin(char *pin1)
{
    uim_set_pin_protection_req_msg_v01 req = {0};
    uim_set_pin_protection_resp_msg_v01 resp = {0};
    qmi_error_type_v01 qmi_error;

    if(pin1 == NULL){
        return RIL_ERROR_INVALID_INPUT;
    }

    if(strlen(pin1) < SIM_PIN1_MIN_LENGTH ||
       strlen(pin1) > SIM_PIN1_MAX_LENGTH ){
        return RIL_ERROR_INVALID_INPUT;
    }

    req.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    req.set_pin_protection.pin_id = UIM_PIN_ID_PIN_1_V01;
    req.set_pin_protection.pin_operation = UIM_PIN_OPERATION_ENABLE_V01;
    req.set_pin_protection.pin_value_len = strlen(pin1);
    memcpy(req.set_pin_protection.pin_value, pin1, req.set_pin_protection.pin_value_len);

    qmi_error = qmi_client_send_msg_sync(uim_client_handle,
                                         QMI_UIM_SET_PIN_PROTECTION_REQ_V01,
                                         &req,
                                         sizeof(req),
                                         &resp,
                                         sizeof(resp),
                                         5000);
    if(qmi_error == QMI_ERR_NONE_V01)
    {
        if(resp.resp.result == QMI_RESULT_SUCCESS_V01)
        {
            ril_sim_enable_pin_resp(&resp);
            return RIL_SUCCESS;
        }else
        {
            log_e("qmi resp err. error = %d", resp.resp.error);
            if(resp.resp.error == QMI_ERR_INCORRECT_PIN_V01 || 
               resp.resp.error == QMI_ERR_PIN_BLOCKED_V01 || 
               resp.resp.error == QMI_ERR_PIN_PERM_BLOCKED_V01){
                ril_sim_enable_pin_resp(&resp);
                return RIL_SUCCESS;
            }
            return RIL_ERROR_INVALID_QMI_RESULT;
        }
    }else
    {
        log_e("qmi send failed. qmi_error = %d", qmi_error);
        return RIL_ERROR_SEND_QMI_FAIL;
    }
}

int ril_sim_disable_pin(char *pin1)
{
    uim_set_pin_protection_req_msg_v01 req = {0};
    uim_set_pin_protection_resp_msg_v01 resp = {0};
    qmi_error_type_v01 qmi_error;

    if(pin1 == NULL){
        return RIL_ERROR_INVALID_INPUT;
    }

    if(strlen(pin1) < SIM_PIN1_MIN_LENGTH ||
       strlen(pin1) > SIM_PIN1_MAX_LENGTH ){
        return RIL_ERROR_INVALID_INPUT;
    }

    req.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    req.set_pin_protection.pin_id = UIM_PIN_ID_PIN_1_V01;
    req.set_pin_protection.pin_operation = UIM_PIN_OPERATION_DISABLE_V01;
    req.set_pin_protection.pin_value_len = strlen(pin1);
    memcpy(req.set_pin_protection.pin_value, pin1, req.set_pin_protection.pin_value_len);

    qmi_error = qmi_client_send_msg_sync(uim_client_handle,
                                         QMI_UIM_SET_PIN_PROTECTION_REQ_V01,
                                         &req,
                                         sizeof(req),
                                         &resp,
                                         sizeof(resp),
                                         5000);
    if(qmi_error == QMI_ERR_NONE_V01)
    {
        if(resp.resp.result == QMI_RESULT_SUCCESS_V01)
        {
            ril_sim_disable_pin_resp(&resp);
            return RIL_SUCCESS;
        }else
        {
            log_e("qmi resp err. error = %d", resp.resp.error);
            if(resp.resp.error == QMI_ERR_INCORRECT_PIN_V01 || 
               resp.resp.error == QMI_ERR_PIN_BLOCKED_V01 || 
               resp.resp.error == QMI_ERR_PIN_PERM_BLOCKED_V01){
                ril_sim_disable_pin_resp(&resp);
                return RIL_SUCCESS;
            }
            return RIL_ERROR_INVALID_QMI_RESULT;
        }
    }else
    {
        log_e("qmi send failed. qmi_error = %d", qmi_error);
        return RIL_ERROR_SEND_QMI_FAIL;
    }
} 

int ril_sim_verify_pin(char *pin1)
{
    uim_verify_pin_req_msg_v01 req = {0};
    uim_verify_pin_resp_msg_v01 resp = {0};
    qmi_error_type_v01 qmi_error;

    if(pin1 == NULL){
        return RIL_ERROR_INVALID_INPUT;
    }

    if(strlen(pin1) < SIM_PIN1_MIN_LENGTH ||
       strlen(pin1) > SIM_PIN1_MAX_LENGTH ){
        return RIL_ERROR_INVALID_INPUT;
    }
    
    req.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    req.verify_pin.pin_id = UIM_PIN_ID_PIN_1_V01;
    req.verify_pin.pin_value_len = strlen(pin1);
    memcpy(req.verify_pin.pin_value, pin1, req.verify_pin.pin_value_len);

    qmi_error = qmi_client_send_msg_sync(uim_client_handle,
                                         QMI_UIM_VERIFY_PIN_REQ_V01,
                                         &req,
                                         sizeof(req),
                                         &resp,
                                         sizeof(resp),
                                         5000);
    if(qmi_error == QMI_ERR_NONE_V01)
    {
        if(resp.resp.result == QMI_RESULT_SUCCESS_V01)
        {
            ril_sim_verify_pin_resp(&resp);
            return RIL_SUCCESS;
        }else
        {
            log_e("qmi resp err. error = %d", resp.resp.error);
            if(resp.resp.error == QMI_ERR_INCORRECT_PIN_V01 || 
               resp.resp.error == QMI_ERR_PIN_BLOCKED_V01 || 
               resp.resp.error == QMI_ERR_PIN_PERM_BLOCKED_V01){
                ril_sim_verify_pin_resp(&resp);
                return RIL_SUCCESS;
            }
            return RIL_ERROR_INVALID_QMI_RESULT;
        }
    }else
    {
        log_e("qmi send failed. qmi_error = %d", qmi_error);
        return RIL_ERROR_SEND_QMI_FAIL;
    }
}

int ril_sim_verify_puk(char *puk1, char *new_pin1)
{
    uim_unblock_pin_req_msg_v01 req = {0};
    uim_unblock_pin_resp_msg_v01 resp = {0};
    qmi_error_type_v01 qmi_error;

    if(puk1 == NULL){
        return RIL_ERROR_INVALID_INPUT;
    }

    if(strlen(puk1) < SIM_PIN1_MIN_LENGTH ||
       strlen(puk1) > SIM_PIN1_MAX_LENGTH ){
        return RIL_ERROR_INVALID_INPUT;
    }

    if(new_pin1 == NULL){
        return RIL_ERROR_INVALID_INPUT;
    }

    if(strlen(new_pin1) < SIM_PIN1_MIN_LENGTH ||
       strlen(new_pin1) > SIM_PIN1_MAX_LENGTH ){
        return RIL_ERROR_INVALID_INPUT;
    }

    req.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    req.unblock_pin.pin_id = UIM_PIN_ID_PIN_1_V01;
    req.unblock_pin.puk_value_len = strlen(puk1);
    memcpy(req.unblock_pin.puk_value, puk1, req.unblock_pin.puk_value_len);
    req.unblock_pin.new_pin_value_len = strlen(new_pin1);
    memcpy(req.unblock_pin.new_pin_value, new_pin1, req.unblock_pin.new_pin_value_len);

    qmi_error = qmi_client_send_msg_sync(uim_client_handle,
                                         QMI_UIM_UNBLOCK_PIN_REQ_V01,
                                         &req,
                                         sizeof(req),
                                         &resp,
                                         sizeof(resp),
                                         5000);
    if(qmi_error == QMI_ERR_NONE_V01)
    {
        if(resp.resp.result == QMI_RESULT_SUCCESS_V01)
        {
            ril_sim_verify_puk_resp(&resp);
            return RIL_SUCCESS;
        }else
        {
            log_e("qmi resp err. error = %d", resp.resp.error);
            if(resp.resp.error == QMI_ERR_INCORRECT_PIN_V01 || 
               resp.resp.error == QMI_ERR_PIN_BLOCKED_V01 || 
               resp.resp.error == QMI_ERR_PIN_PERM_BLOCKED_V01){
                ril_sim_verify_puk_resp(&resp);
                return RIL_SUCCESS;
            }
            return RIL_ERROR_INVALID_QMI_RESULT;
        }
    }else
    {
        log_e("qmi send failed. qmi_error = %d", qmi_error);
        return RIL_ERROR_SEND_QMI_FAIL;
    }
}

char uim_bin_to_hexchar(uint8_t bin)
{
    if (bin < 0x0a)
    {
        return (bin + '0');
    }
    else if (bin <= 0x0f)
    {
        return (bin + 'A' - 0x0a);
    }

    log_e("Invalid binary value: 0x%x\n", bin);
    return 0;
}

int ril_sim_get_imsi(char *imsi, int imsi_buff_len)
{
    uim_read_transparent_req_msg_v01 qmi_imsi_req = {0};
    uim_read_transparent_resp_msg_v01 resp = {0};
    qmi_error_type_v01 qmi_error;
    uint8_t gsm_path[4] = {0x00, 0x3F, 0x20, 0x7F}; /* DF GSM */
    uint8_t uicc_path[4] = {0x00, 0x3F, 0xFF, 0x7F}; /* ADF USIM/CSIM/ISIM */

    if( imsi_buff_len < 16 )
    {
        log_e("ril_sim_get_imsi error: imsi_buff_len=%d", imsi_buff_len);
        return RIL_ERROR_INVALID_INPUT;
    }
    memset(imsi, '\0', sizeof(imsi_buff_len));

    qmi_imsi_req.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    if( g_sim_info.sim_type == CARD_TYPE_ICC )
    {
        qmi_imsi_req.file_id.path_len = sizeof(gsm_path);
        memcpy(qmi_imsi_req.file_id.path, gsm_path, qmi_imsi_req.file_id.path_len);
    }else if( g_sim_info.sim_type == CARD_TYPE_UICC )
    {
        qmi_imsi_req.file_id.path_len = sizeof(uicc_path);
        memcpy(qmi_imsi_req.file_id.path, uicc_path, qmi_imsi_req.file_id.path_len);
    }else
    {
        log_e("[ril_sim_get_imsi] unknown card app type.");
    }
    qmi_imsi_req.file_id.file_id = 0x6F07;
    qmi_imsi_req.read_transparent.offset = 0;
    qmi_imsi_req.read_transparent.length = 0;

    qmi_error = qmi_client_send_msg_sync(uim_client_handle,
                                         QMI_UIM_READ_TRANSPARENT_REQ_V01,
                                         &qmi_imsi_req,
                                         sizeof(qmi_imsi_req),
                                         &resp,
                                         sizeof(resp),
                                         5000);
    if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
         ( qmi_error != QMI_NO_ERR ) ||
         ( resp.resp.result != QMI_NO_ERR ) )
    {
        log_e("Send fail. error=%d.", resp.resp.result);
        return RIL_ERROR_SEND_QMI_FAIL;
    }

    if( resp.read_result_valid )
    {
        int raw_imsi_len = 0;

        raw_imsi_len = resp.read_result.content[0];
        if(raw_imsi_len >= resp.read_result.content_len)
        {
            log_e("[ril_sim_get_imsi] Invalid IMSI data length: 0x%x\n",raw_imsi_len);
            return RIL_ERROR_INVALID_QMI_RESULT;
        }

        for (int src = 1, dst = 0; src <= raw_imsi_len; src++)
        {
           /* Only process lower part of byte for second and subsequent bytes */
           if (src > 1)
           {
               imsi[dst++] = uim_bin_to_hexchar(resp.read_result.content[src] & 0x0F);
           }

           /* Process upper part of byte for all bytes */
           imsi[dst++] = uim_bin_to_hexchar(resp.read_result.content[src] >> 4);
        }
    }else
    {
        log_e("[ril_sim_get_imsi] read result: %d", resp.read_result_valid);
        return RIL_ERROR_INVALID_QMI_RESULT;
    }
    return RIL_SUCCESS;
}

int ril_sim_get_mnc_length(sim_type_enum sim_type, int *mnc_len)
{
    uim_read_transparent_req_msg_v01 read_ad_req = {0};
    uim_read_transparent_resp_msg_v01 resp = {0};
    qmi_error_type_v01 qmi_error;
    uint8_t gsm_path[4] = {0x00, 0x3F, 0x20, 0x7F}; /* DF GSM */
    uint8_t uicc_path[4] = {0x00, 0x3F, 0xFF, 0x7F}; /* ADF USIM/CSIM/ISIM */

    read_ad_req.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    if( g_sim_info.sim_type == CARD_TYPE_ICC )
    {
        read_ad_req.file_id.path_len = sizeof(gsm_path);
        memcpy(read_ad_req.file_id.path, gsm_path, read_ad_req.file_id.path_len);
    }else if( g_sim_info.sim_type == CARD_TYPE_UICC )
    {
        read_ad_req.file_id.path_len = sizeof(uicc_path);
        memcpy(read_ad_req.file_id.path, uicc_path, read_ad_req.file_id.path_len);
    }else
    {
        log_e("[ril_sim_get_mnc_length] unknown card app type.");
    }
    read_ad_req.file_id.file_id = 0x6FAD;
    read_ad_req.read_transparent.offset = 0;
    read_ad_req.read_transparent.length = 0;

    qmi_error = qmi_client_send_msg_sync(uim_client_handle,
                                         QMI_UIM_READ_TRANSPARENT_REQ_V01,
                                         &read_ad_req,
                                         sizeof(read_ad_req),
                                         &resp,
                                         sizeof(resp),
                                         5000);
    if ( ( qmi_error == QMI_TIMEOUT_ERR ) ||
         ( qmi_error != QMI_NO_ERR ) ||
         ( resp.resp.result != QMI_NO_ERR ) )
    {
        log_e("Send fail. error=%d.", resp.resp.result);
        return RIL_ERROR_SEND_QMI_FAIL;
    }

    if( resp.read_result_valid )
    {
        if (resp.read_result.content_len < 3) {
            log_e("Corrupt AD data on SIM");
            *mnc_len = 0;
            return RIL_ERROR_UNKNOWN;
        }

        if (resp.read_result.content_len == 3) {
            log_e("MNC length not present in EF_AD");
            *mnc_len = 0;
            return RIL_ERROR_UNKNOWN;
        }

        int len = (int)(resp.read_result.content[3] & 0xf);
        if (len == 2 || len == 3) {
            *mnc_len = len;
            return RIL_SUCCESS;
        } else {
            log_e("Received invalid or unset MNC Length=%d", len);
            return RIL_ERROR_UNKNOWN;
        }
    }
    else
    {
        log_e("[ril_sim_get_imsi] read result valid: %d", resp.read_result_valid);
        return RIL_ERROR_INVALID_QMI_RESULT;
    }
    return RIL_ERROR_UNKNOWN;
}

void ril_unsolicited_uim_ind_handler( qmi_client_type user_handle,
                                      unsigned int msg_id,
                                      void *ind_buf,
                                      unsigned int ind_buf_len,
                                      void *ind_cb_data )
{
    uint32_t decoded_payload_len;
    void *decoded_payload;
    qmi_client_error_type err_code;
    uim_status_change_ind_msg_v01 *ind_msg;
 
    log_d("ril_unsolicited_uim_ind_handler, msg %d", msg_id);

    if(ind_buf != NULL)
    {
        qmi_idl_get_message_c_struct_len( uim_serv_obj,
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
                                   uim_client_handle,
                                   QMI_IDL_INDICATION,
                                   msg_id,
                                   ind_buf,
                                   ind_buf_len,
                                   decoded_payload,
                                   decoded_payload_len);
            if (err_code != QMI_NO_ERR)
            {
                log_e("Error: Decoding unsolicited indication with id = %d, returned in error = %d", msg_id, (int)err_code);
                if(decoded_payload != NULL)
                    free(decoded_payload);
                return ;
            }
        }else
        {
            log_e("Error: decoded_payload = NULL or decoded_payload_len=0.");
            return ;
        }
    }else
    {
        log_e("Error: ind_buf = NULL.\n");
        return ;
    }


    if(msg_id == QMI_UIM_STATUS_CHANGE_IND_V01)
    {
        ind_msg = (uim_status_change_ind_msg_v01 *)decoded_payload;

        if(ind_msg->card_status_valid)
        {
            log_d("card_info_len = %d", ind_msg->card_status.card_info_len);
            for(int i=0; i<ind_msg->card_status.card_info_len; i++)
            {
                log_d("card_info[%d].card_state = %d", i, ind_msg->card_status.card_info[i].card_state);
                log_d("app_info_len = %d", ind_msg->card_status.card_info[i].app_info_len);
                for(int j=0; j<ind_msg->card_status.card_info[i].app_info_len; j++)
                {
                    log_d("app_info[%d].app_state = %d", j, ind_msg->card_status.card_info[i].app_info[j].app_state);
                }
            }
            updateSimStateCache(&ind_msg->card_status);
            g_sim_info.sim_state_valid = TRUE;
        }else
        {
            g_sim_info.sim_state_valid = FALSE;
        }
    }

    if(decoded_payload != NULL)
        free(decoded_payload);
}

#endif
