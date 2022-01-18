/*
 * profile_management_data_store.c
 *
 *  Created on: May 2, 2019
 *      Author: joseph
 */
#include <libxml/tree.h>
#include <libxml/xmlsave.h>
#include <libxml/xmlwriter.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include "lv_pocket_router/src/util/data_store.h"
#include "lv_pocket_router/src/settings/profile_management_data_store.h"
#include "lv_pocket_router/src/util/debug_log.h"

#define ENCODING_TYPE                   "UTF-8"

char profile_apn[PROFILE_APN_MAX_LENGTH + 1];
char profile_user_name[PROFILE_USER_NAME_MAX_LENGTH + 1];
char profile_password[PROFILE_PW_MAX_LENGTH + 1];
char profile_pdptype[PROFILE_PDP_TYPE_MAX_LENGTH];

void write_new_profile(PROFILE_DATA profile) {
    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;
    xmlDocPtr doc = xmlReadFile(DEFAULT_DATA_STORE_FILE, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        printf("Failed to read %s", DEFAULT_DATA_STORE_FILE);
        return ;
    }
    xmlNodePtr cur = xmlDocGetRootElement(doc);
    xmlNodePtr curNode = cur->xmlChildrenNode;
    if(curNode != NULL) {
        xmlNodePtr profle_node = xmlNewNode(NULL, BAD_CAST (const xmlChar *)PROFILE_HEADER);
        xmlNewProp(profle_node, BAD_CAST PROFILE_NAME, BAD_CAST profile.profile_name);
        xmlNewProp(profle_node, BAD_CAST PROFILE_APN, BAD_CAST profile.apn);
        xmlNewProp(profle_node, BAD_CAST PROFILE_USER_NAME, BAD_CAST profile.user_name);
        xmlNewProp(profle_node, BAD_CAST PROFILE_PASSWORD, BAD_CAST profile.password);
        xmlNewProp(profle_node, BAD_CAST PROFILE_PDPTYPE, BAD_CAST profile.pdp_type);
        xmlNewProp(profle_node, BAD_CAST PROFILE_MCC, BAD_CAST profile.sim_mcc);
        xmlNewProp(profle_node, BAD_CAST PROFILE_MNC, BAD_CAST profile.sim_mnc);
        xmlAddChild(cur, profle_node);
        xmlSaveFormatFileEnc(DEFAULT_DATA_STORE_FILE, doc, ENCODING_TYPE,XML_SAVE_FORMAT);
    }
    xmlFreeDoc(doc);
}

//if char* search_profile_name == TWM
//ex:<apn profile_name="TWM" apn="internet" user_name="test" password="aaaaaa"/>
//return internet
char* get_profile_apn(char* search_profile_name, char* mcc, char* mnc) {
    xmlDocPtr doc = xmlReadFile(DEFAULT_DATA_STORE_FILE, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        printf("Failed to read %s", DEFAULT_DATA_STORE_FILE);
    }
    xmlNodePtr  curNode;
    curNode = xmlDocGetRootElement(doc);
    curNode = curNode->xmlChildrenNode;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST (const xmlChar *)PROFILE_HEADER)) {
            if (strcmp(((char *)xmlGetProp(curNode,PROFILE_MCC)), mcc) == 0 &&
                strcmp(((char *)xmlGetProp(curNode,PROFILE_MNC)), mnc) == 0 &&
                    strcmp(((char *)xmlGetProp(curNode, PROFILE_NAME)), search_profile_name) == 0) {
                        memset(profile_apn, '\0', sizeof(profile_apn));
                        strcpy(profile_apn, (char *)(xmlGetProp(curNode, PROFILE_APN)));
                        break;
            }
        }
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);
    return profile_apn;
}

//if char* search_profile_name == TWM
//ex:<apn profile_name="TWM" apn="internet" user_name="test" password="aaaaaa"/>
//return test
char* get_profile_user_name(char* search_profile_name, char* mcc, char* mnc) {
    xmlDocPtr doc = xmlReadFile(DEFAULT_DATA_STORE_FILE, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        printf("Failed to read %s", DEFAULT_DATA_STORE_FILE);
    }
    xmlNodePtr  curNode;
    curNode = xmlDocGetRootElement(doc);
    curNode = curNode->xmlChildrenNode;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST (const xmlChar *)PROFILE_HEADER)) {
            if (strcmp(((char *)xmlGetProp(curNode,PROFILE_MCC)), mcc) == 0 &&
                strcmp(((char *)xmlGetProp(curNode,PROFILE_MNC)), mnc) == 0 &&
                    strcmp(((char *)xmlGetProp(curNode, PROFILE_NAME)), search_profile_name) == 0) {
                        memset(profile_user_name, '\0', sizeof(profile_user_name));
                        strcpy(profile_user_name, (char *)(xmlGetProp(curNode, PROFILE_USER_NAME)));
                        break;
            }
        }
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);
    return profile_user_name;
}

