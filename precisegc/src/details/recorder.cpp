#include <libprecisegc/details/recorder.hpp>

namespace precisegc { namespace details {

recorder::recorder()
    : m_heap_size(0)
    , m_last_heap_size(0)
    , m_last_alloc_time(gc_clock::now().time_since_epoch())
    , m_last_gc_time(gc_clock::now().time_since_epoch())
    , m_last_gc_duration(gc_clock::duration(0))
    , m_gc_cnt(0)
    , m_pause_cnt(0)
{}

void recorder::register_alloc_request()
{
//    m_last_alloc_time.store(gc_clock::now().time_since_epoch(), std::memory_order_release);
}

void recorder::register_allocation(size_t size)
{
    m_heap_size.fetch_add(size, std::memory_order_acq_rel);
}

void recorder::register_pause(const gc_pause_stat& pause_stat)
{
    if (pause_stat.type != gc_pause_type::NO_PAUSE) {
        m_pause_cnt.fetch_add(1, std::memory_order_acq_rel);
    }
}

void recorder::register_sweep(const gc_sweep_stat& sweep_stat, const gc_pause_stat& pause_stat)
{
    gc_clock::duration now = gc_clock::now().time_since_epoch();
    size_t freed = sweep_stat.shrunk + sweep_stat.swept;

    // we use std::memory_order_relaxed here because it is expected that this method will be called during stop-the-world pause
    m_heap_size.fetch_sub(freed, std::memory_order_relaxed);
    m_last_heap_size.store(m_heap_size.load(std::memory_order_relaxed), std::memory_order_relaxed);
    m_last_alloc_time.store(now, std::memory_order_relaxed);
    m_last_gc_time.store(now, std::memory_order_relaxed);
    m_last_gc_duration.store(pause_stat.duration, std::memory_order_relaxed);

    m_gc_cnt.fetch_add(1, std::memory_order_relaxed);
    if (pause_stat.type != gc_pause_type::NO_PAUSE) {
        m_pause_cnt.fetch_add(1, std::memory_order_relaxed);
    }
}

gc_stat recorder::stat() const
{
    gc_stat stat;

    gc_clock::duration now = gc_clock::now().time_since_epoch();

    stat.heap_size              = m_heap_size.load(std::memory_order_acquire);
    stat.heap_gain              = stat.heap_size - m_last_heap_size.load(std::memory_order_acquire);
//    stat.last_alloc_timediff    = now - m_last_alloc_time.load(std::memory_order_acquire);
    stat.last_alloc_timediff    = gc_clock::duration(0);
    stat.last_gc_timediff       = now - m_last_gc_time.load(std::memory_order_acquire);
    stat.last_gc_duration       = m_last_gc_duration.load(std::memory_order_acquire);

    return stat;
}

size_t recorder::gc_cycles_count() const
{
    return m_gc_cnt.load(std::memory_order_acquire);
}

size_t recorder::pause_count() const
{
    return m_pause_cnt.load(std::memory_order_acquire);
}

}}
