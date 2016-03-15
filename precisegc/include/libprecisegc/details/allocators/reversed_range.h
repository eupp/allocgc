#ifndef DIPLOMA_REVERSE_RANGE_H
#define DIPLOMA_REVERSE_RANGE_H

#include <iterator>

namespace precisegc { namespace details { namespace allocators {

template<typename Range>
class reversed_range
{
public:
    typedef std::reverse_iterator<typename Range::iterator> iterator;

    reversed_range(const Range& rng)
        : m_begin(rng.end())
        , m_end(rng.begin())
    {}

    iterator begin() const
    {
        return m_begin;
    }

    iterator end() const
    {
        return m_end;
    }

private:
    iterator m_begin;
    iterator m_end;
};

}}}

#endif //DIPLOMA_REVERSE_RANGE_H
