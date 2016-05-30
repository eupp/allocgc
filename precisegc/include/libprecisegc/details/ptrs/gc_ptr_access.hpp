#ifndef DIPLOMA_GC_PTR_ACCESS_H
#define DIPLOMA_GC_PTR_ACCESS_H

#include <type_traits>

#include "libprecisegc/gc_ptr.h"

namespace precisegc {

template <typename T>
class gc_ptr;

namespace details { namespace ptrs {

template <typename T>
class gc_ptr_access
{
    typedef typename std::remove_extent<T>::type U;
public:
    static gc_ptr<T> create(U* raw_ptr)
    {
        return gc_ptr<T>(raw_ptr);
    }
};

}}}

#endif //DIPLOMA_GC_PTR_ACCESS_H
