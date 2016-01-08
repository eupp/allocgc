#ifndef DIPLOMA_ITERATO_BASE_H
#define DIPLOMA_ITERATO_BASE_H

#include <cstddef>
#include <iterator>
#include <type_traits>

#include "iterator_access.h"

namespace precisegc { namespace details {

template <typename Derived, typename Category, typename Value,
        typename Distance = ptrdiff_t, typename Pointer = Value*, typename Reference = Value&>
class iterator_base: public std::iterator<Category, Value, Distance, Pointer, Reference>
{
    static const bool is_decrementable = std::integral_constant<bool,
               std::is_same<Category, std::bidirectional_iterator_tag>::value
            || std::is_same<Category, std::random_access_iterator_tag>::value
    >::value;

    Derived* cast_this() noexcept
    {
        return static_cast<Derived*>(this);
    }

    const Derived* cast_this() const noexcept
    {
        return static_cast<const Derived*>(this);
    }

public:

    Derived operator++() noexcept
    {
        iterator_access<Derived>::increment(cast_this());
        return *cast_this();
    }

    Derived operator++(int) noexcept
    {
        Derived it = *cast_this();
        iterator_access<Derived>::increment(cast_this());
        return it;
    }

    typename std::enable_if<is_decrementable, Derived>::type operator--() noexcept
    {
        iterator_access<Derived>::decrement(cast_this());
        return *cast_this();
    };

    typename std::enable_if<is_decrementable, Derived>::type operator--(int) noexcept
    {
        Derived it = *cast_this();
        iterator_access<Derived>::decrement(cast_this());
        return it;
    };

    friend bool operator==(const Derived& it1, const Derived& it2)
    {
        return iterator_access<Derived>::equal(it1, it2);
    }

    friend bool operator!=(const Derived& it1, const Derived& it2)
    {
        return !(it1 == it2);
    }

protected:
    bool equal(const Derived& other) const noexcept;
    void increment() noexcept;
    std::enable_if<is_decrementable, void> decrement() noexcept;
};

}}

#endif //DIPLOMA_ITERATO_BASE_H
