#ifndef ALLOCGC_GC_REF_HPP
#define ALLOCGC_GC_REF_HPP

#include <utility>

#include <liballocgc/details/utils/utility.hpp>
#include <liballocgc/gc_pin.hpp>

namespace allocgc { namespace pointers {

template <typename T, typename GCStrategy>
class gc_ptr;

template <typename T, typename GCStrategy>
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

    friend class gc_ptr<T, GCStrategy>;
private:
    gc_ref(gc_pin<T, GCStrategy>&& pin)
        : m_pin(std::move(pin))
    {}

    gc_pin<T, GCStrategy> m_pin;
};

template <typename T, typename GCStrategy>
class gc_ref<T[], GCStrategy> : private details::utils::noncopyable
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

    friend class gc_ptr<T[], GCStrategy>;
private:
    gc_ref(gc_pin<T[], GCStrategy>&& pin, size_t idx)
        : m_pin(std::move(pin))
        , m_idx(idx)
    {}

    gc_pin<T[], GCStrategy> m_pin;
    size_t m_idx;
};

template <typename T, size_t N, typename GCStrategy>
class gc_ref<T[N], GCStrategy>;

}}

#endif //ALLOCGC_GC_REF_HPP
