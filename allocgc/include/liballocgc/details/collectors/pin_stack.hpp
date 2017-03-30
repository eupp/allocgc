#ifndef ALLOCGC_PIN_STACK_HPP
#define ALLOCGC_PIN_STACK_HPP

#include <cstddef>
#include <atomic>
#include <array>

#include <liballocgc/gc_common.hpp>
#include <liballocgc/details/gc_interface.hpp>
#include <liballocgc/details/utils/utility.hpp>

namespace allocgc { namespace details { namespace collectors {

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
        return std::find(begin(), end(), ptr) != end();
    }

    void trace(const gc_trace_pin_callback& cb) const
    {
        std::for_each(begin(), end(), cb);
    }

    size_t size() const
    {
        return m_size.load(std::memory_order_relaxed);
    }

    bool is_full() const
    {
        return size() == STACK_CAPACITY;
    }
private:
    static const size_t STACK_CAPACITY = 64;

    typedef std::array<byte*, STACK_CAPACITY> stack_t;

    stack_t::const_iterator begin() const
    {
        return m_stack.begin();
    }

    stack_t::const_iterator end() const
    {
        return std::next(m_stack.begin(), m_size.load(std::memory_order_relaxed));
    }

    stack_t m_stack;
    std::atomic<size_t> m_size;
};

}}}

#endif //ALLOCGC_PIN_STACK_HPP
