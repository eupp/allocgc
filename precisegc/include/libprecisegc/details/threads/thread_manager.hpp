#ifndef DIPLOMA_THREAD_MANAGER_HPP
#define DIPLOMA_THREAD_MANAGER_HPP

#include <map>
#include <mutex>
#include <thread>

#include <boost/range/iterator_range.hpp>
#include <boost/range/adaptor/map.hpp>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/utils/gc_exception.hpp>
#include <libprecisegc/details/utils/lock_range.hpp>

namespace precisegc { namespace details { namespace threads {

class managed_thread;

class thread_manager : private noncopyable, private nonmovable
{
    typedef std::map<std::thread::id, managed_thread*> map_type;
    typedef std::recursive_mutex lock_type;
public:
    typedef utils::locked_range<
            boost::select_second_const_range<
                    boost::iterator_range<map_type::const_iterator>
                    >
            , lock_type> range_type;

    class stop_the_world_disabled : public utils::gc_exception
    {
    public:
        stop_the_world_disabled()
            : gc_exception("stop-the-world is disabled by current thread")
        {}
    };

    static thread_manager& instance();

    void register_main_thread();
    void register_thread(managed_thread* thread_ptr);
    void deregister_thread(managed_thread* thread_ptr);

    managed_thread* lookup_thread(std::thread::id thread_id) const;

    range_type get_managed_threads() const;

    void stop_the_world();
    void start_the_world();
private:
    thread_manager() = default;

    map_type m_threads;
    mutable lock_type m_lock;
};

}}}

#endif //DIPLOMA_THREAD_MANAGER_HPP
