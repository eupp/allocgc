#include <libprecisegc/details/threads/thread_manager.hpp>

#include <libprecisegc/details/logging.h>
#include <libprecisegc/details/threads/stw_manager.hpp>
#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/threads/threads_snapshot.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>

namespace precisegc { namespace details { namespace threads {

thread_manager& thread_manager::instance()
{
    static thread_manager tm;
    return tm;
}

thread_manager::~thread_manager()
{
    std::lock_guard<lock_type> lock(m_lock);
    m_threads.clear();
}

void thread_manager::register_main_thread()
{
    managed_thread& main_thread = managed_thread::this_thread();

    logging::info() << "Register main thread " << main_thread.get_id();
    std::lock_guard<lock_type> lock(m_lock);
    m_threads[main_thread.get_id()] = &main_thread;
}

void thread_manager::register_thread(managed_thread* thread_ptr)
{
    logging::info() << "Register new managed thread " << thread_ptr->get_id();
    std::lock_guard<lock_type> lock(m_lock);
    m_threads[thread_ptr->get_id()] = thread_ptr;
}

void thread_manager::deregister_thread(managed_thread* thread_ptr)
{
    logging::info() << "Deregister managed thread " << thread_ptr->get_id();
    std::lock_guard<lock_type> lock(m_lock);
    m_threads.erase(thread_ptr->get_id());
}

managed_thread* thread_manager::lookup_thread(std::thread::id thread_id) const
{
    std::lock_guard<lock_type> lock(m_lock);
    auto it = m_threads.find(thread_id);
    return it != m_threads.end() ? it->second : nullptr;
}

threads_snapshot thread_manager::threads() const
{
    std::unique_lock<lock_type> lock(m_lock, std::try_to_lock);
    auto range = range_type(
            boost::make_iterator_range(m_threads.begin(), m_threads.end()) | boost::adaptors::map_values,
            std::move(lock)
    );
    return threads_snapshot(std::move(range));
}

world_snapshot thread_manager::stop_the_world() const
{
    std::unique_lock<lock_type> lock(m_lock);
    auto range = range_type(
            boost::make_iterator_range(m_threads.begin(), m_threads.end()) | boost::adaptors::map_values,
            std::move(lock)
    );
    return world_snapshot(std::move(range));
}

}}}
