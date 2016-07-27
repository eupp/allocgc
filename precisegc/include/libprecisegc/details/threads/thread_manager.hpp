#ifndef DIPLOMA_THREAD_MANAGER_HPP
#define DIPLOMA_THREAD_MANAGER_HPP

#include <map>
#include <mutex>
#include <thread>

#include <boost/range/iterator_range.hpp>
#include <boost/range/adaptor/map.hpp>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_exception.hpp>
#include <libprecisegc/details/utils/locked_range.hpp>

namespace precisegc { namespace details { namespace threads {

class managed_thread;
class threads_snapshot;
class world_snapshot;

class thread_manager : private utils::noncopyable, private utils::nonmovable
{
    typedef std::map<std::thread::id, managed_thread*> map_type;
    typedef std::mutex lock_type;
public:
    typedef utils::locked_range<
            boost::select_second_const_range<
                    boost::iterator_range<map_type::const_iterator>
            >
            , lock_type> threads_range_type;

    static thread_manager& instance();

    ~thread_manager();

    void register_main_thread();
    void register_thread(managed_thread* thread_ptr);
    void deregister_thread(managed_thread* thread_ptr);

    managed_thread* lookup_thread(std::thread::id thread_id) const;

    threads_range_type threads_snapshot() const;
    world_snapshot stop_the_world() const;
private:
    thread_manager() = default;

    map_type m_threads;
    mutable lock_type m_lock;
};

}}}

#endif //DIPLOMA_THREAD_MANAGER_HPP
