#ifndef DIPLOMA_RECORDER_HPP
#define DIPLOMA_RECORDER_HPP

#include <atomic>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/gc_clock.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details {

class recorder : private utils::noncopyable, private utils::nonmovable
{
public:
    recorder();

    void register_alloc_request();
    void register_allocation(size_t size);
    void register_page(const byte* page, size_t size);
    void register_pause(const gc_pause_stat& pause_stat);
    void register_sweep(const gc_sweep_stat& sweep_stat, const gc_pause_stat& pause_stat);

    gc_stat stat() const;

    size_t gc_cycles_count() const;
    size_t pause_count() const;
private:
    std::atomic<size_t> m_heap_size;
    std::atomic<size_t> m_last_heap_size;
    std::atomic<gc_clock::duration> m_last_alloc_time;
    std::atomic<gc_clock::duration> m_last_gc_time;
    std::atomic<gc_clock::duration> m_last_gc_duration;
    std::atomic<size_t> m_gc_cnt;
    std::atomic<size_t> m_pause_cnt;
};

}}

#endif //DIPLOMA_RECORDER_HPP
