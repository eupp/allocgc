#ifndef GC_NEW_H
#define GC_NEW_H

#include <cstdio>
#include <cassert>
#include <vector>
#include <type_traits>
#include <utility>
#include <pthread.h>

#include <libprecisegc/gc_ptr.hpp>
#include <libprecisegc/details/type_meta.hpp>
#include <libprecisegc/details/gc_unsafe_scope.h>
#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/ptrs/gc_ptr_access.hpp>
#include <libprecisegc/details/ptrs/gc_new_impl.hpp>

namespace precisegc {

template <typename T, typename... Args>
auto gc_new(Args&&... args)
    -> typename details::ptrs::gc_new_if<T>::single_object
{
    using namespace precisegc::details;
    using namespace precisegc::details::ptrs;
    gc_ptr<T> res;
    {
        gc_unsafe_scope unsafe_scope;
        void* ptr = gc_new_impl<T>(1, std::forward<Args>(args)...);
        T* typed_ptr = reinterpret_cast<T*>(ptr);
        res = gc_ptr_access<T>::create(typed_ptr);
    }
    gc_initation_point(initation_point_type::AFTER_ALLOC);
    return res;
};

template <typename T>
auto gc_new(size_t n)
    -> typename details::ptrs::gc_new_if<T>::unknown_bound
{
    typedef typename std::remove_extent<T>::type U;
    using namespace precisegc::details;
    using namespace precisegc::details::ptrs;
    gc_ptr<T> res;
    {
        gc_unsafe_scope unsafe_scope;
        void* ptr = gc_new_impl<U>(n);
        U* typed_ptr = reinterpret_cast<U*>(ptr);
        res = gc_ptr_access<T>::create(typed_ptr);
    }
    gc_initation_point(initation_point_type::AFTER_ALLOC);
    return res;
};

template<typename T, typename... Args>
auto gc_new(Args&&...)
    -> typename details::ptrs::gc_new_if<T>::known_bound
    = delete;

}

#endif //GC_NEW_H