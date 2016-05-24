#include "gc_init.h"

#include <iostream>

#include <libprecisegc/details/threads/thread_manager.hpp>

#include "details/gc_initator.h"
#include "details/logging.h"

namespace precisegc {

static bool init_flag = false;

int gc_init()
{
    if (!init_flag) {
        details::logging::init(std::clog, details::logging::loglevel::OFF);

        details::threads::thread_manager::instance().register_main_thread();

        const size_t MEM_UPPER_BOUND = 64 * 1024 * 1024;
        details::init_initator(0.6 * MEM_UPPER_BOUND, MEM_UPPER_BOUND);

        init_flag = true;
    }

    return 0;
}

}
