#include <libprecisegc/details/gc_hooks.hpp>

#include <libprecisegc/gc_handle.hpp>
#include <libprecisegc/details/gc_facade.hpp>

#include <libprecisegc/details/collectors/static_root_set.hpp>
#include <libprecisegc/details/allocators/memory_index.hpp>

namespace precisegc {

static details::gc_facade gc_instance{};

byte* gc_handle::rbarrier() const
{
    return gc_instance.rbarrier(*this);
}

void gc_handle::wbarrier(const gc_handle& other)
{
    gc_instance.wbarrier(*this, other);
}

void gc_handle::interior_wbarrier(ptrdiff_t offset)
{
    gc_instance.interior_wbarrier(*this, offset);
}

gc_handle::pin_guard gc_handle::pin() const
{
    return pin_guard(*this);
}

gc_handle::stack_pin_guard gc_handle::push_pin() const
{
    return stack_pin_guard(*this);
}

void gc_handle::reset()
{
    gc_instance.wbarrier(*this, null);
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
        : m_ptr(gc_instance.register_pin(handle))
{}

gc_handle::pin_guard::pin_guard(pin_guard&& other)
        : m_ptr(other.m_ptr)
{
    other.m_ptr = nullptr;
}

gc_handle::pin_guard::~pin_guard()
{
    gc_instance.deregister_pin(m_ptr);
}

gc_handle::pin_guard& gc_handle::pin_guard::operator=(pin_guard&& other)
{
    std::swap(m_ptr, other.m_ptr);
    return *this;
}

byte* gc_handle::pin_guard::get() const noexcept
{
    return m_ptr;
}

gc_handle::stack_pin_guard::stack_pin_guard(const gc_handle& handle)
        : m_ptr(gc_instance.push_pin(handle))
{}

gc_handle::stack_pin_guard::stack_pin_guard(stack_pin_guard&& other)
        : m_ptr(other.m_ptr)
{
    other.m_ptr = nullptr;
}

gc_handle::stack_pin_guard::~stack_pin_guard()
{
    gc_instance.pop_pin(m_ptr);
}

byte* gc_handle::stack_pin_guard::get() const noexcept
{
    return m_ptr;
}

gc_handle gc_handle::null{nullptr};

namespace details {

void gc_initialize(std::unique_ptr<gc_strategy> strategy)
{
    gc_instance.init(std::move(strategy));
}

bool gc_is_heap_ptr(const gc_handle* ptr)
{
    using namespace threads;
    using namespace collectors;
    return !allocators::memory_index::get_descriptor(reinterpret_cast<const byte*>(ptr)).is_null();
}

gc_alloc::response gc_allocate(const gc_alloc::request& rqst)
{
    return gc_instance.allocate(rqst);
}

void gc_abort(const gc_alloc::response& rsp)
{
    gc_instance.abort(rsp);
}

void gc_commit(const gc_alloc::response& rsp)
{
    gc_instance.commit(rsp);
}

void gc_commit(const gc_alloc::response& rsp, const gc_type_meta* type_meta)
{
    gc_instance.commit(rsp, type_meta);
}

gc_offsets gc_make_offsets(const gc_alloc::response& rsp)
{
    return gc_instance.make_offsets(rsp);
}

void gc_register_handle(gc_handle& handle, byte* ptr)
{
    gc_instance.register_handle(handle, ptr);
}

void gc_deregister_handle(gc_handle& handle)
{
    gc_instance.deregister_handle(handle);
}

void gc_register_thread(const thread_descriptor& descr)
{
    gc_instance.register_thread(descr);
}

void gc_deregister_thread(std::thread::id id)
{
    gc_instance.deregister_thread(id);
}

void gc_initiation_point(initiation_point_type ipoint, const gc_options& opt)
{
    gc_instance.initiation_point(ipoint, opt);
}

bool gc_increase_heap_size(size_t alloc_size)
{
    return gc_instance.increment_heap_size(alloc_size);
}

void gc_decrease_heap_size(size_t size)
{
    gc_instance.decrement_heap_size(size);
}

void gc_set_heap_limit(size_t size)
{
    gc_instance.set_heap_limit(size);
}

void gc_expand_heap()
{
    gc_instance.expand_heap();
}

gc_stat gc_get_stats()
{
    return gc_instance.stats();
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

}

}