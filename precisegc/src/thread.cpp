#include "thread.h"

#include <sys/mman.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>

#include "thread_list.h"
#include "logging.h"

namespace precisegc {

using namespace ::precisegc::details;

static void remove_thread(pthread_t thread)
{
    thread_list& threads = thread_list::instance();
    auto it = threads.find(thread);
    if (it != threads.end()) {
        threads.remove(it);
    }
}

void* start_routine(void* hand)
{
    thread_handler* handler = (thread_handler*) hand;
    handler->stack = StackMap::getInstance();
    handler->flags = 0;
    handler->tlflags = & new_obj_flags_tl_instance;

    void* ret = handler->routine(handler->arg);
    dprintf("Finishing thread %d\n", handler->pthread);
    lock_guard<mutex> lock(thread_list::instance_mutex);
    remove_thread(handler->pthread);
    return ret;
}

int thread_create(pthread_t* thread, const pthread_attr_t* attr, void* (* routine)(void*), void* arg)
{
    lock_guard<mutex> lock(thread_list::instance_mutex);
    thread_list& threads = thread_list::instance();
    assert(!threads.empty());
    thread_handler handler;
    // fill thread, routine & arg, rest will be filled in start_routine
    handler.routine = routine;
    handler.arg = arg;
    auto it = threads.insert(handler);
    int res = pthread_create(thread, attr, start_routine, &(*it));
    it->pthread = *thread;

    logging::info() << "Thread " << *thread << " created";

    return res;
}

void thread_join(pthread_t thread, void** thread_return)
{
    pthread_join(thread, thread_return);
}

void thread_exit(void** retval)
{
    lock_guard<mutex> lock(thread_list::instance_mutex);
    remove_thread(pthread_self());
    pthread_exit(retval);
}

void thread_cancel(pthread_t thread)
{
    lock_guard<mutex> lock(thread_list::instance_mutex);
    remove_thread(thread);
    pthread_cancel(thread);
}

thread_handler* get_thread_handler()
{
    lock_guard<mutex> lock(thread_list::instance_mutex);
    thread_list& threads = thread_list::instance();
    assert(!threads.empty());
    pthread_t thread = pthread_self();
    return &(*threads.find(thread));
}

}
