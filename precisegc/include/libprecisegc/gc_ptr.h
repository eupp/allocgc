#ifndef DIPLOMA_GC_PTR_H
#define DIPLOMA_GC_PTR_H

#include <cstdint>
#include <utility>

#include "details/gc_untyped_ptr.h"
#include "details/gc_untyped_pin.h"

#define set_stack_flag(x)		(void *)	((uintptr_t)x | (uintptr_t)1)
#define is_stack_pointer(x)		(bool)		((uintptr_t)x & (uintptr_t)1)
#define clear_stack_flag(x)		(void *)	((uintptr_t)x & ~(uintptr_t)1)
#define clear_both_flags(x)		(void *)	((uintptr_t)x & ~(uintptr_t)3)

namespace precisegc {

template <typename T>
class gc_ptr;

template<typename T>
class gc_pin: private details::gc_untyped_pin
{
public:
    gc_pin(const gc_ptr<T>& ptr);

    T* get() const;

    T* operator->();
    const T* operator->() const;
};

template <typename T>
class gc_ptr: private details::gc_untyped_ptr
{
public:
    gc_ptr() {}

    gc_ptr(nullptr_t)
        : gc_untyped_ptr(nullptr)
    {}

    gc_ptr(const gc_ptr& other)
        : gc_untyped_ptr(other)
    {}

    gc_ptr(gc_ptr&& other)
        : gc_untyped_ptr(std::move(other))
    {}

    gc_ptr& operator=(nullptr_t)
    {
        details::gc_untyped_ptr::operator=(nullptr);
        return *this;
    }

    gc_ptr& operator=(const gc_ptr& other)
    {
        details::gc_untyped_ptr::operator=(other);
        return *this;
    }

    gc_ptr& operator=(gc_ptr&& other)
    {
        details::gc_untyped_ptr::operator=(std::move(other));
        return *this;
    }

    void swap(gc_ptr& other)
    {
        details::gc_untyped_ptr::swap(other);
    }

    explicit operator bool() const
    {
        return details::gc_untyped_ptr::operator bool;
    }

    void reset()
    {
        details::gc_untyped_ptr::operator=(nullptr);
    }

    friend class gc_pin<T>;
private:
    gc_ptr(T* ptr)
        : gc_untyped_ptr((void*) ptr)
    {}
};

template <typename T>
class gc_ptr<T[]>: private details::gc_untyped_ptr
{
public:
    gc_ptr() {}

    gc_ptr(nullptr_t)
            : gc_untyped_ptr(nullptr)
    {}

    gc_ptr(const gc_ptr& other)
            : gc_untyped_ptr(other)
    {}

    gc_ptr(gc_ptr&& other)
            : gc_untyped_ptr(std::move(other))
    {}

    gc_ptr& operator=(nullptr_t)
    {
        details::gc_untyped_ptr::operator=(nullptr);
        return *this;
    }

    gc_ptr& operator=(const gc_ptr& other)
    {
        details::gc_untyped_ptr::operator=(other);
        return *this;
    }

    gc_ptr& operator=(gc_ptr&& other)
    {
        details::gc_untyped_ptr::operator=(std::move(other));
        return *this;
    }

    void swap(gc_ptr& other)
    {
        details::gc_untyped_ptr::swap(other);
    }

    explicit operator bool() const
    {
        return details::gc_untyped_ptr::operator bool;
    }

    void reset()
    {
        details::gc_untyped_ptr::operator=(nullptr);
    }

    friend class gc_pin<T[]>;
private:
    gc_ptr(T* ptr)
            : gc_untyped_ptr((void*) ptr)
    {}
};

template <typename T, size_t N>
class gc_ptr<T[N]>;

template <typename T>
gc_pin<T>::gc_pin(const gc_ptr<T>& ptr)
    : gc_untyped_pin(ptr)
{}

template <typename T>
T* gc_pin<T>::get() const
{
    return reinterpret_cast<T*>(details::gc_untyped_pin::get());
}

template <typename T>
T* gc_pin<T>::operator->()
{
    return get();
}

template <typename T>
const T* gc_pin<T>::operator->() const
{
    return get();
}

}

#endif //DIPLOMA_GC_PTR_BASE_H