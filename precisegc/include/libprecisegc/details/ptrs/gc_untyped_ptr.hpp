#ifndef DIPLOMA_GC_PTR_BASE_H
#define DIPLOMA_GC_PTR_BASE_H

#include <cstddef>
#include <cstdint>
#include <atomic>

#include <libprecisegc/details/ptrs/gc_untyped_pin.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details { namespace ptrs {

class gc_untyped_ptr
{
public:
    gc_untyped_ptr();
//    gc_untyped_ptr(nullptr_t) noexcept;
    gc_untyped_ptr(void* ptr);
    gc_untyped_ptr(const gc_untyped_ptr& other);
    gc_untyped_ptr(gc_untyped_ptr&& other);
    ~gc_untyped_ptr();

    gc_untyped_ptr& operator=(nullptr_t);
    gc_untyped_ptr& operator=(const gc_untyped_ptr& other);
    gc_untyped_ptr& operator=(gc_untyped_ptr&& other);

    gc_untyped_pin untyped_pin() const;

    bool is_null() const;
    bool is_root() const;

    bool equal(const gc_untyped_ptr& other) const;

    void advance(ptrdiff_t n);

    void  set(void* ptr);
    void* get() const;

    void swap(gc_untyped_ptr& other);
    friend void swap(gc_untyped_ptr& a, gc_untyped_ptr& b);
private:
    bool check_bounds(ptrdiff_t n) const;

    void register_root();
    void delete_root();

    atomic_byte_ptr m_ptr;
    const bool m_root_flag;
};

}}}

#endif //DIPLOMA_GC_PTR_BASE_H
