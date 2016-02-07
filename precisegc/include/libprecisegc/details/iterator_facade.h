#ifndef DIPLOMA_ITERATO_BASE_H
#define DIPLOMA_ITERATO_BASE_H

#include <cstddef>
#include <iterator>
#include <type_traits>

#include "iterator_access.h"

namespace precisegc { namespace details {

template <typename Derived, typename Category, typename Value,
        typename Distance = ptrdiff_t, typename Pointer = Value*, typename Reference = Value&>
class iterator_facade: public std::iterator<Category, Value, Distance, Pointer, Reference>
{
    Derived* cast_this() noexcept
    {
        return static_cast<Derived*>(this);
    }

    const Derived* cast_this() const noexcept
    {
        return static_cast<const Derived*>(this);
    }

public:

    Derived& operator++() noexcept
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

    template <typename U = Category>
    auto operator--() noexcept
        -> typename std::enable_if<std::is_base_of<std::bidirectional_iterator_tag, U>::value, Derived&>::type
    {
        iterator_access<Derived>::decrement(cast_this());
        return *cast_this();
    };

    template <typename U = Category>
    auto operator--(int) noexcept
        -> typename std::enable_if<std::is_base_of<std::bidirectional_iterator_tag, U>::value, Derived>::type
    {
        Derived it = *cast_this();
        iterator_access<Derived>::decrement(cast_this());
        return it;
    };

    template <typename U = Category>
    auto operator+=(Distance n) noexcept
        -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, U>::value, Derived&>::type
    {
        iterator_access<Derived>::advance(cast_this(), n);
        return *cast_this();
    }

    template <typename U = Category>
    auto operator-=(Distance n) noexcept
        -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, U>::value, Derived&>::type
    {
        iterator_access<Derived>::advance(cast_this(), -n);
        return *cast_this();
    }

    template <typename U = Category>
    friend auto operator+(const Derived& it, Distance n) noexcept
        -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, U>::value, Derived>::type
    {
        return Derived(it) += n;
    }

    template <typename U = Category>
    friend auto operator+(Distance n, const Derived& it) noexcept
        -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, U>::value, Derived>::type
    {
        return Derived(it) += n;
    }

    template <typename U = Category>
    friend auto operator-(const Derived& it, Distance n) noexcept
        -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, U>::value, Derived>::type
    {
        return Derived(it) -= n;
    }

    template <typename U = Category>
    friend auto operator-(const Derived& it1, const Derived& it2) noexcept
        -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, U>::value, Distance>::type
    {
        return iterator_access<Derived>::difference(it1, it2);
    }

    friend bool operator==(const Derived& it1, const Derived& it2) noexcept
    {
        return iterator_access<Derived>::equal(it1, it2);
    }

    friend bool operator!=(const Derived& it1, const Derived& it2) noexcept
    {
        return !(it1 == it2);
    }

    template <typename U = Category>
    friend auto operator<(const Derived& it1, const Derived& it2) noexcept
        -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, U>::value, bool>::type
    {
        return iterator_access<Derived>::less_than(it1, it2);
    }

    template <typename U = Category>
    friend auto operator<=(const Derived& it1, const Derived& it2) noexcept
        -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, U>::value, bool>::type
    {
        return (it1 < it2) || (it1 == it2);
    }

    template <typename U = Category>
    friend auto operator>(const Derived& it1, const Derived& it2) noexcept
        -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, U>::value, bool>::type
    {
        return !(it1 <= it2);
    }

    template <typename U = Category>
    friend auto operator>=(const Derived& it1, const Derived& it2) noexcept
        -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, U>::value, bool>::type
    {
        return !(it1 < it2);
    }

protected:
    bool equal(const Derived&) const noexcept;

    template <typename U = Category>
    auto less_than(const Derived&) const noexcept
        -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, U>::value, bool>::type;

    void increment() noexcept;

    template <typename U = Category>
    auto decrement() noexcept
        -> typename std::enable_if<std::is_base_of<std::bidirectional_iterator_tag, U>::value, void>::type;

    template <typename U = Category>
    auto advance(Distance n) noexcept
        -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, U>::value, void>::type
    {
        if (n < 0) {
            for (Distance i = 0; i > n; --i) {
                iterator_access<Derived>::decrement(cast_this());
            }
        } else {
            for (Distance i = 0; i < n; ++i) {
                iterator_access<Derived>::increment(cast_this());
            }
        }
    };

    template <typename U = Category>
    auto difference(const Derived& it) const noexcept
        -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, U>::value, Distance>::type
    {
        Derived it_ = *cast_this();
        while (!iterator_access<Derived>::equal(it_, it)) {
            iterator_access<Derived>::decrement(&it_);
        }
    };
};

}}

#endif //DIPLOMA_ITERATO_BASE_H
