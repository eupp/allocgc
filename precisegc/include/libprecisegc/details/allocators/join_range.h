#ifndef DIPLOMA_JOIN_RANGE_H
#define DIPLOMA_JOIN_RANGE_H

#include "iterator_range.h"
#include "../iterator_facade.h"
#include "../iterator_access.h"

namespace precisegc { namespace details { namespace allocators {

namespace details {

template<typename RangeIter>
class join_range_iterator : public iterator_facade<join_range_iterator,
        std::bidirectional_iterator_tag,
        typename RangeIter::value_type::iterator_type::value_type>
{
    typedef typename RangeIter outer_iterator_t;
    typedef typename RangeIter::value_type::iterator_type inner_iterator_t;
public:
    typedef typename RangeIter::value_type::iterator_type::value_type value_type;

    join_range_iterator(const RangeIter& first_range, const RangeIter& last_range)
    {
        auto rng = first_range->get_range();
        m_begin_it = rng.begin();
        m_end_it = rng.end();
        m_curr_it = m_begin_it;

        m_curr_range = first_range;
        m_end_range = last_range;
    }

    // for end of range construction
    explicit join_range_iterator(const RangeIter& end)
    {
        m_curr_range = end;
        m_end_range = end;
    }

    join_range_iterator(const join_range_iterator&) noexcept = default;

    join_range_iterator(join_range_iterator&&) noexcept = default;

    join_range_iterator& operator=(const join_range_iterator&) noexcept = default;

    join_range_iterator& operator=(join_range_iterator&&) noexcept = default;

    value_type operator*() const noexcept
    {
        return *m_curr_it;
    }

    friend class iterator_access<join_range_iterator>;

private:
    bool equal(const join_range_iterator& other) const
    {
        return m_curr_it == other.m_curr_it || (m_curr_range == other.m_curr_range && m_curr_range == m_end_range);
    }

    void increment()
    {
        ++m_curr_it;
        if (m_curr_it == m_end_it) {
            ++m_curr_range;
            if (m_curr_range != m_end_range) {
                auto rng = m_curr_range->get_range();
                m_curr_it = rng.begin();
                m_end_it = rng.end();
            } else {
                return;
            }
        }
    }

    void decrement()
    {
        if (m_curr_it == m_begin_it) {
            --m_curr_range;
            auto rng = m_curr_range->get_range();
            m_curr_it = rng.end();
            m_end_it = rng.end();
        }
        --m_curr_it;
    }

    inner_iterator_t m_begin_it;
    inner_iterator_t m_end_it;
    inner_iterator_t m_curr_it;

    outer_iterator_t m_curr_range;
    outer_iterator_t m_end_range;
};

}

template <typename Range>
class join_range
{
    typedef typename Range::iterator RangeIter;
public:
    typedef details::join_range_iterator<RangeIter> iterator_type;

    explicit join_range(const Range& rng)
        : m_range(iterator_type(rng.begin(), rng.end()), iterator_type(rng.end()))
    {}

    iterator_type begin() const
    {
        return m_range.begin();
    }

    iterator_type end() const
    {
        return m_range.end();
    }
private:
    iterator_range<RangeIter> m_range;
};

}}}

#endif //DIPLOMA_JOIN_RANGE_H
