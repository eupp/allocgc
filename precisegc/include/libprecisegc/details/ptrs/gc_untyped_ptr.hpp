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
    gc_untyped_ptr() noexcept;
//    gc_untyped_ptr(nullptr_t) noexcept;
    gc_untyped_ptr(void* ptr) noexcept;
    gc_untyped_ptr(const gc_untyped_ptr& other) noexcept;
    gc_untyped_ptr(gc_untyped_ptr&& other) noexcept;
    ~gc_untyped_ptr() noexcept;

    gc_untyped_ptr& operator=(nullptr_t) noexcept;
    gc_untyped_ptr& operator=(const gc_untyped_ptr& other) noexcept;
    gc_untyped_ptr& operator=(gc_untyped_ptr&& other) noexcept;

    void swap(gc_untyped_ptr& other) noexcept;

    explicit operator bool() const noexcept;

    bool is_root() const noexcept;

    void set(void* ptr) noexcept;
    void* get() const noexcept;
protected:
    atomic_byte_ptr m_ptr;
private:
    static thread_local gc_new_stack& gcnew_stack;

    void register_root() noexcept;
    void delete_root() noexcept;
    
    const bool m_root_flag;
};

void swap(gc_untyped_ptr& a, gc_untyped_ptr& b) noexcept;

}}}

#endif //DIPLOMA_GC_PTR_BASE_H
