#include <libprecisegc/details/collectors/gc_core.hpp>

#include <libprecisegc/details/collectors/pin_set.hpp>
#include <libprecisegc/details/collectors/pin_stack.hpp>
#include <libprecisegc/details/collectors/stack_bitmap.hpp>
#include <libprecisegc/details/collectors/conservative_stack.hpp>
#include <libprecisegc/details/collectors/conservative_pin_set.hpp>
#include <libprecisegc/details/collectors/gc_new_stack.hpp>
#include <libprecisegc/details/collectors/gc_new_stack_entry.hpp>
#include <libprecisegc/details/allocators/memory_index.hpp>

namespace precisegc { namespace details { namespace collectors {

thread_local threads::gc_thread_descriptor* gc_core::this_thread = nullptr;

gc_core::gc_core(const thread_descriptor& main_thrd_descr)
{
    register_thread(main_thrd_descr);
}

gc_core::~gc_core()
{
//    allocators::memory_index::clear();
}

gc_alloc::response gc_core::allocate(const gc_alloc::request& rqst)
{
    gc_alloc::response rsp = m_heap.allocate(rqst);

    gc_new_stack_entry* stack_entry = reinterpret_cast<gc_new_stack_entry*>(rsp.buffer());
    stack_entry->obj_start = rsp.obj_start();
    stack_entry->meta_requested = rqst.type_meta() == nullptr;

    this_thread->register_stack_entry(stack_entry);

    return rsp;
}

void gc_core::abort(const gc_alloc::response& rsp)
{
    gc_new_stack_entry* stack_entry = reinterpret_cast<gc_new_stack_entry*>(rsp.buffer());
    this_thread->deregister_stack_entry(stack_entry);
}

void gc_core::commit(const gc_alloc::response& rsp)
{
    gc_new_stack_entry* stack_entry = reinterpret_cast<gc_new_stack_entry*>(rsp.buffer());

    this_thread->deregister_stack_entry(stack_entry);

    assert(stack_entry->descriptor);
    stack_entry->descriptor->commit(rsp.cell_start());
}

void gc_core::commit(const gc_alloc::response& rsp, const gc_type_meta* type_meta)
{
    gc_new_stack_entry* stack_entry = reinterpret_cast<gc_new_stack_entry*>(rsp.buffer());

    this_thread->deregister_stack_entry(stack_entry);

    assert(stack_entry->descriptor);
    stack_entry->descriptor->commit(rsp.cell_start(), type_meta);
}

gc_offsets gc_core::make_offsets(const gc_alloc::response& rsp)
{
    gc_new_stack_entry* stack_entry = reinterpret_cast<gc_new_stack_entry*>(rsp.buffer());
    return boost::make_iterator_range(stack_entry->offsets.begin(), stack_entry->offsets.end());
}

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
    using namespace allocators;

    memory_descriptor descr = memory_index::get_descriptor(reinterpret_cast<byte*>(&handle));
    if (descr.is_stack_descriptor()) {
        gc_handle_access::set<std::memory_order_relaxed>(handle, gc_tagging::set_root_bit(ptr, true));
        this_thread->register_root(&handle);
    } else if (descr.is_null()) {
        gc_handle_access::set<std::memory_order_relaxed>(handle, gc_tagging::set_root_bit(ptr, true));
        m_static_roots.register_root(&handle);
    } else {
        gc_handle_access::set<std::memory_order_relaxed>(handle, gc_tagging::set_root_bit(ptr, false));
        if (this_thread) {
            this_thread->register_heap_ptr(&handle);
        }
    }
}

void gc_core::deregister_handle(gc_handle& handle)
{
    using namespace allocators;

    memory_descriptor descr = memory_index::get_descriptor(reinterpret_cast<byte*>(&handle));
    if (descr.is_stack_descriptor()) {
        this_thread->deregister_root(&handle);
    } else if (descr.is_null()) {
        m_static_roots.deregister_root(&handle);
    } else {
        if (this_thread) {
            this_thread->deregister_heap_ptr(&handle);
        }
    }
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

void gc_core::register_thread(const thread_descriptor& descr)
{
    using namespace threads;

    assert(descr.id == std::this_thread::get_id());

    std::unique_ptr<gc_thread_descriptor> gc_thrd_descr =
            utils::make_unique<
                    gc_thread_descriptor_impl<conservative_stack, conservative_pin_set, gc_new_stack>
            >(descr);

    this_thread = gc_thrd_descr.get();
    m_thread_manager.register_thread(descr.id, std::move(gc_thrd_descr));
}

void gc_core::deregister_thread(std::thread::id id)
{
    assert(id == std::this_thread::get_id());

    m_thread_manager.deregister_thread(id);
}

static_root_set* gc_core::get_static_roots()
{
    return &m_static_roots;
}

void gc_core::destroy_unmarked_dptrs()
{
    m_dptr_storage.destroy_unmarked();
}

threads::world_snapshot gc_core::stop_the_world()
{
    return m_thread_manager.stop_the_world();
}

gc_heap_stat gc_core::collect(const threads::world_snapshot& snapshot, size_t threads_available)
{
    return m_heap.collect(snapshot, threads_available, &m_static_roots);
}

}}}

