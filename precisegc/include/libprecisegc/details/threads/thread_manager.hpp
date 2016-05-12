#ifndef DIPLOMA_THREAD_MANAGER_HPP
#define DIPLOMA_THREAD_MANAGER_HPP

#include <list>
#include <mutex>

#include <libprecisegc/details/util.h>
#include <libprecisegc/details/threads/managed_thread.hpp>

namespace precisegc { namespace details { namespace threads {

class thread_manager : private noncopyable, private nonmovable
{
public:
    thread_manager();
    thread_manager(thread_manager&&);

    thread_manager& operator=(thread_manager&);

    void register_thread(managed_thread* thread_ptr);
    void deregister_thread(managed_thread* thread_ptr);

    void stop_the_world();
    void start_the_world();
private:
    std::list<managed_thread*> m_threads;
    std::mutex m_stw_mutex;
};

}}}

#endif //DIPLOMA_THREAD_MANAGER_HPP
