#ifndef DIPLOMA_MANAGED_THREAD_ACCESS_HPP
#define DIPLOMA_MANAGED_THREAD_ACCESS_HPP

#include <libprecisegc/details/collectors/packet_manager.hpp>
#include <libprecisegc/details/stack_map.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace threads {

class managed_thread;
class this_managed_thread;

class managed_thread_accessor : private utils::noncopyable, private utils::nonmovable
{
public:
    managed_thread_accessor() = delete;
private:
    static root_stack_map& root_set(managed_thread* thread);
    static pin_stack_map& pin_set(managed_thread* thread);

    static void set_this_managed_thread_pointer(managed_thread* thread);

    friend class this_managed_thread;
};

}}}

#endif //DIPLOMA_MANAGED_THREAD_ACCESS_HPP
