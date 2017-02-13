#ifndef DIPLOMA_GC_NEW_STACK_HPP
#define DIPLOMA_GC_NEW_STACK_HPP

#include <cassert>

#include <libprecisegc/details/collectors/gc_new_stack_entry.hpp>
#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_interface.hpp>

namespace precisegc { namespace details { namespace collectors {

class gc_new_stack : private utils::noncopyable, private utils::nonmovable
{
public:
    gc_new_stack()
        : m_stack_top(nullptr)
        , m_depth(0)
    {}

    void register_stack_entry(gc_new_stack_entry* stack_entry)
    {
        new (&stack_entry->offsets) std::vector<size_t>();
        stack_entry->prev = m_stack_top;
        m_stack_top = stack_entry;
        ++m_depth;
    }

    void deregister_stack_entry(gc_new_stack_entry* stack_entry)
    {
        assert(m_stack_top == stack_entry);
        stack_entry->offsets.~vector<size_t>();
        m_stack_top = m_stack_top->prev;
        --m_depth;
    }

    void register_child(gc_handle* child)
    {
        assert(m_stack_top);
        if (m_stack_top->meta_requested) {
            std::uintptr_t top  = reinterpret_cast<std::uintptr_t>(m_stack_top->obj_start);
            std::uintptr_t curr = reinterpret_cast<std::uintptr_t>(child);
            m_stack_top->offsets.push_back(curr - top);
        }
    }

    size_t size() const
    {
        return m_depth;
    }

    void trace(const gc_trace_pin_callback& cb) const
    {
        gc_new_stack_entry* curr = m_stack_top;
        while (curr) {
            cb(curr->obj_start);
            curr = curr->prev;
        }
    }
private:
    gc_new_stack_entry* m_stack_top;
    size_t m_depth;
};

}}}

#endif //DIPLOMA_GC_NEW_STACK_HPP
