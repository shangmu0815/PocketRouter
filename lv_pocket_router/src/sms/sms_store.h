
#if FEATURE_ROUTER
#define SMS_XML_PATH         "/data/misc/pocketrouter/sms_storage.xml"
#define TEMP_SMS_XML_PATH    "/data/misc/pocketrouter/temp_sms_storage.xml"
#define TEMP_CLEAN_SMS_PATH  "/data/misc/pocketrouter/temp_clean_sms.xml"
#else
#define SMS_XML_PATH         "Data_Store/sms_storage.xml"
#define TEMP_SMS_XML_PATH    "Data_Store/temp_sms_storage.xml"
#define TEMP_CLEAN_SMS_PATH  "Data_Store/temp_clean_sms.xml"
#endif

#define SMS_STORE_VERSION_KEY "sms_version"
#define SMS_HEADER            "thread"
#define SMS_ID                "id"
#define SMS_PHONE_NUMBER      "phone_number"
#define SMS_DATE              "date"
#define SMS_CONTENT           "content"
#define SMS_READ              "read"
#define SMS_IN_SIM            "in_sim" //0:MT SMS, 1:SIM SMS
#define SMS_REC_NUM           "rec_num"//number id of sim message
#define SMS_ENCODING_TYPE     "encoding_type"

typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int min;
} SMS_TIME;

typedef struct {
    char* number;
    char* date;
    char* content;
    char* in_sim;
    char* rec_num;
    char* encoding_type;
    char* read;
} SMS_THREAD;

enum SMS_XML_TYPE {
    SMS_XML,
    TEMP_SMS_XML,
    TEMP_CLEAN_SMS_XML
};

void sms_create_xml(int type);
void write_new_sms(SMS_THREAD thread);
void set_sms_read(int sms_id);
void delete_sms(int sms_id, int sim_id);
int get_sms_num();
int get_sms_unread();
int get_sms_max_id();
void clean_sms_xml();
