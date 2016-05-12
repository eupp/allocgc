#ifndef DIPLOMA_MANAGED_THREAD_HPP
#define DIPLOMA_MANAGED_THREAD_HPP

#include <libprecisegc/details/util.h>
#include <libprecisegc/details/root_set.hpp>

namespace precisegc { namespace details { namespace threads {

class managed_thread : private noncopyable, private nonmovable
{
public:
    static managed_thread& this_thread();

    root_set& get_root_set();
    root_set& get_pin_set();
private:
    managed_thread();
    ~managed_thread();

    root_set m_root_set;
    root_set m_pin_set;
};

}}}

#endif //DIPLOMA_MANAGED_THREAD_HPP
