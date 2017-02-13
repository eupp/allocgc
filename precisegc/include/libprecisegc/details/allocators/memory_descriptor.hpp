#ifndef DIPLOMA_MEMORY_DESCRIPTOR_HPP
#define DIPLOMA_MEMORY_DESCRIPTOR_HPP

#include <libprecisegc/gc_common.hpp>
#include <libprecisegc/details/allocators/gc_memory_descriptor.hpp>

namespace precisegc { namespace details { namespace allocators {

class memory_descriptor
{
public:
    inline constexpr memory_descriptor() noexcept
        : m_descriptor(0)
    {}

    inline constexpr memory_descriptor(std::nullptr_t) noexcept
        : m_descriptor(0)
    {}

    memory_descriptor(const memory_descriptor&) = default;
    memory_descriptor& operator=(const memory_descriptor&) = default;

    memory_descriptor(memory_descriptor&&) = default;
    memory_descriptor& operator=(memory_descriptor&&) = default;

    static constexpr memory_descriptor make_stack_descriptor(byte* descriptor)
    {
        return memory_descriptor(reinterpret_cast<std::uintptr_t>(descriptor) | STACK_TAG);
    }

    static constexpr memory_descriptor make_gc_heap_descriptor(gc_memory_descriptor* descriptor)
    {
        return memory_descriptor(reinterpret_cast<std::uintptr_t>(descriptor));
    }

    inline bool is_null() const
    {
        return m_descriptor == 0;
    }

    inline bool is_stack_descriptor() const
    {
        return m_descriptor & STACK_TAG;
    }

    inline bool is_gc_heap_descriptor() const
    {
        return !is_null() && !is_stack_descriptor();
    }

    byte* to_stack_descriptor() const
    {
        assert(is_stack_descriptor());
        return reinterpret_cast<byte*>(m_descriptor & ~STACK_TAG);
    }

    gc_memory_descriptor* to_gc_descriptor() const
    {
        assert(is_gc_heap_descriptor());
        return reinterpret_cast<gc_memory_descriptor*>(m_descriptor);
    }

    explicit operator bool() const
    {
        return !is_null();
    }
private:
    static const std::uintptr_t STACK_TAG = 1;
    static const std::uintptr_t GC_HEAP_TAG = 0;

    explicit inline constexpr memory_descriptor(std::uintptr_t descriptor)
        : m_descriptor(reinterpret_cast<std::uintptr_t>(descriptor))
    {}

    std::uintptr_t m_descriptor;
};

}}}

#endif //DIPLOMA_MEMORY_DESCRIPTOR_HPP
