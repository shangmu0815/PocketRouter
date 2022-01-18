#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>
#ifdef FEATURE_ROUTER
#include "wireless_messaging_service_v01.h"
#endif
#include "ril.h"
//#include "sms_ril.h"
#include "lv_pocket_router/src/util/debug_log.h"

#define USIM_PATH "3F007FFF6F3C"  //MF_SIM + DF_ADF
#define SIM_PATH "3F007F106F3C"    //MF_SIM + DF_TELECOM
#define MAXRAWOUT 255
#define QMI_REQ_TIMEOUT 5000
#define MAX_MESSAGE_LIST 100

#define LOG_BUF_SIZE    1024
static char LOG_BUF[LOG_BUF_SIZE];
static size_t LOG_BUF_CUR =0;

#define LOG_BUF_CLEAR       LOG_BUF_CUR=0; LOG_BUF[0]=0
#define LOG_BUF_APPEND(...) sms_log_append(&LOG_BUF_CUR, LOG_BUF, LOG_BUF_SIZE, __VA_ARGS__)
#define LOG_BUF_END         log_d("%s\n", LOG_BUF)

#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
qmi_idl_service_object_type sms_service_obj, uim_service_obj;
qmi_client_os_params   sms_os_params;
qmi_client_type        sms_user_handle;
extern qmi_client_type uim_client_handle;
bool waitforread;


#define MAX_UD_LEN 140
char sDefaultTables[] =     {'@'   ,0xa3 ,'$'     ,0xa5 ,0xe8 ,0xe9 ,0xf9 ,0xec ,0xf2 ,0xc7 ,0x0a ,0xd8 ,0xf8 ,0x0d ,0xc5 ,0xe5 ,     \
                                            0xff,'_'    ,0xff  ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,0xff ,' '     ,0xc6 ,0xe6 ,0xdf ,0xc9 ,       \
                                            ' '     ,'!'    ,'\"'     ,'#'    ,0xa4 ,'%'    ,'&'    , '\''   , '('    , ')'    ,'*'     ,'+'      ,','    ,'-'     ,'.'      ,'/'    ,      \
                                            '0'    ,'1'    ,'2'     ,'3'     ,'4'     ,'5'    ,'6'     ,'7'     ,'8'     ,'9'     ,':'     ,';'     ,'<'    ,'='     ,'>'     ,'?'    ,     \
                                            0xa1 ,'A'    ,'B'     ,'C'    ,'D'    ,'E'     ,'F'     ,'G'    ,'H'   ,'I'      ,'J'      ,'K'    ,'L'     ,'M'    ,'N'     ,'O'     ,      \
                                            'P'     ,'Q'   ,'R'     ,'S'     ,'T'     ,'U'     ,'V'    ,'W'     ,'X'    ,'Y'    ,'Z'    ,0xc4 ,0xd6 ,0xd1 ,0xdc ,0xa7 ,     \
                                            0xbf ,'a'    ,'b'     ,'c'      ,'d'     ,'e'     ,'f'     ,'g'      ,'h'    ,'i'     ,'j'     ,'k'      ,'l'     ,'m'    ,'n'     ,'o'    ,    \
                                            'p'     ,'q'    ,'r'     ,'s'      ,'t'      ,'u'     ,'v'     ,'w'     ,'x'    ,'y'    ,'z'     ,0xe4  ,0xf6 ,0xf1 ,0xfc  ,0xe0    \
                                            };
char sExtensionTables[] = {0x0c ,'^'   ,'{'     ,'}'     ,'\\'    ,'['      ,'~'    ,']'      ,'|'     ,0xff  \
                                           };

char sExtensionIndex[] = {0x0a ,0x014 ,0x28 ,0x29 ,0x2f ,0x3c ,0x3d ,0x3e ,0x40 ,0x65  \
                                           };
char Greek_delta[3] = {0xce, 0x94, 0x00};
char Greek_phi[3] = {0xcf, 0x86, 0x00};
char Greek_gamma[3] = {0xce, 0xb3, 0x00};
char Greek_lambda[3] = {0xce, 0xbb, 0x00};
char Greek_omega[3] = {0xcf, 0x89, 0x00};
char Greek_pi[3] = {0xcf, 0x80, 0x00};
char Greek_psi[3] = {0xce, 0xa8, 0x00};
char Greek_sigma[3] = {0xce, 0xa3, 0x00};
char Greek_theta[3] = {0xce, 0xb8, 0x00};
char Greek_xi[3] = {0xce, 0xbe, 0x00};
char Sign_euro[4] = {0xe2, 0x82, 0xac, 0x00};
char *GreekArry[] = {Greek_delta, Greek_phi, Greek_gamma, Greek_lambda, Greek_omega, Greek_pi, Greek_psi, Greek_sigma, Greek_theta, Greek_xi};
unsigned char sGreekIndex[] = {0x10, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A};

unsigned char getDefaultAlphabet(unsigned char alphabet);
unsigned char getExtensionAlphabet(unsigned char alphabet);
int getGreekAlphabet(char *message_content, unsigned char alphabet);
void rilSmsIndRegister(ril_sms_init_data *ind_ptr);

ril_new_sms_ind sms_ind_ptr = NULL;
ril_delete_sms_resp sms_delete_ptr = NULL;
ril_mem_full_ind sms_full_ptr = NULL;
int g_sim_capacity =0;
static pthread_t mm_tid;

void sms_log_append(size_t* pcur, char* str, size_t size, const char * format, ... ){
    size_t cur = *pcur;
    size_t written=0;

    va_list args;
    va_start (args, format);
    if(cur < size)
    {
        written+= vsnprintf(&str[cur], size-cur, format, args);
        cur += (written > 0) ? written : 0;
        *pcur = cur;
    }
    va_end (args);
}

void print_sms_raw_uint8(const uint8* data, uint32_t data_len) {
    uint32_t i;
    uint32_t j;
    uint8 k;

    log_d("RAW uint8 in hex with space (data_len=%d):\n", data_len);
    if ((data_len < 0) ||(data_len > MAXRAWOUT)){
        printf("print_sms_raw_uint8: invalid data length\n");
        //return;
        data_len = MAXRAWOUT;
    }
    
    for (i = 0, k=1; i < data_len; i += 8, k++) {
        LOG_BUF_CLEAR;
        LOG_BUF_APPEND("%d:  ", k);
        for (j = i; j < i + 8 && j < data_len; j++) {
            LOG_BUF_APPEND("%.2x ", data[j]);
        }
        LOG_BUF_END;
    }
}

