#ifndef DIPLOMA_POINTER_DECORATOR_H
#define DIPLOMA_POINTER_DECORATOR_H

#include <iterator>
#include <memory>

#include "../iterator_facade.h"
#include "../iterator_access.h"

namespace precisegc { namespace details { namespace allocators {

template <typename Derived, typename Decorated>
class pointer_decorator : public iterator_facade<
        Derived,
        std::random_access_iterator_tag,
        typename std::pointer_traits<Decorated>::element_type,
        typename std::pointer_traits<Decorated>::difference_type
    >
{
    typedef typename std::pointer_traits<Decorated>::difference_type Distance;
public:
    pointer_decorator(Decorated decorated)
        : m_decorated(decorated)
    {}

    pointer_decorator() = default;

    pointer_decorator(const pointer_decorator&) = default;
    pointer_decorator(pointer_decorator&&) = default;

    pointer_decorator& operator=(const pointer_decorator&) = default;
    pointer_decorator& operator=(pointer_decorator&&) = default;

    Decorated& get_wrapped()
    {
        return m_decorated;
    }

    const Decorated& get_wrapped() const
    {
        return m_decorated;
    }

    const Decorated& get_const_wrapped() const
    {
        return m_decorated;
    }
protected:
    bool equal(const pointer_decorator& other) const noexcept
    {
        return m_decorated == other.m_decorated;
    }

    bool less_than(const pointer_decorator& other) const noexcept
    {
        return m_decorated < other.m_decorated;
    }

    void increment() noexcept
    {
        ++m_decorated;
    }

    void decrement() noexcept
    {
        --m_decorated;
    }

    void advance(Distance n) noexcept
    {
        m_decorated += n;
    }

    Distance difference(const pointer_decorator& other) const noexcept
    {
        return m_decorated - other.m_decorated;
    }
private:
    Decorated m_decorated;
};

}}}

#endif //DIPLOMA_POINTER_DECORATOR_H
