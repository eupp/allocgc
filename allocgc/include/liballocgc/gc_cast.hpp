#ifndef ALLOCGC_GC_CAST_HPP
#define ALLOCGC_GC_CAST_HPP

#include <liballocgc/gc_ptr.hpp>
#include <liballocgc/details/gc_unsafe_scope.hpp>

namespace allocgc { namespace pointers {

template <typename T, typename U, typename GCStrategy>
pointers::gc_ptr<T, GCStrategy> static_pointer_cast(const pointers::gc_ptr<U, GCStrategy>& ptr)
{
    using namespace details;
    gc_unsafe_scope unsafe_scope;
    T* casted = static_cast<T*>(ptr.get());
    return pointers::gc_ptr<T, GCStrategy>(casted);
};

template <typename T, typename U, typename GCStrategy>
pointers::gc_ptr<T, GCStrategy> dynamic_pointer_cast(const pointers::gc_ptr<U, GCStrategy>& ptr)
{
    using namespace details;
    gc_unsafe_scope unsafe_scope;
    T* casted = dynamic_cast<T*>(ptr.get());
    return pointers::gc_ptr<T, GCStrategy>(casted);
};

template <typename T, typename U, typename GCStrategy>
pointers::gc_ptr<T, GCStrategy> const_pointer_cast(const pointers::gc_ptr<U, GCStrategy>& ptr)
{
    typedef typename std::remove_extent<T>::type V;

    using namespace details;
    gc_unsafe_scope unsafe_scope;
    V* casted = const_cast<V*>(ptr.get());
    return pointers::gc_ptr<T, GCStrategy>(casted);
};

//template <typename T, typename U, typename GCStrategy>
//pointers::gc_ptr<T[], GCStrategy> const_pointer_cast(const pointers::gc_ptr<U[], GCStrategy>& ptr)
//{
//    using namespace details;
//    gc_unsafe_scope unsafe_scope;
//    T* casted = const_cast<T*>(ptr.get());
//    return pointers::gc_ptr<T[], GCStrategy>(casted);
//};

template <typename T, typename U, typename GCStrategy>
pointers::gc_ptr<T, GCStrategy> reinterpret_pointer_cast(const pointers::gc_ptr<U, GCStrategy>& ptr)
{
    typedef typename std::remove_extent<T>::type V;

    using namespace details;
    gc_unsafe_scope unsafe_scope;
    V* casted = reinterpret_cast<V*>(ptr.get());
    return pointers::gc_ptr<T, GCStrategy>(casted);
};

template <typename T, typename R, R T::*member, typename GCStrategy>
pointers::gc_ptr<R, GCStrategy> take_interior(const pointers::gc_ptr<T, GCStrategy>& ptr)
{
    details::gc_unsafe_scope unsafe_scope;
    T* raw = ptr.get();
    return pointers::gc_ptr<R, GCStrategy>(&(raw->*member));
};

}}

#endif //ALLOCGC_GC_CAST_HPP
