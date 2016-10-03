#ifndef DIPLOMA_OBJECT_META_H
#define DIPLOMA_OBJECT_META_H

#include <cstring>
#include <atomic>

#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/managed_ptr.hpp>
#include <libprecisegc/details/type_meta.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details {

class object_meta
{
public:
    // computes pointer to object_meta by pointer to start of managed cell and its size
    static object_meta* get_meta_ptr(byte* ptr, size_t obj_size)
    {
        return get_meta_ptr_by_cell_start(ptr);
    }

//    // computes pointer to object_meta by managed pointer to managed cell
//    static object_meta* get_meta_ptr(const managed_ptr& ptr)
//    {
//        return ptr.is_derived()
//               ? get_meta_ptr_by_cell_start(ptr.get_cell_begin())
//               : reinterpret_cast<object_meta*>(ptr.get() - sizeof(object_meta));
//    }

    // computes pointer to object_meta by pointer to location somewhere inside managed cell
    // that tagged with derived bit
    static object_meta* get_meta_ptr(byte* tagged_ptr)
    {
//        if (gc_tagging::derived_bit(tagged_ptr)) {
//            auto cell_ptr = managed_ptr(tagged_ptr);
//            return cell_ptr.get_meta();
//        } else {
//        }
        return reinterpret_cast<object_meta*>(tagged_ptr - sizeof(object_meta));
    }

    // computes pointer to object itself by pointer to start of managed cell and its size
    static byte* get_object_ptr(byte* ptr, size_t obj_size)
    {
        return get_object_ptr_by_cell_start(ptr);
    }

//    // computes pointer to object itself by managed pointer to managed cell
//    static byte* get_object_ptr(const managed_ptr& ptr)
//    {
//        return ptr.is_derived()
//               ? get_object_ptr_by_cell_start(ptr.get_cell_begin())
//               : ptr.get();
//    }

    // computes pointer to object itself by pointer to location somewhere inside managed cell
    // that tagged with derived bit
    static byte* get_object_ptr(byte* tagged_ptr)
    {
//        if (gc_tagging::derived_bit(tagged_ptr)) {
//            auto cell_ptr = managed_ptr(tagged_ptr);
//            return cell_ptr.get_obj_begin();
//        } else {
//        }
        return get_object_ptr_by_cell_start(tagged_ptr);
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

    bool is_plain_object() const noexcept
    {
        return m_type_meta == 0 || m_type_meta == FORWARD_BIT || get_type_meta()->is_plain_type();
    }

    const type_meta* get_type_meta() const noexcept
    {
        return reinterpret_cast<const type_meta*>(m_type_meta & ~FORWARD_BIT);
    }

    void set_type_meta(const type_meta* cls_meta) noexcept
    {
        std::uintptr_t frwd_bit = is_forwarded() ? FORWARD_BIT : 0;
        m_type_meta = reinterpret_cast<std::uintptr_t>(cls_meta) | frwd_bit;
    }

    byte* get_object_begin() const
    {
        return reinterpret_cast<byte*>(const_cast<object_meta*>(this)) + sizeof(object_meta);
    }

    byte* get_array_element_begin(byte* ptr) const
    {
        assert(contains(ptr));
        byte*  obj_begin = get_object_begin();
        size_t elem_size = type_size();
        size_t elem_ind = (ptr - obj_begin) / elem_size;
        return obj_begin + elem_ind * elem_size;
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

    template <typename Functor>
    void trace_children(Functor&& f) const
    {
        const type_meta* tmeta = get_type_meta();
        if (!tmeta) {
            return;
        }

        size_t obj_size = tmeta->type_size();
        auto offsets = tmeta->offsets();
        if (offsets.empty()) {
            return;
        }

        byte* obj = get_object_begin();
        size_t offsets_size = offsets.size();
        for (size_t i = 0; i < m_count; i++) {
            for (size_t j = 0; j < offsets_size; j++) {
                f(reinterpret_cast<gc_handle*>(obj + offsets[j]));
            }
            obj += obj_size;
        }
    }
private:
    static const std::uintptr_t FORWARD_BIT = 1;

    // computes pointer to object_meta by pointer to start of managed cell
    static object_meta* get_meta_ptr_by_cell_start(byte* ptr)
    {
        return reinterpret_cast<object_meta*>(ptr);
    }

    // computes pointer to object itself by pointer to start of managed cell
    static byte* get_object_ptr_by_cell_start(byte* ptr)
    {
        return ptr + sizeof(object_meta);
    }

    bool contains(byte* ptr) const
    {
        byte* obj_begin = get_object_begin();
        return (obj_begin <= ptr) && (ptr < obj_begin + m_count * type_size());
    }

    void* get_forward_pointer_address()
    {
        return reinterpret_cast<void*>(reinterpret_cast<byte*>(this) + sizeof(object_meta));
    }

    const void* get_forward_pointer_address() const
    {
        return reinterpret_cast<const void*>(reinterpret_cast<const byte*>(this) + sizeof(object_meta));
    }

    std::uintptr_t m_type_meta;
    size_t m_count;
};

} }

#endif //DIPLOMA_OBJECT_META_H
