#include <liballocgc/details/gc_facade.hpp>

#include <cassert>
#include <memory>
#include <iostream>

#include <liballocgc/details/allocators/memory_index.hpp>
#include <liballocgc/details/utils/scope_guard.hpp>
#include <liballocgc/details/gc_unsafe_scope.hpp>
#include <liballocgc/details/logging.hpp>

namespace allocgc { namespace details {

const size_t gc_facade::HEAP_START_LIMIT = 4 * 1024 * 1024;

const double gc_facade::INCREASE_FACTOR      = 2.0;
const double gc_facade::MARK_THRESHOLD       = 0.6;
const double gc_facade::COLLECT_THRESHOLD    = 1.0;

gc_facade::gc_facade()
    : m_manager(nullptr)
    , m_heap_size(0)
    , m_heap_limit(0)
    , m_heap_maxlimit(0)
{
    logging::touch();
    // same hack as with logger
    allocators::memory_index::init();
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

bool gc_facade::compare(const gc_handle& a, const gc_handle& b)
{
    gc_unsafe_scope unsafe_scope;
    return a.rbarrier() == b.rbarrier();
}

void gc_facade::register_thread(const thread_descriptor& descr)
{
    assert(m_strategy);
    m_strategy->register_thread(descr);
}

void gc_facade::deregister_thread(std::thread::id id)
{
    assert(m_strategy);
    m_strategy->deregister_thread(id);
}

bool gc_facade::increment_heap_size(size_t alloc_size)
{
    std::unique_lock<std::mutex> lock(m_heap_mutex);
    size_t size = m_heap_size + alloc_size;
    if (size > COLLECT_THRESHOLD * m_heap_limit) {
        return false;
    }
    else if (size > MARK_THRESHOLD * m_heap_limit) {
        gc_options opt;
        opt.kind = gc_kind::CONCURRENT_MARK;
        opt.gen  = 0;

        lock.unlock();
        initiation_point(initiation_point_type::HEAP_LIMIT_EXCEEDED, opt);
        lock.lock();
    }
    m_heap_size += alloc_size;
    return true;
}

void gc_facade::decrement_heap_size(size_t size)
{
    std::lock_guard<std::mutex> lock(m_heap_mutex);
    m_heap_size -= size;
}

void gc_facade::set_heap_limit(size_t size)
{
    std::lock_guard<std::mutex> lock(m_heap_mutex);
    m_heap_limit = (size == std::numeric_limits<size_t>::max()) ? HEAP_START_LIMIT : size;
    m_heap_maxlimit = size;
}

void gc_facade::expand_heap()
{
    std::lock_guard<std::mutex> lock(m_heap_mutex);
    size_t increased_size = INCREASE_FACTOR * m_heap_limit;
    m_heap_limit = std::min(increased_size, m_heap_maxlimit);
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

gc_stat gc_facade::stats() const
{
    gc_unsafe_scope unsafe_scope;
    return m_manager.stats();
}

bool gc_facade::is_interior_pointer(const gc_handle& handle, byte* iptr)
{
    using namespace allocators;

    byte* ptr = handle.rbarrier();
    gc_memory_descriptor* descr = memory_index::get_descriptor(ptr).to_gc_descriptor();
    byte* cell_begin = descr->cell_start(ptr);
    byte* cell_end   = cell_begin + descr->cell_size(ptr);
    return (cell_begin <= iptr) && (iptr < cell_end);
}

bool gc_facade::is_interior_offset(const gc_handle& handle, ptrdiff_t shift)
{
    return is_interior_pointer(handle, handle.rbarrier() + shift);
}

}}
