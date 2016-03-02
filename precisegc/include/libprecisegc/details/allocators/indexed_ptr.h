#ifndef DIPLOMA_INDEXED_POINTER_H
#define DIPLOMA_INDEXED_POINTER_H

#include "types.h"
#include "pointer_decorator.h"
#include "../iterator_access.h"

namespace precisegc { namespace details { namespace allocators {

template <typename T, typename Index>
class indexed_ptr : public pointer_decorator<indexed_ptr<T, Index>, T*>
{
    typedef pointer_decorator<indexed_ptr<T, Index>, T*> pointer_decorator_t;
public:
    using pointer_decorator_t::element_type;
    using pointer_decorator_t::difference_type;

    typedef Index index_type;
    typedef typename Index::entry_type entry_type;

    indexed_ptr(T* ptr)
        : pointer_decorator_t(ptr)
    {}

    indexed_ptr()
        : pointer_decorator_t(nullptr)
    {}

    indexed_ptr(nullptr_t)
        : pointer_decorator_t(nullptr)
    {}

    indexed_ptr(const indexed_ptr&) = default;
    indexed_ptr(indexed_ptr&&) = default;

    indexed_ptr& operator=(const indexed_ptr&) = default;
    indexed_ptr& operator=(indexed_ptr&&) = default;

    entry_type* get_indexed_entry() const
    {
        return indexer.get_entry(reinterpret_cast<byte*>(this->get_wrapped()));
    }

    static indexed_ptr index(T* ptr, size_t size, entry_type* entry)
    {
        indexer.index(reinterpret_cast<byte*>(ptr), size * sizeof(T), entry);
        return indexed_ptr(ptr);
    }

    static void remove_index(indexed_ptr ptr, size_t size)
    {
        indexer.remove_index(reinterpret_cast<byte*>(ptr.get_wrapped()), size);
    }

    friend class iterator_access<indexed_ptr>;
private:
    static index_type indexer;
};

template <typename T, typename Entry, typename InternalAlloc>
typename indexed_ptr<T, Entry, InternalAlloc>::index_type indexed_ptr<T, Entry, InternalAlloc>::indexer;

}}}

#endif //DIPLOMA_INDEXED_POINTER_H
