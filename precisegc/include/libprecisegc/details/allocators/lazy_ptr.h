#ifndef DIPLOMA_LAZY_PTR_H
#define DIPLOMA_LAZY_PTR_H

#include <memory>

namespace precisegc { namespace details { namespace allocators {

template<typename Ptr, typename Creator>
class lazy_ptr: private Creator
{
public:
    typedef std::pointer_traits<Ptr>::element_type element_type;

    lazy_ptr() = default;

    lazy_ptr(nullptr_t)
        : m_ptr(nullptr)
    {}

    lazy_ptr(Ptr ptr)
        : m_ptr(ptr)
    {}

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
        return m_ptr.operator->();
    }

    element_type* get() const
    {
        create();
        return m_ptr.get();
    }

    void reset(Ptr ptr)
    {
        m_ptr = ptr;
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
            m_ptr = Creator::operator();
        }
    }

    mutable Ptr m_ptr;
};

}}}

#endif //DIPLOMA_LAZY_PTR_H
