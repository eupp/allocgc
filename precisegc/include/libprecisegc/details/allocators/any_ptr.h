#ifndef DIPLOMA_ANY_PTR_H
#define DIPLOMA_ANY_PTR_H

#include <memory>

namespace precisegc { namespace details { namespace allocators {

template <typename Ptr>
class any_ptr
{
    typedef std::pointer_traits<Ptr>::element_type element_type;
public:

    any_ptr() = default;

    any_ptr(nullptr_t)
        : m_ptr(nullptr)
    {}
    
    any_ptr(Ptr ptr)
        : m_ptr(ptr)
    {}

    any_ptr(const any_ptr&) = default;
    any_ptr(any_ptr&&) = default;

    any_ptr& operator=(const any_ptr&) = default;
    any_ptr& operator=(any_ptr&&) = default;

    template <typename T>
    T* as() const
    {
        element_type* ptr = m_ptr.get();
        return reinterpret_cast<T*>(ptr);
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
    Ptr m_ptr;
};

}}}

#endif //DIPLOMA_ANY_PTR_H
