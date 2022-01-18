/*
 * copy from hardware/libhardware_legacy/include/hardware_legacy/power.h
 */
#include <stdint.h>

#if __cplusplus
extern "C" {
#endif

enum {
    PARTIAL_WAKE_LOCK = 1,  // the cpu stays on, but the screen is off
    FULL_WAKE_LOCK = 2      // the screen is also on
};

// while you have a lock held, the device will stay on at least at the
// level you request.
int acquire_wake_lock(int lock, const char* id);
int release_wake_lock(const char* id);


#if __cplusplus
} // extern "C"
#endif
