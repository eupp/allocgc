#include <libprecisegc/gc.hpp>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/gc_factory.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc {

using namespace details;

int gc_init(const gc_init_options& options)
{
    static bool init_flag = false;
    if (!init_flag) {
        logging::init(std::clog, options.loglevel);
        threads::thread_manager::instance().register_main_thread();

        auto strategy = gc_factory::create_gc(options);
        auto init_policy = gc_factory::create_initiation_policy(strategy.get(), options);
        gc_initialize(std::move(strategy), std::move(init_policy));

        if (options.print_stat) {
            gc_enable_print_stats();
        }

        init_flag = true;
    }

    return 0;
}

gc_stat gc_stats()
{
    return gc_get_stats();
}

void gc()
{
    gc_initiation_point(details::initiation_point_type::USER_REQUEST);
}

}

