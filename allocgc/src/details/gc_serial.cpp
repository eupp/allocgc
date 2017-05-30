#include <liballocgc/details/collectors/gc_serial.hpp>

#include <liballocgc/details/gc_unsafe_scope.hpp>
#include <liballocgc/details/threads/world_snapshot.hpp>
#include <netinet/in.h>

namespace allocgc { namespace details { namespace collectors {

gc_serial::gc_serial()
    : gc_core(this, nullptr)
//    , m_threads_available(opt.threads_available)
    , m_threads_available(std::thread::hardware_concurrency())
{}

void gc_serial::wbarrier(gc_handle& dst, const gc_handle& src)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(src);
    gc_handle_access::set<std::memory_order_relaxed>(dst, ptr);
}

gc_run_stat gc_serial::gc(const gc_options& options)
{
    if ((options.kind != gc_kind::MARK_COLLECT) && (options.kind != gc_kind::COLLECT)) {
        return gc_run_stat();
    }

    gc_safe_scope safe_scope;
    std::lock_guard<std::mutex> lock(m_mutex);

    gc_run_stat stats = sweep();
    shrink();
    register_gc_run(stats);
    return stats;
}

gc_run_stat gc_serial::sweep()
{
    auto snapshot = stop_the_world();

    trace_uninit(snapshot);
    trace_roots(snapshot);
    trace_pins(snapshot);

    start_concurrent_marking(m_threads_available);
    start_marking();

    gc_run_stat stats;
    stats.heap_stat = collect(snapshot, m_threads_available);

    stats.pause_stat.type       = gc_pause_type::MARK_COLLECT;
    stats.pause_stat.duration   = snapshot.time_since_stop_the_world();

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

