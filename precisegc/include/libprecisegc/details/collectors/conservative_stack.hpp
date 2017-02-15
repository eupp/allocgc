#ifndef DIPLOMA_CONSERVATIVE_STACK_HPP
#define DIPLOMA_CONSERVATIVE_STACK_HPP

#include <libprecisegc/gc_common.hpp>
#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/allocators/memory_index.hpp>

namespace precisegc { namespace details { namespace collectors {

class conservative_stack
{
public:
    inline conservative_stack(std::thread::id id, byte* stack_start_addr,
                              byte* stack_start_addr_approx, byte* stack_end_addr_approx)
        : m_id(id)
        , m_stack_start(reinterpret_cast<gc_handle*>(stack_start_addr))
        , m_stack_end(reinterpret_cast<gc_handle*>(stack_end_addr_approx))
    {}

    inline void register_root(gc_handle* root)
    {
        return;
    }

    inline void deregister_root(gc_handle* root)
    {
        gc_handle_access::set<std::memory_order_relaxed>(*root, nullptr);
    }

    void trace(const gc_trace_callback& cb) const
    {
        using namespace allocators;

        gc_handle* stack_end = reinterpret_cast<gc_handle*>(threads::stw_manager::instance().get_thread_stack_end(m_id));
        for (gc_handle* it = m_stack_start; it != stack_end; STACK_DIRECTION == stack_growth_direction::UP
                                                              ? ++it : --it) {
            byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(*it);
            memory_descriptor descr = memory_index::get_descriptor(ptr);
            if (descr.is_gc_heap_descriptor()) {
                cb(it);
            }
        }
    }
private:
    std::thread::id m_id;
    gc_handle* m_stack_start;
    gc_handle* m_stack_end;
};

}}}

#endif //DIPLOMA_CONSERVATIVE_STACK_HPP
