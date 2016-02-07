#ifndef DIPLOMA_GC_PTR_BASE_H
#define DIPLOMA_GC_PTR_BASE_H

#include <cstddef>
#include <cstdint>
#include <atomic>

#include "gc_untyped_pin.h"

namespace precisegc { namespace details {

class gc_untyped_ptr
{
public:
    gc_untyped_ptr() noexcept;
    gc_untyped_ptr(nullptr_t) noexcept;
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

    friend class gc_untyped_pin;
protected:
    void* get() const noexcept;
private:
    static const uintptr_t ROOT_FLAG_BIT;

    static void* set_root_flag(void* ptr, bool root_flag) noexcept;
    static void* clear_root_flag(void* ptr) noexcept;
    static bool is_root_flag_set(void* ptr) noexcept;

    void set(void* ptr) noexcept;

    void register_root() noexcept;
    void delete_root() noexcept;

    void* m_ptr;
};

void swap(gc_untyped_ptr& a, gc_untyped_ptr& b) noexcept;

}}

#endif //DIPLOMA_GC_PTR_BASE_H
