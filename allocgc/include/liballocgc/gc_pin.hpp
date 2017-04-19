#ifndef ALLOCGC_GC_PIN_HPP
#define ALLOCGC_GC_PIN_HPP

#include <cstddef>

#include <liballocgc/details/gc_hooks.hpp>
#include <liballocgc/details/gc_untyped_ptr.hpp>
#include <liballocgc/details/utils/utility.hpp>

namespace allocgc { namespace pointers {

template <typename T, typename GCStrategy>
class gc_ptr;

template <typename T, typename GCStrategy>
class gc_pin : private details::utils::noncopyable
{
    typedef typename pointers::gc_untyped_ptr<GCStrategy>::pin_guard pin_guard;
public:
    gc_pin(gc_pin&&) = default;
    gc_pin& operator=(gc_pin&&) = default;

    T* get() const
    {
        return reinterpret_cast<T*>(m_pin.get());
    }

    T& operator*() const
    {
        return *get();
    }

    T* operator->() const
    {
        return get();
    }

    friend class gc_ptr<T, GCStrategy>;
private:
    gc_pin(pin_guard pin)
        : m_pin(std::move(pin))
    {}

    pin_guard m_pin;
};

template <typename T, typename GCStrategy>
class gc_pin<T[], GCStrategy> : private details::utils::noncopyable
{
    typedef typename pointers::gc_untyped_ptr<GCStrategy>::pin_guard pin_guard;
public:
    gc_pin(gc_pin&& other) = default;

    gc_pin& operator=(gc_pin&&) = default;

    T* get() const
    {
        return reinterpret_cast<T*>(m_pin.get());
    }

    T& operator*() const
    {
        return *get();
    }

    T& operator[](size_t n) const
    {
        return *(get() + n);
    }

    friend class gc_ptr<T[], GCStrategy>;
private:
    gc_pin(pin_guard pin)
        : m_pin(std::move(pin))
    {}

    pin_guard m_pin;
};

template <typename T, size_t N, typename GCStrategy>
class gc_pin<T[N], GCStrategy>;

template <typename T, typename GCStrategy>
class gc_stack_pin : private details::utils::noncopyable
{
    typedef typename pointers::gc_untyped_ptr<GCStrategy>::stack_pin_guard stack_pin_guard;
public:
    gc_stack_pin(gc_stack_pin&& other) = default;

    gc_stack_pin& operator=(gc_stack_pin&&) = delete;

    T* get() const
    {
        return reinterpret_cast<T*>(m_pin.get());
    }

    T* operator->() const
    {
        return get();
    }

    friend class gc_ptr<T, GCStrategy>;
private:
    gc_stack_pin(stack_pin_guard pin)
        : m_pin(std::move(pin))
    {}

    stack_pin_guard m_pin;
};

}}

#endif //ALLOCGC_GC_PIN_HPP
