#ifndef DIPLOMA_DPTR_DESCRIPTOR_HPP
#define DIPLOMA_DPTR_DESCRIPTOR_HPP

#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details { namespace collectors {

struct dptr_descriptor
{
    byte* m_origin;
    byte* m_derived;
    // temporary solution: to deallocate all descriptors that point to unmarked objects
    // we maintain a list of all descriptors
    dptr_descriptor* m_next;
};

}}}

#endif //DIPLOMA_DPTR_DESCRIPTOR_HPP
