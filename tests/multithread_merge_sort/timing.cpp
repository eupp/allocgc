//
// Created by danya on 26.05.15.
//
#include "timing.h"

unsigned stats_rtclock (void) {
    struct timeval t;
    struct timezone tz;

    if (gettimeofday( &t, &tz ) == -1)
        return 0;
    return (unsigned)(t.tv_sec * 1000 + t.tv_usec / 1000);
}