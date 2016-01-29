#ifndef DIPLOMA_ITERATOR_ACCESS_H
#define DIPLOMA_ITERATOR_ACCESS_H

#include "iterator_util.h"

namespace precisegc { namespace details {

template <typename Iterator>
class iterator_access
{
    static const bool is_decrementable = iterator_category_traits<typename Iterator::Category>::is_decrementable;
    static const bool is_random_access = iterator_category_traits<typename Iterator::Category>::is_random_access;

public:
    static bool equal(const Iterator& it1, const Iterator& it2) noexcept
    {
        return it1.equal(it2);
    }

    static auto less_than(const Iterator& it1, const Iterator& it2) noexcept
        -> typename std::enable_if<is_random_access, bool>::type
    {
        return it1.less_than(it2);
    }

    static void increment(Iterator* it) noexcept
    {
        it->increment();
    }

    static auto decrement(Iterator* it) noexcept
        -> typename std::enable_if<is_decrementable, void>::type
    {
        it->decrement();
    }

    static auto advance(Iterator* it, size_t n) noexcept
        -> typename std::enable_if<is_random_access, void>::type
    {
        it->advance(n);
    };

    static auto difference(const Iterator& it1, const Iterator& it2) noexcept
        -> typename std::enable_if<is_random_access, typename Iterator::difference_type>::type
    {
        return it1.difference(it2);
    };
};

}}

#endif //DIPLOMA_ITERATOR_ACCESS_H
