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
    static_assert(sizeof(byte*) == sizeof(atomic_byte_ptr),
                  "atomic<byte*> has different size than byte*");

    static_assert(alignof(byte*) == alignof(atomic_byte_ptr),
                  "atomic<byte*> has different alignment than byte*");

    typedef byte* storage_t;
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

    storage_t m_storage;
};

class gc_handle_access
{
public:
    static byte* get(const gc_handle& handle);
    static void  set(gc_handle& handle, byte* ptr);
    static void  advane(gc_handle& handle, ptrdiff_t n);
    static byte* get_atomic(const gc_handle& handle, std::memory_order order);
    static void  set_atomic(gc_handle& handle, byte* ptr, std::memory_order order);
    static void  advance_atomic(gc_handle& handle, ptrdiff_t n, std::memory_order order);
};

}}

#endif //DIPLOMA_GC_HANDLE_HPP