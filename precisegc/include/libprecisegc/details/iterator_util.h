#ifndef DIPLOMA_ITERATOR_UTIL_H
#define DIPLOMA_ITERATOR_UTIL_H

#include <type_traits>
#include <iterator>

namespace precisegc { namespace details {

template <typename Category>
struct iterator_category_traits
{
    static const bool is_decrementable = std::integral_constant<bool,
            std::is_same<Category, std::bidirectional_iterator_tag>::value
            || std::is_same<Category, std::random_access_iterator_tag>::value
    >::value;

    static const bool is_random_access = std::integral_constant<bool,
            std::is_same<Category, std::random_access_iterator_tag>::value
    >::value;
};

}}

#endif //DIPLOMA_ITERATOR_UTIL_H
