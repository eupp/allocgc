#include <libprecisegc/details/collectors/marker.hpp>

namespace precisegc { namespace details { namespace collectors {

marker::marker(packet_manager* manager, remset* rset)
    : m_packet_manager(manager)
    , m_remset(rset)
    , m_running_threads_cnt(0)
    , m_concurrent_flag(false)
    , m_done(false)
{}

marker::~marker()
{
    m_done.store(true, std::memory_order_release);
}

void marker::trace_roots(const threads::world_snapshot& snapshot, static_root_set* static_roots)
{
    auto output_packet = m_packet_manager->pop_output_packet();

    auto trace_cb = [this, &output_packet] (gc_handle* root) {
        byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(*root);
        byte* obj_start = gc_tagging::get_obj_start(ptr);
        if (obj_start) {
            gc_cell cell = gc_index_object(obj_start);
            cell.set_mark(true);
            push_root_to_packet(cell, output_packet);

            logging::debug() << "root: " << (void*) root;
        }
    };

    logging::info() << "Static roots count = " << static_roots->size();
    static_roots->trace(gc_trace_callback{std::ref(trace_cb)});

    snapshot.trace_roots(gc_trace_callback{std::ref(trace_cb)});

    m_packet_manager->push_packet(std::move(output_packet));
}

void marker::trace_pins(const threads::world_snapshot& snapshot)
{
    auto output_packet = m_packet_manager->pop_output_packet();

    auto trace_cb = [this, &output_packet] (byte* ptr) {
        if (ptr) {
            gc_cell cell = gc_index_object(ptr);
            cell.set_mark(true);
            cell.set_pin(true);
            push_root_to_packet(cell, output_packet);

            logging::debug() << "pin: " << (void*) ptr;
        }
    };

    snapshot.trace_pins(gc_trace_callback{std::ref(trace_cb)});

    m_packet_manager->push_packet(std::move(output_packet));
}

}}}
