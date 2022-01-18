#include <libxml/tree.h>
#include <libxml/xmlsave.h>
#include <libxml/xmlwriter.h>

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "data_store.h"
#include "lv_pocket_router/res/values/string_value.h"
#include "lv_pocket_router/src/util/debug_log.h"

#define ENCODING_TYPE                   "UTF-8"

#define MAX_VALUE_LENGTH                64

char value_buf[MAX_VALUE_LENGTH];

int get_ds_int(const char* xpathExpr) {
    char cValue[MAX_VALUE_LENGTH];
    get_ds_data(DEFAULT_DATA_STORE_FILE, xpathExpr, &cValue);
    return atoi(cValue);
}

int ds_get_int(char* key) {
    char cValue[MAX_VALUE_LENGTH];
    memset(cValue, 0, sizeof(cValue));
    get_ds_data(DEFAULT_DATA_STORE_FILE, key, &cValue);
    return atoi(cValue);
}

long int ds_get_long_int(char* key) {
    char cValue[MAX_VALUE_LENGTH];
    memset(cValue, 0, sizeof(cValue));
    get_ds_data(DEFAULT_DATA_STORE_FILE, key, &cValue);
    return atol(cValue);
}

long long int ds_get_long_long_int(char* key) {
    char cValue[MAX_VALUE_LENGTH];
    memset(cValue, 0, sizeof(cValue));
    get_ds_data(DEFAULT_DATA_STORE_FILE, key, &cValue);
    long long int value = atoll(cValue);
    if (value >= LLONG_MAX || value < 0) {
        log_d("DS return 0 since overflow found");
        return 0;
    }
    return value;
}

bool ds_get_bool(char* key) {
    char cValue[MAX_VALUE_LENGTH];
    memset(cValue, 0, sizeof(cValue));
    get_ds_data(DEFAULT_DATA_STORE_FILE, key, &cValue);
    return strcmp(cValue,"true") == 0;
}

char* ds_get_value(char* key) {
    memset(value_buf, 0, sizeof(value_buf));
    get_ds_data(DEFAULT_DATA_STORE_FILE, key, &value_buf);
    return value_buf;
}

DS_RES ds_set_int(char* key, int value) {
    // TODO check key can't have special char
    char cValue[MAX_VALUE_LENGTH];
    sprintf(cValue, "%d", value);
    return ds_set_data(DEFAULT_DATA_STORE_FILE, key, cValue);
}

DS_RES ds_set_long_int(char* key, long int value) {
    // TODO check key can't have special char
    char cValue[MAX_VALUE_LENGTH];
    sprintf(cValue, "%ld", value);
    return ds_set_data(DEFAULT_DATA_STORE_FILE, key, cValue);
}

DS_RES ds_set_long_long_int(char* key, long long int value) {
    // TODO check key can't have special char
    char cValue[MAX_VALUE_LENGTH];
    sprintf(cValue, "%lld", value);
    return ds_set_data(DEFAULT_DATA_STORE_FILE, key, cValue);
}

DS_RES ds_set_bool(char* key, bool value) {
    char cValue[MAX_VALUE_LENGTH];
    if (value)
        sprintf(cValue, "%s", "true");
    else
        sprintf(cValue, "%s", "false");
    return ds_set_data(DEFAULT_DATA_STORE_FILE, key, cValue);
}

DS_RES ds_set_value(char* key, char* value) {
    return ds_set_data(DEFAULT_DATA_STORE_FILE, key, value);
}

void ds_create_file(const char *filepath)
{
    xmlTextWriterPtr writer;
    int res;

    writer = xmlNewTextWriterFilename(filepath, 0);
    if (writer == NULL) {
        log_e("Error creating xml writer for %s\n", filepath);
        return;
    }

    res = xmlTextWriterStartDocument(writer, NULL, ENCODING_TYPE, NULL);
    if (res < 0) {
        log_e("Error at xmlTextWriterStartDocument for %s\n", filepath);
        return;
    }

    res = xmlTextWriterStartElement(writer, BAD_CAST DEFAULT_ROOT_ELEMENT);
    if (res < 0) {
        log_e("Error at xmlTextWriterStartElement for %s\n", filepath);
        return;
    }

    res = xmlTextWriterEndDocument(writer);
    if (res < 0) {
        log_e("Error at xmlTextWriterEndDocument for %s\n", filepath);
        return;
    }

    xmlFreeTextWriter(writer);
}

