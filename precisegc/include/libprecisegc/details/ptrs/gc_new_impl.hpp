#ifndef DIPLOMA_GC_NEW_IMPL_H
#define DIPLOMA_GC_NEW_IMPL_H

#include <cassert>
#include <utility>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/type_meta.hpp>
#include <libprecisegc/details/object_meta.h>
#include <libprecisegc/details/ptrs/gc_new_stack.hpp>


namespace precisegc {

template <typename T>
class gc_ptr;

namespace details { namespace ptrs {

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

template <typename T, typename... Args>
void* gc_new_impl(size_t n, Args&&... args)
{
    gc_new_stack::activation_entry activation_entry;

    size_t size = n * sizeof(T) + sizeof(object_meta);
    managed_ptr cell_ptr = gc_allocate(size);
//    assert(cell_ptr.is_live());
    byte* ptr = cell_ptr.get();
    size_t aligned_size = cell_ptr.cell_size();
    assert(ptr);

    void* begin = ptr;
    void* end = ptr + n * sizeof(T);

    // if class meta is not yet created - push pointer to current object and empty offsets
    // on gc_new_stack stack and call T constructor in order to fill offsets,
    // then create type_meta with gotten offsets
    if (!type_meta_provider<T>::is_created()) {
        gc_new_stack::stack_entry stack_entry(ptr, aligned_size);
        new (ptr) T(std::forward<Args>(args)...);
        type_meta_provider<T>::create_meta(gc_new_stack::offsets());
        begin += sizeof(T);
    }

    // construct remaining objects
    for (void *it = begin; it < end; it += sizeof(T)) {
        new (it) T(std::forward<Args>(args)...);
    }

    // construct object_meta
    {
        new (object_meta::get_meta_ptr(ptr, aligned_size)) object_meta(type_meta_provider<T>::get_meta_ptr(), n, ptr);
    }

    return ptr;
};

}}}

#endif //DIPLOMA_GC_NEW_IMPL_H
