#ifndef DIPLOMA_GC_HANDLE_HPP
#define DIPLOMA_GC_HANDLE_HPP

#include <cstddef>
#include <type_traits>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details {

class gc_handle_access;

class gc_handle : private utils::noncopyable, private utils::nonmovable
{
public:
    class pin_guard : private utils::noncopyable
    {
    public:
        pin_guard(pin_guard&& other);
        ~pin_guard();

        pin_guard& operator=(pin_guard&& other);

        byte* get() const noexcept;

        friend class gc_handle;
    private:
        pin_guard(const gc_handle& handle);

        byte* m_ptr;
    };

    gc_handle() = default;

    constexpr gc_handle(byte* ptr)
        : m_ptr(ptr)
    {}

    byte* rbarrier() const;
    void  wbarrier(const gc_handle& other);

    // reset handle to point to some different location inside same cell that was pointed to before
    void interior_wbarrier(byte* ptr);
    void interior_shift(ptrdiff_t shift);

    pin_guard pin() const;

    void reset();

    bool equal(const gc_handle& other) const;
    bool is_null() const;
private:
    friend class gc_handle_access;

    atomic_byte_ptr m_ptr;
};

class gc_handle_access
{
public:
    template <int MemoryOrder>
    static byte* get(const gc_handle& handle)
    {
        return handle.m_ptr.load(static_cast<std::memory_order>(MemoryOrder));
    }

    template <int MemoryOrder>
    static void  set(gc_handle& handle, byte* ptr)
    {
        handle.m_ptr.store(ptr, static_cast<std::memory_order>(MemoryOrder));
    }

    template <int MemoryOrder>
    static void  advance(gc_handle& handle, ptrdiff_t n)
    {
        handle.m_ptr.fetch_add(n, static_cast<std::memory_order>(MemoryOrder));
    }
};

}}

#endif //DIPLOMA_GC_HANDLE_HPP