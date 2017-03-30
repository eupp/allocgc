#ifndef ALLOCGC_TIME_UTIL_H
#define ALLOCGC_TIME_UTIL_H

#include <sys/time.h>

inline timeval tv_now()
{
    timeval tv;
    gettimeofday(&tv, nullptr);
    return tv;
}

inline timespec ts_now()
{
    timespec ts;
    timeval tv = tv_now();
    ts.tv_sec = tv.tv_sec;
    ts.tv_nsec = 0;
    return ts;
}

#endif //ALLOCGC_TIME_UTIL_H
