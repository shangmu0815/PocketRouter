/*
 * copy from hardware/libhardware_legacy/power.c 
 */
#include "wake_lock.h"
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>

#include "lv_pocket_router/src/util/debug_log.h"

enum {
    ACQUIRE_PARTIAL_WAKE_LOCK = 0,
    RELEASE_WAKE_LOCK,
    OUR_FD_COUNT
};

const char * const OLD_PATHS[] = {
    "/sys/android_power/acquire_partial_wake_lock",
    "/sys/android_power/release_wake_lock",
};

const char * const NEW_PATHS[] = {
    "/sys/power/wake_lock",
    "/sys/power/wake_unlock",
};

//XXX static pthread_once_t g_initialized = THREAD_ONCE_INIT;
static int g_initialized = 0;
static int g_fds[OUR_FD_COUNT];
static int g_error = -1;

static int open_file_descriptors(const char * const paths[]) {
#if defined USE_ANDROID_FBDEV
    int i;
    for (i=0; i<OUR_FD_COUNT; i++) {
        int fd = open(paths[i], O_RDWR | O_CLOEXEC);
        if (fd < 0) {
            g_error = -errno;
            log_e(stderr, "fatal error opening \"%s\": %s\n", paths[i],
                strerror(errno));
            return -1;
        }
        g_fds[i] = fd;
    }

    g_error = 0;
#endif
    return 0;
}

static inline void initialize_fds(void) {
#if defined USE_ANDROID_FBDEV
    // XXX: should be this:
    //pthread_once(&g_initialized, open_file_descriptors);
    // XXX: not this:
    if (g_initialized == 0) {
        if(open_file_descriptors(NEW_PATHS) < 0)
            open_file_descriptors(OLD_PATHS);
        g_initialized = 1;
    }
#endif
}

int acquire_wake_lock(int lock, const char* id) {
#if defined USE_ANDROID_FBDEV
    initialize_fds();

    if (g_error) return g_error;

    int fd;
    ssize_t ret;

    if (lock != PARTIAL_WAKE_LOCK) {
        return -EINVAL;
    }

    fd = g_fds[ACQUIRE_PARTIAL_WAKE_LOCK];

    ret = write(fd, id, strlen(id));
    if (ret < 0) {
        return -errno;
    }
    return ret;
#else
    return 0;
#endif
}

int release_wake_lock(const char* id) {
#if defined USE_ANDROID_FBDEV
    initialize_fds();

    if (g_error) return g_error;

    ssize_t len = write(g_fds[RELEASE_WAKE_LOCK], id, strlen(id));
    if (len < 0) {
        return -errno;
    }
    return len;
#else
    return 0;
#endif
}
