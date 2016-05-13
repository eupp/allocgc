#include <libprecisegc/details/threads/thread_manager.hpp>

#include <libprecisegc/details/logging.h>
#include <libprecisegc/details/threads/stw_manager.hpp>
#include <libprecisegc/details/threads/managed_thread.hpp>

namespace precisegc { namespace details { namespace threads {

thread_manager& thread_manager::instance()
{
    static thread_manager tm;
    return tm;
}

void thread_manager::register_thread(managed_thread* thread_ptr)
{
    logging::info() << "Register new managed thread " << thread_ptr->get_id();
    std::lock_guard<std::mutex> lock(m_lock);
    m_threads[thread_ptr->get_id()] = thread_ptr;
}

void thread_manager::deregister_thread(managed_thread* thread_ptr)
{
    logging::info() << "Deregister managed thread " << thread_ptr->get_id();
    std::lock_guard<std::mutex> lock(m_lock);
    m_threads.erase(thread_ptr->get_id());
}

managed_thread* thread_manager::lookup_thread(std::thread::id thread_id) const
{
    std::lock_guard<std::mutex> lock(m_lock);
    auto it = m_threads.find(thread_id);
    return it != m_threads.end() ? it->second : nullptr;
}

void thread_manager::stop_the_world()
{
    static stw_manager& stwm = stw_manager::instance();

    logging::info() << "Thread " << std::this_thread::get_id() << " is requesting stop-the-world";

    if (stwm.is_stop_the_world_disabled()) {
        throw stop_the_world_disabled();
    }

    m_lock.lock();
    for (auto thread: m_threads) {
        if (thread.second->native_handle() != managed_thread::this_thread().native_handle()) {
            stwm.suspend_thread(thread.second->native_handle());
        }
    }
    stwm.wait_for_world_stop();

    logging::info() << "World stopped";
}

thread_manager::range_type thread_manager::get_managed_threads() const
{
    return range_type(
            boost::make_iterator_range(m_threads.begin(), m_threads.end()) | boost::adaptors::map_values,
            m_lock
        );
}

void thread_manager::start_the_world()
{
    static stw_manager& stwm = stw_manager::instance();

    logging::info() << "Thread " << std::this_thread::get_id() << " is requesting start-the-world";

    for (auto thread: m_threads) {
        if (thread.second->native_handle() != managed_thread::this_thread().native_handle()) {
            stwm.resume_thread(thread.second->native_handle());
        }
    }
    stwm.wait_for_world_start();
    m_lock.unlock();

    logging::info() << "World started";
}

}}}
