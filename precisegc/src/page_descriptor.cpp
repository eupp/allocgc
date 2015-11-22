#include "page_descriptor.h"

#include <cstddef>
#include <cassert>

namespace precisegc { namespace details {

void* page_descriptor::allocate()
{
    assert(!is_full());
    void* res = m_free;
    m_free = (void*) ((size_t) m_free + m_obj_size);
    return res;
}

bool page_descriptor::is_full() const noexcept
{
    return (size_t) m_free == (size_t) m_page + m_page_size;
}

}}