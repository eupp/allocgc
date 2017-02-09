#include <libprecisegc/details/collectors/marker.hpp>

#include <libprecisegc/details/collectors/gc_tagging.hpp>
#include <libprecisegc/details/collectors/memory_index.hpp>

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
    trace_roots([&snapshot] (const gc_trace_callback& cb) { snapshot.trace_roots(cb); }, static_roots, 0);
}

void marker::trace_pins(const threads::world_snapshot& snapshot)
{
    trace_pins([&snapshot] (const gc_trace_pin_callback& cb) { snapshot.trace_pins(cb); }, 0);
}

void marker::trace_remset()
{
    assert(m_remset);
    m_remset->flush_buffers();
    auto output_packet = m_packet_manager->pop_output_packet();
    for (auto it = m_remset->begin(); it != m_remset->end(); ++it) {
        byte* obj_start = gc_tagging::get_obj_start(*it);
        if (obj_start) {
            gc_cell cell = gc_index_object(obj_start);
            cell.set_mark(true);
            push_root_to_packet(cell, output_packet);

            logging::debug() << "remset ptr: " << (void*) gc_tagging::get(*it);
        }
    }
    m_remset->clear();
    m_packet_manager->push_packet(std::move(output_packet));
}

void marker::mark()
{
    m_concurrent_flag = false;
    ++m_running_threads_cnt;
    worker_routine();
    for (auto& worker: m_workers) {
        if (worker.get_id() != std::this_thread::get_id()) {
            worker.join();
        }
    }
}

void marker::concurrent_mark(size_t threads_num)
{
    m_concurrent_flag = true;
    m_running_threads_cnt = threads_num;
    m_workers.resize(threads_num);
    for (auto& worker: m_workers) {
        worker = std::thread(&marker::worker_routine, this);
    }
}

void marker::worker_routine()
{
    auto input_packet = m_packet_manager->pop_input_packet();
    packet_manager::mark_packet_handle output_packet = nullptr;
    while (true) {
        while (!input_packet) {

            if (m_remset && output_packet) {
                for (size_t i = 0; i < POP_REMSET_COUNT; ++i) {
                    byte* ptr = m_remset->get();
                    if (ptr) {
                        push_to_packet(gc_index_object(ptr), output_packet);
                    } else {
                        break;
                    }
                }

                input_packet = m_packet_manager->pop_input_packet();
                if (input_packet) {
                    break;
                }
            }

            if (output_packet) {
                m_packet_manager->push_packet(std::move(output_packet));
            }
            if (m_packet_manager->is_no_input() || m_done.load(std::memory_order_acquire)) {
//                    if (--m_running_threads_cnt == 0 && m_concurrent_flag) {
//                            gc_initiation_point(initiation_point_type::CONCURRENT_MARKING_FINISHED,
//                                                initiation_point_data::create_empty_data());
//                    }
                return;
            }

            std::this_thread::yield();
            input_packet = m_packet_manager->pop_input_packet();
        }
        if (!output_packet) {
            output_packet = m_packet_manager->pop_output_packet();
        }
        auto trace_cb = [this, &output_packet] (gc_handle* handle) {
            trace(handle, output_packet);
        };
        while (!input_packet->is_empty()) {
            gc_cell cell = input_packet->pop();
            cell.trace(gc_trace_callback{std::ref(trace_cb)});
        }

        auto empty_packet = std::move(input_packet);
        input_packet = m_packet_manager->pop_input_packet();
        m_packet_manager->push_packet(std::move(empty_packet));
    }
}

void marker::push_root_to_packet(const gc_cell& cell, packet_manager::mark_packet_handle& output_packet)
{
    if (output_packet->is_full()) {
        m_packet_manager->push_packet(std::move(output_packet));
        output_packet = m_packet_manager->pop_output_packet();
    }
    output_packet->push(cell);
}

void marker::push_to_packet(const gc_cell& cell, packet_manager::mark_packet_handle& output_packet)
{
    if (output_packet->is_full()) {
        size_t attempts = 0;
        do {
            auto new_packet = m_packet_manager->pop_output_packet();
            m_packet_manager->push_packet(std::move(output_packet));
            output_packet = std::move(new_packet);
            ++attempts;
        } while (!output_packet && attempts < POP_OUTPUT_ATTEMPTS);
        if (attempts == POP_OUTPUT_ATTEMPTS) {
            throw marking_overflow_exception();
        }
    }
    output_packet->push(cell);
}

void marker::trace(gc_handle* handle, packet_manager::mark_packet_handle& output_packet)
{
    byte* ptr = gc_handle_access::get<std::memory_order_acquire>(*handle);
    byte* obj_start = gc_tagging::get_obj_start(ptr);
    if (obj_start) {
        gc_cell cell = gc_index_object(obj_start);
        if (!cell.get_mark()) {
            cell.set_mark(true);
            push_to_packet(cell, output_packet);
        }
    }
}

}}}
