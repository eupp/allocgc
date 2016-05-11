#include "gc_init.h"

#include <iostream>

#include "details/thread_list.h"
#include "details/gc_pause.h"
#include "details/gc_initator.h"
#include "details/logging.h"

namespace precisegc {

static bool init_flag = false;

static void create_first_thread()
{
    thread_handler first_thread;
    first_thread.pthread = pthread_self();
//    first_thread.stack = StackMap::getInstance();
    first_thread.flags = 0;
    first_thread.routine = nullptr;
    first_thread.arg = nullptr;
    first_thread.stack.reset(new StackMap());
    first_thread.pins.reset(new StackMap());
    first_thread.mark_queue.reset(new details::gc_mark_queue());
    details::thread_list::instance().insert(first_thread);

    details::logging::info() << "Thread " << pthread_self() << " main";
}

int gc_init()
{
    if (!init_flag) {
        details::logging::init(std::clog, details::logging::loglevel::OFF);

        create_first_thread();
        details::gc_pause_init();

        const size_t MEM_UPPER_BOUND = 32 * 1024 * 1024;
        details::init_initator(0.7 * MEM_UPPER_BOUND, MEM_UPPER_BOUND);

        init_flag = true;
    }

    return 0;
}

}
