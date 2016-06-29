#ifndef DIPLOMA_GC_PIN_HPP
#define DIPLOMA_GC_PIN_HPP

#include <cstddef>

#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc {

template <typename T>
class gc_ptr;

template <typename T>
class gc_pin : private details::utils::noncopyable
{
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

    T* operator->() const
    {
        return get();
    }

    friend class gc_ptr<T>;
private:
    gc_pin(details::gc_handle::pin_guard pin)
        : m_pin(std::move(pin))
    {}

    details::gc_handle::pin_guard m_pin;
};

template <typename T>
class gc_pin<T[]> : private details::utils::noncopyable
{
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

    friend class gc_ptr<T[]>;
private:
    gc_pin(details::gc_handle::pin_guard pin)
            : m_pin(std::move(pin))
    {}

    details::gc_handle::pin_guard m_pin;
};

template <typename T, size_t N>
class gc_pin<T[N]>;

}

#endif //DIPLOMA_GC_PIN_HPP