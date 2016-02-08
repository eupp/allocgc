#include "gc_untyped_pin.h"

#include "gc_consistency.h"
#include "index_tree.h"

namespace precisegc { namespace details {

gc_untyped_pin::gc_untyped_pin(const gc_untyped_ptr& ptr)
{
    gc_consistency_lock consistency_lock;
    m_raw_ptr = ptr.get();
    m_pd = (page_descriptor*) _GC_::IT_get_page_descr(m_raw_ptr);
    m_pd->set_object_mark(m_raw_ptr, true);
    m_pd->set_object_pin(m_raw_ptr, true);
}

gc_untyped_pin::~gc_untyped_pin()
{
    gc_consistency_lock consistency_lock;
    m_pd->set_object_pin(m_raw_ptr, false);
    m_pd->set_object_mark(m_raw_ptr, false);
}

void* gc_untyped_pin::get() const noexcept
{
    return m_raw_ptr;
}

}}