#include <libprecisegc/details/threads/thread_manager.hpp>

#include <libprecisegc/details/threads/stw_manager.hpp>
#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/threads/managed_thread_accessor.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <libprecisegc/details/logging.hpp>

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
    managed_thread& main_thread = managed_thread::main_thread();
    managed_thread_accessor::set_this_managed_thread_pointer(&main_thread);
    std::lock_guard<lock_type> lock(m_lock);
    logging::info() << "Register main thread";
    m_threads[main_thread.get_id()] = &main_thread;
}

void thread_manager::register_thread(managed_thread* thread_ptr)
{
    std::lock_guard<lock_type> lock(m_lock);
    logging::info() << "Register new managed thread " << thread_ptr->get_id();
    m_threads[thread_ptr->get_id()] = thread_ptr;
}

void thread_manager::deregister_thread(managed_thread* thread_ptr)
{
    std::lock_guard<lock_type> lock(m_lock);
    logging::info() << "Deregister managed thread " << thread_ptr->get_id();
    m_threads.erase(thread_ptr->get_id());
}

managed_thread* thread_manager::lookup_thread(std::thread::id thread_id) const
{
    std::lock_guard<lock_type> lock(m_lock);
    auto it = m_threads.find(thread_id);
    return it != m_threads.end() ? it->second : nullptr;
}

thread_manager::threads_range_type thread_manager::threads_snapshot() const
{
    std::unique_lock<lock_type> lock(m_lock);
    return threads_range_type(
            boost::make_iterator_range(m_threads.begin(), m_threads.end()) | boost::adaptors::map_values,
            std::move(lock)
    );
}

world_snapshot thread_manager::stop_the_world() const
{
    return world_snapshot(threads_snapshot());
}

}}}
