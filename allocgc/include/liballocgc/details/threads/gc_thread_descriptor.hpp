#ifndef ALLOCGC_GC_THREAD_DESCRIPTOR_HPP
#define ALLOCGC_GC_THREAD_DESCRIPTOR_HPP

#include <thread>
#include <cassert>

#include <liballocgc/gc_common.hpp>
#include <liballocgc/details/constants.hpp>
#include <liballocgc/details/gc_interface.hpp>
#include <liballocgc/details/logging.hpp>
#include <liballocgc/details/allocators/memory_index.hpp>
#include <liballocgc/details/collectors/gc_new_stack_entry.hpp>
#include <liballocgc/details/collectors/static_root_set.hpp>
#include <liballocgc/details/threads/return_address.hpp>

namespace allocgc { namespace details { namespace threads {

class gc_thread_descriptor
{
public:
    virtual ~gc_thread_descriptor() {}

    virtual void register_stack_entry(collectors::gc_new_stack_entry* stack_entry) = 0;
    virtual void deregister_stack_entry(collectors::gc_new_stack_entry* stack_entry) = 0;

    virtual void register_root(gc_handle* handle) = 0;
    virtual void deregister_root(gc_handle* handle) = 0;

    virtual void register_heap_ptr(gc_handle* handle) = 0;
    virtual void deregister_heap_ptr(gc_handle* handle) = 0;

    virtual void register_pin(byte* pin) = 0;
    virtual void deregister_pin(byte* pin) = 0;
    virtual void push_pin(byte* pin) = 0;
    virtual void pop_pin(byte* pin) = 0;

    virtual void trace_roots(const gc_trace_callback& cb) const = 0;
    virtual void trace_pins(const gc_trace_pin_callback& cb) const = 0;
    virtual void trace_uninit(const gc_trace_obj_callback& cb) const = 0;


    virtual std::thread::id get_id() const = 0;
    virtual std::thread::native_handle_type native_handle() const = 0;
};

template <typename StackRootSet, typename PinSet, typename UninitStack>
class gc_thread_descriptor_impl : public gc_thread_descriptor, private utils::noncopyable, private utils::nonmovable
{
    class stack_descriptor
    {
        inline byte* stack_start_addr_approx(byte* stack_addr)
        {
            byte* aligned_addr = reinterpret_cast<byte*>(
                    reinterpret_cast<std::uintptr_t>(stack_addr) & ~((1ull << PAGE_SIZE_LOG2) - 1)
            );
            return STACK_DIRECTION == stack_growth_direction::UP ? aligned_addr : aligned_addr + PAGE_SIZE;
        }
    public:
        inline stack_descriptor(byte* stack_start_addr)
            : m_stack_start(stack_start_addr_approx(stack_start_addr))
            , m_stack_end(m_stack_start + STACK_DIRECTION * (threads::stack_maxsize() - PAGE_SIZE))
        {
            logging::info() << "stack start addr=" << (void*) m_stack_start
                            << "; stack end addr=" << (void*) m_stack_end;
        }

        inline bool is_stack_ptr(const gc_handle* ptr) const
        {
            const byte* p = reinterpret_cast<const byte*>(ptr);
            return STACK_DIRECTION == stack_growth_direction::UP
                   ? (m_stack_start <= p) && (p < m_stack_end)
                   : (m_stack_start >= p) && (p > m_stack_end);
        }

        inline byte* stack_start_addr() const
        {
            return m_stack_start;
        }

        inline byte* stack_end_addr() const
        {
            return m_stack_end;
        }

        inline byte* stack_min_addr() const
        {
            return STACK_DIRECTION == stack_growth_direction::UP ? m_stack_start : m_stack_end;
        }

        inline byte* stack_max_addr() const
        {
            return STACK_DIRECTION == stack_growth_direction::UP ? m_stack_end : m_stack_start;
        }

        inline size_t stack_size() const
        {
            return STACK_DIRECTION == stack_growth_direction::UP
                   ? (m_stack_end - m_stack_start)
                   : (m_stack_start - m_stack_end);
        }
    private:
        byte* m_stack_start;
        byte* m_stack_end;
    };
public:
    gc_thread_descriptor_impl(const thread_descriptor& descr)
        : m_stack_descr(descr.stack_start_addr)
        , m_stack_roots(descr.id, descr.stack_start_addr,
                        m_stack_descr.stack_start_addr(), m_stack_descr.stack_end_addr())
        , m_id(descr.id)
        , m_native_handle(descr.native_handle)
    {
        assert(m_stack_descr.stack_size() % PAGE_SIZE == 0);
        allocators::memory_index::index_stack_memory(m_stack_descr.stack_min_addr(),
                                                     m_stack_descr.stack_size(),
                                                     m_stack_descr.stack_start_addr());
    }

    ~gc_thread_descriptor_impl()
    {
        allocators::memory_index::deindex(m_stack_descr.stack_min_addr(),
                                          m_stack_descr.stack_size());
    }

    void register_stack_entry(collectors::gc_new_stack_entry* stack_entry) override
    {
        m_uninit_stack.register_stack_entry(stack_entry);
    }

    void deregister_stack_entry(collectors::gc_new_stack_entry* stack_entry) override
    {
        m_uninit_stack.deregister_stack_entry(stack_entry);
    }

    void register_root(gc_handle* handle) override
    {
        assert(m_stack_descr.is_stack_ptr(handle));
        m_stack_roots.register_root(handle);
    }

    void deregister_root(gc_handle* handle) override
    {
        assert(m_stack_descr.is_stack_ptr(handle));
        m_stack_roots.deregister_root(handle);
    }

    void register_heap_ptr(gc_handle* handle) override
    {
        m_uninit_stack.register_child(handle);
    }

    void deregister_heap_ptr(gc_handle* handle) override
    {
        return;
    }

    void register_pin(byte* pin) override
    {
        m_pin_set.register_pin(pin);
    }

    void deregister_pin(byte* pin) override
    {
        m_pin_set.deregister_pin(pin);
    }

    void push_pin(byte* ptr) override
    {
        m_pin_set.push_pin(ptr);
    }

    void pop_pin(byte* ptr) override
    {
        m_pin_set.pop_pin(ptr);
    }

    void trace_roots(const gc_trace_callback& cb) const override
    {
        logging::info() << "Trace stack of thread " << m_id;

        m_stack_roots.trace(cb);
    }

    void trace_pins(const gc_trace_pin_callback& cb) const override
    {
        m_pin_set.trace(cb);
    }

    void trace_uninit(const gc_trace_obj_callback& cb) const override
    {
        m_uninit_stack.trace(cb);
    }

    std::thread::id get_id() const override
    {
        return m_id;
    }

    std::thread::native_handle_type native_handle() const override
    {
        return m_native_handle;
    }
private:
    stack_descriptor                m_stack_descr;
    StackRootSet                    m_stack_roots;
    PinSet                          m_pin_set;
    UninitStack                     m_uninit_stack;
    std::thread::id                 m_id;
    std::thread::native_handle_type m_native_handle;
};

}}}

#endif //ALLOCGC_GC_THREAD_DESCRIPTOR_HPP
