#ifndef DIPLOMA_GC_PIN_H
#define DIPLOMA_GC_PIN_H

#include "details/page_descriptor.h"
#include "index_tree.h"

template <typename T>
class gc_pin
{
public:

    gc_pin(T* ptr)
        : m_ptr(ptr)
        , m_pd((precisegc::details::page_descriptor*) _GC_::IT_get_page_descr(ptr))
    {
        m_pd->set_object_mark(m_ptr, true);
        m_pd->set_object_pin(m_ptr, true);
    }

    ~gc_pin()
    {
        m_pd->set_object_pin(m_ptr, false);
        m_pd->set_object_mark(m_ptr, false);
    }

    T* operator->()
    {
        return m_ptr;
    }

private:
    T* m_ptr;
    precisegc::details::page_descriptor* m_pd;
};

#endif //DIPLOMA_GC_PIN_H
