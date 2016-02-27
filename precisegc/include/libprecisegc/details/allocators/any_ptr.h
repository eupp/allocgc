#ifndef DIPLOMA_ANY_PTR_H
#define DIPLOMA_ANY_PTR_H

#include <memory>

namespace precisegc { namespace details { namespace allocators {

class any_ptr
{
public:

    any_ptr() = default;

    any_ptr(nullptr_t)
        : m_ptr(nullptr)
    {}

    template <typename T>
    explicit any_ptr(T* ptr)
        : m_ptr(reinterpret_cast<void*>(ptr))
    {}

    any_ptr(const any_ptr&) = default;
    any_ptr(any_ptr&&) = default;

    any_ptr& operator=(const any_ptr&) = default;
    any_ptr& operator=(any_ptr&&) = default;

    template <typename T>
    T* as() const
    {
        return reinterpret_cast<T*>(m_ptr);
    }

    void reset()
    {
        m_ptr = nullptr;
    }

    template <typename T>
    void reset(T* ptr)
    {
        m_ptr = reinterpret_cast<void*>(ptr);
    }

    explicit operator bool() const
    {
        return m_ptr != nullptr;
    }

private:
    void* m_ptr;
};

}}}

#endif //DIPLOMA_ANY_PTR_H
