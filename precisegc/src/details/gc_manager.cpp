#include <libprecisegc/details/gc_manager.hpp>

#include <cassert>
#include <unordered_map>

#include <libprecisegc/details/logging.hpp>
#include <libprecisegc/details/allocators/gc_core_allocator.hpp>

namespace precisegc { namespace details {

gc_manager::gc_manager(gc_strategy* strategy, bool print_stats_flag, const std::ostream& stream)
    : m_strategy(strategy)
    , m_gc_cnt(0)
    , m_gc_time(0)
    , m_print_stream(stream.rdbuf())
    , m_print_stats_flag(print_stats_flag)
{
    m_print_stream.rdbuf()->pubsetbuf(0, 0);
}

void gc_manager::gc(const gc_options& opt)
{
    assert(m_strategy);

    if (!check_gc_kind(opt.kind)) {
        return;
    }

    gc_run_stat run_stats = m_strategy->gc(opt);
    if (run_stats.pause_stat.type == gc_pause_type::SKIP) {
        return;
    }

    logging::info() << "GC pause (" << gc_pause_type_to_str(run_stats.pause_stat.type)
                    << ") duration "<< duration_to_str(run_stats.pause_stat.duration);
    register_gc_run(run_stats);
    print_gc_run_stats(run_stats);
}

void gc_manager::register_allocation(size_t size)
{
    return;
}

void gc_manager::register_page(const byte* page, size_t size)
{
//    logging::debug() << "Page allocated: addr=" << (void*) page << ", size=" << size;
}

void gc_manager::deregister_page(const byte* page, size_t size)
{
//    logging::debug() << "Page deallocated: addr=" << (void*) page << ", size=" << size;
}

gc_stat gc_manager::stats() const
{
    gc_stat stats = {
            .gc_count   = m_gc_cnt,
            .gc_time    = m_gc_time
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

bool gc_manager::check_gc_kind(gc_kind kind)
{
    if (kind == gc_kind::CONCURRENT_MARK && !m_strategy->info().incremental_flag) {
        return false;
    }
    return true;
}

void gc_manager::register_gc_run(const gc_run_stat& stats)
{
    ++m_gc_cnt;
    m_gc_time += stats.pause_stat.duration;
}

void gc_manager::print_gc_run_stats(const gc_run_stat& stats)
{
    if (!m_print_stats_flag) {
        return;
    }

    static std::string text =
            "****************************************************************************\n"
            "   GC SUMMARY                                                               \n"
            "pause type: xxxxxxxxx yy                                                    \n"
            "pause time: xxxxxxxxx yy                                                    \n"
            "heap size:  xxxxxxxxx yy                                                    \n"
            "occupied:   xxxxxxxxx yy                                                    \n"
            "live:       xxxxxxxxx yy                                                    \n"
            "swept:      xxxxxxxxx yy                                                    \n"
            "copied:     xxxxxxxxx yy                                                    \n"
            "****************************************************************************\n";

    static const std::string placeholder = "xxxxxxxxx yy";

    static const size_t pause_type_pos = text.find(placeholder, 0);
    static const size_t pause_time_pos = text.find(placeholder, pause_type_pos + placeholder.size());
    static const size_t heap_size_pos  = text.find(placeholder, pause_time_pos + placeholder.size());
    static const size_t occupied_pos   = text.find(placeholder, heap_size_pos + placeholder.size());
    static const size_t live_pos       = text.find(placeholder, occupied_pos + placeholder.size());
    static const size_t swept_pos      = text.find(placeholder, live_pos + placeholder.size());
    static const size_t copied_pos     = text.find(placeholder, swept_pos + placeholder.size());

    std::string gc_type_str = gc_pause_type_to_str(stats.pause_stat.type);
    gc_type_str.resize(20, ' ');

    text.replace(pause_type_pos, gc_type_str.size(), gc_type_str);
    text.replace(pause_time_pos, placeholder.size(), duration_to_str(stats.pause_stat.duration, 9));
    text.replace(heap_size_pos, placeholder.size(), heapsize_to_str(stats.heap_stat.mem_before_gc, 9));
    text.replace(occupied_pos, placeholder.size(), heapsize_to_str(stats.heap_stat.mem_occupied, 9));
    text.replace(live_pos, placeholder.size(), heapsize_to_str(stats.heap_stat.mem_live, 9));
    text.replace(swept_pos, placeholder.size(), heapsize_to_str(stats.heap_stat.mem_freed, 9));
    text.replace(copied_pos, placeholder.size(), heapsize_to_str(stats.heap_stat.mem_copied, 9));

    m_print_stream << text;
}

}}
