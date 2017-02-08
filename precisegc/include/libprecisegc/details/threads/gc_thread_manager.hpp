#ifndef DIPLOMA_THREAD_MANAGER_HPP
#define DIPLOMA_THREAD_MANAGER_HPP

#include <map>
#include <mutex>
#include <thread>
#include <memory>

#include <boost/range/iterator_range.hpp>
#include <boost/range/adaptor/map.hpp>

#include <libprecisegc/details/threads/gc_thread_descriptor.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/utils/locked_range.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details { namespace threads {

class threads_snapshot;
class world_snapshot;

class gc_thread_manager : private utils::noncopyable, private utils::nonmovable
{
    typedef std::map<std::thread::id, std::unique_ptr<gc_thread_descriptor>> map_type;
    typedef std::mutex lock_type;
public:
    typedef utils::locked_range<
            boost::select_second_const_range<
                    boost::iterator_range<map_type::const_iterator>
            >
            , lock_type> threads_range_type;

    gc_thread_manager() = default;

    ~gc_thread_manager();

    void register_thread(std::thread::id id, std::unique_ptr<gc_thread_descriptor> descr);
    void deregister_thread(std::thread::id id);

    gc_thread_descriptor* lookup_thread(std::thread::id thread_id) const;

    threads_range_type threads_snapshot() const;
    world_snapshot stop_the_world() const;
private:
    map_type m_threads;
    mutable lock_type m_lock;
};

}}}

#endif //DIPLOMA_THREAD_MANAGER_HPP
