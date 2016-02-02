#pragma once
#include <pthread.h>
#include "stack.h"
#include "deref_roots.h"
#include "tlvars.h"

#define thread_in_safepoint(h) (h->flags & 2)
#define enter_safepoint(h) {h->flags |= 2;}
#define exit_safepoint(h) {h->flags &= ~2;}

#define without_gc_before() {\
     pthread_mutex_lock(&gc_mutex);\
     thread_handler * ___current_thread_handler = get_thread_handler();\
     enter_safepoint(___current_thread_handler);\
     if (gc_thread) {\
        dprintf("Thread %d reached safepoint\n", ___current_thread_handler->thread);\
        pthread_cond_signal(&safepoint_reached);\
        dprintf("Thread %d wait end of gc\n", ___current_thread_handler->thread);\
        pthread_cond_wait(&gc_is_finished, &gc_mutex);\
        dprintf("Thread %d:Continue to work after gc\n", ___current_thread_handler->thread);\
     }\
     exit_safepoint(___current_thread_handler);\
}

#define without_gc_after() {\
     pthread_mutex_unlock(&gc_mutex);\
}

#define safepoint() if (more_than_one) {\
        without_gc_before()\
        without_gc_after()\
}

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

extern pthread_mutex_t gc_mutex;
extern pthread_cond_t gc_is_finished;
extern pthread_cond_t safepoint_reached;
extern thread_handler* gc_thread;
extern volatile bool more_than_one;

int thread_create(pthread_t* thread, const pthread_attr_t* attr,
                  void* (* routine)(void*), void* arg);

void thread_join(pthread_t thread, void** thread_return);

void thread_exit(void** retval);

void thread_cancel(pthread_t thread);

thread_handler* get_thread_handler();

}