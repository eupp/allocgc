#ifndef ALLOCGC_GC_UNTYPED_PTR_HPP
#define ALLOCGC_GC_UNTYPED_PTR_HPP

#include <liballocgc/gc_common.hpp>
#include <liballocgc/details/gc_facade.hpp>

namespace allocgc { namespace pointers {

template <typename GCStrategy>
class gc_untyped_ptr
{
    typedef allocgc::details::gc_facade<GCStrategy> gc_facade;
public:
    gc_untyped_ptr()
        : gc_untyped_ptr(nullptr)
    {}

    gc_untyped_ptr(byte* ptr)
    {
        gc_facade::register_handle(m_handle, ptr);
    }

    gc_untyped_ptr(const gc_untyped_ptr& other)
        : gc_untyped_ptr()
    {
        wbarrier(other.m_handle);
    }

    ~gc_untyped_ptr()
    {
        gc_facade::deregister_handle(m_handle);
    }

    gc_untyped_ptr& operator=(std::nullptr_t)
    {
        reset();
        return *this;
    }

    gc_untyped_ptr& operator=(const gc_untyped_ptr& other)
    {
        wbarrier(other.m_handle);
        return *this;
    }

    byte* rbarrier() const
    {
        return gc_facade::rbarrier(m_handle);
    }

    void  wbarrier(const gc_handle& other)
    {
        gc_facade::wbarrier(m_handle, other);
    }

    // reset handle to point to some different location inside same cell that was pointed to before
    void interior_wbarrier(ptrdiff_t offset)
    {
        gc_facade::interior_wbarrier(m_handle, offset);
    }

    void reset()
    {
        gc_facade::wbarrier(m_handle, gc_handle::null);
    }

    bool is_null() const
    {
        return rbarrier() == nullptr;
    }

    bool is_root() const
    {
//    return gc_is_root(m_handle);
        return false;
    }

    bool equal(const gc_untyped_ptr& other) const
    {
        return gc_facade::compare(m_handle, other.m_handle);
    }

    void advance(ptrdiff_t n)
    {
        if (n != 0) {
            interior_wbarrier(n);
        }
    }

    void swap(gc_untyped_ptr& other)
    {
        gc_untyped_ptr tmp = (*this);
        (*this) = other;
        other = tmp;
    }

    friend void swap(gc_untyped_ptr& a, gc_untyped_ptr& b)
    {
        a.swap(b);
    }

    class pin_guard : private details::utils::noncopyable
    {
    public:
        pin_guard(pin_guard&& other)
                : m_ptr(other.m_ptr)
        {
            other.m_ptr = nullptr;
        }

        ~pin_guard()
        {
            gc_facade::deregister_pin(m_ptr);
        }

        pin_guard& operator=(pin_guard&& other)
        {
            std::swap(m_ptr, other.m_ptr);
            return *this;
        }

        byte* get() const noexcept
        {
            return m_ptr;
        }

        friend class gc_untyped_ptr;
    private:
        pin_guard(const gc_handle& handle)
            : m_ptr(gc_facade::register_pin(handle))
        {}

        byte* m_ptr;
    };

    class stack_pin_guard : private details::utils::noncopyable
    {
    public:
        stack_pin_guard(stack_pin_guard&& other)
                : m_ptr(other.m_ptr)
        {
            other.m_ptr = nullptr;
        }

        ~stack_pin_guard()
        {
            gc_facade::pop_pin(m_ptr);
        }

        stack_pin_guard& operator=(stack_pin_guard&& other) = delete;

        byte* get() const noexcept
        {
            return m_ptr;
        }

        friend class gc_untyped_ptr;
    private:
        stack_pin_guard(const gc_handle& handle)
                : m_ptr(gc_facade::push_pin(handle))
        {}

        byte* m_ptr;
    };

    pin_guard untyped_pin() const
    {
        return pin_guard(m_handle);
    }

    stack_pin_guard push_untyped_pin() const
    {
        return stack_pin_guard(m_handle);
    }
private:
    gc_handle m_handle;
};

}}

#endif //ALLOCGC_GC_UNTYPED_PTR_HPP
