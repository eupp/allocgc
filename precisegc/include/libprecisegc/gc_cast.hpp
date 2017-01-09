#ifndef DIPLOMA_GC_CAST_HPP
#define DIPLOMA_GC_CAST_HPP

#include <libprecisegc/gc_ptr.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>

namespace precisegc {

template <typename T, typename U>
gc_ptr<T> static_pointer_cast(const gc_ptr<U>& ptr)
{
    using namespace details;
    gc_unsafe_scope unsafe_scope;
    T* casted = static_cast<T*>(ptr.get());
    return gc_ptr<T>(casted);
};

template <typename T, typename U>
gc_ptr<T> dynamic_pointer_cast(const gc_ptr<U>& ptr)
{
    using namespace details;
    gc_unsafe_scope unsafe_scope;
    T* casted = dynamic_cast<T*>(ptr.get());
    return gc_ptr<T>(casted);
};

template <typename T, typename U>
gc_ptr<T> reinterpret_pointer_cast(const gc_ptr<U>& ptr)
{
    using namespace details;
    gc_unsafe_scope unsafe_scope;
    T* casted = reinterpret_cast<T*>(ptr.get());
    return gc_ptr<T>(casted);
};

}

#endif //DIPLOMA_GC_CAST_HPP
