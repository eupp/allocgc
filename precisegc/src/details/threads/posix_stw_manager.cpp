#include <libprecisegc/details/threads/stw_manager.hpp>

#include <atomic>

#include <libprecisegc/details/logging.hpp>
#include <libprecisegc/details/threads/posix_signal.hpp>
#include <libprecisegc/details/threads/posix_thread.hpp>
#include <libprecisegc/details/threads/return_address.hpp>

namespace precisegc { namespace details { namespace threads {

void stw_manager::sighandler()
{
    stw_manager& stwm = stw_manager::instance();

//    logging::debug() << "Thread enters stop-the-world signal handler";

    stwm.m_stack_ends[std::this_thread::get_id()] = frame_address();

    ++stwm.m_threads_suspended_cnt;
    std::atomic_thread_fence(std::memory_order_seq_cst);
    stwm.m_barrier.notify();

    stwm.m_event.wait();
    --stwm.m_threads_suspended_cnt;
    std::atomic_thread_fence(std::memory_order_seq_cst);
    stwm.m_barrier.notify();

//    logging::debug() << "Thread leaves stop-the-world signal handler";
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

bool stw_manager::is_stop_the_world_disabled() const
{
    return posix_signal::instance().is_locked();
}

void stw_manager::suspend_thread(std::thread::native_handle_type thread)
{
    logging::debug() << "Sending stop-the-world signal to thread " << thread;

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
    std::atomic_thread_fence(std::memory_order_seq_cst);
}

void stw_manager::wait_for_world_start()
{
    std::atomic_thread_fence(std::memory_order_seq_cst);
    m_event.notify(m_threads_cnt);
    m_barrier.wait(m_threads_cnt);
    m_threads_cnt = 0;
}

size_t stw_manager::threads_suspended() const
{
    return m_threads_suspended_cnt;
}

byte* stw_manager::get_thread_stack_end(std::thread::id id)
{
    auto it = m_stack_ends.find(id);
    return it != m_stack_ends.end() ? it->second : frame_address();
}

}}}
