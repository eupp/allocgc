//
// Created by danya on 26.05.15.
//
//#ifndef DIPLOMA_TIMING_H
//#define DIPLOMA_TIMING_H
//
//#endif //DIPLOMA_TIMING_H

#pragma once
#include <sys/time.h>
#include <cstdlib>
#include <iostream>

//  These macros were a quick hack for the Macintosh.
#define currentTime() stats_rtclock()
#define elapsedTime(x) (x)

using std::cout;
using std::endl;

/* Get the current time in milliseconds */
unsigned stats_rtclock (void);