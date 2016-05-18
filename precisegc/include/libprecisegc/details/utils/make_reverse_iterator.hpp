#ifndef DIPLOMA_MAKE_REVERSE_ITERATOR_HPP
#define DIPLOMA_MAKE_REVERSE_ITERATOR_HPP

#include <iterator>

namespace precisegc { namespace details { namespace utils {

template <typename Iterator>
std::reverse_iterator<Iterator> make_reverse_iterator(const Iterator& it)
{
    return std::reverse_iterator<Iterator>(it);
}

}}}

#endif //DIPLOMA_MAKE_REVERSE_ITERATOR_HPP
