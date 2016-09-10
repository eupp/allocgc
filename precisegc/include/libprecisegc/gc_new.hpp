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
#include <libprecisegc/details/threads/this_managed_thread.hpp>
#include <libprecisegc/details/utils/scope_guard.hpp>
#include <libprecisegc/details/logging.hpp>

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
        static gc_ptr<T> create(T* ptr)
        {
            return create_internal(ptr);
        }

        friend gc_ptr<T> gc_new<T>(Args&&...);
    };
private:
    static gc_ptr<T> create_internal(T* ptr)
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
    static gc_ptr<T[]> create(U* ptr)
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

    T* typed_ptr;
    gc_unsafe_scope unsafe_scope;
    {
        gc_new_stack::activation_entry activation_entry;

        gc_alloc_descriptor descr = gc_allocate(sizeof(T), 1, type_meta_provider<T>::get_meta());
        byte* ptr = descr.first.decorated().get();
        typed_ptr = reinterpret_cast<T*>(ptr);
        size_t size = descr.first.size();
        object_meta* obj_meta = descr.second;

        threads::this_managed_thread::pin(ptr);
        auto guard = utils::make_scope_guard([ptr] {
            threads::this_managed_thread::unpin(ptr);
        });

        if (!type_meta_provider<T>::is_meta_created()) {
            gc_new_stack::stack_entry stack_entry(ptr, size);
            new (typed_ptr) T(std::forward<Args>(args)...);
            obj_meta->set_type_meta(type_meta_provider<T>::create_meta(gc_new_stack::offsets()));
        } else {
            new (typed_ptr) T(std::forward<Args>(args)...);
        }

        gc_new_cell(descr.first.decorated());
    }

    return precisegc::internals::gc_ptr_factory<T>::template instance<Args...>::create(typed_ptr);
};

template <typename T>
auto gc_new(size_t n)
    -> typename internals::gc_new_if<T>::unknown_bound
{
    using namespace precisegc::details;
    using namespace precisegc::details::ptrs;

    typedef typename std::remove_extent<T>::type U;

    U* typed_ptr;
    gc_unsafe_scope unsafe_scope;
    {
        gc_new_stack::activation_entry activation_entry;

        gc_alloc_descriptor descr = gc_allocate(sizeof(U), n, type_meta_provider<T>::get_meta());
        byte* ptr = descr.first.decorated().get();
        typed_ptr = reinterpret_cast<U*>(ptr);
        size_t size = descr.first.size();
        object_meta* obj_meta = descr.second;

        U* begin = typed_ptr;
        U* end = typed_ptr + n;

        threads::this_managed_thread::pin(ptr);
        auto guard = utils::make_scope_guard([ptr] {
            threads::this_managed_thread::unpin(ptr);
        });

        if (!type_meta_provider<U>::is_meta_created()) {
            gc_new_stack::stack_entry stack_entry(ptr, size);
            new (begin++) U();
            obj_meta->set_type_meta(type_meta_provider<U>::create_meta(gc_new_stack::offsets()));
        }

        for (U* it = begin; it < end; ++it) {
            new (it) U();
        }

        gc_new_cell(descr.first.decorated());
    }

    return precisegc::internals::gc_ptr_factory<U[]>::create(typed_ptr);
};

template<typename T, typename... Args>
auto gc_new(Args&&...)
        -> typename internals::gc_new_if<T>::known_bound
        = delete;

}

#endif //GC_NEW_H