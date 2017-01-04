#include <libprecisegc/details/gc_facade.hpp>

#include <cassert>
#include <memory>
#include <iostream>

#include <libprecisegc/details/collectors/memory_index.hpp>
#include <libprecisegc/details/threads/this_managed_thread.hpp>
#include <libprecisegc/details/utils/scope_guard.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details {

gc_facade::gc_facade()
    : m_manager(nullptr)
{
    logging::touch();
    // same hack as with logger
    collectors::memory_index::add_to_index(nullptr, 0, nullptr);
}

void gc_facade::init(std::unique_ptr<gc_strategy> strategy)
{
    m_strategy = std::move(strategy);
    m_manager.set_strategy(m_strategy.get());
}

gc_strategy* gc_facade::get_strategy() const
{
    return m_strategy.get();
}

std::unique_ptr<gc_strategy> gc_facade::set_strategy(std::unique_ptr<gc_strategy> strategy)
{
    strategy.swap(m_strategy);
    m_manager.set_strategy(m_strategy.get());
    return std::move(strategy);
}

allocators::gc_alloc_response gc_facade::allocate(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta)
{
    try {
        return try_allocate(obj_size, obj_cnt, tmeta);
    } catch (gc_bad_alloc& ) {
        return try_allocate(obj_size, obj_cnt, tmeta);
    }
}

allocators::gc_alloc_response gc_facade::try_allocate(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta)
{
    assert(m_strategy);
    return m_strategy->allocate(obj_size, obj_cnt, tmeta);
}

void gc_facade::commit(gc_cell& cell)
{
    assert(m_strategy);
    m_strategy->commit(cell);
}

void gc_facade::commit(gc_cell& cell, const gc_type_meta* meta)
{
    assert(m_strategy);
    m_strategy->commit(cell, meta);
}

byte* gc_facade::rbarrier(const gc_handle& handle)
{
    assert(m_strategy);
    return m_strategy->rbarrier(handle);
}

void gc_facade::wbarrier(gc_handle& dst, const gc_handle& src)
{
    assert(m_strategy);
    m_strategy->wbarrier(dst, src);
}

void gc_facade::interior_wbarrier(gc_handle& handle, ptrdiff_t offset)
{
    assert(m_strategy);
    // this assertion doesn't work properly in current version
//    assert(is_interior_offset(handle, offset));
    m_strategy->interior_wbarrier(handle, offset);
}

byte* gc_facade::pin(const gc_handle& handle)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = handle.rbarrier();
    if (ptr) {
        threads::this_managed_thread::pin(ptr);
    }
    return ptr;
}

void gc_facade::unpin(byte* ptr)
{
    if (ptr) {
        threads::this_managed_thread::unpin(ptr);
    }
}

byte* gc_facade::push_pin(const gc_handle& handle)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = handle.rbarrier();
    threads::this_managed_thread::push_pin(ptr);
    return ptr;
}

void gc_facade::pop_pin(byte* ptr)
{
    if (ptr) {
        threads::this_managed_thread::pop_pin(ptr);
    }
}

bool gc_facade::compare(const gc_handle& a, const gc_handle& b)
{
    gc_unsafe_scope unsafe_scope;
    return a.rbarrier() == b.rbarrier();
}

void gc_facade::initiation_point(initiation_point_type ipt, const gc_options& opt)
{
    gc_safe_scope safe_scope;
    std::lock_guard<std::mutex> lock(m_gc_mutex);

    if (ipt == initiation_point_type::USER_REQUEST) {
        logging::info() << "Thread initiates gc by user's request";
        m_manager.gc(opt);
    } else if (ipt == initiation_point_type::HEAP_LIMIT_EXCEEDED) {
//        logging::info() << "Heap limit exceeded - thread initiates gc";
        m_manager.gc(opt);
    } else if (ipt == initiation_point_type::CONCURRENT_MARKING_FINISHED) {
        logging::info() << "Concurrent marking finished - Thread initiates gc";
        m_manager.gc(opt);
    } else if (ipt == initiation_point_type::START_MARKING) {
        m_manager.gc(opt);
    } else if (ipt == initiation_point_type::START_COLLECTING) {
        m_manager.gc(opt);
    }
}

bool gc_facade::is_printer_enabled() const
{
    return m_manager.print_stats_flag();
}

void gc_facade::set_printer_enabled(bool enabled)
{
    m_manager.set_print_stats_flag(enabled);
}

void gc_facade::register_page(const byte* page, size_t size)
{
    m_manager.register_page(page, size);
}

void gc_facade::deregister_page(const byte* page, size_t size)
{
    m_manager.deregister_page(page, size);
}

gc_info gc_facade::info() const
{
    assert(m_strategy);
    return m_strategy->info();
}

gc_stat gc_facade::stats() const
{
    gc_unsafe_scope unsafe_scope;
    return m_manager.stats();
}

bool gc_facade::is_interior_pointer(const gc_handle& handle, byte* iptr)
{
    byte* ptr = handle.rbarrier();
    gc_memory_descriptor* descr = collectors::memory_index::index_memory(ptr);
    byte* cell_begin = descr->cell_start(ptr);
    byte* cell_end   = cell_begin + descr->cell_size(ptr);
    return (cell_begin <= iptr) && (iptr < cell_end);
}

bool gc_facade::is_interior_offset(const gc_handle& handle, ptrdiff_t shift)
{
    return is_interior_pointer(handle, handle.rbarrier() + shift);
}

}}
