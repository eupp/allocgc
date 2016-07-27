#ifndef DIPLOMA_MANAGED_THREAD_ACCESSOR_HPP
#define DIPLOMA_MANAGED_THREAD_ACCESSOR_HPP

#include <libprecisegc/details/threads/stack_map.hpp>
#include <libprecisegc/details/threads/tlab.hpp>
#include <libprecisegc/details/collectors/packet_manager.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace threads {

class thread_manager;
class world_snapshot;
class managed_thread;
class this_managed_thread;

class managed_thread_accessor : private utils::noncopyable, private utils::nonmovable
{
public:
    managed_thread_accessor() = delete;
private:
    static root_stack_map& root_set(managed_thread* thread);
    static pin_stack_map& pin_set(managed_thread* thread);
    static tlab& lab(managed_thread* thread);

    static void set_this_managed_thread_pointer(managed_thread* thread);

    friend class thread_manager;
    friend class world_snapshot;
    friend class managed_thread;
    friend class this_managed_thread;
};

}}}

#endif //DIPLOMA_MANAGED_THREAD_ACCESSOR_HPP
