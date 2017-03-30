#ifndef ALLOCGC_GC_REF_HPP
#define ALLOCGC_GC_REF_HPP

#include <utility>

#include <liballocgc/details/utils/utility.hpp>
#include <liballocgc/gc_pin.hpp>

namespace allocgc {

template <typename T>
class gc_ptr;

template <typename T>
class gc_ref : private details::utils::noncopyable
{
public:
    gc_ref(gc_ref&&) = default;

    gc_ref& operator=(gc_ref&&) = delete;

    operator T&() const
    {
        return *m_pin;
    }

    gc_ref& operator=(const T& other)
    {
        *m_pin = other;
        return *this;
    }

    friend class gc_ptr<T>;
private:
    gc_ref(gc_pin<T>&& pin)
        : m_pin(std::move(pin))
    {}

    gc_pin<T> m_pin;
};

template <typename T>
class gc_ref<T[]> : private details::utils::noncopyable
{
public:
    gc_ref(gc_ref&&) = default;

    gc_ref& operator=(gc_ref&&) = delete;

    operator T&() const
    {
        return m_pin[m_idx];
    }

    gc_ref& operator=(const T& other)
    {
        m_pin[m_idx] = other;
        return *this;
    }

    friend class gc_ptr<T[]>;
private:
    gc_ref(gc_pin<T[]>&& pin, size_t idx)
        : m_pin(std::move(pin))
        , m_idx(idx)
    {}

    gc_pin<T[]> m_pin;
    size_t m_idx;
};

template <typename T, size_t N>
class gc_ref<T[N]>;

}

#endif //ALLOCGC_GC_REF_HPP