int ril_sms_decode_7bit_content(char *alpha_content, char *encoded_bytes, int encoded_bytes_len)
{
    unsigned char shift;
    unsigned char previous_value;
    unsigned char current_value;
    unsigned char alphabet;
    bool lookforextension;
    unsigned char isochar;
    int iter_i;
    int iter_j;
    
    log_d("ril_sms_decode_7bit_content\n");

    iter_i = 0;
    iter_j = 0;

    while (iter_i < encoded_bytes_len)
    {
        shift = (iter_i) % 7;
                
        current_value = encoded_bytes[iter_i++];
        //syslog(LOG_DEBUG, "current_value: 0x%.2x, previous_value:0x%.2x, shift: %d\n", current_value, previous_value, shift);
        alphabet =
            ((current_value << shift) | (previous_value >> (8 - shift))) & 0x7F;
        //syslog(LOG_DEBUG, "alphabet: 0x%.2x\n", alphabet);
        if(lookforextension == FALSE) 
        {
             if(alphabet != 0x1b)
            {
                isochar = getDefaultAlphabet(alphabet);
                if(isochar != 0xff) 
                    alpha_content[iter_j++] = isochar;
                else
                {
                    iter_j += getGreekAlphabet(alpha_content + iter_j, isochar);
                }
            } else {
                lookforextension = TRUE;
            }                    
        } else {
            if(alphabet == 0x65)
            {
                strcpy(alpha_content + iter_j, Sign_euro);
                iter_j += 3;
            }
            else
            {
                alpha_content[iter_j++] = getExtensionAlphabet(alphabet);
            }
            lookforextension = FALSE;
        }

        if (shift == 6)
        {
            if ((current_value >> 1) == 0x0D && iter_i == encoded_bytes_len)
            {
                break;
            }
            alphabet = current_value >> 1;
            
            if(lookforextension == FALSE) {
                if(alphabet == 0x1b) {
                    lookforextension = TRUE;
                } else {
                    isochar = getDefaultAlphabet(alphabet);
                    if(isochar != 0xff)
                        alpha_content[iter_j++] = isochar;
                    else
                    {
                        iter_j += getGreekAlphabet(alpha_content + iter_j, isochar);
                    }
                }
            } else {
                if(alphabet == 0x65)
                {
                    strcpy(alpha_content + iter_j, Sign_euro);
                    iter_j += 3;
                }
                else
                {
                    alpha_content[iter_j++] = getExtensionAlphabet(alphabet);
                }
                lookforextension = FALSE;
            }
            //log_d("message_content: %s\n", message_content);
            //syslog(LOG_DEBUG, "message_content: %s\n", address_number);
        }
        previous_value = current_value;
    }
    alpha_content[iter_j] = '\0';
    return iter_j;
}

int ril_sms_decode_address_number(
    char *address_number,
    uint8_t * encoded_bytes
)
{
    int encoded_bytes_len;
    int iter_i;
    int iter_j;
    int temp;

    encoded_bytes_len = 0;
    iter_i = 0;
    iter_j = 0;
    temp = 0;

    log_d("SIM SMS address number decode\n");
        //The length of the encoded bytes of phone number
    encoded_bytes_len = (((encoded_bytes[iter_i++] - 1) / 2) + 1) + 2;
	if(0xd0 == encoded_bytes[iter_i])
    {
        iter_i ++;
        ril_sms_decode_7bit_content(address_number, encoded_bytes + iter_i, encoded_bytes_len - iter_i);
            
    } else {
    	if (0x91 == encoded_bytes[iter_i++])
    	{
        	address_number[iter_j++] = '+';
    	}

    	while (iter_i < encoded_bytes_len)
    	{
        	address_number[iter_j++] = (encoded_bytes[iter_i] & 0x0F) + 48;
        	temp = ((encoded_bytes[iter_i++] >> 4) & 0x0F);
        	if (0x0F != temp)
        	{
            	address_number[iter_j++] = temp + 48;
        	}
    	}
    	address_number[iter_j] = '\0';
    }

    log_d("ril_sms_decode_address_number: %s\n", address_number);

    return (encoded_bytes_len);
}

int ril_sms_decode_timestamp(
    char *timestamp,
    uint8_t * encoded_bytes
)
{
    int i;
    int index;
    unsigned char time;
    char tmp[2 +1];
    log_d("SIM SMS time stamp decode\n");
    

    index = 0;

    for(i = 0; i < 7; i ++)
    {
        time = 0;
        time = encoded_bytes[index] << 4 & 0xF0;
        time |= encoded_bytes[index] >> 4 & 0x0F;
    
        memset(tmp, 0, sizeof(tmp));
    
        sprintf(tmp, "%.2x", time);
        strcat(timestamp, tmp);
        index ++;
    }

    log_d("ril_sms_decode_timestamp: %s\n", timestamp);

    return 7;
    
}

unsigned char getExtensionAlphabet(unsigned char alphabet)
{
    int i;
    int UBound = sizeof(sExtensionIndex)/sizeof(sExtensionIndex[0]);

    for(i = 0; i < 9; i ++)
    {
        if(alphabet == sExtensionIndex[i]) 
        {
            //log_d("getExtensionAlphabet: 0x%.2x, sExtensionTables: 0x%.2x\n", alphabet, sExtensionTables[i]);
            return sExtensionTables[i];
        }
    }
    log_d("getExtensionAlphabet: 0x%.2x NOT FOUND!", alphabet);
    return 0xff;    
}

int getGreekAlphabet(char *message_content, unsigned char alphabet)
{
    int i;
    int UBound = sizeof(sGreekIndex)/sizeof(sGreekIndex[0]);

    log_d("getGreekAlphabet: %d\n", alphabet);

    for(i = 0; i < UBound - 1; i ++)
    {
        if(alphabet == sGreekIndex[i]) 
        {
            //log_d("getGreekAlphabet GreekArry: %s\n", GreekArry[i]);
            strcpy(message_content, GreekArry[i]);
            return 2;
        }
    }
    log_d("getGreekAlphabet: 0x%.2x NOT FOUND!", alphabet);
    return 0;    
}

unsigned char getDefaultAlphabet(unsigned char alphabet)
{

    //log_d("getDefaultAlphabet: 0x%.2x, sDefaultTables: 0x%.2x\n", alphabet, sDefaultTables[alphabet]);
    
    return sDefaultTables[alphabet];
}

