#include "sms_store.h"
#include <libxml/tree.h>
#include <libxml/xmlsave.h>
#include <libxml/xmlwriter.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "lv_pocket_router/src/util/debug_log.h"
#include "lv_pocket_router/src/util/util.h"
#include "lv_pocket_router/src/ril/ril.h"
#include "lv_pocket_router/src/sms/sms.h"

#define SMS_STORE_VERSION        "1"

#define SMS_ACTION_SET_READ         0
#define SMS_ACTION_DELETE           1

#define SMS_ELEMENT                 "sms"
#define ENCODING_TYPE               "UTF-8"

//for sms xml recovery
static int sms_max_id = -1;

//to check if need launch reload popup
uint32_t write_sms_timestamp = 0;
int write_sms_cnt = 0;

void sms_action(int sms_id, int action) {
    xmlDocPtr doc = xmlReadFile(SMS_XML_PATH, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        log_e("Failed to read %s", SMS_XML_PATH);
        return ;
    }

    xmlDocPtr cur = xmlDocGetRootElement(doc);
    xmlNodePtr curNode = cur->xmlChildrenNode;
    bool found = false;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST SMS_STORE_VERSION_KEY)) {
            curNode = curNode->next;
            continue ;
        }
        if (xmlStrcmp(curNode->name, BAD_CAST SMS_HEADER)) {
            curNode = curNode->next;
            continue;
        }
        char* str_id = xmlGetProp(curNode, BAD_CAST SMS_ID);
        char* in_sim = xmlGetProp(curNode, BAD_CAST SMS_IN_SIM);
        char* rec_num = xmlGetProp(curNode, BAD_CAST SMS_REC_NUM);
        if (str_id != NULL && sms_id == atoi(str_id)) {
            found = true;
            if (action == SMS_ACTION_SET_READ) {
                char* is_read = xmlGetProp(curNode, BAD_CAST SMS_READ);
                if(is_read != NULL && !strcmp(is_read, "false")){
                    xmlSetProp(curNode, BAD_CAST SMS_READ, BAD_CAST "true");
                    if(in_sim != NULL && !strcmp(in_sim, "true")){
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
                        int sim_rec_num = atoi(rec_num);
                        rilSmsSetReadStatus(sim_rec_num, RIL_SMS_MESSAGE_STATUS_READ);
#endif
                    }
                }
                xmlFree(is_read);
            } else if (action == SMS_ACTION_DELETE) {
                xmlUnlinkNode(curNode);
                xmlFreeNode(curNode);
            }
            xmlFree(str_id);
            xmlFree(in_sim);
            xmlFree(rec_num);
            break;
        }
        xmlFree(str_id);
        xmlFree(in_sim);
        xmlFree(rec_num);
        curNode = curNode->next;
    }

    if (found) {
        xmlSaveFormatFileEnc(SMS_XML_PATH, doc, ENCODING_TYPE, XML_SAVE_FORMAT);
    }
    xmlFreeDoc(doc);

    //TODO will check if need to be done later on
    if (action == SMS_ACTION_SET_READ) {
        //update_unread_message();
    }
}

void delete_sms(int sms_id, int sim_id) {
    sms_action(sms_id, SMS_ACTION_DELETE);

//check if need to delete in sim card
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    log_d("delete_sms sim_id = %d", sim_id);
    if(sim_id > 0) {
        //TODO we set sim del cb func as NULL for now
        rilSmsDeleteMessageFromSim(sim_id, NULL);
    }
#endif
}

void set_sms_read(int sms_id) {
    sms_action(sms_id, SMS_ACTION_SET_READ);
}

int get_sms_unread() {
    int unread = 0;

    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;
    xmlDocPtr doc = xmlReadFile(SMS_XML_PATH, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        log_e("Failed to read %s", SMS_XML_PATH);
        return unread;
    }

    xmlNodePtr cur = xmlDocGetRootElement(doc);
    xmlNodePtr curNode = cur->xmlChildrenNode;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST SMS_STORE_VERSION_KEY)) {
            curNode = curNode->next;
            continue ;
        }
        if (xmlStrcmp(curNode->name, BAD_CAST SMS_HEADER)) {
            curNode = curNode->next;
            continue;
        }
        char* read = xmlGetProp(curNode, BAD_CAST SMS_READ);
        if (read != NULL && strcmp(read, "false") == 0) {
            unread++;
        }
        xmlFree(read);
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);
    return unread;
}

