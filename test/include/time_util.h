#ifndef DIPLOMA_TIME_UTIL_H
#define DIPLOMA_TIME_UTIL_H

#include <sys/time.h>

inline timespec now()
{
    timespec ts;
    timeval tv;
    gettimeofday(&tv, nullptr);
    ts.tv_sec = tv.tv_sec;
    ts.tv_nsec = 0;
    return ts;
}

#endif //DIPLOMA_TIME_UTIL_H
