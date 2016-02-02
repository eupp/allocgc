#ifndef DIPLOMA_SIGNAL_SAFE_SYNC_H
#define DIPLOMA_SIGNAL_SAFE_SYNC_H

#include <cassert>
#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>

#include "noncopyable.h"

namespace precisegc { namespace details {

class signal_safe_event: public noncopyable
{
public:
    signal_safe_event();
    ~signal_safe_event();

    void wait();
    void notify(size_t cnt);
private:
    int m_pipefd[2];
};

class signal_safe_barrier: public noncopyable
{
public:
    signal_safe_barrier();
    ~signal_safe_barrier();

    void wait(size_t cnt);
    size_t wait_for(size_t cnt, timeval* tv);

    void notify();
private:
    int m_pipefd[2];
};

}}

#endif //DIPLOMA_SIGNAL_SAFE_SYNC_H
