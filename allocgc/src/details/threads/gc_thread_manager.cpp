#include <liballocgc/details/threads/gc_thread_manager.hpp>

#include <liballocgc/details/threads/stw_manager.hpp>
#include <liballocgc/details/threads/world_snapshot.hpp>
#include <liballocgc/details/logging.hpp>

namespace allocgc { namespace details { namespace threads {

gc_thread_manager::~gc_thread_manager()
{
    std::lock_guard<lock_type> lock(m_lock);
    m_threads.clear();
}

void gc_thread_manager::register_thread(std::thread::id id, std::unique_ptr<gc_thread_descriptor> descr)
{
    std::lock_guard<lock_type> lock(m_lock);
    logging::info() << "Register new managed thread " << id;
    m_threads[id] = std::move(descr);
}

void gc_thread_manager::deregister_thread(std::thread::id id)
{
    std::lock_guard<lock_type> lock(m_lock);
    logging::info() << "Deregister managed thread " << id;
    m_threads.erase(id);
}

gc_thread_descriptor* gc_thread_manager::lookup_thread(std::thread::id thread_id) const
{
    std::lock_guard<lock_type> lock(m_lock);
    auto it = m_threads.find(thread_id);
    return it != m_threads.end() ? it->second.get() : nullptr;
}

gc_thread_manager::threads_range_type gc_thread_manager::threads_snapshot() const
{
    std::unique_lock<lock_type> lock(m_lock);
    return threads_range_type(
            boost::make_iterator_range(m_threads.begin(), m_threads.end()) | boost::adaptors::map_values,
            std::move(lock)
    );
}

world_snapshot gc_thread_manager::stop_the_world() const
{
    return world_snapshot(threads_snapshot());
}

}}}
