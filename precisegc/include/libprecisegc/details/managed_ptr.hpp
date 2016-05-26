#ifndef DIPLOMA_MANAGED_PTR_H
#define DIPLOMA_MANAGED_PTR_H

#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/allocators/index_tree.h>
#include <libprecisegc/details/allocators/indexed_ptr.h>
#include <libprecisegc/details/gc_exception.hpp>
#include <libprecisegc/details/utils/to_string.hpp>
#include <libprecisegc/details/managed_memory_descriptor.hpp>

namespace precisegc { namespace details {

typedef allocators::index_tree<managed_memory_descriptor, std::allocator<byte>> index_type;

class unindexed_memory_exception : public gc_exception
{
public:
    unindexed_memory_exception(byte* ptr)
        : gc_exception(("Inappropriate access to unindexed memory by address " + utils::to_string(ptr)).c_str())
    {}
};

class managed_ptr
{
public:
    managed_ptr();
    managed_ptr(nullptr_t);
    managed_ptr(byte* ptr);
    managed_ptr(byte* ptr, managed_memory_descriptor* descriptor);

    managed_ptr(const managed_ptr&) = default;
    managed_ptr(managed_ptr&&) = default;

    managed_ptr& operator=(const managed_ptr&) = default;
    managed_ptr& operator=(managed_ptr&&) = default;

    bool get_mark() const;
    bool get_pin() const;

    void set_mark(bool mark) const;
    void set_pin(bool pin) const;

    void set_live(bool live);
    bool is_live() const;

    void sweep() const;

    object_meta* get_meta() const;
    byte* get_cell_begin() const;
    byte* get_obj_begin() const;

    byte* get() const;
    managed_memory_descriptor* get_descriptor() const;

    void advance(ptrdiff_t n);

    void swap(managed_ptr& other);
    friend void swap(managed_ptr& a, managed_ptr& b);

    static managed_ptr add_to_index(byte* ptr, size_t size, managed_memory_descriptor* descr);
    static void remove_from_index(byte* ptr, size_t size);
    static void remove_from_index(managed_ptr& ptr, size_t size);
    static managed_memory_descriptor* index(byte* ptr);
private:
    typedef allocators::index_tree<managed_memory_descriptor, std::allocator<byte>> index_type;

    static index_type indexer;

    byte* m_ptr;
    managed_memory_descriptor* m_descr;
};

}}

#endif //DIPLOMA_MANAGED_PTR_H
