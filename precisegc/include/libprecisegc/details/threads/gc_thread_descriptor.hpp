#ifndef DIPLOMA_GC_THREAD_DESCRIPTOR_HPP
#define DIPLOMA_GC_THREAD_DESCRIPTOR_HPP

#include <thread>
#include <cassert>

#include <libprecisegc/gc_new_stack_entry.hpp>
#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/logging.hpp>
#include <libprecisegc/details/collectors/static_root_set.hpp>

namespace precisegc { namespace details { namespace threads {

class gc_thread_descriptor
{
public:
    virtual ~gc_thread_descriptor() {}

    virtual void register_stack_entry(gc_new_stack_entry* stack_entry) = 0;
    virtual void deregister_stack_entry(gc_new_stack_entry* stack_entry) = 0;

    virtual void register_handle(gc_handle* handle, collectors::static_root_set* static_roots, bool is_root) = 0;
    virtual void deregister_handle(gc_handle* handle, collectors::static_root_set* static_roots, bool is_root) = 0;

    virtual void register_pin(byte* pin) = 0;
    virtual void deregister_pin(byte* pin) = 0;
    virtual void push_pin(byte* pin) = 0;
    virtual void pop_pin(byte* pin) = 0;

    virtual void trace_roots(const gc_trace_callback& cb) const = 0;
    virtual void trace_pins(const gc_trace_pin_callback& cb) const = 0;
};

template <typename StackRootSet, typename PinSet, typename PinStack, typename InitStack>
class gc_thread_descriptor_impl : public gc_thread_descriptor, private utils::noncopyable, private utils::nonmovable
{
public:
    void register_stack_entry(gc_new_stack_entry* stack_entry) override
    {
        m_init_stack.register_stack_entry(stack_entry);
    }

    void deregister_stack_entry(gc_new_stack_entry* stack_entry) override
    {
        m_init_stack.deregister_stack_entry(stack_entry);
    }

    void register_handle(gc_handle* handle, collectors::static_root_set* static_roots, bool is_root) override
    {
        assert(handle);
        assert(static_roots);

        if (is_root) {
            register_root(handle, static_roots);
        } else {
            m_init_stack.register_child(handle);
        }
    }

    void register_root(gc_handle* root, collectors::static_root_set* static_roots)
    {
        if (m_stack_roots.is_stack_ptr(root)) {
            m_stack_roots.register_root(root);
        } else {
            static_roots->register_root(root);
        }
    }

    void deregister_handle(gc_handle* handle, collectors::static_root_set* static_roots, bool is_root) override
    {
        if (is_root) {
            deregister_root(handle, static_roots);
        }
    }

    void deregister_root(gc_handle* root, collectors::static_root_set* static_roots)
    {
        if (m_stack_roots.is_stack_ptr(root)) {
            m_stack_roots.deregister_root(root);
        } else {
            static_roots->deregister_root(root);
        }
    }

    void register_pin(byte* pin) override
    {
        m_pin_set.insert(pin);
    }

    void deregister_pin(byte* pin) override
    {
        m_pin_set.remove(pin);
    }

    void push_pin(byte* ptr) override
    {
        if (!m_pin_stack.is_full()) {
            m_pin_stack.push_pin(ptr);
        } else {
            m_pin_set.insert(ptr);
        }
    }

    void pop_pin(byte* ptr) override
    {
        if (m_pin_stack.top() == ptr) {
            m_pin_stack.pop_pin();
        } else {
            m_pin_set.remove(ptr);
        }
    }

    void trace_roots(const gc_trace_callback& cb) const override
    {
        logging::info() << "Thread "        << m_id << " "
                        << "roots count = " << m_stack_roots.size();
        m_stack_roots.trace(cb);
    }

    void trace_pins(const gc_trace_pin_callback& cb) const override
    {
        logging::info() << "Thread "       << m_id << " "
                        << "pins count = " << m_pin_set.size() + m_pin_stack.size() + m_init_stack.size();
        m_pin_set.trace(cb);
        m_pin_stack.trace(cb);
        m_init_stack.trace(cb);
    }
private:
    StackRootSet m_stack_roots;
    PinSet m_pin_set;
    PinStack m_pin_stack;
    InitStack m_init_stack;
    std::thread::id m_id;
};

}}}

#endif //DIPLOMA_GC_THREAD_DESCRIPTOR_HPP
