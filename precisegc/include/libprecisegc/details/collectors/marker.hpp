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
#include <libprecisegc/details/collectors/gc_tagging.hpp>
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

    void trace_roots(const threads::world_snapshot& snapshot, static_root_set* static_roots);
    void trace_pins(const threads::world_snapshot& snapshot);
    void trace_remset();

    // tmp for debug
    template <typename Functor>
    void trace_roots(Functor&& tracer, static_root_set* static_roots, int)
    {
        using namespace allocators;

        auto output_packet = m_packet_manager->pop_output_packet();

        auto trace_cb = [this, &output_packet] (gc_handle* root) {
            byte* ptr = gc_tagging::clear_root_bit(gc_handle_access::get<std::memory_order_relaxed>(*root));
            if (ptr) {
                gc_memory_descriptor* descr = memory_index::get_descriptor(ptr).to_gc_descriptor();
                byte* obj_start = gc_box::get_obj_start(descr->cell_start(ptr));
                gc_cell cell = allocators::memory_index::get_gc_cell(obj_start);
                cell.set_mark(true);
                push_root_to_packet(cell, output_packet);

                logging::info() << "root: " << (void*) root << "; point to: " << (void*) obj_start;
            }
        };

        if (static_roots) {
            logging::info() << "Static roots count = " << static_roots->size();
            static_roots->trace(gc_trace_callback{std::ref(trace_cb)});
        }

        tracer(gc_trace_callback{std::ref(trace_cb)});

        m_packet_manager->push_packet(std::move(output_packet));
    }

    // tmp for debug
    template <typename Functor>
    void trace_pins(Functor&& tracer, int)
    {
        auto output_packet = m_packet_manager->pop_output_packet();

        auto trace_cb = [this, &output_packet] (byte* ptr) {
            if (ptr) {
                gc_cell cell = allocators::memory_index::get_gc_cell(ptr);
                cell.set_mark(true);
                cell.set_pin(true);
                push_root_to_packet(cell, output_packet);

                logging::debug() << "pin: " << (void*) ptr;
            }
        };

        tracer(gc_trace_pin_callback{std::ref(trace_cb)});

        m_packet_manager->push_packet(std::move(output_packet));
    }

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
    std::vector<utils::scoped_thread> m_workers;
    std::atomic<size_t> m_running_threads_cnt;
    std::atomic<bool> m_concurrent_flag;
    std::atomic<bool> m_done;
};

}}}

#endif //DIPLOMA_MARKER_HPP
