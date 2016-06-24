#include <libprecisegc/gc_init.h>

#include <iostream>
#include <memory>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/gc_factory.hpp>
#include <libprecisegc/details/logging.h>

namespace precisegc {

static bool init_flag = false;

int gc_init(const gc_options& options)
{
    using namespace details;
    if (!init_flag) {
        logging::init(std::clog, options.loglevel);
        threads::thread_manager::instance().register_main_thread();

        gc_set_strategy(gc_factory::create(options));

        if (options.print_stat) {
            gc_enable_print_stats();
        }

        init_flag = true;
    }

    return 0;
}

}
