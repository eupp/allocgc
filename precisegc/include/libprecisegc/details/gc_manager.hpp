#ifndef DIPLOMA_GC_MANAGER_HPP
#define DIPLOMA_GC_MANAGER_HPP

#include <atomic>
#include <iostream>

#include <libprecisegc/gc_stat.hpp>
#include <libprecisegc/gc_common.hpp>
#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details {

class gc_manager : public gc_launcher, private utils::noncopyable, private utils::nonmovable
{
public:
    gc_manager(gc_strategy* strategy, bool print_stats_flag = false, const std::ostream& stream = std::clog);

    void gc(const gc_options& opt) override;

    void register_allocation(size_t size);
    void register_page(const byte* page, size_t size);
    void deregister_page(const byte* page, size_t size);

    gc_stat  stats() const;

    gc_strategy* get_strategy() const;
    void set_strategy(gc_strategy* strategy);

    bool print_stats_flag() const;
    void set_print_stats_flag(bool value);
private:
    bool check_gc_kind(gc_kind kind);
    void register_gc_run(const gc_run_stats& stats);
    void print_gc_run_stats(const gc_run_stats& stats);

    gc_strategy* m_strategy;
    size_t m_gc_cnt;
    gc_duration m_gc_time;
    std::ostream m_print_stream;
    bool m_print_stats_flag;
};

}}

#endif //DIPLOMA_GC_MANAGER_HPP
