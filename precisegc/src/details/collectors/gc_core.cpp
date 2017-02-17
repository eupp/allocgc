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

gc_core::gc_core(const gc_factory::options& opt, const thread_descriptor& main_thrd_descr, remset* rset)
    : m_marker(&m_packet_manager, rset)
    , m_heap(opt)
{
    logging::init(std::clog, opt.loglevel);

    m_conservative_mode = opt.conservative;

    register_thread(main_thrd_descr);
}

gc_core::~gc_core()
{
//    allocators::memory_index::clear();
}

gc_alloc::response gc_core::allocate(const gc_alloc::request& rqst)
{
    gc_new_stack_entry* stack_entry = reinterpret_cast<gc_new_stack_entry*>(rqst.buffer());
    stack_entry->zeroing_flag = !m_conservative_mode;

    gc_alloc::response rsp = m_heap.allocate(rqst);

    stack_entry->obj_start = rsp.obj_start();
    stack_entry->obj_size  = rqst.alloc_size();
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
    return gc_handle_access::get<std::memory_order_relaxed>(handle);
}

void gc_core::interior_wbarrier(gc_handle& handle, ptrdiff_t offset)
{
    gc_handle_access::advance<std::memory_order_relaxed>(handle, offset);
}

void gc_core::register_handle(gc_handle& handle, byte* ptr)
{
    using namespace allocators;

    memory_descriptor descr = memory_index::get_descriptor(reinterpret_cast<byte*>(&handle));
    gc_handle_access::set<std::memory_order_relaxed>(handle, ptr);
    if (descr.is_stack_descriptor()) {
        this_thread->register_root(&handle);
    } else if (descr.is_null()) {
        m_static_roots.register_root(&handle);
    } else {
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

    std::unique_ptr<gc_thread_descriptor> gc_thrd_descr;

    if (m_conservative_mode) {
        gc_thrd_descr = utils::make_unique<gc_thread_descriptor_impl<conservative_stack, conservative_pin_set, gc_new_stack>>(descr);
    } else {
        gc_thrd_descr = utils::make_unique<gc_thread_descriptor_impl<stack_bitmap, pin_set, gc_new_stack>>(descr);
    }


    this_thread = gc_thrd_descr.get();
    m_thread_manager.register_thread(descr.id, std::move(gc_thrd_descr));
}

void gc_core::deregister_thread(std::thread::id id)
{
    assert(id == std::this_thread::get_id());

    m_thread_manager.deregister_thread(id);
}

threads::world_snapshot gc_core::stop_the_world()
{
    return m_thread_manager.stop_the_world();
}

void gc_core::trace_roots(const threads::world_snapshot& snapshot)
{

    if (m_conservative_mode) {
        snapshot.trace_roots([this] (gc_handle* root) { conservative_root_trace_cb(root); });
    } else {
        snapshot.trace_roots([this] (gc_handle* root) { root_trace_cb(root); });
    }
    m_static_roots.trace([this] (gc_handle* root) { root_trace_cb(root); });
}

void gc_core::trace_pins(const threads::world_snapshot& snapshot)
{
    snapshot.trace_pins([this] (byte* ptr) { pin_trace_cb(ptr); });
}

void gc_core::trace_uninit(const threads::world_snapshot& snapshot)
{
    snapshot.trace_uninit([this] (byte* obj_start, size_t obj_size) { conservative_obj_trace_cb(obj_start, obj_size); });
}

void gc_core::trace_remset()
{
    m_marker.trace_remset();
}

void gc_core::root_trace_cb(gc_handle* root)
{
    byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(*root);
    if (ptr) {
        gc_cell cell = allocators::memory_index::get_gc_cell(ptr);
        cell.set_mark(true);
        m_marker.add_root(cell);

        logging::debug() << "root: " << (void*) root /* << "; point to: " << (void*) obj_start */;
    }
}

void gc_core::pin_trace_cb(byte* ptr)
{
    if (ptr) {
        gc_cell cell = allocators::memory_index::get_gc_cell(ptr);
        cell.set_mark(true);
        cell.set_pin(true);
        m_marker.add_root(cell);

        logging::debug() << "pin: " << (void*) ptr;
    }
}

void gc_core::conservative_root_trace_cb(gc_handle* root)
{
    byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(*root);
    if (ptr) {
        gc_cell cell = allocators::memory_index::get_gc_cell(ptr);
        if (!cell.get_mark() && cell.is_init()) {
            cell.set_mark(true);
            cell.set_pin(true);
            m_marker.add_root(cell);

            logging::debug() << "root: " << (void*) root << "; point to: " << (void*) ptr;
        }
    }
}

void gc_core::conservative_obj_trace_cb(byte* obj_start, size_t obj_size)
{
    assert(obj_start && obj_size > 0);
    gc_cell cell = allocators::memory_index::get_gc_cell(obj_start);
    cell.set_mark(true);
    cell.set_pin(true);

    logging::info() << "uninitialized object: " << (void*) obj_start /* << "; point to: " << (void*) obj_start */;

    gc_handle* begin = reinterpret_cast<gc_handle*>(obj_start);
    gc_handle* end   = reinterpret_cast<gc_handle*>(obj_start + obj_size);
    for (gc_handle* it = begin; it < end; ++it) {
        byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(*it);
        cell = allocators::memory_index::get_gc_cell(ptr);
        if (!cell.get_mark() && cell.is_init()) {
            cell.set_mark(true);
            cell.set_pin(true);
            m_marker.add_root(cell);

            logging::debug() << "pointer from uninitialized object: " << (void*) ptr /* << "; point to: " << (void*) obj_start */;
        }
    }
}

void gc_core::start_concurrent_marking(size_t threads_available)
{
    m_marker.concurrent_mark(threads_available);
}

void gc_core::start_marking()
{
    m_marker.mark();
}

gc_heap_stat gc_core::collect(const threads::world_snapshot& snapshot, size_t threads_available)
{
    return m_heap.collect(snapshot, threads_available, &m_static_roots);
}

}}}

