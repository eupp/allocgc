#ifndef DIPLOMA_PIN_STACK_HPP
#define DIPLOMA_PIN_STACK_HPP

#include <atomic>
#include <cstddef>
#include <memory>

#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace threads {

class pin_stack : private utils::noncopyable, private utils::nonmovable
{
public:
    pin_stack()
        : m_stack(new byte*[MIN_STACK_CAPACITY])
        , m_capacity(MIN_STACK_CAPACITY)
        , m_size(0)
    {}

    ~pin_stack()
    {
        delete[] m_stack.load(std::memory_order_relaxed);
    }

    void push_pin(byte* ptr)
    {
        byte** stack = m_stack.load(std::memory_order_relaxed);
        size_t capacity = m_capacity.load(std::memory_order_relaxed);
        size_t size = m_size.load(std::memory_order_relaxed);

        if (size == capacity) {
            stack = replace_stack(stack, size, capacity, 2 * capacity);
        }

        stack[size] = ptr;
        m_size.store(size + 1, std::memory_order_relaxed);
    }

    void pop_pin()
    {
        byte** stack = m_stack.load(std::memory_order_relaxed);
        size_t capacity = m_capacity.load(std::memory_order_relaxed);
        size_t size = m_size.load(std::memory_order_relaxed);

        if (4 * size <= capacity && capacity > MIN_STACK_CAPACITY) {
            replace_stack(stack, size, capacity, 2 * capacity);
        }

        m_size.store(size - 1, std::memory_order_relaxed);
    }

    bool contains(byte* ptr) const
    {
        byte** stack_begin = m_stack.load(std::memory_order_relaxed);
        byte** stack_end = stack_begin + m_size.load(std::memory_order_relaxed);
        return std::find(stack_begin, stack_end, ptr) != stack_end;
    }

    template <typename Functor>
    void trace(Functor&& f) const
    {
        byte** stack_begin = m_stack.load(std::memory_order_relaxed);
        byte** stack_end = stack_begin + m_size.load(std::memory_order_relaxed);
        std::for_each(stack_begin, stack_end, std::forward<Functor>(f));
    }

    size_t count() const
    {
        return m_size.load(std::memory_order_relaxed);
    }
private:
    static const size_t MIN_STACK_CAPACITY = 4096;

    byte** replace_stack(byte** stack, size_t size, size_t capacity, size_t new_capacity)
    {
        auto new_stack = std::unique_ptr<byte*[]>(new byte*[new_capacity]);
        std::copy(stack, stack + size, new_stack.get());

        m_stack.store(new_stack.get(), std::memory_order_relaxed);
        m_capacity.store(new_capacity, std::memory_order_relaxed);
        delete[] stack;
        return new_stack.release();
    }

    std::atomic<byte**> m_stack;
    std::atomic<size_t> m_capacity;
    std::atomic<size_t> m_size;
};

}}}

#endif //DIPLOMA_PIN_STACK_HPP
