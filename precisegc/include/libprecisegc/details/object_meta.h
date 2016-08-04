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

    object_meta(const type_meta* meta = nullptr)
        : m_type_meta(meta)
        , m_count(0)
    {}

    size_t type_size() const noexcept
    {
        return m_type_meta->type_size();
    }

    size_t object_count() const noexcept
    {
        return m_count.load(std::memory_order_relaxed);
    }

    size_t object_size() const noexcept
    {
        return object_count() * type_size();
    }

    void set_object_count(size_t count) noexcept
    {
        m_count.store(count, std::memory_order_relaxed);
    }

    const type_meta* get_type_meta() const noexcept
    {
        return m_type_meta;
    }

    void set_type_meta(const type_meta* cls_meta) noexcept
    {
        m_type_meta = cls_meta;
    }

    byte* forward_pointer() const noexcept
    {
        byte* ptr;
        memcpy(&ptr, get_forward_pointer_address(), sizeof(void*));
        return ptr;
    }

    void set_forward_pointer(byte* ptr) noexcept
    {
        memcpy(get_forward_pointer_address(), &ptr, sizeof(void*));
    }
private:
    void* get_forward_pointer_address()
    {
        return reinterpret_cast<void*>(reinterpret_cast<byte*>(this) - sizeof(void*));
    }

    const void* get_forward_pointer_address() const
    {
        return reinterpret_cast<const void*>(reinterpret_cast<const byte*>(this) - sizeof(void*));
    }

    const type_meta* m_type_meta;
    std::atomic<size_t> m_count;
};

} }

#endif //DIPLOMA_OBJECT_META_H
