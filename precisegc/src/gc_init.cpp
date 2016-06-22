#include <libprecisegc/gc_init.h>

#include <iostream>
#include <memory>

#include <libprecisegc/details/utils/make_unique.hpp>
#include <libprecisegc/details/garbage_collector.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/initation_policy.hpp>
#include <libprecisegc/details/serial_gc.hpp>
#include <libprecisegc/details/incremental_gc.hpp>
#include <libprecisegc/details/logging.h>

namespace precisegc {

static bool init_flag = false;

int gc_init(gc_options ops)
{
    using namespace details;
    if (!init_flag) {
        details::logging::init(std::clog, ops.loglevel);
        details::threads::thread_manager::instance().register_main_thread();

        const size_t HEAP_MAXSIZE       = 64 * 1024 * 1024;             // 32 Mb
        const size_t HEAP_STARTSIZE     = HEAP_MAXSIZE;
        const double THRESHOLD          = 1.0;
        const double MARKING_THRESHOLD  = 0.6;
        const double INCREASE_FACTOR    = 2.0;

        if (ops.type == gc_type::SERIAL) {
            auto policy = utils::make_unique<space_based_policy>(HEAP_STARTSIZE, THRESHOLD, INCREASE_FACTOR, HEAP_MAXSIZE);
            auto gc     = utils::make_unique<serial_gc>(ops.compacting, std::move(policy));
            gci().set_strategy(std::move(gc));
        } else if (ops.type == gc_type::INCREMENTAL) {
            auto policy = utils::make_unique<incremental_space_based_policy>(HEAP_STARTSIZE, MARKING_THRESHOLD, THRESHOLD, INCREASE_FACTOR, HEAP_MAXSIZE);
            auto gc     = utils::make_unique<incremental_gc>(ops.compacting, std::move(policy));
            gci().set_strategy(std::move(gc));
        }

        if (ops.print_stat) {
            gci().set_printer_enabled(true);
        }

        init_flag = true;
    }

    return 0;
}

}
