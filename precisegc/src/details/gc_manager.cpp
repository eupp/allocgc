#include <libprecisegc/details/gc_manager.hpp>

#include <cassert>

#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details {

gc_manager::gc_manager(gc_strategy* strategy, bool print_stats_flag, const std::ostream& stream)
    : m_strategy(strategy)
    , m_heap_size(0)
    , m_last_heap_size(0)
    , m_last_gc_time(gc_clock::now().time_since_epoch())
    , m_last_gc_duration(gc_clock::duration(0))
    , m_gc_cnt(0)
    , m_gc_time(0)
    , m_print_stream(stream.rdbuf())
    , m_print_stats_flag(print_stats_flag)
{
    m_print_stream.rdbuf()->pubsetbuf(0, 0);
}

void gc_manager::gc(gc_phase phase)
{
    assert(m_strategy);

    if (!check_gc_phase(phase)) {
        return;
    }

    gc_options options = {
            .phase      = phase
    };
    gc_run_stats run_stats = m_strategy->gc(options);

    logging::info() << "GC pause (" << gc_type_to_str(run_stats.type)
                    << ") duration "<< duration_to_str(run_stats.pause_duration);
    register_gc_run(run_stats);
    print_gc_run_stats(run_stats);
}

void gc_manager::register_allocation(size_t size)
{
    return;
}

void gc_manager::register_page(const byte* page, size_t size)
{
    logging::debug() << "Page allocated: addr=" << (void*) page << ", size=" << size;
    m_heap_size.fetch_add(size, std::memory_order_acq_rel);
}

void gc_manager::deregister_page(const byte* page, size_t size)
{
    logging::debug() << "Page deallocated: addr=" << (void*) page << ", size=" << size;
}

gc_state gc_manager::state() const
{
    size_t heap_size = m_heap_size.load(std::memory_order_acquire);
    gc_state state = {
            .heap_size              = heap_size,
            .heap_gain              = heap_size - m_last_heap_size.load(std::memory_order_acquire),
            .last_gc_time           = m_last_gc_time.load(std::memory_order_acquire),
            .last_gc_duration       = m_last_gc_duration.load(std::memory_order_acquire)
    };
    return state;
}

gc_stat gc_manager::stats() const
{
    gc_stat stats = {
            .gc_count   = m_gc_cnt.load(std::memory_order_acquire),
            .gc_time    = gc_clock::duration(m_gc_time.load(std::memory_order_acquire))
    };
    return stats;
}

gc_strategy* gc_manager::get_strategy() const
{
    return m_strategy;
}

void gc_manager::set_strategy(gc_strategy* strategy)
{
    m_strategy = strategy;
}

bool gc_manager::print_stats_flag() const
{
    return m_print_stats_flag;
}

void gc_manager::set_print_stats_flag(bool value)
{
    m_print_stats_flag = value;
}

bool gc_manager::check_gc_phase(gc_phase phase)
{
    return (phase == gc_phase::MARK && m_strategy->info().incremental_flag) || phase == gc_phase::COLLECT;
}

void gc_manager::register_gc_run(const gc_run_stats& stats)
{
    assert(stats.mem_swept <= m_heap_size);

    gc_clock::duration now = gc_clock::now().time_since_epoch();

    // we use std::memory_order_relaxed here because it is expected that this method will be called during stop-the-world pause
    m_heap_size.fetch_sub(stats.mem_swept, std::memory_order_relaxed);
    m_last_heap_size.store(m_heap_size.load(std::memory_order_relaxed), std::memory_order_relaxed);
    m_last_gc_time.store(now, std::memory_order_relaxed);
    m_last_gc_duration.store(stats.pause_duration, std::memory_order_relaxed);

    m_gc_cnt.fetch_add(1, std::memory_order_relaxed);
    m_gc_time.fetch_add(stats.pause_duration.count(), std::memory_order_relaxed);
}

void gc_manager::print_gc_run_stats(const gc_run_stats& stats)
{
    if (!m_print_stats_flag) {
        return;
    }

    static std::string text =
            "****************************************************************************\n"
            "   GC SUMMARY                                                               \n"
            "pause type: xxxx yy                                                         \n"
            "pause time: xxxx yy                                                         \n"
            "heap size:  xxxx yy                                                         \n"
            "swept:      xxxx yy                                                         \n"
            "copied:     xxxx yy                                                         \n"
            "****************************************************************************\n";

    static const std::string placeholder = "xxxx yy";

    static const size_t pause_type_pos = text.find(placeholder, 0);
    static const size_t pause_time_pos = text.find(placeholder, pause_type_pos + placeholder.size());
    static const size_t heap_size_pos  = text.find(placeholder, pause_time_pos + placeholder.size());
    static const size_t swept_pos      = text.find(placeholder, heap_size_pos + placeholder.size());
    static const size_t copied_pos     = text.find(placeholder, swept_pos + placeholder.size());

    std::string gc_type_str = gc_type_to_str(stats.type);
    gc_type_str.resize(20, ' ');

    text.replace(pause_type_pos, gc_type_str.size(), gc_type_str);
    text.replace(pause_time_pos, placeholder.size(), duration_to_str(stats.pause_duration, 4));
    text.replace(heap_size_pos, placeholder.size(), heapsize_to_str(m_heap_size.load(std::memory_order_acquire), 4));
    text.replace(swept_pos, placeholder.size(), heapsize_to_str(stats.mem_swept, 4));
    text.replace(copied_pos, placeholder.size(), heapsize_to_str(stats.mem_copied, 4));

    m_print_stream << text;
}

}}
