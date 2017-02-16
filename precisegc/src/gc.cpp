#include <libprecisegc/gc.hpp>

#include <libprecisegc/gc_factory.hpp>
#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/collectors/gc_serial.hpp>
#include <libprecisegc/details/collectors/gc_incremental.hpp>
#include <libprecisegc/details/threads/gc_thread_manager.hpp>
#include <libprecisegc/details/threads/return_address.hpp>
#include <libprecisegc/details/threads/posix_thread.hpp>
#include <libprecisegc/details/utils/make_unique.hpp>

namespace precisegc {

using namespace details;
using namespace details::threads;
using namespace details::collectors;

std::unique_ptr<gc_strategy> gc_factory::create(const options& opt)
{
    thread_descriptor main_thrd_descr;
    main_thrd_descr.id = std::this_thread::get_id();
    main_thrd_descr.native_handle = threads::this_thread_native_handle();
    main_thrd_descr.stack_start_addr = threads::frame_address();

    gc_set_heap_limit(opt.heapsize);

    if (opt.incremental) {
        return utils::make_unique<gc_incremental>(opt, main_thrd_descr);
    } else {
        return utils::make_unique<gc_serial>(opt, main_thrd_descr);
    }
}

int gc_init(std::unique_ptr<gc_strategy> strategy)
{
    gc_initialize(std::move(strategy));
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

