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
gc_ptr<T> const_pointer_cast(const gc_ptr<U>& ptr)
{
    using namespace details;
    gc_unsafe_scope unsafe_scope;
    T* casted = const_cast<T*>(ptr.get());
    return gc_ptr<T>(casted);
};

template <typename T, typename U>
gc_ptr<T[]> const_pointer_cast(const gc_ptr<U[]>& ptr)
{
    using namespace details;
    gc_unsafe_scope unsafe_scope;
    T* casted = const_cast<T*>(ptr.get());
    return gc_ptr<T[]>(casted);
};

template <typename T, typename U>
gc_ptr<T> reinterpret_pointer_cast(const gc_ptr<U>& ptr)
{
    typedef typename std::remove_extent<T>::type V;

    using namespace details;
    gc_unsafe_scope unsafe_scope;
    V* casted = reinterpret_cast<V*>(ptr.get());
    return gc_ptr<T>(casted);
};

template <typename T, typename R, R T::*member>
gc_ptr<R> take_interior(const gc_ptr<T>& ptr)
{
    details::gc_unsafe_scope unsafe_scope;
    T* raw = ptr.get();
    return gc_ptr<R>(&(raw->*member));
};

}

#endif //DIPLOMA_GC_CAST_HPP