int ril_sms_decode_7bit_message_content(
    char *message_content,
    uint8_t * encoded_bytes,
    uint32_t *message_content_length,
    uint8_t bUDH
)
{
    int encoded_bytes_len;
    int iter_i;
    int iter_j;
    unsigned char shift;
    unsigned char previous_value;
    unsigned char current_value;
    unsigned char alphabet;
    bool lookforextension;
    unsigned char isochar;
	int udh_len;
	int fill_bits;
	int sb_idx, ud_idx;
    int total_bytes;
    unsigned char shift_byte[MAX_UD_LEN];
    unsigned char numofchar;
    unsigned char decodedchar;
    unsigned char numof7bit;

    encoded_bytes_len = 0;
    iter_i = 0;
    iter_j = 0;
    shift = 0;
    previous_value = 0;
    current_value = 0;
    alphabet = 0;
    lookforextension = false;
	udh_len = 0;
    fill_bits = 0;
    decodedchar = 0;
  
        if (encoded_bytes != NULL && message_content != NULL)
        {
            //Both concatenated and text messages have the same decoding structure.
            //concatenated msgs are sent in 2 or 3 PDUs are received as inducidual msgs

			if(bUDH == TRUE)
            {
                total_bytes = (encoded_bytes[iter_i] * 7) / 8;
                if(((encoded_bytes[iter_i] * 7) % 8) > 0)
                {
                    log_d("ril_sms_decode_7bit_message_content add one more byte.\n");
                    total_bytes ++;
                }
                log_d("ril_sms_decode_7bit_message_content encoded_bytes: 0x%.2x, total_bytes: %d\n", encoded_bytes[iter_i], total_bytes);
            
                log_d("ril_sms_decode_7bit_message_content udh_len: 0x%.2x\n", encoded_bytes[iter_i + 1]);
                udh_len = encoded_bytes[iter_i + 1];
                fill_bits = 7 - ((udh_len + 1) * 8) % 7;

                numof7bit = ((udh_len + 1) * 8) / 7;
                if(fill_bits != 0)
                    numof7bit += 1;    
                
                numofchar = encoded_bytes[iter_i] - numof7bit;
                
                encoded_bytes_len = ((total_bytes - udh_len - 1) * 8) / 7;
                
                iter_i += (udh_len + 1);
                log_d("ril_sms_decode_7bit_message_content start byte: 0x%.2x, fill_bits: %d, numof7bit: %d\n", encoded_bytes[iter_i + 1], fill_bits, numof7bit);

                memset(shift_byte, 0, sizeof(shift_byte));

                ud_idx = iter_i + 1;
                                
                for(sb_idx = 0; sb_idx < total_bytes - udh_len; sb_idx ++)
                {
                    shift_byte[sb_idx] = ((encoded_bytes[ud_idx] >> fill_bits) | (encoded_bytes[ud_idx + 1] << (8 - fill_bits))) & 0xFF;
                    //syslog(LOG_DEBUG, "ril_sms_decode_7bit_message_content temp_byte[%d]: 0x%.2x\n", i, temp_byte[i]);

                    ud_idx ++;
                }
                
                encoded_bytes_len = sb_idx;
                iter_i = 1;
                
                //print_raw_data(shift_byte, encoded_bytes_len);            
                memcpy(encoded_bytes + 1, shift_byte, encoded_bytes_len);
                print_sms_raw_uint8(encoded_bytes + 1, encoded_bytes_len);
            }
            else
            {
            	//the length of the message content
           	    numofchar = encoded_bytes[iter_i];
                encoded_bytes_len = (((encoded_bytes[iter_i++] * 7) / 8) + 1) + 1;
			}
            log_d("ril_sms_decode_7bit_message_content encoded_bytes_len: %d, numofchar: %d\n", encoded_bytes_len, numofchar);
            
            //the message content in GSM structure is decoded
            //the appended 'x' last bits is taken into previous value and "or"ed with current
            //bits
            while (iter_i < encoded_bytes_len)
            {
                //shift = iter_j & 0x07;
                shift = (iter_i - 1) % 7;
                current_value = encoded_bytes[iter_i++];
                //log_d("current_value: 0x%.2x, previous_value:0x%.2x, shift: %d\n", current_value, previous_value, shift);
                //message_content[iter_j++] =
                alphabet =
                    ((current_value << shift) | (previous_value >> (8 - shift))) & 0x7F;
                if(lookforextension == false) 
                {
                    if(alphabet != 0x1b)
                    {
                        isochar = getDefaultAlphabet(alphabet);
                        if(isochar != 0xff) 
                            message_content[iter_j++] = isochar;
                        else
                        {
                            iter_j += getGreekAlphabet(message_content + iter_j, isochar);
                        }
                    } else {
                        lookforextension = true;
                    }                    
                } else {
                    if(alphabet == 0x65)
                    {
                        strcpy(message_content + iter_j, Sign_euro);
                        iter_j += 3;
                    }
                    else
                    {
                        message_content[iter_j++] = getExtensionAlphabet(alphabet);
                    }
                    lookforextension = false;
                }
                //log_d("alphabet: 0x%.2x, message_content: 0x%.2x\n", alphabet, message_content[iter_j - 1]);
                decodedchar ++;
                if(decodedchar >= numofchar)
                    break;
                
                if (shift == 6)
                {
                    if ((current_value >> 1) == 0x0D && iter_i == encoded_bytes_len)
                    {
                        break;
                    }
                    alphabet = current_value >> 1;
                    if(lookforextension == false) {
                        //alphabet = current_value >> 1;
                        //message_content[iter_j++] = current_value >> 1;
                        if(alphabet == 0x1b) {
                            lookforextension = true;
                        } else {
                            isochar = getDefaultAlphabet(alphabet);
                            if(isochar != 0xff)
                                message_content[iter_j++] = isochar;
                            else
                            {
                                iter_j += getGreekAlphabet(message_content + iter_j, isochar);
                            }
                        }
                    } else {
                        if(alphabet == 0x65)
                        {
                            strcpy(message_content + iter_j, Sign_euro);
                            iter_j += 3;
                        }
                        else
                        {
                            message_content[iter_j++] = getExtensionAlphabet(alphabet);
                        }
                        lookforextension = false;
                    }
                    //log_d("message_content: %s\n", message_content);
                    decodedchar ++;
                    if(decodedchar >= numofchar)
                        break;
                }
                previous_value = current_value;
            }
            message_content[iter_j] = '\0';
            *message_content_length = iter_j;
        }

    return encoded_bytes_len;
}

