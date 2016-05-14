#pragma once

#include <pthread.h>
#include <memory>

#include "stack.h"
#include "details/barrier_buffer.h"

namespace precisegc {

struct thread_handler
{
    pthread_t pthread;
    void* (* routine)(void*); // for init
    void* arg;
    size_t flags;
    std::shared_ptr<StackMap> stack;
    std::shared_ptr<StackMap> pins;
    std::shared_ptr<details::barrier_buffer> mark_queue;
};

int thread_create(pthread_t* thread, const pthread_attr_t* attr,
                  void* (* routine)(void*), void* arg);

void thread_join(pthread_t thread, void** thread_return);

void thread_exit(void** retval);

void thread_cancel(pthread_t thread);

thread_handler* get_thread_handler();

}