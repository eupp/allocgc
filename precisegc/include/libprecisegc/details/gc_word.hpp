#ifndef DIPLOMA_GC_HANDLE_HPP
#define DIPLOMA_GC_HANDLE_HPP

#include <cstddef>
#include <type_traits>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details {

class gc_handle_access;

class gc_word : private utils::noncopyable, private utils::nonmovable
{
public:
    class pin_guard : private utils::noncopyable
    {
    public:
        pin_guard(pin_guard&& other);
        ~pin_guard();

        pin_guard& operator=(pin_guard&& other);

        byte* get() const noexcept;

        friend class gc_word;
    private:
        pin_guard(const gc_word& handle);

        byte* m_ptr;
    };

    class stack_pin_guard : private utils::noncopyable
    {
    public:
        stack_pin_guard(stack_pin_guard&& other);
        ~stack_pin_guard();

        stack_pin_guard& operator=(stack_pin_guard&& other) = delete;

        byte* get() const noexcept;

        friend class gc_word;
    private:
        stack_pin_guard(const gc_word& handle);

        byte* m_ptr;
    };

    gc_word() = default;

    constexpr gc_word(byte* ptr)
        : m_ptr(ptr)
    {}

    byte* rbarrier() const;
    void  wbarrier(const gc_word& other);

    // reset handle to point to some different location inside same cell that was pointed to before
    void interior_wbarrier(ptrdiff_t offset);

    pin_guard pin() const;
    stack_pin_guard push_pin() const;

    void reset();

    bool equal(const gc_word& other) const;
    bool is_null() const;
private:
    friend class gc_handle_access;

    static gc_word null;

    atomic_byte_ptr m_ptr;
};

class gc_handle_access
{
public:
    template <int MemoryOrder>
    static byte* get(const gc_word& handle)
    {
        return handle.m_ptr.load(static_cast<std::memory_order>(MemoryOrder));
    }

    template <int MemoryOrder>
    static void  set(gc_word& handle, byte* ptr)
    {
        handle.m_ptr.store(ptr, static_cast<std::memory_order>(MemoryOrder));
    }

    template <int MemoryOrder>
    static void  advance(gc_word& handle, ptrdiff_t n)
    {
        handle.m_ptr.fetch_add(n, static_cast<std::memory_order>(MemoryOrder));
    }
};

}}

#endif //DIPLOMA_GC_HANDLE_HPP