int ril_sms_decode_ucs2_message_content(
    char *message_content,
    uint8_t * encoded_bytes,
    uint32_t *message_content_length,
    uint8_t bUDH
)
{
    int iter_i = 0;
    int iter_j = 0;
    unsigned long ucsc;
    int encoded_bytes_len;
    int i;
    int udhl = 0;
    
    log_d("ril_sms_decode_ucs2_message_content\n");

    if(bUDH == true)
    {
        encoded_bytes_len = encoded_bytes[iter_i ++];
        udhl = encoded_bytes[iter_i ++];
        encoded_bytes_len = (encoded_bytes_len -(udhl + 1))/2;
        iter_i += udhl;

        log_d("encoded_bytes_len = %d, udhl = %d, encoded_bytes = 0x%.2x\n", encoded_bytes_len, udhl, encoded_bytes[iter_i]);
    }
    else
    {
        encoded_bytes_len = encoded_bytes[iter_i ++]/2;
        log_d("encoded_bytes_len = %d, encoded_bytes = 0x%.2x\n", encoded_bytes_len, encoded_bytes[iter_i]);
    }

    for(i = 0; i < encoded_bytes_len; i ++)
    {        
        ucsc = encoded_bytes[iter_i ++] << 8;
        ucsc |= encoded_bytes[iter_i ++];

        if ( ucsc <= 0x0000007F )
        {
            // * U-00000000 - U-0000007F:  0xxxxxxx
            message_content[iter_j] = (ucsc & 0x7F);
            iter_j += 1;
        }
        else if ( ucsc >= 0x00000080 && ucsc <= 0x000007FF )
        {
            // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
            message_content[iter_j + 1] = (ucsc & 0x3F) | 0x80;
            message_content[iter_j] = ((ucsc >> 6) & 0x1F) | 0xC0;
            iter_j += 2;
        }
        else if ( ucsc >= 0x00000800 && ucsc <= 0x0000FFFF )
        {
            // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
            message_content[iter_j + 2] = (ucsc & 0x3F) | 0x80;
            message_content[iter_j + 1] = ((ucsc >>  6) & 0x3F) | 0x80;
            message_content[iter_j] = ((ucsc >> 12) & 0x0F) | 0xE0;
            iter_j += 3;
        }

        else if ( ucsc >= 0x00010000 && ucsc <= 0x001FFFFF )
        {
            // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            message_content[iter_j + 3] = (ucsc & 0x3F) | 0x80;
            message_content[iter_j + 2] = ((ucsc >>  6) & 0x3F) | 0x80;
            message_content[iter_j + 1] = ((ucsc >> 12) & 0x3F) | 0x80;
            message_content[iter_j] = ((ucsc >> 18) & 0x07) | 0xF0;
            iter_j += 4;
        }
        else if ( ucsc >= 0x00200000 && ucsc <= 0x03FFFFFF )
        {
            // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            message_content[iter_j + 4] = (ucsc & 0x3F) | 0x80;
            message_content[iter_j + 3] = ((ucsc >>  6) & 0x3F) | 0x80;
            message_content[iter_j + 2] = ((ucsc >> 12) & 0x3F) | 0x80;
            message_content[iter_j + 1] = ((ucsc >> 18) & 0x3F) | 0x80;
            message_content[iter_j] = ((ucsc >> 24) & 0x03) | 0xF8;
            iter_j += 5;
        }
        else if ( ucsc >= 0x04000000 && ucsc <= 0x7FFFFFFF )
        {
            // * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            message_content[iter_j + 5] = (ucsc & 0x3F) | 0x80;
            message_content[iter_j + 4] = ((ucsc >>  6) & 0x3F) | 0x80;
            message_content[iter_j + 3] = ((ucsc >> 12) & 0x3F) | 0x80;
            message_content[iter_j + 2] = ((ucsc >> 18) & 0x3F) | 0x80;
            message_content[iter_j + 1] = ((ucsc >> 24) & 0x3F) | 0x80;
            message_content[iter_j] = ((ucsc >> 30) & 0x01) | 0xFC;
            iter_j += 6;
        }
    }

    *message_content_length = iter_j;
    return encoded_bytes_len;
}

int ril_sms_decode_8bit_message_content(
    char *message_content,
    uint8_t * encoded_bytes,
    uint32_t *message_content_length,
    uint8_t bUDH
)
{
    int encoded_bytes_len;
    int iter_i = 0;

    encoded_bytes_len = encoded_bytes[iter_i ++];

    log_d("ril_sms_decode_8bit_message_content, encoded_bytes_len:%d\n", encoded_bytes_len);

//    memcpy(message_content, encoded_bytes + iter_i, encoded_bytes_len);
//    *message_content_length = encoded_bytes_len;

    message_content[0] = '\0';
    *message_content_length = 0;

    return encoded_bytes_len;
}

