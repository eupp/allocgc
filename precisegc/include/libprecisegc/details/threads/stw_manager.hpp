#ifndef DIPLOMA_STW_MANAGER_HPP
#define DIPLOMA_STW_MANAGER_HPP

#include <thread>
#include <atomic>

#include <libprecisegc/details/util.h>
#include <libprecisegc/details/threads/ass_sync.hpp>

namespace precisegc { namespace details { namespace threads {

class stw_manager : private noncopyable, private nonmovable
{
public:
    static stw_manager& instance();

    bool is_stop_the_world_disabled() const;

    void suspend_thread(std::thread::native_handle_type thread);
    void resume_thread(std::thread::native_handle_type thread);

    void wait_for_world_stop();
    void wait_for_world_start();

    size_t threads_suspended() const;
private:
    static void sighandler();

    stw_manager();

    ass_barrier m_barrier;
    ass_event m_event;
    size_t m_threads_cnt;
    std::atomic<size_t> m_threads_suspended_cnt;
};

}}}

#endif //DIPLOMA_STW_MANAGER_HPP