int get_sms_num() {
    int num = 0;

    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;
    xmlDocPtr doc = xmlReadFile(SMS_XML_PATH, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        log_e("Failed to read %s", SMS_XML_PATH);
        return 0;
    }

    xmlNodePtr cur = xmlDocGetRootElement(doc);
    xmlNodePtr curNode = cur->xmlChildrenNode;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST SMS_STORE_VERSION_KEY)) {
            curNode = curNode->next;
            continue ;
        }
        if (xmlStrcmp(curNode->name, BAD_CAST SMS_HEADER)) {
            curNode = curNode->next;
            continue;
        }
        num++;
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);
    return num;
}

//get max id of current sms_storage.xml
int get_sms_max_id() {
    int max_id = 0;

    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;
    xmlDocPtr doc = xmlReadFile(SMS_XML_PATH, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        log_e("Failed to read %s", SMS_XML_PATH);
        return max_id;
    }

    xmlNodePtr cur = xmlDocGetRootElement(doc);
    xmlNodePtr curNode = cur->xmlChildrenNode;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST SMS_STORE_VERSION_KEY)) {
            curNode = curNode->next;
            continue ;
        }
        if (xmlStrcmp(curNode->name, BAD_CAST SMS_HEADER)) {
            curNode = curNode->next;
            continue;
        }
        char* str_id = xmlGetProp(curNode,BAD_CAST SMS_ID);
        if(str_id != NULL && atoi(str_id) > max_id){
            max_id = atoi(str_id);
        }
        xmlFree(str_id);
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);
    return max_id;
}

void specialCharRemove(char* sms_content) {
    int i, length = strlen(sms_content);
    for (i = 0; i < length; i++) {
        char c = sms_content[i];
        // remove non-printing character, except Line Feed & Carriage Return
        if (c >= 1 && c <= 31 && c != 10 && c != 13) {
            int len = strlen(sms_content + (i + 1));
            strncpy(sms_content + i, sms_content + (i + 1), len + 1);
            i--;
            length--;
        }
    }
}

//check if contain iso-8859-1 and convert it to valid UTF-8 encoding
void checkValidUTF8Char(char* sms_content) {
    int i, length = strlen(sms_content);
    for (i = 0; i < length; i++) {
	char c = sms_content[i];
        if ((unsigned int) c >= 128 && (unsigned int) c <= 255 &&
                (strlen(sms_content) == lv_txt_get_encoded_length(sms_content))) {
            int outlen = strlen(sms_content) * 2 + 1;
            int inlen = strlen(sms_content);
            char output[outlen];
            isolat1ToUTF8(output, &outlen, sms_content, &inlen);
            strncpy(sms_content, output, outlen);

            break;
        }
    }
}

void sms_add_xml_prop(xmlDocPtr doc, xmlNodePtr cur, char* path, char* id, char* number,
        char* date, char* encoding, char* in_sim, char* rec_num, char* read, char* content){
    xmlNodePtr thread_node = xmlNewNode(NULL, BAD_CAST (const xmlChar *) SMS_HEADER);

    //check if sms contain empty id in xml, if yes, assign max_id to recover xml, smallest id is 1
    char new_id[15];
    memset(new_id, '\0', sizeof(new_id));
    if(id == NULL || (id != NULL && strlen(id) == 0)){
        if(sms_max_id < 0){
            sms_max_id = get_sms_max_id();
        }
        sprintf(new_id, "%d", ++sms_max_id);
        log_e("Error: sms id empty, assign new id %s", new_id);
        xmlNewProp(thread_node, BAD_CAST SMS_ID, BAD_CAST new_id);
    }else{
        xmlNewProp(thread_node, BAD_CAST SMS_ID, BAD_CAST id);
    }
    xmlNewProp(thread_node, BAD_CAST SMS_PHONE_NUMBER, BAD_CAST number);
    xmlNewProp(thread_node, BAD_CAST SMS_DATE, BAD_CAST date);
    xmlNewProp(thread_node, BAD_CAST SMS_ENCODING_TYPE, BAD_CAST encoding);
    xmlNewProp(thread_node, BAD_CAST SMS_IN_SIM, BAD_CAST in_sim);
    xmlNewProp(thread_node, BAD_CAST SMS_REC_NUM, BAD_CAST rec_num);
    xmlNewProp(thread_node, BAD_CAST SMS_READ, BAD_CAST read);
    xmlNewProp(thread_node, BAD_CAST SMS_CONTENT, BAD_CAST content);
    xmlAddChild(cur, thread_node);
    xmlSaveFormatFileEnc(path, doc, ENCODING_TYPE, XML_SAVE_FORMAT);
}

