#ifndef DIPLOMA_ITERATO_BASE_H
#define DIPLOMA_ITERATO_BASE_H

#include <cstddef>
#include <iterator>
#include <type_traits>

#include "iterator_access.h"
#include "iterator_util.h"

namespace precisegc { namespace details {

template <typename Derived, typename Category, typename Value,
        typename Distance = ptrdiff_t, typename Pointer = Value*, typename Reference = Value&>
class iterator_base: public std::iterator<Category, Value, Distance, Pointer, Reference>
{
    static const bool is_decrementable = iterator_category_traits<Category>::is_decrementable;
    static const bool is_random_access = iterator_category_traits<Category>::is_random_access;

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

    auto operator--() noexcept
        -> typename std::enable_if<is_decrementable, Derived&>::type
    {
        iterator_access<Derived>::decrement(cast_this());
        return *cast_this();
    };

    auto operator--(int) noexcept
        -> typename std::enable_if<is_decrementable, Derived>::type
    {
        Derived it = *cast_this();
        iterator_access<Derived>::decrement(cast_this());
        return it;
    };

    auto operator+=(Distance n) noexcept
        -> typename std::enable_if<is_random_access, Derived&>::type
    {
        iterator_access<Derived>::advance(cast_this(), n);
        return *cast_this();
    }

    auto operator-=(Distance n) noexcept
        -> typename std::enable_if<is_random_access, Derived&>::type
    {
        iterator_access<Derived>::advance(cast_this(), -n);
        return *cast_this();
    }

    friend auto operator+(const Derived& it, Distance n) noexcept
        -> typename std::enable_if<is_random_access, Derived>::type
    {
        return Derived(it) += n;
    }

    friend auto operator+(Distance n, const Derived& it) noexcept
        -> typename std::enable_if<is_random_access, Derived>::type
    {
        return Derived(it) += n;
    }

    friend auto operator-(const Derived& it, Distance n) noexcept
        -> typename std::enable_if<is_random_access, Derived>::type
    {
        return Derived(it) -= n;
    }

    friend auto operator-(const Derived& it1, const Derived& it2) noexcept
        -> typename std::enable_if<is_random_access, Distance>::type
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

    friend auto operator<(const Derived& it1, const Derived& it2) noexcept
        -> typename std::enable_if<is_random_access, bool>::type
    {
        return iterator_access<Derived>::less_than(it1, it2);
    }

    friend auto operator<=(const Derived& it1, const Derived& it2) noexcept
        -> typename std::enable_if<is_random_access, bool>::type
    {
        return (it1 < it2) || (it1 == it2);
    }

    friend auto operator>(const Derived& it1, const Derived& it2) noexcept
        -> typename std::enable_if<is_random_access, bool>::type
    {
        return !(it1 <= it2);
    }

    friend auto operator>=(const Derived& it1, const Derived& it2) noexcept
        -> typename std::enable_if<is_random_access, bool>::type
    {
        return !(it1 < it2);
    }

protected:
    bool equal(const Derived&) const noexcept;
    std::enable_if<is_random_access, bool>::type less_than(const Derived&) const noexcept;

    void increment() noexcept;
    std::enable_if<is_decrementable, void>::type decrement() noexcept;

    std::enable_if<is_random_access, void>::type advance(Distance n) noexcept
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

    std::enable_if<is_random_access, ptrdiff_t>::type difference(const Derived& it) const noexcept
    {
        Derived it_ = *cast_this();
        while (!iterator_access<Derived>::equal(it_, it)) {
            iterator_access<Derived>::decrement(&it_);
        }
    };
};

}}

#endif //DIPLOMA_ITERATO_BASE_H
