#ifndef DIPLOMA_GC_PTR_BASE_H
#define DIPLOMA_GC_PTR_BASE_H

#include <cstddef>
#include <cstdint>
#include <atomic>

namespace precisegc { namespace details {

class gc_ptr_base
{
public:
    gc_ptr_base() noexcept;
    gc_ptr_base(void* ptr) noexcept;
    gc_ptr_base(const gc_ptr_base& other) noexcept;
    gc_ptr_base(gc_ptr_base&& other) noexcept;
    ~gc_ptr_base() noexcept;

    gc_ptr_base& operator=(nullptr_t) noexcept;
    gc_ptr_base& operator=(const gc_ptr_base& other) noexcept;
    gc_ptr_base& operator=(gc_ptr_base&& other) noexcept;

    void swap(gc_ptr_base& other) noexcept;

    void* get() const noexcept;

    explicit operator bool() const noexcept;

    bool is_root() const noexcept;
private:
    static const uintptr_t ROOT_FLAG_BIT;

    static void* set_root_flag(void* ptr, bool root_flag) noexcept;
    static void* clear_root_flag(void* ptr) noexcept;
    static bool is_root_flag_set(void* ptr) noexcept;

    void set(void* ptr) noexcept;

    void* m_ptr;
};

void swap(gc_ptr_base& a, gc_ptr_base& b) noexcept;

}}

#endif //DIPLOMA_GC_PTR_BASE_H
