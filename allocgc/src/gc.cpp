#include <liballocgc/gc.hpp>

#include <liballocgc/details/gc_hooks.hpp>
#include <liballocgc/details/collectors/gc_serial.hpp>
#include <liballocgc/details/collectors/gc_cms.hpp>
#include <liballocgc/details/threads/gc_thread_manager.hpp>
#include <liballocgc/details/threads/return_address.hpp>
#include <liballocgc/details/threads/posix_thread.hpp>
#include <liballocgc/details/utils/make_unique.hpp>

namespace allocgc {

using namespace details;
using namespace details::threads;
using namespace details::collectors;

gc_handle gc_handle::null{nullptr};

int gc_init(const gc_params& params)
{
    logging::init(std::clog, params.loglevel);

    allocators::memory_index::init();

    thread_descriptor main_thrd_descr;
    main_thrd_descr.id = std::this_thread::get_id();
    main_thrd_descr.native_handle = threads::this_thread_native_handle();
    main_thrd_descr.stack_start_addr = threads::frame_address();

    gc_register_thread(main_thrd_descr);

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

namespace serial {

void set_heap_limit(size_t limit)
{
    gc_facade<gc_serial>::set_heap_limit(limit);
}

}

}

