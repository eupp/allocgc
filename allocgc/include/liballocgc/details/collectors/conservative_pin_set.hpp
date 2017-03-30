#ifndef ALLOCGC_CONSERVATIVE_PIN_SET_HPP
#define ALLOCGC_CONSERVATIVE_PIN_SET_HPP

#include <liballocgc/gc_common.hpp>
#include <liballocgc/details/gc_interface.hpp>

namespace allocgc { namespace details { namespace collectors {

class conservative_pin_set
{
public:
    inline void register_pin(byte* pin)
    {
        return;
    }

    inline void deregister_pin(byte* pin)
    {
        return;
    }

    inline void push_pin(byte* pin)
    {
        return;
    }

    inline void pop_pin(byte* pin)
    {
        return;
    }

    inline void trace(const gc_trace_pin_callback& cb) const
    {
        return;
    }
};

}}}

#endif //ALLOCGC_CONSERVATIVE_PIN_SET_HPP
