#ifndef DIPLOMA_MARKER_HPP
#define DIPLOMA_MARKER_HPP

#include <vector>
#include <mutex>
#include <memory>
#include <atomic>
#include <condition_variable>

#include <libprecisegc/details/gc_cell.hpp>
#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/collectors/remset.hpp>
#include <libprecisegc/details/collectors/packet_manager.hpp>
#include <libprecisegc/details/allocators/memory_index.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <libprecisegc/details/utils/scoped_thread.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace collectors {

class marking_overflow_exception : public gc_exception
{
public:
    marking_overflow_exception()
        : gc_exception("Mark stack overflow")
    {}
};

class marker
{
public:
    marker(packet_manager* manager, remset* rset);

    ~marker();

    void add_root(const gc_cell& cell);

    void trace_remset();

    void mark();
    void concurrent_mark(size_t threads_num);
private:
    static const size_t POP_REMSET_COUNT = 16;
    static const size_t POP_OUTPUT_ATTEMPTS = 2;

    void worker_routine();

    void push_root_to_packet(const gc_cell& cell, packet_manager::mark_packet_handle& output_packet);
    void push_to_packet(const gc_cell& cell, packet_manager::mark_packet_handle& output_packet);

    void trace(gc_handle* handle, packet_manager::mark_packet_handle& output_packet);

    packet_manager* m_packet_manager;
    remset* m_remset;
    packet_manager::mark_packet_handle m_roots_packet;
    std::vector<utils::scoped_thread> m_workers;
    std::atomic<size_t> m_running_threads_cnt;
    std::atomic<bool> m_concurrent_flag;
    std::atomic<bool> m_done;
};

}}}

#endif //DIPLOMA_MARKER_HPP
