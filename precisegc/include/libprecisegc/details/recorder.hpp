#ifndef DIPLOMA_RECORDER_HPP
#define DIPLOMA_RECORDER_HPP

#include <atomic>

#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details {

class recorder : private utils::noncopyable, private utils::nonmovable
{
public:
    recorder();

    void register_alloc_request();
    void register_allocation(size_t size);
    void register_gc(const gc_sweep_stat& sweep_stat, const gc_pause_stat& pause_stat);
    gc_stat stat() const;
private:
    std::atomic<size_t> m_heap_size;
    std::atomic<size_t> m_heap_gain;
    gc_time_point m_last_alloc_tp;
    gc_time_point m_last_gc_tp;
};

}}

#endif //DIPLOMA_RECORDER_HPP