//rename "file" to  sms_storage.xml
void rename_sms_xml(char* file){
    //delete sms_storage.xml
    char cmd[100];
    memset(cmd, '\0', sizeof(cmd));
    sprintf(cmd, "rm -r %s", SMS_XML_PATH);
    systemCmd(cmd);

    //rename file to sms_storage.xml
    memset(cmd, '\0', sizeof(cmd));
    sprintf(cmd, "mv %s %s", file, SMS_XML_PATH);
    systemCmd(cmd);
}

void sms_copy_xml_prop(xmlNodePtr curNode, xmlDocPtr doc, xmlNodePtr cur, char* path){
    //get sms_storage xml data
    char* id = xmlGetProp(curNode,BAD_CAST SMS_ID);
    char* phone = xmlGetProp(curNode,BAD_CAST SMS_PHONE_NUMBER);
    char* date = xmlGetProp(curNode,BAD_CAST SMS_DATE);
    char* content = xmlGetProp(curNode,BAD_CAST SMS_CONTENT);
    char* read = xmlGetProp(curNode,BAD_CAST SMS_READ);
    char* in_sim = xmlGetProp(curNode,BAD_CAST SMS_IN_SIM);
    char* sim_id = xmlGetProp(curNode,BAD_CAST SMS_REC_NUM);
    char* encoding = xmlGetProp(curNode,BAD_CAST SMS_ENCODING_TYPE);

    //add message to temp xml
    sms_add_xml_prop(doc, cur, path, id,
            phone, date, encoding, in_sim, sim_id, read, content);

    xmlFree(id);
    xmlFree(phone);
    xmlFree(date);
    xmlFree(content);
    xmlFree(read);
    xmlFree(in_sim);
    xmlFree(sim_id);
    xmlFree(encoding);
}

//clean sms_storage xml while device bootup and enter sms page
void clean_sms_xml(){
    sms_create_xml(TEMP_CLEAN_SMS_XML);
    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;
    xmlDocPtr doc = xmlReadFile(SMS_XML_PATH, NULL, XML_PARSE_RECOVER);
    xmlDocPtr temp_doc = xmlReadFile(TEMP_CLEAN_SMS_PATH, NULL, XML_PARSE_RECOVER);

    if (doc == NULL) {
        log_e("Failed to read %s", SMS_XML_PATH);
        return ;
    }
    if (temp_doc == NULL) {
        log_e("Failed to read %s", TEMP_CLEAN_SMS_PATH);
        return ;
    }
    xmlNodePtr cur = xmlDocGetRootElement(doc);
    xmlNodePtr curNode = cur->xmlChildrenNode;
    xmlNodePtr temp_cur = xmlDocGetRootElement(temp_doc);

    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST SMS_STORE_VERSION_KEY)) {
            curNode = curNode->next;
            continue ;
        }
        if (xmlStrcmp(curNode->name, BAD_CAST SMS_HEADER)) {
            log_e("clean_sms_xml: sms_storage.xml file corrupted");
            curNode = curNode->next;
            continue ;
        }
        sms_copy_xml_prop(curNode, temp_doc, temp_cur, TEMP_CLEAN_SMS_PATH);
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);
    xmlFreeDoc(temp_doc);

    rename_sms_xml(TEMP_CLEAN_SMS_PATH);
}

