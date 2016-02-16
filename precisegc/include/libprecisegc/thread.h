#pragma once
#include <pthread.h>
#include "stack.h"
#include "tlvars.h"

namespace precisegc {

struct thread_handler
{
    pthread_t pthread;
    void* (* routine)(void*); // for init
    void* arg;
    size_t flags;
    StackMap* stack;
    tlvars* tlflags;
};

int thread_create(pthread_t* thread, const pthread_attr_t* attr,
                  void* (* routine)(void*), void* arg);

void thread_join(pthread_t thread, void** thread_return);

void thread_exit(void** retval);

void thread_cancel(pthread_t thread);

thread_handler* get_thread_handler();

}