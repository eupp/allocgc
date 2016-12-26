#ifndef GC_NEW_HPP
#define GC_NEW_HPP

#include <cstdio>
#include <cassert>
#include <vector>
#include <type_traits>
#include <utility>
#include <pthread.h>

#include <libprecisegc/gc_ptr.hpp>
#include <libprecisegc/details/logging.hpp>
#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_type_meta.hpp>
#include <libprecisegc/details/gc_type_meta_factory.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/threads/gc_new_stack.hpp>
#include <libprecisegc/details/threads/this_managed_thread.hpp>
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

    gc_type_meta* tmeta = gc_type_meta_factory<T>::get();
    gc_alloc_response descr = gc_allocate(sizeof(T), 1, tmeta);

    if (!tmeta) {
        gc_new_stack::stack_entry stack_entry(descr.get(), descr.size(), true);
        new (descr.get()) T(std::forward<Args>(args)...);
        descr.descriptor()->set_type_meta(descr.get(),
                                          gc_type_meta_factory<T>::create(this_managed_thread::gc_ptr_offsets()));
    } else {
        gc_new_stack::stack_entry stack_entry(descr.get(), descr.size(), false);
        new (descr.get()) T(std::forward<Args>(args)...);
    }

    gc_commit(descr);

//    logging::debug() << "allocate cell addr=" << (void*) descr.get();

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

    gc_type_meta* tmeta = gc_type_meta_factory<U>::get();
    gc_alloc_response descr = gc_allocate(sizeof(U), n, tmeta);

    U* begin = reinterpret_cast<U*>(descr.get());
    U* end = begin + n;

    if (!tmeta) {
        gc_new_stack::stack_entry stack_entry(descr.get(), descr.size(), true);
        new (begin++) U();
        descr.descriptor()->set_type_meta(descr.get(),
                                          gc_type_meta_factory<U>::create(this_managed_thread::gc_ptr_offsets()));
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

#endif //GC_NEW_HPP