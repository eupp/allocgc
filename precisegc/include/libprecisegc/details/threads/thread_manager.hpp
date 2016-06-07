#ifndef DIPLOMA_THREAD_MANAGER_HPP
#define DIPLOMA_THREAD_MANAGER_HPP

#include <map>
#include <mutex>
#include <thread>

#include <boost/range/iterator_range.hpp>
#include <boost/range/adaptor/map.hpp>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_exception.hpp>
#include <libprecisegc/details/utils/lock_range.hpp>

namespace precisegc { namespace details { namespace threads {

class managed_thread;
class world_snapshot;

namespace internals {
struct thread_manager_access;
}

class thread_manager : private utils::noncopyable, private utils::nonmovable
{
    typedef std::map<std::thread::id, managed_thread*> map_type;
    typedef std::recursive_mutex lock_type;

    typedef utils::locked_range<
    boost::select_second_const_range<
            boost::iterator_range<map_type::const_iterator>
        >
        , lock_type> range_type;

    friend class internals::thread_manager_access;
public:
    static thread_manager& instance();

    ~thread_manager();

    void register_main_thread();
    void register_thread(managed_thread* thread_ptr);
    void deregister_thread(managed_thread* thread_ptr);

    managed_thread* lookup_thread(std::thread::id thread_id) const;

    world_snapshot stop_the_world();
private:
    thread_manager() = default;

    range_type get_managed_threads() const;

    map_type m_threads;
    mutable lock_type m_lock;
};

namespace internals {
struct thread_manager_access
{
    typedef thread_manager::range_type range_type;

    static range_type get_managed_threads(const thread_manager& manager);
};
}

}}}

#endif //DIPLOMA_THREAD_MANAGER_HPP
