#ifndef _ARDUINO_H_
#define _ARDUINO_H_
#include <time.h>

static inline uint32_t millis(void) {
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint32_t)(t.tv_sec*1000L + t.tv_nsec/1000000L);
}
#endif // _ARDUINO_H_
