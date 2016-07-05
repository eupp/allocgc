#include <libprecisegc/details/garbage_collector.hpp>

#include <cassert>
#include <memory>
#include <iostream>

#include <libprecisegc/details/utils/scope_guard.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details {

garbage_collector::garbage_collector()
    : m_printer(std::clog)
    , m_printer_enabled(false)
{}

void garbage_collector::init(std::unique_ptr<gc_strategy> strategy, std::unique_ptr<initiation_policy> init_policy)
{
    m_strategy = std::move(strategy);
    m_initiation_policy = std::move(init_policy);

    m_gc_info = m_strategy->info();
}

gc_strategy* garbage_collector::get_strategy() const
{
    return m_strategy.get();
}

void garbage_collector::set_strategy(std::unique_ptr<gc_strategy> strategy)
{
    m_strategy = std::move(strategy);
}

std::unique_ptr<gc_strategy> garbage_collector::reset_strategy(std::unique_ptr<gc_strategy> strategy)
{
    strategy.swap(m_strategy);
    return std::move(strategy);
}

managed_ptr garbage_collector::allocate(size_t size)
{
    assert(m_strategy);
    managed_ptr p;
    try {
        p = m_strategy->allocate(size);
    } catch (gc_bad_alloc& ) {
        initiation_point(initiation_point_type::GC_BAD_ALLOC);
        try {
            p = m_strategy->allocate(size);
        } catch (gc_bad_alloc& ) {
            // give another chance to incremental collector
            if (m_gc_info.incremental) {
                initiation_point(initiation_point_type::GC_BAD_ALLOC);
            }
        }
        p = m_strategy->allocate(size);
    }
    m_recorder.register_allocation(p.cell_size());
    return p;
}

byte* garbage_collector::rbarrier(const gc_handle& handle)
{
    assert(m_strategy);
    return m_strategy->rbarrier(handle);
}

void garbage_collector::wbarrier(gc_handle& dst, const gc_handle& src)
{
    assert(m_strategy);
    m_strategy->wbarrier(dst, src);
}

void garbage_collector::interior_wbarrier(gc_handle& handle, byte* ptr)
{
    assert(m_strategy);
    assert(!ptr || is_interior_pointer(handle, ptr));
    m_strategy->interior_wbarrier(handle, ptr);
}

void garbage_collector::interior_shift(gc_handle& handle, ptrdiff_t shift)
{
    assert(m_strategy);
    assert(is_interior_shift(handle, shift));
    m_strategy->interior_shift(handle, shift);
}

byte* garbage_collector::pin(const gc_handle& handle)
{
    assert(m_strategy);
    return m_strategy->pin(handle);
}

void garbage_collector::unpin(byte* ptr)
{
    assert(ptr);
    assert(m_strategy);
    m_strategy->unpin(ptr);
}

bool garbage_collector::compare(const gc_handle& a, const gc_handle& b)
{
    assert(m_strategy);
    return m_strategy->compare(a, b);
}

void garbage_collector::initiation_point(initiation_point_type ipt, const initiation_point_data& ipd)
{
    assert(m_initiation_policy);

    gc_unsafe_scope::enter_safepoint();
    auto guard = utils::make_scope_guard([] { gc_unsafe_scope::leave_safepoint(); });

    if (ipt == initiation_point_type::USER_REQUEST) {

        logging::info() << "Thread initiates gc by user's request";
        std::lock_guard<std::mutex> lock(m_gc_mutex);
        m_strategy->gc(gc_phase::SWEEP);

    } else if (ipt == initiation_point_type::GC_BAD_ALLOC) {

        logging::info() << "GC_BAD_ALLOC received - Thread initiates gc";
        std::lock_guard<std::mutex> lock(m_gc_mutex);
        m_strategy->gc(gc_phase::SWEEP);

    } else if (ipt == initiation_point_type::HEAP_EXPANSION) {

        gc_phase phase = m_initiation_policy->check(ipt, ipd, state());
        if (phase == gc_phase::MARK && m_gc_info.incremental && m_gc_info.support_concurrent_mark) {
            m_strategy->gc(phase);
        } else if (phase == gc_phase::SWEEP) {
            throw gc_bad_alloc();
        }
    }
}

bool garbage_collector::is_printer_enabled() const
{
    return m_printer_enabled;
}

void garbage_collector::set_printer_enabled(bool enabled)
{
    m_printer_enabled = enabled;
}

void garbage_collector::register_page(const byte* page, size_t size)
{
    m_recorder.register_page(page, size);
}

void garbage_collector::deregister_page(const byte* page, size_t size)
{
    return;
}

void garbage_collector::register_pause(const gc_pause_stat& pause_stat)
{
    m_recorder.register_pause(pause_stat);
    if (m_printer_enabled) {
        m_printer.print_pause_stat(pause_stat);
    }
}

void garbage_collector::register_sweep(const gc_sweep_stat& sweep_stat, const gc_pause_stat& pause_stat)
{
    m_recorder.register_sweep(sweep_stat, pause_stat);
    if (m_printer_enabled) {
        m_printer.print_sweep_stat(sweep_stat, pause_stat);
    }
    m_initiation_policy->update(state());
}

gc_info garbage_collector::info() const
{
    assert(m_strategy);
    return m_strategy->info();
}

gc_stat garbage_collector::stats() const
{
    gc_unsafe_scope unsafe_scope;
    gc_stat stat;
    stat.gc_count = m_recorder.gc_cycles_count();
    stat.gc_time = m_recorder.gc_pause_time();
    return stat;
}

gc_state garbage_collector::state() const
{
    return m_recorder.state();
}

bool garbage_collector::check_gc_phase(gc_phase phase)
{
    return (phase == gc_phase::MARK && m_gc_info.incremental) || phase == gc_phase::SWEEP;
}

bool garbage_collector::is_interior_pointer(const gc_handle& handle, byte* p)
{
    managed_ptr cell_ptr(handle.rbarrier());
    byte* cell_begin = cell_ptr.get_cell_begin();
    byte* cell_end   = cell_begin + cell_ptr.cell_size();
    return (cell_begin <= p) && (p <= cell_end);
}

bool garbage_collector::is_interior_shift(const gc_handle& handle, ptrdiff_t shift)
{
    return is_interior_pointer(handle, handle.rbarrier() + shift);
}

}}