//adjust messages order based on receive date for sms_storage.xml
void adjust_xml_order(SMS_THREAD thread, char* sms_content, char* id_str, int pos){
    log_d("insert new sms to position:%d", pos);
    int i = 0;
    int last = get_sms_num();

    sms_create_xml(TEMP_SMS_XML);
    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;
    xmlDocPtr doc = xmlReadFile(SMS_XML_PATH, NULL, XML_PARSE_RECOVER);
    xmlDocPtr temp_doc = xmlReadFile(TEMP_SMS_XML_PATH, NULL, XML_PARSE_RECOVER);

    if (doc == NULL) {
        log_e("Failed to read %s", SMS_XML_PATH);
        return ;
    }
    if (temp_doc == NULL) {
        log_e("Failed to read %s", TEMP_SMS_XML_PATH);
        return ;
    }
    xmlNodePtr cur = xmlDocGetRootElement(doc);
    xmlNodePtr curNode = cur->xmlChildrenNode;
    xmlNodePtr temp_cur = xmlDocGetRootElement(temp_doc);

    while (curNode != NULL && i < last) {
        if (!xmlStrcmp(curNode->name, BAD_CAST SMS_STORE_VERSION_KEY)) {
            curNode = curNode->next;
            continue ;
        }
        if (xmlStrcmp(curNode->name, BAD_CAST SMS_HEADER)) {
            curNode = curNode->next;
            continue;
        }
        //insert new message to temp xml
        if(i == pos){
            sms_add_xml_prop(temp_doc, temp_cur, TEMP_SMS_XML_PATH,
                    id_str, thread.number, thread.date, thread.encoding_type,
                    thread.in_sim, thread.rec_num, thread.read, sms_content);
            i++;
        }
        sms_copy_xml_prop(curNode, temp_doc, temp_cur, TEMP_SMS_XML_PATH);
        i++;
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);
    xmlFreeDoc(temp_doc);

    rename_sms_xml(TEMP_SMS_XML_PATH);
}

//return true if t1's date >= t2's date
bool sms_cmp_time(SMS_TIME t1, SMS_TIME t2){
    bool res = false;
    if (t1.year > t2.year){
        res = true;
    } else if (t1.year == t2.year){
        if (t1.month > t2.month){
             res = true;
        } else if (t1.month == t2.month){
            if (t1.day > t2.day){
                res = true;
            } else if (t1.day == t2.day){
                if (t1.hour > t2.hour){
                    res = true;
                } else if (t1.hour == t2.hour){
                    if (t1.min >= t2.min){
                        res = true;
                    }
                }
            }
        }
    }
    return res;
}

SMS_TIME sms_sep_time(SMS_TIME t, char* buffer){
    if(buffer != NULL){
        t.year = atoi(strdup(strsep(&buffer, "/")));
        if(buffer != NULL){
            t.month = atoi(strdup(strsep(&buffer, "/")));
            if(buffer != NULL){
                t.day = atoi(strdup(strsep(&buffer, " ")));
                if(buffer != NULL){
                    t.hour = atoi(strdup(strsep(&buffer, ":")));
                    if(buffer != NULL){
                        t.min = atoi(strdup(strsep(&buffer, "")));
                    }
                }
            }
        }
    }
    return t;
}

