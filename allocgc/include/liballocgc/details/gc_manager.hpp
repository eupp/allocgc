#ifndef ALLOCGC_GC_MANAGER_HPP
#define ALLOCGC_GC_MANAGER_HPP

#include <atomic>
#include <iostream>

#include <liballocgc/gc_common.hpp>
#include <liballocgc/gc_strategy.hpp>
#include <liballocgc/details/gc_interface.hpp>
#include <liballocgc/details/utils/utility.hpp>

namespace allocgc { namespace details {

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
    void register_gc_run(const gc_run_stat& stats);
    void print_gc_run_stats(const gc_run_stat& stats);

    gc_strategy* m_strategy;
    size_t m_gc_cnt;
    gc_clock::duration m_gc_time;
    std::ostream m_print_stream;
    bool m_print_stats_flag;
};

}}

#endif //ALLOCGC_GC_MANAGER_HPP
