#ifndef DIPLOMA_PIN_STACK_HPP
#define DIPLOMA_PIN_STACK_HPP

#include <cstddef>
#include <atomic>
#include <array>

#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace threads {

class pin_stack : private utils::noncopyable, private utils::nonmovable
{
public:
    pin_stack()
        : m_size(0)
    {}

    byte* top() const
    {
        assert(m_size > 0);
        return m_stack[m_size.load(std::memory_order_relaxed) - 1];
    }

    void push_pin(byte* ptr)
    {
        size_t size = m_size.load(std::memory_order_relaxed);
        m_stack[size] = ptr;
        m_size.store(size + 1, std::memory_order_relaxed);
    }

    void pop_pin()
    {
        size_t size = m_size.load(std::memory_order_relaxed);
        m_size.store(size - 1, std::memory_order_relaxed);
    }

    bool contains(byte* ptr) const
    {
        return std::find(m_stack.begin(), m_stack.end(), ptr) != m_stack.end();
    }

    template <typename Functor>
    void trace(Functor&& f) const
    {
        std::for_each(m_stack.begin(), m_stack.end(), std::forward<Functor>(f));
    }

    size_t count() const
    {
        return m_size.load(std::memory_order_relaxed);
    }

    bool is_full() const
    {
        return count() == STACK_CAPACITY;
    }
private:
    static const size_t STACK_CAPACITY = 64;

    std::array<byte*, STACK_CAPACITY> m_stack;
    std::atomic<size_t> m_size;
};

}}}

#endif //DIPLOMA_PIN_STACK_HPP