DS_RES get_ds_data(const char *filename, const char* xpathExpr, char* value) {
    xmlDocPtr   doc;
    xmlNodePtr  cur;

    //ds_file_init(filename);

    doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse %s\n", filename);
        log_e("Failed to parse %s, get_ds_data errno %d %s", filename, errno, strerror(errno));
        return DS_ERROR;
    }

    cur = xmlDocGetRootElement(doc);

    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
        if (cur->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(cur->name, (const xmlChar *) xpathExpr)) {
                char * content = (char*)xmlNodeGetContent(cur);
                strncpy(value, content, strlen(content));
                free(content);
                break;
            }
        }
        cur = cur->next;
    }

    xmlFreeDoc(doc);
    return DS_OK;
}

DS_RES ds_set_data(const char *filename, const char* xpathExpr, char* value) {
    xmlDocPtr   doc;
    xmlNodePtr  cur;
    xmlNodePtr  last;
    bool bNodeFound = false;

    //ds_file_init(filename);

    xmlKeepBlanksDefault(0);
    xmlIndentTreeOutput = 1;
    doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse %s\n", filename);
        log_e("Failed to parse %s, ds_set_data errno %d %s", filename, errno, strerror(errno));
        return DS_ERROR;
    }

    cur = xmlDocGetRootElement(doc);
    cur = cur->xmlChildrenNode;

    while (cur != NULL) {
        last = cur;
        if (cur->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(cur->name, (const xmlChar *) xpathExpr)) {
                bNodeFound = true;
                xmlNodeSetContent(cur, BAD_CAST value);
                break;
            }
        }
        cur = cur->next;
    }

    if (bNodeFound == false) {
        xmlNodePtr   node, node1 = NULL;
        last = xmlDocGetRootElement(doc);
        node = xmlNewNode(NULL, BAD_CAST (const xmlChar *) xpathExpr);
        node1 = xmlNewText(BAD_CAST (const xmlChar *) value);
        xmlAddChild(node, node1);
        xmlAddChild(last, node);
    }

    xmlSaveFormatFileEnc(filename, doc, ENCODING_TYPE, XML_SAVE_FORMAT);

    /* dump the resulting document */
    //xmlDocDump(stdout, doc);

    xmlFreeDoc(doc);

    sync();

    return DS_OK;
}

bool get_ds_exist(const char *filename, const char* xpathExpr) {
    bool exist = false;
    xmlDocPtr   doc;
    xmlNodePtr  cur;

    doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse %s\n", filename);
        log_e("Failed to parse %s, get_ds_data errno %d %s", filename, errno, strerror(errno));
        return exist;
    }

    cur = xmlDocGetRootElement(doc);

    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
        if (cur->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(cur->name, (const xmlChar *) xpathExpr)) {
                exist = true;
                break;
            }
        }
        cur = cur->next;
    }

    xmlFreeDoc(doc);
    return exist;
}

DS_RES ds_integrity_check() {
    // try read language value to check if file readable as integrity check
    char cValue[MAX_VALUE_LENGTH];
    memset(cValue, 0, sizeof(cValue));
    int res = get_ds_data(DEFAULT_DATA_STORE_FILE, DS_KEY_LANGUAGE, &cValue);
    if (DS_ERROR == res || !get_ds_exist(DEFAULT_DATA_STORE_FILE, DS_KEY_LANGUAGE)) {
        if (remove(DEFAULT_DATA_STORE_FILE) == 0) {
            log_d("%s deleted", DEFAULT_DATA_STORE_FILE);
        } else {
            log_e("Failed to delete %s", DEFAULT_DATA_STORE_FILE);
        }

        // check if any customer default data_storage.xml
        // replace with cust xml
        struct stat buffer;
        int exist = stat("/oem/data/data_storage.xml", &buffer);
        if (exist == 0) {
            if (0 == systemCmd("cp /oem/data/data_storage.xml /data/misc/pocketrouter/data_storage.xml")) {
                return DS_OK;
            }
        }

        return DS_ERROR;
    }
    return DS_OK;
}

