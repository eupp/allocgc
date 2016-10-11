#ifndef DIPLOMA_OBJECT_META_H
#define DIPLOMA_OBJECT_META_H

#include <cstring>
#include <atomic>

#include <libprecisegc/details/gc_word.hpp>
#include <libprecisegc/details/gc_type_meta.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details { namespace collectors {

class traceable_object_meta
{
public:
    traceable_object_meta(size_t count, const gc_type_meta* meta)
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

    gc_type_meta::offsets_t offsets() const noexcept
    {
        return get_type_meta()->offsets();
    }

    bool is_plain_object() const noexcept
    {
        return m_type_meta == 0 || m_type_meta == FORWARD_BIT || get_type_meta()->is_plain_type();
    }

    const gc_type_meta* get_type_meta() const noexcept
    {
        return reinterpret_cast<const gc_type_meta*>(m_type_meta & ~FORWARD_BIT);
    }

    void set_type_meta(const gc_type_meta* cls_meta) noexcept
    {
        std::uintptr_t frwd_bit = is_forwarded() ? FORWARD_BIT : 0;
        m_type_meta = reinterpret_cast<std::uintptr_t>(cls_meta) | frwd_bit;
    }

    bool is_forwarded() const
    {
        return m_type_meta & FORWARD_BIT;
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
        m_type_meta |= FORWARD_BIT;
        memcpy(get_forward_pointer_address(), &ptr, sizeof(void*));
    }
private:
    static const std::uintptr_t FORWARD_BIT = 1;

    void* get_forward_pointer_address()
    {
        return reinterpret_cast<void*>(reinterpret_cast<byte*>(this) + sizeof(traceable_object_meta));
    }

    const void* get_forward_pointer_address() const
    {
        return reinterpret_cast<const void*>(reinterpret_cast<const byte*>(this) + sizeof(traceable_object_meta));
    }

    std::uintptr_t m_type_meta;
    size_t m_count;
};

}}}

#endif //DIPLOMA_OBJECT_META_H
