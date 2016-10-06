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
#include <libprecisegc/details/collectors/traceable_object_meta.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/threads/gc_new_stack.hpp>
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
    using namespace precisegc::details::threads;

    gc_unsafe_scope unsafe_scope;

    gc_alloc_descriptor descr = gc_allocate(sizeof(T), 1, type_meta_provider<T>::get_meta());

    if (!type_meta_provider<T>::is_meta_created()) {
        gc_new_stack::stack_entry stack_entry(descr.get(), descr.size(), true);
        new (descr.get()) T(std::forward<Args>(args)...);
        descr.descriptor()->set_type_meta(descr.get(),
                                          type_meta_provider<T>::create_meta(this_managed_thread::gc_ptr_offsets()));
    } else {
        gc_new_stack::stack_entry stack_entry(descr.get(), descr.size(), false);
        new (descr.get()) T(std::forward<Args>(args)...);
    }

    gc_commit(descr);

    return precisegc::internals::gc_ptr_factory<T>::template instance<Args...>::create(
            reinterpret_cast<T*>(descr.get())
    );
};

template <typename T>
auto gc_new(size_t n)
    -> typename internals::gc_new_if<T>::unknown_bound
{
    using namespace precisegc::details;
    using namespace precisegc::details::threads;

    typedef typename std::remove_extent<T>::type U;

    gc_unsafe_scope unsafe_scope;

    gc_alloc_descriptor descr = gc_allocate(sizeof(U), n, type_meta_provider<T>::get_meta());

    U* begin = reinterpret_cast<U*>(descr.get());
    U* end = begin + n;

    if (!type_meta_provider<U>::is_meta_created()) {
        gc_new_stack::stack_entry stack_entry(descr.get(), descr.size(), true);
        new (begin++) U();
        descr.descriptor()->set_type_meta(descr.get(),
                                          type_meta_provider<U>::create_meta(this_managed_thread::gc_ptr_offsets()));
    } else {
        gc_new_stack::stack_entry stack_entry(descr.get(), descr.size(), false);
        new (begin++) U();
    }

    for (U* it = begin; it < end; ++it) {
        new (it) U();
    }

    gc_commit(descr);

    return precisegc::internals::gc_ptr_factory<U[]>::create(
            reinterpret_cast<U*>(descr.get())
    );
};

template<typename T, typename... Args>
auto gc_new(Args&&...)
        -> typename internals::gc_new_if<T>::known_bound
        = delete;

}

#endif //GC_NEW_H