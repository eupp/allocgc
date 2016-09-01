#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_handle.hpp>

#include <libprecisegc/details/garbage_collector.hpp>

namespace precisegc { namespace details {

static garbage_collector gc_instance{};

void gc_initialize(std::unique_ptr<gc_strategy> strategy, std::unique_ptr<initiation_policy> init_policy)
{
    gc_instance.init(std::move(strategy), std::move(init_policy));
}

std::pair<managed_ptr, object_meta*> gc_allocate(size_t size, const type_meta* tmeta)
{
    return gc_instance.allocate(size, tmeta);
}

void gc_initiation_point(initiation_point_type ipoint, const initiation_point_data& ipd)
{
    gc_instance.initiation_point(ipoint, ipd);
}

gc_info gc_get_info()
{
    return gc_instance.info();
}

gc_stat gc_get_stats()
{
    return gc_instance.stats();
}

gc_state gc_get_state()
{
    return gc_instance.state();
}

void gc_enable_print_stats()
{
    gc_instance.set_printer_enabled(true);
}

void gc_disable_print_stats()
{
    gc_instance.set_printer_enabled(false);
}

void gc_register_page(const byte* page, size_t size)
{
    gc_instance.register_page(page, size);
}

void gc_deregister_page(const byte* page, size_t size)
{
    gc_instance.deregister_page(page, size);
}

byte* gc_handle::rbarrier() const
{
    return gc_instance.rbarrier(*this);
}

void gc_handle::wbarrier(const gc_handle& other)
{
    gc_instance.wbarrier(*this, other);
}

void gc_handle::interior_wbarrier(byte* ptr)
{
    gc_instance.interior_wbarrier(*this, ptr);
}

void gc_handle::interior_shift(ptrdiff_t shift)
{
    gc_instance.interior_shift(*this, shift);
}

gc_handle::pin_guard gc_handle::pin() const
{
    return pin_guard(*this);
}

void gc_handle::reset()
{
    gc_instance.interior_wbarrier(*this, nullptr);
}

bool gc_handle::equal(const gc_handle& other) const
{
    return gc_instance.compare(*this, other);
}

bool gc_handle::is_null() const
{
    return rbarrier() == nullptr;
}

gc_handle::pin_guard::pin_guard(const gc_handle& handle)
    : m_ptr(gc_instance.pin(handle))
{}

gc_handle::pin_guard::pin_guard(pin_guard&& other)
    : m_ptr(other.m_ptr)
{
    other.m_ptr = nullptr;
}

gc_handle::pin_guard::~pin_guard()
{
    if (m_ptr) {
        gc_instance.unpin(m_ptr);
    }
}

gc_handle::pin_guard& gc_handle::pin_guard::operator=(pin_guard&& other)
{
    m_ptr = other.m_ptr;
    other.m_ptr = nullptr;
}

byte* gc_handle::pin_guard::get() const noexcept
{
    return m_ptr;
}

byte* gc_handle_access::get(const gc_handle& handle, std::memory_order order)
{
    const atomic_byte_ptr* atomic_ptr = reinterpret_cast<const atomic_byte_ptr*>(&handle.m_ptr);
    return atomic_ptr->load(order);
}

void gc_handle_access::set(gc_handle& handle, byte* ptr, std::memory_order order)
{
    atomic_byte_ptr* atomic_ptr = reinterpret_cast<atomic_byte_ptr*>(&handle.m_ptr);
    atomic_ptr->store(ptr, order);
}

void gc_handle_access::advance(gc_handle& handle, ptrdiff_t n, std::memory_order order)
{
    atomic_byte_ptr* atomic_ptr = reinterpret_cast<atomic_byte_ptr*>(&handle.m_ptr);
    atomic_ptr->fetch_add(n, order);
}

}}