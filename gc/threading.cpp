//
// Created by evagl on 26.03.15.
//
#include <sys/mman.h>
#include <assert.h>
#include <pthread.h>
#include "threading.h"

pthread_mutex_t gc_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gc_is_finished = PTHREAD_COND_INITIALIZER;
pthread_cond_t safepoint_reached = PTHREAD_COND_INITIALIZER;
thread_handler* gc_thread;
volatile bool more_than_one = false;

thread_handler *first_thread = nullptr;
static thread_handler *free_list = nullptr;
static pthread_mutex_t free_list_mutex = PTHREAD_MUTEX_INITIALIZER;
constexpr size_t PAGE_SIZE = 4096;
constexpr size_t NUM_OF_HANDLERS = PAGE_SIZE / sizeof(thread_handler);
static thread_handler handlers[32];
static bool init = false;

inline thread_handler *new_thread_handler() {
    pthread_mutex_lock(&free_list_mutex);
    if (free_list == nullptr) {
        if (!init) {
            for (int i = 0; i < 32; ++i) {
                handlers[i].next = free_list;
                free_list = handlers + i;
            }
            init = true;
        } else {
            void *data = mmap(0, PAGE_SIZE,
                              PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
            assert(data != MAP_FAILED);
            for (int i = 0; i < NUM_OF_HANDLERS; ++i) {
                thread_handler *handler = (thread_handler *) data;
                handler->next = free_list;
                free_list = handler;
                data += sizeof(thread_handler);
            }
        }
    }
    thread_handler *result = free_list;
    free_list = free_list->next;
    pthread_mutex_unlock(&free_list_mutex);
    return result;
}

void remove_thread(pthread_t thread) {
    without_gc_before()
    thread_handler *curr = first_thread;
    thread_handler *prev = nullptr;
    while (!pthread_equal(curr->thread, thread)) {
        prev = curr;
        curr = curr->next;
    }
    if (prev == nullptr) {
        first_thread = curr->next;
    } else {
        prev->next = curr->next;
    }
    if (first_thread->next == nullptr) {
        more_than_one = false;
    }
    without_gc_after()
    pthread_mutex_lock(&free_list_mutex);
    curr->next = free_list;
    free_list = curr;
    pthread_mutex_unlock(&free_list_mutex);
}

void *start_routine(void *hand) {
    thread_handler *handler = (thread_handler *) hand;
    handler->stack = StackMap::getInstance();
    handler->thread = pthread_self();
    handler->stack_bottom = __builtin_frame_address(0);
    handler->flags = 0;
    handler->tlflags = &new_obj_flags_tl_instance;
    dprintf("Starting thread %d\n", handler->thread);

    pthread_mutex_lock(&gc_mutex);
    if (gc_thread) {
        dprintf("waiting for gc before starting thread %d\n", handler->thread);
        pthread_cond_wait(&gc_is_finished, &gc_mutex);
    }
    more_than_one = true;
    handler->next = first_thread;
    first_thread = handler;
    pthread_mutex_unlock(&gc_mutex);

    void *returnValue = handler->routine(handler->arg);
    dprintf("Finishing thread %d\n", handler->thread);
    remove_thread(handler->thread);
    return returnValue;
}

void create_first_handler() {
    first_thread = new_thread_handler();
    first_thread->thread = pthread_self();
    first_thread->next = nullptr;
    first_thread->stack = StackMap::getInstance();
    first_thread->stack_bottom = nullptr;
    first_thread->flags = 0;
    first_thread->tlflags = &new_obj_flags_tl_instance;
    first_thread->next = nullptr;
}

int thread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*routine)(void *), void *arg) {
    if (!first_thread) {
        create_first_handler();
    }
    thread_handler *handler = new_thread_handler();
    handler->routine = routine;
    handler->arg = arg;
    return pthread_create(thread, attr, start_routine, handler);
}

void thread_join(pthread_t thread, void **thread_return) {
    pthread_mutex_lock(&gc_mutex);
    thread_handler *curr = get_thread_handler();
    curr->stack_top = __builtin_frame_address(0);
    enter_safepoint(curr);
    if (gc_thread) {
        dprintf("Thread %d reached safepoint in join\n", curr->thread);
        pthread_cond_signal(&safepoint_reached);
    }
    pthread_mutex_unlock(&gc_mutex);

    pthread_join(thread, thread_return);

    pthread_mutex_lock(&gc_mutex);
    exit_safepoint(curr);
    pthread_mutex_unlock(&gc_mutex);
}

void thread_exit(void **retval) {
    remove_thread(pthread_self());
    pthread_exit(retval);
}

void thread_cancel(pthread_t thread) {
    remove_thread(thread);
    pthread_cancel(thread);
}

thread_handler* get_thread_handler() {
    if (!first_thread) {
        create_first_handler();
    }
    pthread_t thread = pthread_self();
    thread_handler *curr = first_thread;
    while (!pthread_equal(curr->thread, thread)) {
        curr = curr->next;
    }
    return curr;
}
