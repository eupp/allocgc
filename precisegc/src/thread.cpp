#include "thread.h"

#include <sys/mman.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>

#include "thread_list.h"

namespace precisegc {

using namespace ::precisegc::details;

pthread_mutex_t gc_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gc_is_finished = PTHREAD_COND_INITIALIZER;
pthread_cond_t safepoint_reached = PTHREAD_COND_INITIALIZER;
thread_handler* gc_thread;
volatile bool more_than_one = false;

static void remove_thread(pthread_t thread)
{
    without_gc_before();
    mutex_lock<mutex> lock(thread_list::instance_mutex);
    thread_list& threads = thread_list::instance();
    auto it = threads.find(thread);
    if (it != threads.end()) {
        threads.remove(it);
        if (threads.size() == 1) {
            more_than_one = false;
        }
    }
    without_gc_after()
}

void* start_routine(void* hand)
{
    thread_handler* handler = (thread_handler*) hand;
    handler->stack = StackMap::getInstance();
    handler->flags = 0;
    handler->tlflags = & new_obj_flags_tl_instance;

    dprintf("Starting thread %d\n", handler->pthread);
    pthread_mutex_lock(& gc_mutex);
    if (gc_thread) {
        dprintf("waiting for gc before starting thread %d\n", handler->pthread);
        pthread_cond_wait(& gc_is_finished, & gc_mutex);
    }
    more_than_one = true;
    pthread_mutex_unlock(&gc_mutex);

    void* ret = handler->routine(handler->arg);
    dprintf("Finishing thread %d\n", handler->pthread);
    remove_thread(handler->pthread);
    return ret;
}

static void create_first_thread()
{
    thread_handler first_thread;
    first_thread.pthread = pthread_self();
    first_thread.stack = StackMap::getInstance();
    first_thread.flags = 0;
    first_thread.tlflags = &new_obj_flags_tl_instance;
    first_thread.routine = nullptr;
    first_thread.arg = nullptr;
    details::thread_list::instance().insert(first_thread);
}

int thread_create(pthread_t* thread, const pthread_attr_t* attr, void* (* routine)(void*), void* arg)
{
    mutex_lock<mutex> lock(thread_list::instance_mutex);
    thread_list& threads = thread_list::instance();
    if (threads.empty()) {
        create_first_thread();
    }
    thread_handler handler;
    // fill thread, routine & arg, rest will be filled in start_routine
    handler.routine = routine;
    handler.arg = arg;
    auto it = threads.insert(handler);
    int res = pthread_create(thread, attr, start_routine, &(*it));
    it->pthread = *thread;
    return res;
}

void thread_join(pthread_t thread, void** thread_return)
{
    pthread_mutex_lock(&gc_mutex);
    thread_handler* curr = get_thread_handler();
    enter_safepoint(curr);
    if (gc_thread) {
        dprintf("Thread %d reached safepoint in join\n", curr->pthread);
        pthread_cond_signal(&safepoint_reached);
    }
    pthread_mutex_unlock(& gc_mutex);

    pthread_join(thread, thread_return);

    pthread_mutex_lock(&gc_mutex);
    exit_safepoint(curr);
    pthread_mutex_unlock(&gc_mutex);
}

void thread_exit(void** retval)
{
    remove_thread(pthread_self());
    pthread_exit(retval);
}

void thread_cancel(pthread_t thread)
{
    remove_thread(thread);
    pthread_cancel(thread);
}

thread_handler* get_thread_handler()
{
    mutex_lock<mutex> lock(thread_list::instance_mutex);
    thread_list& threads = thread_list::instance();
    if (threads.empty()) {
        create_first_thread();
    }
    pthread_t thread = pthread_self();
    return &(*threads.find(thread));
}

}
