#ifndef DIPLOMA_ITERATOR_RANGE_H
#define DIPLOMA_ITERATOR_RANGE_H

namespace precisegc { namespace details { namespace allocators {

template <typename Iter>
class iterator_range
{
public:
    typedef Iter iterator_type;

    iterator_range(const Iter& b, const Iter& e)
        : m_begin(b)
        , m_end(e)
    {}

    Iter begin() const
    {
        return m_begin;
    }

    Iter end() const
    {
        return m_end;
    }
private:
    Iter m_begin;
    Iter m_end;
};

}}}

#endif //DIPLOMA_ITERATOR_RANGE_H
