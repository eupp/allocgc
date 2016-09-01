#ifndef DIPLOMA_OBJECT_META_H
#define DIPLOMA_OBJECT_META_H

#include <cstring>
#include <atomic>

#include <libprecisegc/details/type_meta.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details {

class object_meta
{
public:
    // computes pointer to object_meta by pointer to object and its size
    static object_meta* get_meta_ptr(void* ptr, size_t obj_size)
    {
        return (object_meta*) ((size_t) ptr + obj_size - sizeof(object_meta));
    }

    object_meta(size_t count, const type_meta* meta)
        : m_type_meta(reinterpret_cast<std::uintptr_t>(meta))
        , m_count(count)
    {}

    size_t type_size() const noexcept
    {
        return get_type_meta()->type_size();
    }

    size_t object_count() const noexcept
    {
        return m_count;
    }

    size_t object_size() const noexcept
    {
        return object_count() * type_size();
    }

    void set_object_count(size_t count) noexcept
    {
        m_count = count;
    }

    void increment_object_count() noexcept
    {
        ++m_count;
    }

    type_meta::offsets_range offsets() const noexcept
    {
        return get_type_meta()->offsets();
    }

    const type_meta* get_type_meta() const noexcept
    {
        return reinterpret_cast<const type_meta*>(m_type_meta.load(std::memory_order_relaxed) & ~FORWARD_BIT);
    }

    void set_type_meta(const type_meta* cls_meta) noexcept
    {
        std::uintptr_t frwd_bit = is_forwarded() ? FORWARD_BIT : 0;
        m_type_meta.store(reinterpret_cast<std::uintptr_t>(cls_meta) | frwd_bit, std::memory_order_relaxed);
    }

    bool is_forwarded() const
    {
        return m_type_meta.load(std::memory_order_relaxed) & FORWARD_BIT;
    }

    byte* forward_pointer() const noexcept
    {
        assert(is_forwarded());
        byte* ptr;
        memcpy(&ptr, get_forward_pointer_address(), sizeof(void*));
        return ptr;
    }

    void set_forward_pointer(byte* ptr) noexcept
    {
        std::uintptr_t tmeta = m_type_meta.load(std::memory_order_relaxed);
        m_type_meta.store(tmeta | FORWARD_BIT, std::memory_order_relaxed);
        memcpy(get_forward_pointer_address(), &ptr, sizeof(void*));
    }
private:
    static const std::uintptr_t FORWARD_BIT = 1;

    void* get_forward_pointer_address()
    {
        return reinterpret_cast<void*>(reinterpret_cast<byte*>(this) - sizeof(void*));
    }

    const void* get_forward_pointer_address() const
    {
        return reinterpret_cast<const void*>(reinterpret_cast<const byte*>(this) - sizeof(void*));
    }

    std::atomic<std::uintptr_t> m_type_meta;
    size_t m_count;
};

} }

#endif //DIPLOMA_OBJECT_META_H
