#ifndef DIPLOMA_ITERATOR_ACCESS_H
#define DIPLOMA_ITERATOR_ACCESS_H

namespace precisegc { namespace details {

template <typename Iterator>
class iterator_access
{
public:
    static bool equal(const Iterator& it1, const Iterator& it2) noexcept
    {
        return it1.equal(it2);
    }

    static void increment(Iterator* it) noexcept
    {
        it->increment();
    }

    static void decrement(Iterator* it) noexcept
    {
        it->decrement();
    }
};

}}

#endif //DIPLOMA_ITERATOR_ACCESS_H