int ril_sms_decode_raw_message(ril_sms_message_storage_t storage_type, unsigned char *raw, ril_sms_pp_ind_msg *pp_msg)
{
    int index;
    int smsc_len;
    unsigned char status;
    unsigned char fo;
    unsigned char oa_len;
    unsigned char pid;
    unsigned char dcs;
    unsigned char mclass;
    unsigned char udl;
    int encoding;
    unsigned char bUDH = false;
    unsigned char vpf = 0;
    
    log_d("ril_sms_decode_raw_message storage_type: %d\n", storage_type);
    
    if((raw == NULL) || (pp_msg == NULL))
    {
        log_d("Null raw or pp_msg pointer\n");        
        return 0;
    }
    
    index = 0;

    if(storage_type == RIL_SMS_STORAGE_UIM)
    {
    	status = raw[index] & 0x01;

    	log_d("record status: %d\n", raw[index]);
    
    	if(status == 0x00)
    	{
    		log_d("Empty record on SIM\n");
        	return 0;
    	}

    	status = raw[index] & 0x05;
    	if(status == 0x05){
        	pp_msg->message_status = RIL_SMS_MESSAGE_STATUS_SENT;
    	} else {
        	status = raw[index] & 0x02;
        	if(status == 0x00){
            	pp_msg->message_status = RIL_SMS_MESSAGE_STATUS_READ;
        	} else {
            	pp_msg->message_status = RIL_SMS_MESSAGE_STATUS_UNREAD;
        	}
    	}
	    log_d("pp_msg->message_status: %d\n", pp_msg->message_status);
    
	    index ++;

    	smsc_len = raw[index ++];

    	//todo: decode smsc
    	index += smsc_len;
    }
    else{
        pp_msg->message_status = RIL_SMS_MESSAGE_STATUS_UNREAD;
    }

    fo = raw[index ++];

    log_d("MO SMS, skip TP-MR. fo:0x%.2x\n", fo);
     
    if(pp_msg->message_status == RIL_SMS_MESSAGE_STATUS_SENT)
    {
        log_d("MO SMS, skip TP-MR. MR:%d\n",raw[index]);
        index ++;
    }
        
    oa_len = raw[index];

    index += ril_sms_decode_address_number(pp_msg->source_address, &raw[index]);

    pid = raw[index ++];
    dcs = raw[index ++];

    log_d("pid: %x, dcs: %x\n", pid, dcs);
    
    if((dcs & 0x20) == 0x00) {
        pp_msg->message_class = RIL_SMS_MESSAGE_CLASS_NONE;
    } else {
        mclass = dcs & 0x03;

        switch (mclass)
        {
            case 0x00:
                pp_msg->message_class = RIL_SMS_MESSAGE_CLASS_0;
                break;
            case 0x01:
                pp_msg->message_class = RIL_SMS_MESSAGE_CLASS_1;
                break;
            case 0x02:
                pp_msg->message_class = RIL_SMS_MESSAGE_CLASS_2;
                break;
            case 0x03:
                pp_msg->message_class = RIL_SMS_MESSAGE_CLASS_3;
                break;
        }
        
    }
    
    if(pp_msg->message_status == RIL_SMS_MESSAGE_STATUS_SENT)
    {
        vpf = (fo & 0x18);
        log_d("vpf:0x%.2x\n", vpf);

        if(vpf == 0x00)
        {
            log_d("Skip timestamp field.\n");
        }else if(vpf == 0x10) {
            log_d("relative vpf, skip\n");
            index ++;
        }else{
            ril_sms_decode_timestamp(pp_msg->timestamp, &raw[index]);
            index += 7;            
        }
    }else {
        ril_sms_decode_timestamp(pp_msg->timestamp, &raw[index]);
        index += 7;
    }

    switch ((dcs >> 2) & 0x3) 
    {
        case 0: // GSM 7 bit default alphabet
            encoding = RIL_SMS_ENCODING_7BIT;
            break;

        case 2: // UCS 2 (16bit)
            encoding = RIL_SMS_ENCODING_16BIT;
            break;

        case 1: // 8 bit data
            encoding = RIL_SMS_ENCODING_8BIT;
            break;

        case 3: // reserved
            encoding = RIL_SMS_ENCODING_8BIT;
            break;
    }
    pp_msg->encoding_type = encoding;

    log_d("encoding type: %d\n", pp_msg->encoding_type);

    if((fo & 0x40) !=0)
    {
        log_d("UDH exist\n");
        bUDH = true;
    }

    switch(encoding)
    {
        case RIL_SMS_ENCODING_7BIT:
    index += ril_sms_decode_7bit_message_content(pp_msg->message_content, &raw[index], &(pp_msg->message_content_length), bUDH);
            log_d("7bit_message message_content: %s, message_content_length: %d\n", pp_msg->message_content, pp_msg->message_content_length);
        break;
        case RIL_SMS_ENCODING_16BIT:
            index += ril_sms_decode_ucs2_message_content(pp_msg->message_content, &raw[index], &(pp_msg->message_content_length), bUDH);
            log_d("ucs2_message message_content: %s, message_content_length: %d\n", pp_msg->message_content, pp_msg->message_content_length);
        break;
        default:
            index += ril_sms_decode_8bit_message_content(pp_msg->message_content, &raw[index], &(pp_msg->message_content_length), bUDH);
            print_sms_raw_uint8(pp_msg->message_content, pp_msg->message_content_length);
        break;
    }
    return index;
}

qmi_client_recv_msg_async_cb ril_sms_qmi_async_cb(qmi_client_type user_handle, unsigned int msg_id, void *resp_c_struct, unsigned int resp_c_struct_len, void *resp_cb_data, qmi_client_error_type transp_err)
{
    wms_delete_resp_msg_v01 *resp_msg;
    wms_async_send_ack_resp_msg_v01 *ack_resp;
    
    log_d("ril_sms_qmi_async_cb msg_id: %d\n", msg_id);
    print_sms_raw_uint8(resp_c_struct, resp_c_struct_len);

    switch(msg_id)
    {
        case QMI_WMS_DELETE_REQ_V01:
            resp_msg = (wms_delete_resp_msg_v01 *)resp_c_struct;

            log_d("ril_sms_qmi_async_cb error: %d, result: %d\n", resp_msg->resp.error, resp_msg->resp.result);

            if(sms_delete_ptr != NULL)
            {
                if(resp_msg->resp.result == QMI_NO_ERR)
                    sms_delete_ptr(RIL_RESULT_SUCCESS);
                else
                    sms_delete_ptr(RIL_RESULT_FAILURE);
            }    
            break;
        case QMI_WMS_ASYNC_SEND_ACK_RESP_V01:
            ack_resp = (wms_async_send_ack_resp_msg_v01 *)resp_c_struct;
            log_d("ril_sms_qmi_async_cb error: %d, result: %d\n", ack_resp->resp.error, ack_resp->resp.result);
            break;
    }

    free(resp_c_struct);
}

bool rilSmsDeleteMessageFromSim(int rec_num, ril_delete_sms_resp *resp_cb)
{
    wms_delete_req_msg_v01 req_msg;
    wms_delete_resp_msg_v01 *resp_msg;
    qmi_client_error_type qmi_error = QMI_NO_ERR;
        
    int cbdata;
    qmi_txn_handle txn_handle;
    bool ret = false;

    log_d("rilSmsDeleteMessageFromSim() rec_num: %d", rec_num);

    if(rec_num > g_sim_capacity) {
        log_d("invalid rec_num, g_sim_capacity: %d", g_sim_capacity);
        return false;
    }
    
    resp_msg = (wms_delete_resp_msg_v01*) malloc(sizeof(wms_delete_resp_msg_v01));
                
    memset(&req_msg, 0x00, sizeof(wms_delete_req_msg_v01));
    memset(resp_msg, 0x00, sizeof(wms_delete_resp_msg_v01));

    req_msg.storage_type = WMS_STORAGE_TYPE_UIM_V01;

    req_msg.index_valid = 0x01;
    req_msg.index = rec_num - 1;
    req_msg.message_mode_valid = 0x01;
    req_msg.message_mode = WMS_MESSAGE_MODE_GW_V01;


    qmi_error = qmi_client_send_msg_async (sms_user_handle,
                                            QMI_WMS_DELETE_REQ_V01,
                                   			&req_msg,
                                            sizeof(wms_delete_req_msg_v01),
                                   			resp_msg,
                                            sizeof(wms_delete_resp_msg_v01),
                                            ril_sms_qmi_async_cb,
                                            (void*)&cbdata,
                                            &txn_handle);
    
     log_d("rilSmsDeleteMessageFromSim() qmi_error: %d\n", qmi_error);

    if(qmi_error == QMI_NO_ERR) {
        sms_delete_ptr = resp_cb;
        ret = true;
    }

    return ret;

}

