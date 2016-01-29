#ifndef DIPLOMA_ITERATOR_ACCESS_H
#define DIPLOMA_ITERATOR_ACCESS_H

#include <iterator>
#include <type_traits>

namespace precisegc { namespace details {

template <typename Iterator>
class iterator_access
{
public:
    static bool equal(const Iterator& it1, const Iterator& it2) noexcept
    {
        return it1.equal(it2);
    }

    template <typename U = typename Iterator::iterator_category>
    static auto less_than(const Iterator& it1, const Iterator& it2) noexcept
        -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, U>::value, bool>::type
    {
        return it1.less_than(it2);
    }

    static void increment(Iterator* it) noexcept
    {
        it->increment();
    }

    template <typename U = typename Iterator::iterator_category>
    static auto decrement(Iterator* it) noexcept
        -> typename std::enable_if<std::is_base_of<std::bidirectional_iterator_tag, U>::value, void>::type
    {
        it->decrement();
    }

    template <typename U = typename Iterator::iterator_category>
    static auto advance(Iterator* it, size_t n) noexcept
        -> typename std::enable_if<std::is_base_of<std::random_access_iterator_tag, U>::value, void>::type
    {
        it->advance(n);
    };

    template <typename U = typename Iterator::iterator_category>
    static auto difference(const Iterator& it1, const Iterator& it2) noexcept
        -> typename std::enable_if<
            std::is_base_of<std::random_access_iterator_tag, U>::value,
            typename Iterator::difference_type
        >::type
    {
        return it1.difference(it2);
    };
};

}}

#endif //DIPLOMA_ITERATOR_ACCESS_H