//if char* search_profile_name == TWM
//ex:<apn profile_name="TWM" apn="internet" user_name="test" password="aaaaaa"/>
//return aaaaaa
char* get_profile_password(char* search_profile_name, char* mcc, char* mnc) {
    xmlDocPtr doc = xmlReadFile(DEFAULT_DATA_STORE_FILE, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        printf("Failed to read %s", DEFAULT_DATA_STORE_FILE);
    }
    xmlNodePtr  curNode;
    curNode = xmlDocGetRootElement(doc);
    curNode = curNode->xmlChildrenNode;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST (const xmlChar *)PROFILE_HEADER)) {
            if (strcmp(((char *)xmlGetProp(curNode,PROFILE_MCC)), mcc) == 0 &&
                strcmp(((char *)xmlGetProp(curNode,PROFILE_MNC)), mnc) == 0 &&
                    strcmp(((char *)xmlGetProp(curNode, PROFILE_NAME)), search_profile_name) == 0) {
                        memset(profile_password, '\0', sizeof(profile_password));
                        strcpy(profile_password, (char *)(xmlGetProp(curNode, PROFILE_PASSWORD)));
                        break;
            }
        }
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);
    return profile_password;
}

char* get_profile_pdptype(char* search_profile_name, char* mcc, char* mnc) {
    xmlDocPtr doc = xmlReadFile(DEFAULT_DATA_STORE_FILE, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        printf("Failed to read %s", DEFAULT_DATA_STORE_FILE);
    }
    xmlNodePtr  curNode;
    curNode = xmlDocGetRootElement(doc);
    curNode = curNode->xmlChildrenNode;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST (const xmlChar *)PROFILE_HEADER)) {
            if (strcmp(((char *)xmlGetProp(curNode,PROFILE_MCC)), mcc) == 0 &&
                strcmp(((char *)xmlGetProp(curNode,PROFILE_MNC)), mnc) == 0 &&
                    strcmp(((char *)xmlGetProp(curNode, PROFILE_NAME)), search_profile_name) == 0) {
                        memset(profile_pdptype, '\0', sizeof(profile_pdptype));
                        strcpy(profile_pdptype, (char *)(xmlGetProp(curNode, PROFILE_PDPTYPE)));
                        break;
            }
        }
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);
    return profile_pdptype;
}

int search_keyword_in_xml_header_num(char* search) {
    xmlDocPtr   doc;
    xmlNodePtr  curNode;
    int search_data_num = 0;
    if (access( DEFAULT_DATA_STORE_FILE, F_OK) != -1) {
        //data_storage.xml exist
        doc = xmlReadFile(DEFAULT_DATA_STORE_FILE, NULL, 0);
        if (doc != NULL) {
            curNode = xmlDocGetRootElement(doc);
            curNode = curNode->xmlChildrenNode;
            while (curNode != NULL) {
                if (curNode->type == XML_ELEMENT_NODE) {
                    if (!strcmp(curNode->name, search)) {
                        search_data_num++;
                    }
                }
                curNode = curNode->next;
            }
            xmlFreeDoc(doc);
        }
    } else {
        //data_storage.xml does not exist
        search_data_num = -1;
    }
    return search_data_num;
}

void delete_profile_name_node(char * delete_profile_name, char* mcc, char* mnc) {//ex:TWM or FEB
    xmlDocPtr   doc;
    xmlNodePtr  curNode;
    doc = xmlReadFile(DEFAULT_DATA_STORE_FILE, NULL, 0);
    bool found = false;
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse %s\n", DEFAULT_DATA_STORE_FILE);
    }
    curNode = xmlDocGetRootElement(doc);
    curNode = curNode->xmlChildrenNode;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST (const xmlChar *)PROFILE_HEADER)) {
            if (strcmp(((char *)xmlGetProp(curNode,PROFILE_MCC)), mcc) == 0 &&
                strcmp(((char *)xmlGetProp(curNode,PROFILE_MNC)), mnc) == 0 &&
                    strcmp(((char *)xmlGetProp(curNode, PROFILE_NAME)), delete_profile_name) == 0) {
                    //if find the delete profile node
                    xmlUnlinkNode(curNode);
                    xmlFreeNode(curNode);
                    found = true;
                    break;
            }
        }
        curNode = curNode->next;
    }
    if (found) {
        xmlSaveFormatFileEnc(DEFAULT_DATA_STORE_FILE, doc, ENCODING_TYPE, XML_SAVE_FORMAT);
    }
    xmlFreeDoc(doc);
}

