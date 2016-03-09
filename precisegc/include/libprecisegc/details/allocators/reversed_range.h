#ifndef DIPLOMA_REVERSE_RANGE_H
#define DIPLOMA_REVERSE_RANGE_H

#include <iterator>

namespace precisegc { namespace details { namespace allocators {

template<typename Range>
class reversed_range
{
public:
    typedef std::reverse_iterator<typename Range::iterator_type> iterator_type;

    reversed_range(const Range& rng)
        : m_begin(rng.end())
        , m_end(rng.begin())
    {}

    iterator_type begin() const
    {
        return m_begin;
    }

    iterator_type end() const
    {
        return m_end;
    }

private:
    iterator_type m_begin;
    iterator_type m_end;
};

}}}

#endif //DIPLOMA_REVERSE_RANGE_H
