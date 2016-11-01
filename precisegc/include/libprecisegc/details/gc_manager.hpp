#ifndef DIPLOMA_GC_MANAGER_HPP
#define DIPLOMA_GC_MANAGER_HPP

#include <atomic>
#include <iostream>

#include <libprecisegc/gc_stat.hpp>
#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details {

class gc_manager : public gc_launcher, private utils::noncopyable, private utils::nonmovable
{
public:
    gc_manager(gc_strategy* strategy, bool print_stats_flag = false, const std::ostream& stream = std::clog);

    void gc(gc_phase phase) override;

    void register_allocation(size_t size);
    void register_page(const byte* page, size_t size);
    void deregister_page(const byte* page, size_t size);

    gc_state state() const override;
    gc_stat  stats() const;

    gc_strategy* get_strategy() const;
    void set_strategy(gc_strategy* strategy);

    bool print_stats_flag() const;
    void set_print_stats_flag(bool value);
private:
    bool check_gc_phase(gc_phase phase);
    void register_gc_run(const gc_run_stats& stats);
    void print_gc_run_stats(const gc_run_stats& stats);

    gc_strategy* m_strategy;
    std::atomic<size_t> m_heap_size;
    std::atomic<size_t> m_last_heap_size;
    std::atomic<gc_clock::duration> m_last_gc_time;
    std::atomic<gc_clock::duration> m_last_gc_duration;
    std::atomic<size_t> m_gc_cnt;
    std::atomic<size_t> m_gc_time;
    std::ostream m_print_stream;
    bool m_print_stats_flag;
};

}}

#endif //DIPLOMA_GC_MANAGER_HPP