//update_profile_name ==> orig profile_name ex:TWM or FEB
void update_profile_name_node(char * update_profile_name,PROFILE_DATA profile, char* mcc, char* mnc) {
    xmlDocPtr   doc;
    xmlNodePtr  curNode;
    doc = xmlReadFile(DEFAULT_DATA_STORE_FILE, NULL, 0);
    bool found = false;
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse %s\n", DEFAULT_DATA_STORE_FILE);
    }
    curNode = xmlDocGetRootElement(doc);
    curNode = curNode->xmlChildrenNode;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST (const xmlChar *)PROFILE_HEADER)) {
            if (strcmp(((char *)xmlGetProp(curNode,PROFILE_MCC)), mcc) == 0 &&
                strcmp(((char *)xmlGetProp(curNode,PROFILE_MNC)), mnc) == 0 &&
                    strcmp(((char *)xmlGetProp(curNode, PROFILE_NAME)), update_profile_name) == 0) {
                //if find the update_profile_name node
                xmlSetProp(curNode, BAD_CAST (const xmlChar *)PROFILE_NAME, BAD_CAST profile.profile_name);
                xmlSetProp(curNode, BAD_CAST (const xmlChar *)PROFILE_APN, BAD_CAST profile.apn);
                xmlSetProp(curNode, BAD_CAST (const xmlChar *)PROFILE_USER_NAME, BAD_CAST profile.user_name);
                xmlSetProp(curNode, BAD_CAST (const xmlChar *)PROFILE_PASSWORD, BAD_CAST profile.password);
                xmlSetProp(curNode, BAD_CAST (const xmlChar *)PROFILE_PDPTYPE, BAD_CAST profile.pdp_type);
                xmlSetProp(curNode, BAD_CAST (const xmlChar *)PROFILE_MCC, BAD_CAST profile.sim_mcc);
                xmlSetProp(curNode, BAD_CAST (const xmlChar *)PROFILE_MNC, BAD_CAST profile.sim_mnc);
                found = true;
                break;
            }
        }
        curNode = curNode->next;
    }
    if (found) {
        xmlSaveFormatFileEnc(DEFAULT_DATA_STORE_FILE, doc, ENCODING_TYPE, XML_SAVE_FORMAT);
    }
    xmlFreeDoc(doc);
}

//check whether the "profile_name" in the <apn profile_name="xxx".../> data_storage.xml
//ex:<apn profile_name="Default" apn="internet" user_name="" password="" pdp_type="IPV4V6"/>
bool search_profile_name(char* profile_name) {
    bool bNodeFound = false;
    xmlDocPtr doc = xmlReadFile(DEFAULT_DATA_STORE_FILE, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        printf("Failed to read %s", DEFAULT_DATA_STORE_FILE);
    }
    xmlNodePtr  curNode;
    curNode = xmlDocGetRootElement(doc);
    curNode = curNode->xmlChildrenNode;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST (const xmlChar *)PROFILE_HEADER)) {
            if (strcmp(((char *)xmlGetProp(curNode, PROFILE_NAME)), profile_name) == 0) {
                bNodeFound = true;
                break;
            }
        }
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);
    return bNodeFound;
}

int search_apn_xml_num_by_mcc_mnc(char* mcc, char* mnc, char* path) {
    xmlDocPtr   doc;
    xmlNodePtr  curNode;
    char *keyword;
    doc = xmlReadFile(path, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse %s\n", path);
        return DS_ERROR;
    }
    curNode = xmlDocGetRootElement(doc);
    curNode = curNode->xmlChildrenNode;
    int search_apn_num = 0;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST (const xmlChar *)PROFILE_HEADER)) {
            if (strcmp(((char *)xmlGetProp(curNode,PROFILE_MCC)), mcc) == 0 &&
                strcmp(((char *)xmlGetProp(curNode,PROFILE_MNC)), mnc) == 0) {
                    search_apn_num++;
            }
        }
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);
    return search_apn_num;
}

bool search_apn_by_mcc_mnc_profilename(char* profile_name,char* mcc, char* mnc) {
    bool bNodeFound = false;
    xmlDocPtr doc = xmlReadFile(DEFAULT_DATA_STORE_FILE, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        log_e("Failed to read %s", DEFAULT_DATA_STORE_FILE);
        return bNodeFound;
    }
    xmlNodePtr  curNode;
    curNode = xmlDocGetRootElement(doc);
    curNode = curNode->xmlChildrenNode;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST (const xmlChar *)PROFILE_HEADER)) {
            if (strcmp(((char *)xmlGetProp(curNode, PROFILE_NAME)), profile_name) == 0 &&
                    strcmp(((char *)xmlGetProp(curNode,PROFILE_MCC)), mcc) == 0 &&
                        strcmp(((char *)xmlGetProp(curNode,PROFILE_MNC)), mnc) == 0) {
                bNodeFound = true;
                break;
            }
        }
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);
    return bNodeFound;
}
