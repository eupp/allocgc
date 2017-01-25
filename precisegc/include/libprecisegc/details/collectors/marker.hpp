#ifndef DIPLOMA_MARKER_HPP
#define DIPLOMA_MARKER_HPP

#include <vector>
#include <stack>
#include <mutex>
#include <memory>
#include <atomic>
#include <condition_variable>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/collectors/remset.hpp>
#include <libprecisegc/details/gc_cell.hpp>
#include <libprecisegc/details/collectors/gc_tagging.hpp>
#include <libprecisegc/details/collectors/memory_index.hpp>
#include <libprecisegc/details/collectors/packet_manager.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/utils/scoped_thread.hpp>

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
    marker(packet_manager* manager, remset* rset)
        : m_packet_manager(manager)
        , m_remset(rset)
        , m_running_threads_cnt(0)
        , m_concurrent_flag(false)
        , m_done(false)
    {}

    ~marker()
    {
        m_done.store(true, std::memory_order_release);
    }

    template <typename Traceable>
    void trace_roots(Traceable&& tracer)
    {
        auto output_packet = m_packet_manager->pop_output_packet();
        tracer.trace([this, &output_packet] (gc_handle* root) {
            byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(*root);
            byte* obj_start = gc_tagging::get_obj_start(ptr);
            if (obj_start) {
                gc_cell cell = gc_index_object(obj_start);
                cell.set_mark(true);
                push_root_to_packet(cell, output_packet);
            }
            logging::debug() << "root: " << (void*) root;
        });
        m_packet_manager->push_packet(std::move(output_packet));
    }

    template <typename Traceable>
    void trace_pins(Traceable&& tracer)
    {
        auto output_packet = m_packet_manager->pop_output_packet();
        tracer.trace([this, &output_packet] (byte* ptr) {
            if (ptr) {
                gc_cell cell = gc_index_object(ptr);
                cell.set_mark(true);
                cell.set_pin(true);
                push_root_to_packet(cell, output_packet);
            }
            logging::debug() << "pin: " << (void*) ptr;
        });
        m_packet_manager->push_packet(std::move(output_packet));
    }

    void trace_remset()
    {
        assert(m_remset);
        m_remset->flush_buffers();
        auto output_packet = m_packet_manager->pop_output_packet();
        for (auto it = m_remset->begin(); it != m_remset->end(); ++it) {
            byte* obj_start = gc_tagging::get_obj_start(*it);
            if (obj_start) {
                gc_cell cell = gc_index_object(obj_start);
                cell.set_mark(true);
                cell.set_pin(true);
                push_root_to_packet(cell, output_packet);
            }
            logging::debug() << "remset ptr: " << (void*) gc_tagging::get(*it);
        }
        m_remset->clear();
        m_packet_manager->push_packet(std::move(output_packet));
    }

    void mark()
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

    void concurrent_mark(size_t threads_num)
    {
        m_concurrent_flag = true;
        m_running_threads_cnt = threads_num;
        m_workers.resize(threads_num);
        for (auto& worker: m_workers) {
            worker = std::thread(&marker::worker_routine, this);
        }
    }
private:
    static const size_t POP_REMSET_COUNT = 16;
    static const size_t POP_OUTPUT_ATTEMPTS = 2;

    void worker_routine()
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

    void push_root_to_packet(const gc_cell& cell, packet_manager::mark_packet_handle& output_packet)
    {
        if (output_packet->is_full()) {
            m_packet_manager->push_packet(std::move(output_packet));
            output_packet = m_packet_manager->pop_output_packet();
        }
        output_packet->push(cell);
    }

    void push_to_packet(const gc_cell& cell, packet_manager::mark_packet_handle& output_packet)
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

    void trace(gc_handle* handle, packet_manager::mark_packet_handle& output_packet)
    {
        byte* ptr = gc_handle_access::get<std::memory_order_acquire>(*handle);
        byte* obj_start = gc_tagging::get_obj_start(ptr);
        if (obj_start) {
//            logging::debug() << "trace object at address: " << (void*) obj_start;
            gc_cell cell = gc_index_object(obj_start);
            if (!cell.get_mark()) {
                cell.set_mark(true);
                push_to_packet(cell, output_packet);
            }
        }
    }

    packet_manager* m_packet_manager;
    remset* m_remset;
    std::vector<utils::scoped_thread> m_workers;
    std::atomic<size_t> m_running_threads_cnt;
    std::atomic<bool> m_concurrent_flag;
    std::atomic<bool> m_done;
};

}}}

#endif //DIPLOMA_MARKER_HPP
