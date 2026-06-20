// Host-build shim. time_us_32 is only used by occasional profiling counters.
#pragma once
#include <stdint.h>
#include <time.h>

static inline uint32_t time_us_32(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000000u + ts.tv_nsec / 1000u);
}