void readSMSRecordFromSIM(int recnum, uim_read_record_resp_msg_v01 *out_buf)
{
    uim_read_record_req_msg_v01 uim_read_req;
    sim_type_enum sim_type;
    qmi_client_error_type qmi_error = QMI_NO_ERR;

    log_d("[readSMSRecordFromSIM] recnum:%d\n", recnum);
    
    memset(&uim_read_req, 0, sizeof(uim_read_record_req_msg_v01));
    memset(out_buf, 0, sizeof(uim_read_record_resp_msg_v01));

    sim_type = get_sim_type();

    uim_read_req.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    
    uim_read_req.file_id.file_id = 0x6F3C;
    
    uim_read_req.file_id.path[0] = 0x00;
    uim_read_req.file_id.path[1] = 0x3F;

    if (sim_type == CARD_TYPE_ICC) {
        uim_read_req.file_id.path[2] = 0x10;
        uim_read_req.file_id.path[3] = 0x7F;
    }
    else {
        uim_read_req.file_id.path[2] = 0xFF;
        uim_read_req.file_id.path[3] = 0x7F;
    }
    
    uim_read_req.file_id.path_len = 4;

    uim_read_req.read_record.record = recnum;
    uim_read_req.read_record.length = 0;   
    
    qmi_error = qmi_client_send_msg_sync(uim_client_handle,
                                          QMI_UIM_READ_RECORD_REQ_V01,
                                          (void*)&uim_read_req,
                                          sizeof(uim_read_record_req_msg_v01),
                                          (void*) out_buf,
                                          sizeof(uim_read_record_resp_msg_v01),
                                          QMI_REQ_TIMEOUT);

    log_d("[readSMSFromSIM] result:%d, errorcode:%d", out_buf->resp.result, out_buf->resp.error);

    print_sms_raw_uint8(out_buf->read_result.content, out_buf->read_result.content_len);

}

void parseMTMessage(char *ind_buf)
{
    int i;
    int index;
    unsigned char storage;
    short len;
    ril_sms_pp_ind_msg pp_msg;
	uim_read_record_resp_msg_v01 uim_read_resp;
    
    if(ind_buf[0] != 0x10)
    {
        log_d("Unknown ind_buf token, return.");
        return;        
    }

    len = (ind_buf[1] | (ind_buf[2] << 8));
    storage = ind_buf[3];
    index = (ind_buf[4] | (ind_buf[5] << 8) | (ind_buf[6] <<16) | (ind_buf[7] << 24));            
            
    log_d("MT SMS parseMTMessage len: %d, storage: %d, index: %d", len, storage, index);

    readSMSRecordFromSIM(index + 1, &uim_read_resp);
 
    memset(&pp_msg, 0x00, sizeof(ril_sms_pp_ind_msg));
            
    if(ril_sms_decode_raw_message(RIL_SMS_STORAGE_UIM, uim_read_resp.read_result.content, &pp_msg) != 0)
    {
        pp_msg.record_num = index + 1;
        pp_msg.message_storage = RIL_SMS_STORAGE_UIM;
                                    
        sms_ind_ptr(&pp_msg);
    }
}

static void ril_sms_send_ack_async(int param)
{
    wms_async_send_ack_req_msg_v01 req;
    wms_async_send_ack_resp_msg_v01 *resp;
    qmi_client_error_type qmi_error = QMI_NO_ERR;
    int t_id;
    int cbdata;
    qmi_txn_handle txn_handle;

    t_id = param;

    log_d("ril_sms_send_ack_async: %d",  t_id);

    resp =(wms_async_send_ack_resp_msg_v01 *)malloc(sizeof(wms_async_send_ack_resp_msg_v01));

    memset(&req, 0x00, sizeof(wms_async_send_ack_req_msg_v01));
    memset(resp, 0x00, sizeof(wms_async_send_ack_resp_msg_v01));

    req.ack_information.success = 0x01;
    req.ack_information.transaction_id = t_id;
    req.ack_information.message_protocol = WMS_MESSAGE_PROTOCOL_WCDMA_V01;

    qmi_error = qmi_client_send_msg_async(sms_user_handle,
                                      QMI_WMS_ASYNC_SEND_ACK_REQ_V01,
                                      &req,
                                      sizeof(wms_async_send_ack_req_msg_v01),
                                      resp,
                                       sizeof(wms_async_send_ack_resp_msg_v01),
                                      ril_sms_qmi_async_cb,
                                      (void*)&cbdata,
                                      &txn_handle
                                      );


   log_d("ril_sms_send_ack_async qmi_error=%d", qmi_error);
}


void parseTransferRouteMTMessage(char *ind_buf)
{
    int i;
    int transaction_id;
    short len, data_len;
    unsigned char ack_ind;
    unsigned char format;
    ril_sms_pp_ind_msg pp_msg;
    unsigned char * raw_buff;

    if(ind_buf[0] != 0x11)
    {
        log_d("Unknown ind_buf token, return.");
        return;        
    }

    len = (ind_buf[1] | (ind_buf[2] << 8));
    ack_ind = ind_buf[3];
    transaction_id = (ind_buf[4] | (ind_buf[5] << 8) | (ind_buf[6] <<16) | (ind_buf[7] << 24));            
    format = ind_buf[8];
    data_len = (ind_buf[9] | (ind_buf[10] << 8));
            
    log_d("parseTransferRouteMessage len: %d, ack_ind: %d, transaction_id: %d, format: %d, data_len: %d", len, ack_ind, transaction_id, format, data_len);

    if(ack_ind == WMS_ACK_INDICATOR_SEND_ACK_V01)
        ril_sms_send_ack_async(transaction_id);

    raw_buff = (unsigned char *) malloc(data_len);
    
    memset(&pp_msg, 0x00, sizeof(ril_sms_pp_ind_msg));
    memset(raw_buff, 0x00, data_len);

    memcpy(raw_buff, &ind_buf[11], data_len);
    print_sms_raw_uint8(raw_buff, data_len);
           
    if(ril_sms_decode_raw_message(RIL_SMS_STORAGE_NONE, raw_buff, &pp_msg) != 0)
    {
        pp_msg.message_storage = RIL_SMS_STORAGE_NONE;
                                    
        sms_ind_ptr(&pp_msg);
    }

}

