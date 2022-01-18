/*
 * profile_management_data_store.h
 *
 *  Created on: May 2, 2019
 *      Author: joseph
 */

#ifndef LV_POCKET_ROUTER_SRC_SETTINGS_PROFILE_MANAGEMENT_DATA_STORE_H_
#define LV_POCKET_ROUTER_SRC_SETTINGS_PROFILE_MANAGEMENT_DATA_STORE_H_

#define PROFILE_HEADER      "apn"
#define PROFILE_NAME        "profile_name"
#define PROFILE_APN         "apn"
#define PROFILE_USER_NAME   "user_name"
#define PROFILE_PASSWORD    "password"
#define PROFILE_PDPTYPE     "pdp_type"
#define PROFILE_MCC         "mcc"
#define PROFILE_MNC         "mnc"

#define PROFILE_NAME_MAX_LENGTH 34
#define PROFILE_APN_MAX_LENGTH 60
#define PROFILE_USER_NAME_MAX_LENGTH 127
#define PROFILE_PW_MAX_LENGTH 127
#define PROFILE_PDP_TYPE_MAX_LENGTH 7
#define PROFILE_MCC_MAX_LENGTH 4
#define PROFILE_MNC_MAX_LENGTH 4

typedef struct {
    char* profile_name;
    char* apn;
    char* user_name;
    char* password;
    char* pdp_type;
    char* sim_mcc;
    char* sim_mnc;
} PROFILE_DATA;

void write_new_profile(PROFILE_DATA profile);
char* get_profile_apn(char* search_profile_name, char* mcc, char* mnc);
char* get_profile_user_name(char* search_profile_name, char* mcc, char* mnc);
char* get_profile_password(char* search_profile_name, char* mcc, char* mnc);
char* get_profile_pdptype(char* search_profile_name, char* mcc, char* mnc);
int search_keyword_in_xml_header_num(char* search);
void delete_profile_name_node(char * delete_profile_name, char* mcc, char* mnc);
void update_profile_name_node(char * update_profile_name,PROFILE_DATA profile, char* mcc, char* mnc);
bool search_profile_name(char* profile_name);
int search_apn_xml_num_by_mcc_mnc(char* mcc, char* mnc, char* path);
bool search_apn_by_mcc_mnc_profilename(char* profile_name,char* mcc, char* mnc);
#endif /* LV_POCKET_ROUTER_SRC_SETTINGS_PROFILE_MANAGEMENT_DATA_STORE_H_ */
