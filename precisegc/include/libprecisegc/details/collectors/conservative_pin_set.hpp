#ifndef DIPLOMA_CONSERVATIVE_PIN_SET_HPP
#define DIPLOMA_CONSERVATIVE_PIN_SET_HPP

#include <libprecisegc/gc_common.hpp>
#include <libprecisegc/details/gc_interface.hpp>

namespace precisegc { namespace details { namespace collectors {

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

#endif //DIPLOMA_CONSERVATIVE_PIN_SET_HPP
