#ifndef GC_NEW_H
#define GC_NEW_H

#include <cstdio>
#include <cassert>
#include <vector>
#include <type_traits>
#include <utility>
#include <pthread.h>

#include <libprecisegc/gc_ptr.hpp>
#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/type_meta.hpp>
#include <libprecisegc/details/object_meta.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/ptrs/gc_new_stack.hpp>
#include <libprecisegc/details/utils/scope_guard.hpp>

namespace precisegc {

namespace internals {

template<typename T>
struct gc_new_if
{
    typedef gc_ptr<T> single_object;
};

template<typename T>
struct gc_new_if<T[]>
{
    typedef gc_ptr<T[]> unknown_bound;
};

template<typename T, size_t N>
struct gc_new_if<T[N]>
{
    typedef void known_bound;
};

}

template <typename T, typename... Args>
auto gc_new(Args&&... args)
    -> typename internals::gc_new_if<T>::single_object;

template <typename T>
auto gc_new(size_t n)
    -> typename internals::gc_new_if<T>::unknown_bound;


namespace internals {

template <typename T>
class gc_ptr_factory
{
    typedef details::byte byte;
public:
    template <typename... Args>
    class instance
    {
    private:
        static gc_ptr<T> create(const details::managed_ptr& ptr)
        {
            return create_internal(ptr);
        }

        friend gc_ptr<T> gc_new<T>(Args&&...);
    };
private:
    static gc_ptr<T> create_internal(const details::managed_ptr& ptr)
    {
        return gc_ptr<T>(ptr);
    }
};

template <typename T>
class gc_ptr_factory<T[]>
{
    typedef details::byte byte;
    typedef typename std::remove_extent<T>::type U;
public:
    static gc_ptr<T[]> create(const details::managed_ptr& ptr)
    {
        return gc_ptr<T[]>(ptr);
    }

    friend gc_ptr<T[]> gc_new<T[]>(size_t);
};

}

template <typename T, typename... Args>
auto gc_new(Args&&... args)
    -> typename internals::gc_new_if<T>::single_object
{
    using namespace precisegc::details;
    using namespace precisegc::details::ptrs;

    managed_ptr mptr;
    {
        gc_unsafe_scope_lock unsafe_scope_lock;
        std::unique_lock<gc_unsafe_scope_lock> lock(unsafe_scope_lock);
        gc_new_stack::activation_entry activation_entry;

        mptr = gc_allocate(sizeof(T) + sizeof(object_meta));
        byte* ptr = mptr.get();
        T* typed_ptr = reinterpret_cast<T*>(ptr);
        size_t size = mptr.cell_size();
        object_meta* obj_meta = object_meta::get_meta_ptr(ptr, size);

        if (!type_meta_provider<T>::is_meta_created()) {
            gc_new_stack::stack_entry stack_entry(ptr, size);
            new (typed_ptr) T(std::forward<Args>(args)...);
            new (obj_meta) object_meta(type_meta_provider<T>::create_meta(gc_new_stack::offsets()), 1);
        } else {
            new (obj_meta) object_meta(type_meta_provider<T>::get_meta());
            mptr.set_pin(true);
            auto guard = utils::make_scope_guard([mptr] { mptr.set_pin(false); });
            lock.unlock();
            new (typed_ptr) T(std::forward<Args>(args)...);
            obj_meta->set_object_count(1);
            guard.commit();
        }
    }

    return precisegc::internals::gc_ptr_factory<T>::template instance<Args...>::create(mptr);
};

template <typename T>
auto gc_new(size_t n)
    -> typename internals::gc_new_if<T>::unknown_bound
{
    using namespace precisegc::details;
    using namespace precisegc::details::ptrs;

    typedef typename std::remove_extent<T>::type U;

    managed_ptr mptr;
    {
        gc_unsafe_scope_lock unsafe_scope_lock;
        std::unique_lock<gc_unsafe_scope_lock> lock(unsafe_scope_lock);
        gc_new_stack::activation_entry activation_entry;

        mptr = gc_allocate(n * sizeof(U) + sizeof(object_meta));
        byte* ptr = mptr.get();
        U* typed_ptr = reinterpret_cast<U*>(ptr);
        size_t size = mptr.cell_size();
        object_meta* obj_meta = object_meta::get_meta_ptr(ptr, size);

        U* begin = typed_ptr;
        U* end = typed_ptr + n;

        if (!type_meta_provider<U>::is_meta_created()) {
            gc_new_stack::stack_entry stack_entry(ptr, size);
            new (begin++) U();
            type_meta_provider<U>::create_meta(gc_new_stack::offsets());
        }

        new (obj_meta) object_meta(type_meta_provider<U>::get_meta());
        mptr.set_pin(true);
        auto guard = utils::make_scope_guard([mptr] { mptr.set_pin(false); });
        lock.unlock();

        for (U* it = begin; it < end; ++it) {
            new (it) U();
            obj_meta->increment_object_count();
        }

        guard.commit();
    }

    return precisegc::internals::gc_ptr_factory<U[]>::create(mptr);
};

template<typename T, typename... Args>
auto gc_new(Args&&...)
        -> typename internals::gc_new_if<T>::known_bound
        = delete;

}

#endif //GC_NEW_H