void write_new_sms(SMS_THREAD thread) {
    if(write_sms_timestamp == 0){
        write_sms_timestamp = lv_tick_get();
    } else{
        uint32_t t = lv_tick_elaps(write_sms_timestamp);
        if(t < 1000){
            write_sms_cnt++;
        }else{
            write_sms_timestamp = 0;
            write_sms_cnt = 0;
        }
    }

    bool insert_sms = false;
    SMS_TIME t, new_t;
    memset(&t, 0, sizeof(SMS_TIME));
    memset(&new_t, 0, sizeof(SMS_TIME));

    // check and remove special char
    char sms_content[(RIL_SMS_MAX_MT_MSG_LENGTH) * 3 + 1];
    memset(&sms_content, 0, sizeof(sms_content));
    strncpy(sms_content, thread.content, strlen(thread.content));
    specialCharRemove(sms_content);
    // check if sms contain valid UTF-8 content
    checkValidUTF8Char(sms_content);

    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;

    xmlDocPtr doc = xmlReadFile(SMS_XML_PATH, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        log_e("Failed to read %s", SMS_XML_PATH);
        return ;
    }

    //clear sms content if is sim sms & using encoding type that not implement/unknown
#if defined (FEATURE_ROUTER) && !defined(ANDROID_BUILD)
    if((atoi(thread.encoding_type) == RIL_SMS_ENCODING_8BIT ||
            atoi(thread.encoding_type) == RIL_SMS_ENCODING_UNKNOWN)
            && strcmp(thread.in_sim, "true") == 0){
        //unsupported encoding type
        log_e("write_new_sms unsupported encoding type, clear content");
        memset(&sms_content, 0, sizeof(sms_content));
    }
#endif

    log_d("write_new_sms number = %s", thread.number);
    log_d("write_new_sms date = %s", thread.date);
    log_d("write_new_sms content = %s", sms_content);
    log_d("write_new_sms in_sim = %s", thread.in_sim);
    log_d("write_new_sms rec_num = %s", thread.rec_num);
    log_d("write_new_sms encoding_type = %s", thread.encoding_type);
    log_d("write_new_sms read status = %s", thread.read);

    //get new message's date info
    char* n_buffer = strdup(thread.date);
    new_t = sms_sep_time(new_t, n_buffer);

    xmlNodePtr cur = xmlDocGetRootElement(doc);
    xmlNodePtr curNode = cur->xmlChildrenNode;
    // Found max id
    int max_id = 0;
    int pos = 0;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST SMS_STORE_VERSION_KEY)) {
            curNode = curNode->next;
            continue ;
        }
        if (xmlStrcmp(curNode->name, BAD_CAST SMS_HEADER)) {
            curNode = curNode->next;
            continue;
        }
        xmlAttrPtr ptr = curNode->properties;
        char* str_id = xmlGetProp(curNode,BAD_CAST SMS_ID);
        if(str_id != NULL && atoi(str_id) > max_id){
            max_id = atoi(str_id);
        }

        //try to find the correct position to insert for new message
        char* date = xmlGetProp(curNode,BAD_CAST SMS_DATE);
        if(date != NULL && (strlen(date) > 0 && strlen(date) <= 16)){
            char* buffer = strdup(date);
            t = sms_sep_time(t, buffer);

            if(sms_cmp_time(new_t, t)){
                pos++;
            }else{
                insert_sms = true;
            }
        }
        xmlFree(str_id);
        xmlFree(date);
        curNode = curNode->next;
    }
    char id_str[15];
    memset(id_str, '\0', sizeof(id_str));
    sprintf(id_str, "%d", ++max_id);
    //save new max id for recovery sms xml to use
    sms_max_id = max_id;
    sms_add_xml_prop(doc, cur, SMS_XML_PATH,
            id_str, thread.number, thread.date, thread.encoding_type,
            thread.in_sim, thread.rec_num, thread.read, sms_content);
    xmlFreeDoc(doc);

    //adjust xml if new message's correct position not in last
    if(insert_sms){
        adjust_xml_order(thread, sms_content, id_str, pos);
    }
    //check if sms reload is necessary
    enable_sms_reload();

    //show reload popup if sms receive 3 sms in 1s
    if(write_sms_cnt > 3){
        enable_reload_static_popup();
    }
}

//delete all sim sms while device bootup
void delete_sim_sms() {
    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;
    xmlDocPtr doc = xmlReadFile(SMS_XML_PATH, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        log_e("Failed to read %s", SMS_XML_PATH);
        return;
    }

    xmlNodePtr cur = xmlDocGetRootElement(doc);
    xmlNodePtr curNode = cur->xmlChildrenNode;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST SMS_STORE_VERSION_KEY)) {
            curNode = curNode->next;
            continue ;
        }
        if (xmlStrcmp(curNode->name, BAD_CAST SMS_HEADER)) {
            curNode = curNode->next;
            continue;
        }
        int del_id = 0;
        int del_sim_id = 0;
        char* id = xmlGetProp(curNode,BAD_CAST SMS_ID);
        char* sim_id = xmlGetProp(curNode,BAD_CAST SMS_REC_NUM);
        char* in_sim = xmlGetProp(curNode, BAD_CAST SMS_IN_SIM);
        if(id != NULL) del_id = atoi(id);
        if(sim_id != NULL) del_sim_id = atoi(sim_id);

        if (in_sim != NULL && (strcmp(in_sim, "true") == 0)) {
            delete_sms(del_id, del_sim_id);
            log_d("delete_sim_sms id: %d", del_id);
        }
        xmlFree(id);
        xmlFree(sim_id);
        xmlFree(in_sim);

        curNode = curNode->next;
    }
    xmlFreeDoc(doc);

    //check if sms reload is necessary
    enable_sms_reload();
}

