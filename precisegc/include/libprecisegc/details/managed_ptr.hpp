#ifndef DIPLOMA_MANAGED_PTR_H
#define DIPLOMA_MANAGED_PTR_H

#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/index_tree.hpp>
#include <libprecisegc/details/gc_exception.hpp>
#include <libprecisegc/details/utils/to_string.hpp>
#include <libprecisegc/details/memory_descriptor.hpp>

namespace precisegc { namespace details {

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
    managed_ptr(byte* ptr, memory_descriptor* descriptor);

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

    size_t cell_size() const;

    object_meta* get_meta() const;
    byte* get_cell_begin() const;
    byte* get_obj_begin() const;

    byte* get() const;
    memory_descriptor* get_descriptor() const;

    void advance(ptrdiff_t n);

    void swap(managed_ptr& other);
    friend void swap(managed_ptr& a, managed_ptr& b);

    explicit operator bool() const;

    static managed_ptr add_to_index(byte* ptr, size_t size, memory_descriptor* descr);
    static void remove_from_index(byte* ptr, size_t size);
    static void remove_from_index(managed_ptr& ptr, size_t size);
    static memory_descriptor* index(byte* ptr);
private:
    typedef index_tree<memory_descriptor> index_type;

    static index_type indexer;

    byte* m_ptr;
    memory_descriptor* m_descr;
};

}}

#endif //DIPLOMA_MANAGED_PTR_H
