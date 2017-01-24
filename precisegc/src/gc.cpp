#include <libprecisegc/gc.hpp>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/allocators/gc_core_allocator.hpp>
#include <libprecisegc/details/threads/gc_thread_manager.hpp>
#include <libprecisegc/details/threads/return_address.hpp>
#include <libprecisegc/details/threads/posix_thread.hpp>
#include <libprecisegc/details/gc_factory.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc {

using namespace details;

int gc_init(const gc_init_options& options)
{
    static bool init_flag = false;
    if (!init_flag) {
        logging::init(std::clog, options.loglevel);
//        threads::gc_thread_manager::instance().register_main_thread(threads::frame_address());

        gc_set_heap_limit(options.heapsize);

        thread_descriptor main_thrd_descr;
        main_thrd_descr.id = std::this_thread::get_id();
        main_thrd_descr.native_handle = threads::this_thread_native_handle();
        main_thrd_descr.stack_start_addr = threads::frame_address();

        auto strategy = gc_factory::create_gc(options, main_thrd_descr);
        gc_initialize(std::move(strategy));

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
    gc_options opt;
    opt.kind = gc_kind::MARK_COLLECT;
    opt.gen  = -1;
    gc_initiation_point(details::initiation_point_type::USER_REQUEST, opt);
}

}

