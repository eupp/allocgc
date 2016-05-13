#ifndef DIPLOMA_THREAD_MANAGER_HPP
#define DIPLOMA_THREAD_MANAGER_HPP

#include <map>
#include <mutex>
#include <thread>

#include <libprecisegc/details/util.h>
#include <libprecisegc/details/utils/gc_exception.hpp>

namespace precisegc { namespace details { namespace threads {

class managed_thread;

class thread_manager : private noncopyable, private nonmovable
{
public:
    class stop_the_world_disabled : public utils::gc_exception
    {
    public:
        stop_the_world_disabled()
            : gc_exception("stop-the-world is disabled by current thread")
        {}
    };

    static thread_manager& instance();

    void register_thread(managed_thread* thread_ptr);
    void deregister_thread(managed_thread* thread_ptr);

    managed_thread* lookup_thread(std::thread::id thread_id) const;

    void stop_the_world();
    void start_the_world();
private:
    thread_manager() = default;

    std::map<std::thread::id, managed_thread*> m_threads;
    mutable std::mutex m_mutex;
};

}}}

#endif //DIPLOMA_THREAD_MANAGER_HPP
