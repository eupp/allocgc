#ifndef ALLOCGC_GC_THREAD_DESCRIPTOR_HPP
#define ALLOCGC_GC_THREAD_DESCRIPTOR_HPP

#include <thread>
#include <cassert>

#include <liballocgc/gc_common.hpp>
#include <liballocgc/details/constants.hpp>
#include <liballocgc/details/gc_interface.hpp>
#include <liballocgc/details/logging.hpp>
#include <liballocgc/details/allocators/memory_index.hpp>
#include <liballocgc/details/collectors/gc_heap.hpp>
#include <liballocgc/details/collectors/gc_new_stack_entry.hpp>
#include <liballocgc/details/collectors/static_root_set.hpp>
#include <liballocgc/details/threads/return_address.hpp>

#include <liballocgc/details/collectors/pin_set.hpp>
#include <liballocgc/details/collectors/pin_stack.hpp>
#include <liballocgc/details/collectors/stack_bitmap.hpp>
#include <liballocgc/details/collectors/gc_new_stack.hpp>

namespace allocgc { namespace details { namespace threads {

class gc_thread_descriptor : private utils::noncopyable, private utils::nonmovable
{
    class stack_descriptor
    {
    public:
        inline stack_descriptor(byte* stack_addr, size_t stack_size)
            : m_stack_addr(stack_addr)
            , m_stack_size(stack_size)
        {
            logging::info() << "stack addr=" << (void*) m_stack_addr
                            << "; stack size=" << (void*) m_stack_size;
        }

        inline bool is_stack_ptr(const gc_handle* ptr) const
        {
            const byte* p = reinterpret_cast<const byte*>(ptr);
            return (m_stack_addr <= p) && (p < m_stack_addr + m_stack_size);
        }

        inline byte* stack_addr() const
        {
            return m_stack_addr;
        }

        inline size_t stack_size() const
        {
            return m_stack_size;
        }
    private:
        byte* m_stack_addr;
        size_t m_stack_size;
    };
public:
    gc_thread_descriptor(const thread_descriptor& descr, gc_heap::tlab* tlab)
        : m_stack_descr(descr.stack_addr, descr.stack_size)
        , m_stack_roots(descr.id, descr.stack_addr, descr.stack_size)
        , m_id(descr.id)
        , m_native_handle(descr.native_handle)
        , m_tlab(tlab)
    {
        assert(m_stack_descr.stack_size() % PAGE_SIZE == 0);
        allocators::memory_index::index_stack_memory(m_stack_descr.stack_addr(),
                                                     m_stack_descr.stack_size(),
                                                     m_stack_descr.stack_addr());
    }

    ~gc_thread_descriptor()
    {
        allocators::memory_index::deindex(m_stack_descr.stack_addr(),
                                          m_stack_descr.stack_size());
    }

    gc_alloc::response allocate(const gc_alloc::request& rqst)
    {
        assert(rqst.alloc_size() <= LARGE_CELL_SIZE);
        return m_tlab->allocate(rqst);
    }

    void register_stack_entry(collectors::gc_new_stack_entry* stack_entry)
    {
        m_uninit_stack.register_stack_entry(stack_entry);
    }

    void deregister_stack_entry(collectors::gc_new_stack_entry* stack_entry)
    {
        m_uninit_stack.deregister_stack_entry(stack_entry);
    }

    void register_root(gc_handle* handle)
    {
        assert(m_stack_descr.is_stack_ptr(handle));
        m_stack_roots.register_root(handle);
    }

    void deregister_root(gc_handle* handle)
    {
        assert(m_stack_descr.is_stack_ptr(handle));
        m_stack_roots.deregister_root(handle);
    }

    void register_heap_ptr(gc_handle* handle)
    {
        m_uninit_stack.register_child(handle);
    }

    void deregister_heap_ptr(gc_handle* handle)
    {
        return;
    }

    void register_pin(byte* pin)
    {
        m_pin_set.register_pin(pin);
    }

    void deregister_pin(byte* pin)
    {
        m_pin_set.deregister_pin(pin);
    }

    void push_pin(byte* ptr)
    {
        m_pin_set.push_pin(ptr);
    }

    void pop_pin(byte* ptr)
    {
        m_pin_set.pop_pin(ptr);
    }

    void trace_roots(const gc_trace_callback& cb) const
    {
        logging::info() << "Trace stack of thread " << m_id;

        m_stack_roots.trace(cb);
    }

    void trace_pins(const gc_trace_pin_callback& cb) const
    {
        m_pin_set.trace(cb);
    }

    void trace_uninit(const gc_trace_obj_callback& cb) const
    {
        m_uninit_stack.trace(cb);
    }

    std::thread::id get_id() const
    {
        return m_id;
    }

    std::thread::native_handle_type native_handle() const
    {
        return m_native_handle;
    }
private:
    stack_descriptor                m_stack_descr;
    collectors::stack_bitmap        m_stack_roots;
    collectors::pin_set             m_pin_set;
    collectors::gc_new_stack        m_uninit_stack;
    gc_heap::tlab*                  m_tlab;
    std::thread::id                 m_id;
    std::thread::native_handle_type m_native_handle;
};

}}}

#endif //ALLOCGC_GC_THREAD_DESCRIPTOR_HPP
