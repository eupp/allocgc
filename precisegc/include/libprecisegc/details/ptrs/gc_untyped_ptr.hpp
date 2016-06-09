#ifndef DIPLOMA_GC_PTR_BASE_H
#define DIPLOMA_GC_PTR_BASE_H

#include <cstddef>
#include <cstdint>
#include <atomic>

#include <libprecisegc/details/ptrs/gc_new_stack.hpp>
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

    void swap(gc_untyped_ptr& other);

    explicit operator bool() const;

    bool is_root() const;

    void set(void* ptr);
    void* get() const;
protected:
    atomic_byte_ptr m_ptr;
private:
    static thread_local gc_new_stack& gcnew_stack;

    void register_root();
    void delete_root();
    
    const bool m_root_flag;
};

void swap(gc_untyped_ptr& a, gc_untyped_ptr& b);

}}}

#endif //DIPLOMA_GC_PTR_BASE_H
