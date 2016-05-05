#ifndef DIPLOMA_MANAGED_THREAD_HPP
#define DIPLOMA_MANAGED_THREAD_HPP

#include <libprecisegc/details/util.h>
#include <libprecisegc/details/root_set.hpp>

namespace precisegc { namespace details { namespace threads {

class managed_thread : private noncopyable
{
public:
    managed_thread();
    managed_thread(managed_thread&& other);

    managed_thread& operator=(managed_thread&& other);

    root_set& get_root_set();
    root_set& get_pin_set();
private:
    root_set m_root_set;
    root_set m_pin_set;
};

}}}

#endif //DIPLOMA_MANAGED_THREAD_HPP