void sms_unsolicited_wms_ind_handler ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
)
{
    int status;

    log_d("sms_unsolicited_wms_ind_handler, msg %d", msg_id);
    print_sms_raw_uint8(ind_buf, ind_buf_len);
    
    switch(msg_id)
    {
        case QMI_WMS_EVENT_REPORT_IND_V01:
            
        	if(sms_ind_ptr != NULL)
        	{
                if(ind_buf[0] == 0x10) {
                    log_d("sms_unsolicited_wms_ind_handler, MT message type.\n");
                    parseMTMessage(ind_buf);
                }
                else if(ind_buf[0] == 0x11) {
                    log_d("sms_unsolicited_wms_ind_handler, transfer route MT message type.\n");
                    parseTransferRouteMTMessage(ind_buf);
                }
        	} else {
            	log_d("sms_unsolicited_wms_ind_handler, no ind function pointer, skip.\n");
        	}     
            break;
        case QMI_WMS_MEMORY_FULL_IND_V01:
        
        	if(sms_full_ptr != NULL)
        	{
            	sms_full_ptr(RIL_SMS_STORAGE_UIM);
        	} else {
            	log_d("sms_unsolicited_wms_ind_handler, no ind function pointer, skip.\n");
        	}
            break;
        case QMI_WMS_ASYNC_SEND_ACK_IND_V01:
            if(ind_buf[0] == 0x01) {
                /* ACK status. Values:
                    0x00 - QMI_ERR_NONE ¡V No error in the request
                    0x01 - QMI_ERR_MALFORMED_MSG ¡VMessage was not formulated correctly by the control point or the message was corrupted during transmission
                    0x02 - QMI_ERR_NO_MEMORY ¡V Device could not allocate memory to formulate a response
                    0x54 - QMI_ERR_ACK_NOT_SENT ¡V ACK could not be sent
                */
                log_d("sms_unsolicited_wms_ind_handler, ACK Status: %d\n", ind_buf[3] );
            }
            else if(ind_buf[0] == 0x10) {
                /* ACK failure cause. Values:
                     0x00 - WMS_ACK_FAILURE_NO_NETWORK_RESPONSE_V01
                     0x01 - WMS_ACK_FAILURE_NETWORK_RELEASED_LINK_V01
                     0x02 - WMS_ACK_FAILURE_ACK_NOT_SENT_V01
                */
                log_d("sms_unsolicited_wms_ind_handler, ACK Failure Cause: %d\n", ind_buf[3] );
            }
            break;
    }
}

void init_sms_listener()
{
    qmi_client_error_type qmi_error = QMI_NO_ERR;
    wms_set_event_report_req_msg_v01 req;
    wms_set_event_report_resp_msg_v01 resp;
    int status;

    memset(&resp, 0, sizeof(resp));
    memset(&req, 0, sizeof(req));  
        
    req.report_mt_message = 0x01;
    req.report_mt_message_valid = 0x01;         
        
    qmi_error = qmi_client_send_msg_sync (sms_user_handle,
                                QMI_WMS_SET_EVENT_REPORT_REQ_V01,
                                (void *)&req,
                                sizeof(req),
                                (void*)&resp,
                                sizeof(resp),
                                QMI_REQ_TIMEOUT);

    log_d("QMI_WMS_SET_EVENT_REPORT_REQ_V01 qmi_error = %d", qmi_error);
        
    if (qmi_error != QMI_NO_ERR || resp.resp.result != QMI_RESULT_SUCCESS_V01)
    {
        log_d("%s(): failed with rc=%d, qmi_err=%d", __func__, qmi_error, resp.resp.error);
    }                              
}
void rilSmsInit(ril_sms_init_data *init_data)
{
    qmi_client_error_type qmi_error = QMI_NO_ERR;

    log_d("rilSmsInit()");

    waitforread = false;

    sms_ind_ptr = NULL;

    sms_service_obj = wms_get_service_object_v01();
    
    qmi_error = qmi_client_init_instance(sms_service_obj,
                                       QMI_CLIENT_INSTANCE_ANY,
                                       sms_unsolicited_wms_ind_handler,
                                       NULL,
                                       &sms_os_params,
                                       30000,
                                       &sms_user_handle );

     log_d("sms_service_obj qmi_error = %d\n", qmi_error);

      init_sms_listener();

    if(init_data != NULL)
    {
        rilSmsIndRegister(init_data);
    }
    
    rilSmsRefershSIMSMS();

}

void rilSmsIndRegister(ril_sms_init_data *ind_ptr)
{
    log_d("rilSmsIndRegister() \n");
    
    if(ind_ptr == NULL)
    {
        log_d("NULL pointer received\n");
    }

    sms_ind_ptr = ind_ptr->sms_ind_cb;
    sms_full_ptr = ind_ptr->sms_full_ind;
    
}

static void rilSmsRefershSIMSMS_Thread(void * param)
{
    qmi_client_error_type qmi_error = QMI_NO_ERR;
    wms_list_messages_req_msg_v01 wms_list_msgs_req;
    wms_list_messages_resp_msg_v01 wms_list_msgs_resp;
    uim_read_record_resp_msg_v01 uim_read_resp;
    ril_sms_pp_ind_msg pp_msg;
    int i;

    log_d("rilSmsRefershSIMSMS_Thread() sleep\n");
    sleep(1);
    log_d("rilSmsRefershSIMSMS_Thread()\n");

    memset(&wms_list_msgs_req, 0, sizeof(wms_list_messages_req_msg_v01));
    memset(&wms_list_msgs_resp, 0, sizeof(wms_list_messages_resp_msg_v01));
            

    wms_list_msgs_req.storage_type = WMS_STORAGE_TYPE_UIM_V01;
    wms_list_msgs_req.message_mode_valid = 0x01;
    wms_list_msgs_req.message_mode = WMS_MESSAGE_MODE_GW_V01;
    
    qmi_error = qmi_client_send_msg_sync (sms_user_handle,
                                QMI_WMS_LIST_MESSAGES_REQ_V01,
                                (void *)&wms_list_msgs_req,
                                sizeof(wms_list_msgs_req),
                                (void*)&wms_list_msgs_resp,
                                sizeof(wms_list_msgs_resp),
                                QMI_REQ_TIMEOUT);

    log_d("rilSmsRefershSIMSMS_Thread() QMI_WMS_LIST_MESSAGES_REQ_V01 qmi_error: %d, result: %d, error: %d\n", 
        qmi_error, wms_list_msgs_resp.resp.result, wms_list_msgs_resp.resp.error);

    log_d("rilSmsRefershSIMSMS_Thread() tuple_len: %d\n", wms_list_msgs_resp.message_tuple_len);
    g_sim_capacity = wms_list_msgs_resp.message_tuple_len;

    for(i = 0; i < wms_list_msgs_resp.message_tuple_len; i ++)
    {
        if(i >= MAX_MESSAGE_LIST)
            break;
            
        readSMSRecordFromSIM(wms_list_msgs_resp.message_tuple[i].message_index + 1, &uim_read_resp);
        
        memset(&pp_msg, 0x00, sizeof(ril_sms_pp_ind_msg));
            
        if(ril_sms_decode_raw_message(RIL_SMS_STORAGE_UIM, uim_read_resp.read_result.content, &pp_msg) != 0)
        {
            pp_msg.record_num = wms_list_msgs_resp.message_tuple[i].message_index + 1;
            pp_msg.message_storage = RIL_SMS_STORAGE_UIM;
                                    
            sms_ind_ptr(&pp_msg);
        }
    }

    waitforread = false;

    pthread_exit(NULL); 
}

