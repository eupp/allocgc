#include <libprecisegc/details/collectors/gc_core.hpp>

namespace precisegc { namespace details { namespace collectors {

threads::gc_thread_descriptor* gc_core::this_thread = nullptr;

byte* gc_core::rbarrier(const gc_handle& handle)
{
    return gc_tagging::get(gc_handle_access::get<std::memory_order_relaxed>(handle));
}

void gc_core::interior_wbarrier(gc_handle& handle, ptrdiff_t offset)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(handle);
    if (gc_tagging::is_derived(ptr)) {
        gc_tagging::reset_derived_ptr(ptr, offset);
    } else {
        dptr_descriptor* descr = m_dptr_storage.make_derived(gc_tagging::clear_root_bit(ptr), offset);
        byte* dptr = gc_tagging::make_derived_ptr(descr, gc_tagging::is_root(ptr));
        gc_handle_access::set<std::memory_order_relaxed>(handle, dptr);
    }
}

void gc_core::register_handle(gc_handle& handle, byte* ptr)
{
    bool is_root = !gc_is_heap_ptr(&handle);
    gc_handle_access::set<std::memory_order_relaxed>(handle, gc_tagging::set_root_bit(ptr, is_root));
    this_thread->register_handle(&handle, &m_static_roots, is_root);
}

void gc_core::deregister_handle(gc_handle& handle)
{
    bool is_root = gc_tagging::is_root(gc_handle_access::get<std::memory_order_relaxed>(handle));
    this_thread->deregister_handle(&handle, &m_static_roots, is_root);
}

byte* gc_core::register_pin(const gc_handle& handle)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = handle.rbarrier();
    if (ptr) {
        this_thread->register_pin(ptr);
    }
    return ptr;
}

void gc_core::deregister_pin(byte* pin)
{
    if (pin) {
        this_thread->deregister_pin(pin);
    }
}

byte* gc_core::push_pin(const gc_handle& handle)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = handle.rbarrier();
    if (ptr) {
        this_thread->push_pin(ptr);
    }
    return ptr;
}

void gc_core::pop_pin(byte* pin)
{
    if (pin) {
        this_thread->pop_pin(pin);
    }
}

void gc_core::register_stack_entry(gc_new_stack_entry* stack_entry)
{
    this_thread->register_stack_entry(stack_entry);
}

void gc_core::deregister_stack_entry(gc_new_stack_entry* stack_entry)
{
    this_thread->deregister_stack_entry(stack_entry);
}

void gc_core::register_thread(std::thread::id id, byte* stack_start_addr)
{
    using namespace threads;

    assert(id == std::this_thread::get_id());

    std::unique_ptr<gc_thread_descriptor> descr =
            utils::make_unique<gc_thread_descriptor_impl<stack_bitmap, pin_set, pin_stack, gc_new_stack>>();

    this_thread = descr.get();
    m_thread_manager.register_thread(id, std::move(descr));
}

void gc_core::deregister_thread(std::thread::id id)
{
    assert(id == std::this_thread::get_id());

    m_thread_manager.deregister_thread(id);
}


}}}

