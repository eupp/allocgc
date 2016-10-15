#include <libprecisegc/details/garbage_collector.hpp>

#include <cassert>
#include <memory>
#include <iostream>

#include <libprecisegc/details/collectors/memory_index.hpp>
#include <libprecisegc/details/threads/this_managed_thread.hpp>
#include <libprecisegc/details/utils/scope_guard.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details {

garbage_collector::garbage_collector()
    : m_manager(nullptr)
{
    logging::touch();
    // same hack as with logger
    collectors::memory_index::add_to_index(nullptr, 0, nullptr);
}

void garbage_collector::init(std::unique_ptr<gc_strategy> strategy, std::unique_ptr<initiation_policy> init_policy)
{
    m_strategy = std::move(strategy);
    m_initiation_policy = std::move(init_policy);
    m_manager.set_strategy(m_strategy.get());
}

gc_strategy* garbage_collector::get_strategy() const
{
    return m_strategy.get();
}

std::unique_ptr<gc_strategy> garbage_collector::set_strategy(std::unique_ptr<gc_strategy> strategy)
{
    strategy.swap(m_strategy);
    m_manager.set_strategy(m_strategy.get());
    return std::move(strategy);
}

gc_alloc_response garbage_collector::allocate(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta)
{
    try {
        return try_allocate(obj_size, obj_cnt, tmeta);
    } catch (gc_bad_alloc& ) {
        initiation_point(initiation_point_type::GC_BAD_ALLOC);
        return try_allocate(obj_size, obj_cnt, tmeta);
    }
}

gc_alloc_response garbage_collector::try_allocate(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta)
{
    assert(m_strategy);
    return m_strategy->allocate(obj_size, obj_cnt, tmeta);
}

void garbage_collector::commit(const gc_alloc_response& ptr)
{
    assert(m_strategy);
    m_strategy->commit(ptr);
}

byte* garbage_collector::rbarrier(const gc_word& handle)
{
    assert(m_strategy);
    return m_strategy->rbarrier(handle);
}

void garbage_collector::wbarrier(gc_word& dst, const gc_word& src)
{
    assert(m_strategy);
    m_strategy->wbarrier(dst, src);
}

void garbage_collector::interior_wbarrier(gc_word& handle, ptrdiff_t offset)
{
    assert(m_strategy);
    // this assertion doesn't work properly in current version
//    assert(is_interior_offset(handle, offset));
    m_strategy->interior_wbarrier(handle, offset);
}

byte* garbage_collector::pin(const gc_word& handle)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = handle.rbarrier();
    if (ptr) {
        threads::this_managed_thread::pin(ptr);
    }
    return ptr;
}

void garbage_collector::unpin(byte* ptr)
{
    if (ptr) {
        threads::this_managed_thread::unpin(ptr);
    }
}

byte* garbage_collector::push_pin(const gc_word& handle)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = handle.rbarrier();
    threads::this_managed_thread::push_pin(ptr);
    return ptr;
}

void garbage_collector::pop_pin(byte* ptr)
{
    if (ptr) {
        threads::this_managed_thread::pop_pin(ptr);
    }
}

bool garbage_collector::compare(const gc_word& a, const gc_word& b)
{
    gc_unsafe_scope unsafe_scope;
    return a.rbarrier() == b.rbarrier();
}

void garbage_collector::initiation_point(initiation_point_type ipt, const initiation_point_data& ipd)
{
    assert(m_initiation_policy);

    gc_safe_scope safe_scope;
    std::lock_guard<std::mutex> lock(m_gc_mutex);

    if (ipt == initiation_point_type::USER_REQUEST) {
        logging::info() << "Thread initiates gc by user's request";
        m_manager.gc(gc_phase::COLLECT);
    } else if (ipt == initiation_point_type::GC_BAD_ALLOC) {
        logging::info() << "GC_BAD_ALLOC received - Thread initiates gc";
        m_manager.gc(gc_phase::COLLECT);
    } else if (ipt == initiation_point_type::CONCURRENT_MARKING_FINISHED) {
        logging::info() << "Concurrent marking finished - Thread initiates gc";
        m_manager.gc(gc_phase::COLLECT);
    } else if (ipt == initiation_point_type::HEAP_EXPANSION) {
        m_initiation_policy->initiation_point(&m_manager, ipt, ipd);
    } else if (ipt == initiation_point_type::START_MARKING) {
        m_manager.gc(gc_phase::MARK);
    } else if (ipt == initiation_point_type::START_COLLECTING) {
        m_manager.gc(gc_phase::COLLECT);
    }
}

bool garbage_collector::is_printer_enabled() const
{
    return m_manager.print_stats_flag();
}

void garbage_collector::set_printer_enabled(bool enabled)
{
    m_manager.set_print_stats_flag(enabled);
}

void garbage_collector::register_page(const byte* page, size_t size)
{
    m_manager.register_page(page, size);
}

void garbage_collector::deregister_page(const byte* page, size_t size)
{
    m_manager.deregister_page(page, size);
}

gc_info garbage_collector::info() const
{
    assert(m_strategy);
    return m_strategy->info();
}

gc_stat garbage_collector::stats() const
{
    gc_unsafe_scope unsafe_scope;
    return m_manager.stats();
}

gc_state garbage_collector::state() const
{
    return m_manager.state();
}

bool garbage_collector::is_interior_pointer(const gc_word& handle, byte* p)
{
    auto idx_ptr = collectors::indexed_managed_object::index(handle.rbarrier());
    byte* cell_begin = idx_ptr.object();
    byte* cell_end   = cell_begin + idx_ptr.cell_size();
    return (cell_begin <= p) && (p < cell_end);
}

bool garbage_collector::is_interior_offset(const gc_word& handle, ptrdiff_t shift)
{
    return is_interior_pointer(handle, handle.rbarrier() + shift);
}

}}
