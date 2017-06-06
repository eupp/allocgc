#include <liballocgc/details/collectors/gc_serial.hpp>

#include <liballocgc/details/gc_unsafe_scope.hpp>
#include <liballocgc/details/threads/world_snapshot.hpp>
#include <netinet/in.h>

namespace allocgc { namespace details { namespace collectors {

gc_serial::gc_serial()
    : gc_core(nullptr)
{}

void gc_serial::wbarrier(gc_handle& dst, const gc_handle& src)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(src);
    gc_handle_access::set<std::memory_order_relaxed>(dst, ptr);
}

gc_runstat gc_serial::gc_impl(const gc_options& options)
{
    if (options.kind != gc_kind::COLLECT) {
        return gc_runstat();
    }

    gc_runstat stats = sweep();
    shrink();
    return stats;
}

gc_runstat gc_serial::sweep()
{
    auto snapshot = stop_the_world();

    trace_uninit(snapshot);
    trace_roots(snapshot);
    trace_pins(snapshot);

    start_concurrent_marking(threads_available());
    start_marking();

    gc_runstat stats;
    stats.collection = collect(snapshot, threads_available());
    stats.pause   = snapshot.time_since_stop_the_world();

    return stats;
}

gc_info gc_serial::info() const
{
    static gc_info inf = {
            .incremental_flag                = false,
            .support_concurrent_marking      = false,
            .support_concurrent_collecting   = false
    };
    return inf;
}

}}}

