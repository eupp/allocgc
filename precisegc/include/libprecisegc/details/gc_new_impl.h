#ifndef DIPLOMA_GC_NEW_IMPL_H
#define DIPLOMA_GC_NEW_IMPL_H

#include <cassert>
#include <utility>

#include "gc_heap.h"
#include "class_meta.h"
#include "object_meta.h"
#include "gc_new_stack.h"
#include "../gc_ptr.h"

namespace precisegc {

template <typename T>
class gc_ptr;

namespace details {

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
    managed_cell_ptr cell_ptr = gc_heap::instance().allocate(size);
    byte* ptr = cell_ptr.get();
    size_t aligned_size = cell_ptr.cell_size();
    assert(ptr);

    // allocate black objects
    cell_ptr.set_mark(true);
    // zero memory
    memset(ptr, 0, aligned_size);
    // release descriptor (object is black and all possible pointers are zeroed)
    cell_ptr.unlock_descriptor();

    void* begin = ptr;
    void* end = ptr + n * sizeof(T);

    // if class meta is not yet created - push pointer to current object and empty offsets
    // on gc_new_stack stack and call T constructor in order to fill offsets,
    // then create class_meta with gotten offsets
    if (!class_meta_provider<T>::is_created()) {
        gc_new_stack::stack_entry stack_entry(ptr);
        new (ptr) T(std::forward<Args>(args)...);
        class_meta_provider<T>::create_meta(gc_new_stack::instance().get_top_offsets());
        begin += sizeof(T);
    }

    // construct remaining objects
    for (void *it = begin; it < end; it += sizeof(T)) {
        new (it) T(std::forward<Args>(args)...);
    }

    // construct object_meta
    new (object_meta::get_meta_ptr(ptr, aligned_size)) object_meta(class_meta_provider<T>::get_meta_ptr(), n, ptr);

    return ptr;
};

}}

#endif //DIPLOMA_GC_NEW_IMPL_H