void upgrade_sms_store() {
    xmlDocPtr doc = xmlReadFile(SMS_XML_PATH, NULL, XML_PARSE_RECOVER);
    if (doc == NULL) {
        return ;
    }

    xmlNodePtr cur = xmlDocGetRootElement(doc);
    xmlNodePtr curNode = cur->xmlChildrenNode;
    bool upgrade = false;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST SMS_STORE_VERSION_KEY)) {
            char* version = (char*)xmlNodeGetContent(curNode);
            upgrade = (atoi(version) != atoi(SMS_STORE_VERSION));
            if (upgrade) {
                log_e("sms store upgrading, old version: %s, current version: %s", version, SMS_STORE_VERSION);
            }
            xmlFree(version);
            break ;
        }
        if (xmlStrcmp(curNode->name, BAD_CAST SMS_HEADER)) {
            curNode = curNode->next;
            continue;
        }
        curNode = curNode->next;
    }
    xmlFreeDoc(doc);

    if (upgrade) {
        //TODO
    }
}

void sms_create_xml(int type) {
    struct stat buffer;
    char path[100];
    memset(path, '\0', sizeof(path));
    if(type == SMS_XML){
        sprintf(path, "%s", SMS_XML_PATH);
    }else if(type == TEMP_SMS_XML){
        sprintf(path, "%s", TEMP_SMS_XML_PATH);
    }else if(type == TEMP_CLEAN_SMS_XML){
        sprintf(path, "%s", TEMP_CLEAN_SMS_PATH);
    }

    int exist = stat(path, &buffer);
    if (exist != 0 || buffer.st_size == 0 ||
            ((type == TEMP_SMS_XML || type == TEMP_CLEAN_SMS_XML) && buffer.st_size != 0)) {
        int res;
        xmlTextWriterPtr writer = xmlNewTextWriterFilename(path, 0);
        if (writer == NULL) {
            log_e("Error creating xml writer for %s", path);
            return;
        }

        res = xmlTextWriterStartDocument(writer, NULL, ENCODING_TYPE, NULL);
        if (res < 0) {
            log_e("Error at xmlTextWriterStartDocument for %s", path);
            return;
        }

        res = xmlTextWriterStartElement(writer, BAD_CAST SMS_ELEMENT);
        if (res < 0) {
            log_e("Error at xmlTextWriterStartElement for %s", path);
            return;
        }

        res = xmlTextWriterEndElement(writer); 
        if (res < 0) {
            log_e("Error at xmlTextWriterEndElement for %s", path);
            return;
        }

        res = xmlTextWriterEndDocument(writer);
        if (res < 0) {
            log_e("Error at xmlTextWriterEndDocument for %s", path);
            return;
        }
        xmlFreeTextWriter(writer);

        //Write sms store version
        xmlDocPtr doc = xmlReadFile(path, NULL, XML_PARSE_RECOVER);
        if (doc == NULL) {
            return ;
        }

        xmlDocPtr cur = xmlDocGetRootElement(doc);
        xmlNodePtr version_node = xmlNewNode(NULL, BAD_CAST (const xmlChar *) SMS_STORE_VERSION_KEY);
        xmlNodePtr value = xmlNewText(BAD_CAST (const xmlChar *) SMS_STORE_VERSION);
        xmlAddChild(version_node, value);
        xmlAddChild(cur, version_node);
        xmlSaveFormatFileEnc(path, doc, ENCODING_TYPE, XML_SAVE_FORMAT);
        xmlFreeDoc(doc);
    } else{
        if(type == SMS_XML){
            //delete all sim sms
            delete_sim_sms();
            //clean sms storage xml every time device bootup
            clean_sms_xml();
        }
    }

    upgrade_sms_store();
}