void rilSmsRefershSIMSMS() {
    pthread_attr_t attr;
    int ret;
    log_d("rilSmsRefershSIMSMS - %d\n", waitforread);

    if(waitforread == true)
    {
        return;
    }
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&mm_tid, &attr, rilSmsRefershSIMSMS_Thread, NULL);    

    log_d("rilSmsRefershSIMSMS ret: %d\n", ret);

    if(ret == 0)
        waitforread = true;
}

void rilSmsGetCapacity(ril_sms_get_capacity *capacity)
{
    qmi_client_error_type qmi_error = QMI_NO_ERR;
    wms_get_store_max_size_req_msg_v01 get_store_size_req_msg;
    wms_get_store_max_size_resp_msg_v01 get_store_size_resp_msg;
    int slots = 0;

    if(capacity == NULL)
    {
        log_d("[rilSmsGetCapacity]: invalid input data."); 
        return;
    }
    memset(&get_store_size_req_msg, 0x00, sizeof(wms_get_store_max_size_req_msg_v01));
    memset(&get_store_size_resp_msg, 0x00, sizeof(wms_get_store_max_size_resp_msg_v01));
    
    get_store_size_req_msg.storage_type = WMS_STORAGE_TYPE_UIM_V01;
    get_store_size_req_msg.message_mode_valid = 0x01;
    get_store_size_req_msg.message_mode = WMS_MESSAGE_MODE_GW_V01;
    
    qmi_error = qmi_client_send_msg_sync(sms_user_handle,
                                       QMI_WMS_GET_STORE_MAX_SIZE_REQ_V01,
                                       &get_store_size_req_msg,
                                       sizeof(wms_get_store_max_size_req_msg_v01),
                                       &get_store_size_resp_msg,
                                       sizeof(wms_get_store_max_size_resp_msg_v01),
                                       5000);

    log_d("get store max size qmi_error: %d, error: %d, result: %d", qmi_error, get_store_size_resp_msg.resp.error, get_store_size_resp_msg.resp.result); 
    log_d("get store max size free_slots_valid: %d, free_solts: %d, max size: %d", get_store_size_resp_msg.free_slots_valid, get_store_size_resp_msg.free_slots, get_store_size_resp_msg.mem_store_max_size); 

    if(get_store_size_resp_msg.free_slots_valid == 0x01)
    {
        capacity->sms_free_slot = get_store_size_resp_msg.free_slots;
        capacity->sms_capacity = get_store_size_resp_msg.mem_store_max_size;
    }
    
}

void rilSmsSetReadStatus(int rec_num, ril_sms_message_status_t status)
{
    uim_write_record_req_msg_v01 uim_write_req;
    uim_write_record_resp_msg_v01 uim_write_resp;
    sim_type_enum sim_type;
    qmi_client_error_type qmi_error = QMI_NO_ERR;
    uim_read_record_resp_msg_v01 uim_read_resp;

    
    log_d("[rilSmsSetReadStatus] rec_num: %d, status: %d", rec_num, status); 
    
    if ((status != RIL_SMS_MESSAGE_STATUS_READ) && (status != RIL_SMS_MESSAGE_STATUS_UNREAD))
    {
        log_d("[rilSmsSetReadStatus] Invalid status"); 
        return;
    }

    memset(&uim_read_resp, 0, sizeof(uim_read_record_resp_msg_v01));
    
    readSMSRecordFromSIM(rec_num, &uim_read_resp);

    memset(&uim_write_req, 0, sizeof(uim_write_record_req_msg_v01));
    memset(&uim_write_resp, 0, sizeof(uim_write_record_resp_msg_v01));
    
    sim_type = get_sim_type();

    uim_write_req.session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
    
    uim_write_req.file_id.file_id = 0x6F3C;
    
    uim_write_req.file_id.path[0] = 0x00;
    uim_write_req.file_id.path[1] = 0x3F;

    if (sim_type == CARD_TYPE_ICC) {
        uim_write_req.file_id.path[2] = 0x10;
        uim_write_req.file_id.path[3] = 0x7F;
    }
    else {
        uim_write_req.file_id.path[2] = 0xFF;
        uim_write_req.file_id.path[3] = 0x7F;
    }

    uim_write_req.file_id.path_len = 4;

    uim_write_req.write_record.record = rec_num;
    uim_write_req.write_record.data_len = uim_read_resp.read_result.content_len;  
    memcpy(uim_write_req.write_record.data, uim_read_resp.read_result.content, uim_read_resp.read_result.content_len);
    
    if(status == RIL_SMS_MESSAGE_STATUS_READ )
        uim_write_req.write_record.data[0] = 0x01;
    
    if(status == RIL_SMS_MESSAGE_STATUS_UNREAD )
        uim_write_req.write_record.data[0] = 0x03;

    print_sms_raw_uint8(uim_write_req.write_record.data, uim_write_req.write_record.data_len);
    
    qmi_error = qmi_client_send_msg_sync(uim_client_handle,
                                          QMI_UIM_WRITE_RECORD_REQ_V01,
                                          (void*)&uim_write_req,
                                          sizeof(uim_write_record_req_msg_v01),
                                          (void*) &uim_write_resp,
                                          sizeof(uim_write_record_resp_msg_v01),
                                          QMI_REQ_TIMEOUT);
    log_d("[rilSmsSetReadStatus] qmi_error: %d, error: %d, result: %d", qmi_error, uim_write_resp.resp.error, uim_write_resp.resp.result); 
    
}

#endif  /* defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD) */
