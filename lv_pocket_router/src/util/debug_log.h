#if SYS_LOG
#include <syslog.h>
#elif ANDROID_LOG
#include <cutils/log.h>
#define LOG_TAG     "PocketRouter"
#endif

#if ANDROID_LOG
#define log_d(format, ...)      ALOGD(format, ## __VA_ARGS__)
//#define log_d(...)              android_printLog(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define log_i(format, ...)      ALOGI(format, ## __VA_ARGS__)
#define log_e(format, ...)      ALOGE(format, ## __VA_ARGS__)
#elif SYS_LOG
#define log_d(format, ...)      syslog(LOG_DEBUG, format, ## __VA_ARGS__)
#define log_i(format, ...)      syslog(LOG_INFO, format, ## __VA_ARGS__)
#define log_e(format, ...)      syslog(LOG_ERR, format, ## __VA_ARGS__)
#else
#define log_d(format, ...)      printf(format "\n", ## __VA_ARGS__)
#define log_i(format, ...)      printf(format "\n", ## __VA_ARGS__)
#define log_e(format, ...)      printf(format "\n", ## __VA_ARGS__)
#endif



