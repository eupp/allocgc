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

    void* get() const noexcept;

    void atomic_store(const gc_untyped_ptr& value);

    friend class gc_untyped_pin;
private:
    static const uintptr_t ROOT_FLAG_BIT;

    void set(void* ptr) noexcept;

    void register_root() noexcept;
    void delete_root() noexcept;

    std::atomic<void*> m_ptr;
    const bool m_root_flag;
};

void swap(gc_untyped_ptr& a, gc_untyped_ptr& b) noexcept;

}}

#endif //DIPLOMA_GC_PTR_BASE_H
