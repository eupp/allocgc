#include <libprecisegc/details/threads/stw_manager.hpp>

#include <type_traits>
#include <pthread.h>

#include <libprecisegc/details/logging.h>
#include <libprecisegc/details/threads/posix_signal.hpp>

namespace precisegc { namespace details { namespace threads {

static_assert(std::is_same<std::thread::native_handle_type, pthread_t>::value, "Only pthreads are supported");

void stw_manager::sighandler()
{
    stw_manager& stwm = stw_manager::instance();

    logging::debug() << "Thread " << std::this_thread::get_id() << " enters stw signal handler";

    ++stwm.m_threads_suspended_cnt;
    stwm.m_barrier.notify();

    stwm.m_event.wait();
    --stwm.m_threads_suspended_cnt;
    stwm.m_barrier.notify();

    logging::debug() << "Thread " << std::this_thread::get_id() << " leaves stw signal handler";
}

stw_manager& stw_manager::instance()
{
    static stw_manager stwm;
    return stwm;
}

stw_manager::stw_manager()
    : m_threads_cnt(0)
    , m_threads_suspended_cnt(0)
{
    posix_signal::instance().set_handler(stw_manager::sighandler);
}

void stw_manager::suspend_thread(std::thread::native_handle_type thread)
{
    logging::debug() << "Sending stw signal to thread " << std::this_thread::get_id();

    ++m_threads_cnt;
    posix_signal::instance().send(thread);
}

void stw_manager::resume_thread(std::thread::native_handle_type thread)
{
    return;
}

void stw_manager::wait_for_world_stop()
{
    m_barrier.wait(m_threads_cnt);

    logging::debug() << "All threads are suspended";
}

void stw_manager::wait_for_world_start()
{
    m_event.notify(m_threads_cnt);
    m_barrier.wait(m_threads_cnt);
    m_threads_cnt = 0;

    logging::debug() << "All threads are resumed";
}

size_t stw_manager::threads_suspended() const
{
    return m_threads_suspended_cnt;
}

}}}
