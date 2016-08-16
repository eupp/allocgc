#ifndef DIPLOMA_MARKER_HPP
#define DIPLOMA_MARKER_HPP

#include <vector>
#include <stack>
#include <mutex>
#include <memory>
#include <atomic>
#include <condition_variable>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/collectors/packet_manager.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/ptrs/trace_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/utils/scoped_thread.hpp>

namespace precisegc { namespace details { namespace collectors {

class marker
{
public:
    marker(packet_manager* manager)
        : m_packet_manager(manager)
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
        tracer.trace([this, &output_packet] (ptrs::gc_untyped_ptr* p) {
            managed_ptr mp = managed_ptr((byte*) p->get());
            if (mp) {
                mp.set_mark(true);
                push_root_to_packet(mp, output_packet);
            }
        });
        m_packet_manager->push_packet(std::move(output_packet));
    }

    template <typename Traceable>
    void trace_pins(Traceable&& tracer)
    {
        auto output_packet = m_packet_manager->pop_output_packet();
        tracer.trace([this, &output_packet] (void* p) {
            managed_ptr mp = managed_ptr((byte*) p);
            if (mp) {
                mp.set_mark(true);
                mp.set_pin(true);
                push_root_to_packet(mp, output_packet);
            }
        });
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
    void worker_routine()
    {
        auto input_packet = m_packet_manager->pop_input_packet();
        std::unique_ptr<mark_packet> output_packet;
        while (true) {
            while (!input_packet) {
                if (output_packet) {
                    m_packet_manager->push_packet(std::move(output_packet));
                }
                if (m_packet_manager->is_no_input() || m_done.load(std::memory_order_acquire)) {
                    if (--m_running_threads_cnt == 0 && m_concurrent_flag) {
//                        try{
                            gc_initiation_point(initiation_point_type::CONCURRENT_MARKING_FINISHED,
                                                initiation_point_data::create_empty_data());
//                        } catch (std::system_error& exc) {
//                            std::cout << exc.code() << ": " << exc.what() << std::endl;
//                            throw;
//                        }


                    }
                    return;
                }
                std::this_thread::yield();
                input_packet = m_packet_manager->pop_input_packet();
            }
            if (!output_packet) {
                output_packet = m_packet_manager->pop_output_packet();
            }
            while (!input_packet->is_empty()) {
                ptrs::trace_ptr(input_packet->pop(), [this, &output_packet] (const managed_ptr& child) {
                    if (output_packet->is_full()) {
                        auto new_packet = m_packet_manager->pop_output_packet();
                        m_packet_manager->push_packet(std::move(output_packet));
                        output_packet = std::move(new_packet);
                    }
                    output_packet->push(child);
                });
            }

            auto empty_packet = std::move(input_packet);
            input_packet = m_packet_manager->pop_input_packet();
            m_packet_manager->push_packet(std::move(empty_packet));
        }
    }

    void push_root_to_packet(const managed_ptr& mp, std::unique_ptr<mark_packet>& output_packet)
    {
        if (output_packet->is_full()) {
            m_packet_manager->push_packet(std::move(output_packet));
            output_packet = m_packet_manager->pop_output_packet();
        }
        output_packet->push(mp);
    }

    packet_manager* m_packet_manager;
    std::vector<utils::scoped_thread> m_workers;
    std::atomic<size_t> m_running_threads_cnt;
    std::atomic<bool> m_concurrent_flag;
    std::atomic<bool> m_done;
};

}}}

#endif //DIPLOMA_MARKER_HPP
