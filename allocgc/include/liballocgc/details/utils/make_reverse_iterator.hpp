#ifndef ALLOCGC_MAKE_REVERSE_ITERATOR_HPP
#define ALLOCGC_MAKE_REVERSE_ITERATOR_HPP

#include <iterator>

#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace allocgc { namespace details { namespace utils {

template <typename Iterator>
std::reverse_iterator<Iterator> make_reverse_iterator(const Iterator& it)
{
    return std::reverse_iterator<Iterator>(it);
}

}}}

#endif //ALLOCGC_MAKE_REVERSE_ITERATOR_HPP
