#ifndef DIPLOMA_LAZY_PTR_H
#define DIPLOMA_LAZY_PTR_H

#include <memory>

#include "pointer_traits.h"

namespace precisegc { namespace details { namespace allocators {

template<typename Ptr, typename Factory>
class lazy_ptr: private Factory
{
public:
    typedef typename std::pointer_traits<Ptr>::element_type element_type;

    lazy_ptr() = default;

    lazy_ptr(nullptr_t)
        : m_ptr(nullptr)
    {}

    lazy_ptr(Ptr ptr)
        : m_ptr(ptr)
    {}

    ~lazy_ptr()
    {
        Factory::destroy(m_ptr);
    }

    lazy_ptr(const lazy_ptr&) = default;
    lazy_ptr(lazy_ptr&&) = default;

    lazy_ptr& operator=(const lazy_ptr&) = default;
    lazy_ptr& operator=(lazy_ptr&&) = default;

    element_type& operator*() const
    {
        create();
        return *m_ptr;
    }

    element_type* operator->() const
    {
        create();
        return pointer_traits<Ptr>::member_of_operator(m_ptr);
    }

    element_type* get() const
    {
        create();
        return m_ptr.get();
    }

    void reset()
    {

    }

    void reset(Ptr ptr)
    {
        m_ptr.reset(ptr);
    }

    Ptr& get_wrapped()
    {
        return m_ptr;
    }

    const Ptr& get_wrapped() const
    {
        return m_ptr;
    }
private:
    void create() const
    {
        if (!m_ptr) {
            m_ptr = Factory::create();
        }
    }

    mutable Ptr m_ptr;
};

}}}

#endif //DIPLOMA_LAZY_PTR_H
