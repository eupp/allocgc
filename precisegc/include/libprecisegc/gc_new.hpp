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

struct alloc_descriptor
{
    alloc_descriptor(const details::indexed_managed_object& mptr,
                     details::byte* obj_ptr,
                     size_t obj_size,
                     details::traceable_object_meta* obj_meta)
        : m_managed_ptr(mptr)
        , m_obj_ptr(obj_ptr)
        , m_obj_size(obj_size)
        , m_obj_meta(obj_meta)
    {}

    details::indexed_managed_object    m_managed_ptr;
    details::byte*          m_obj_ptr;
    size_t                  m_obj_size;
    details::traceable_object_meta*   m_obj_meta;
};

inline alloc_descriptor gc_new_allocate(size_t obj_size, size_t obj_count, const details::type_meta* tmeta)
{
    using namespace details;

    size_t size = obj_count * obj_size + sizeof(traceable_object_meta);
    gc_pointer_type ptr = gc_allocate(size);

    byte* cell_ptr = ptr.decorated().get();
    size_t cell_size = ptr.size();
    byte* obj_ptr = traceable_object_meta::get_object_ptr(cell_ptr, cell_size);
    traceable_object_meta* obj_meta = traceable_object_meta::get_meta_ptr(cell_ptr, cell_size);
    new (obj_meta) traceable_object_meta(obj_count, tmeta);

    return alloc_descriptor(ptr.decorated(), obj_ptr, cell_size - sizeof(details::traceable_object_meta), obj_meta);
}

}

template <typename T, typename... Args>
auto gc_new(Args&&... args)
    -> typename internals::gc_new_if<T>::single_object
{
    using namespace precisegc::details;
    using namespace precisegc::details::threads;

    gc_unsafe_scope unsafe_scope;

    ::precisegc::internals::alloc_descriptor descr =
            ::precisegc::internals::gc_new_allocate(sizeof(T), 1, type_meta_provider<T>::get_meta());
    T* typed_ptr = reinterpret_cast<T*>(descr.m_obj_ptr);

    if (!type_meta_provider<T>::is_meta_created()) {
        gc_new_stack::stack_entry stack_entry(descr.m_obj_ptr, descr.m_obj_size, true);
        new (typed_ptr) T(std::forward<Args>(args)...);
        descr.m_obj_meta->set_type_meta(type_meta_provider<T>::create_meta(this_managed_thread::gc_ptr_offsets()));
    } else {
        gc_new_stack::stack_entry stack_entry(descr.m_obj_ptr, descr.m_obj_size, false);
        new (typed_ptr) T(std::forward<Args>(args)...);
    }

    gc_new_cell(descr.m_managed_ptr);

    return precisegc::internals::gc_ptr_factory<T>::template instance<Args...>::create(typed_ptr);
};

template <typename T>
auto gc_new(size_t n)
    -> typename internals::gc_new_if<T>::unknown_bound
{
    using namespace precisegc::details;
    using namespace precisegc::details::threads;

    typedef typename std::remove_extent<T>::type U;

    gc_unsafe_scope unsafe_scope;

    ::precisegc::internals::alloc_descriptor descr =
            ::precisegc::internals::gc_new_allocate(sizeof(U), n, type_meta_provider<T>::get_meta());
    U* typed_ptr = reinterpret_cast<U*>(descr.m_obj_ptr);

    U* begin = typed_ptr;
    U* end = typed_ptr + n;

    if (!type_meta_provider<U>::is_meta_created()) {
        gc_new_stack::stack_entry stack_entry(descr.m_obj_ptr, descr.m_obj_size, true);
        new (begin++) U();
        descr.m_obj_meta->set_type_meta(type_meta_provider<U>::create_meta(this_managed_thread::gc_ptr_offsets()));
    } else {
        gc_new_stack::stack_entry stack_entry(descr.m_obj_ptr, descr.m_obj_size, false);
        new (begin++) U();
    }

    for (U* it = begin; it < end; ++it) {
        new (it) U();
    }

    gc_new_cell(descr.m_managed_ptr);

    return precisegc::internals::gc_ptr_factory<U[]>::create(typed_ptr);
};

template<typename T, typename... Args>
auto gc_new(Args&&...)
        -> typename internals::gc_new_if<T>::known_bound
        = delete;

}

#endif //GC_NEW_H