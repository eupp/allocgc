#include <libprecisegc/gc_init.h>

#include <iostream>
#include <memory>

#include <libprecisegc/details/utils/make_unique.hpp>
#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/initation_policy.hpp>
#include <libprecisegc/details/serial_gc.hpp>
#include <libprecisegc/details/logging.h>

namespace precisegc {

static bool init_flag = false;

int gc_init(gc_options ops)
{
    using namespace details;

    if (!init_flag) {
        details::logging::init(std::clog, details::logging::loglevel::INFO);

        details::threads::thread_manager::instance().register_main_thread();

        const size_t HEAP_MAXSIZE       = 32 * 1024 * 1024;             // 32 Mb
        const size_t HEAP_STARTSIZE     = 4 * 1024;                     // 4 Mb
        const double THRESHOLD          = 1.0;
        const double MARKING_THRESHOLD  = 0.6;
        const double INCREASE_FACTOR    = 2.0;

        if (ops.strategy == gc_strategy::SERIAL) {
            std::unique_ptr<initation_policy> policy =
                    utils::make_unique<space_based_policy>(HEAP_STARTSIZE, THRESHOLD, INCREASE_FACTOR, HEAP_MAXSIZE);
            std::unique_ptr<gc_interface> gc = utils::make_unique<serial_gc>(ops.compacting, std::move(policy));
            gc_set(std::move(gc));
        }

        init_flag = true;
    }

    return 0;
}

}
