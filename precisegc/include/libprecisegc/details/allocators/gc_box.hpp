#ifndef DIPLOMA_GC_BOX_HPP
#define DIPLOMA_GC_BOX_HPP

#include <libprecisegc/details/gc_alloc_messaging.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details { namespace allocators {

class gc_box
{
public:
    gc_box() = delete;
    gc_box(const gc_box&) = delete;
    gc_box& operator=(const gc_box&) = delete;

    static constexpr size_t box_size(size_t obj_size)
    {
        return obj_size + sizeof(box_meta);
    }

    static constexpr byte* cell_start(byte* obj_start)
    {
        return obj_start - sizeof(box_meta);
    }
    
    static byte* create(byte* cell_start, size_t obj_count, const gc_type_meta* type_meta)
    {
        new(get_box_meta(cell_start)) box_meta(type_meta, obj_count);
        return get_obj_start(cell_start);
    }

    static void destroy(byte* cell_start)
    {
        assert(get_type_meta(cell_start));
        const gc_type_meta* tmeta = get_type_meta(cell_start);
        tmeta->destroy(get_obj_start(cell_start));
    }

    static size_t get_obj_count(byte* cell_start) noexcept
    {
        return get_box_meta(cell_start)->object_count();
    }

    static const gc_type_meta* get_type_meta(byte* cell_start) noexcept
    {
        return get_box_meta(cell_start)->type_meta();
    }

    static void set_type_meta(byte* cell_start, const gc_type_meta* type_meta) noexcept
    {
        return get_box_meta(cell_start)->set_type_meta(type_meta);
    }

    static bool is_forwarded(byte* cell_start) noexcept
    {
        return get_box_meta(cell_start)->is_forwarded();
    }

    static byte* forward_pointer(byte* cell_start) noexcept
    {
        assert(is_forwarded(cell_start));
        return *get_forward_pointer_address(cell_start);
    }

    static void set_forward_pointer(byte* cell_start, byte* forward_pointer) noexcept
    {
        box_meta* meta = get_box_meta(cell_start);
        meta->set_forwarded();
        byte** forward_ptr_addr = get_forward_pointer_address(cell_start);
        *forward_ptr_addr = forward_pointer;
    }
private:
    static const std::uintptr_t FORWARD_BIT = 1;

    static constexpr box_meta* get_box_meta(byte* cell_start)
    {
        reinterpret_cast<box_meta*>(cell_start);
    }

    static constexpr byte* get_obj_start(byte* cell_start)
    {
        return cell_start + sizeof(box_meta);
    }

    static constexpr byte** get_forward_pointer_address(byte* cell_start)
    {
        return reinterpret_cast<byte**>(get_obj_start(cell_start));
    }

    class box_meta
    {
    public:
        box_meta(const gc_type_meta* type_meta, size_t obj_count)
            : m_type_meta(reinterpret_cast<std::uintptr_t>(type_meta))
            , m_count(obj_count)
        {
            assert(obj_count > 0);
        }

        size_t object_count() const noexcept
        {
            return m_count;
        }

        const gc_type_meta* type_meta() const noexcept
        {
            return reinterpret_cast<const gc_type_meta*>(m_type_meta & ~FORWARD_BIT);
        }

        void set_type_meta(const gc_type_meta* cls_meta) noexcept
        {
            std::uintptr_t frwd_bit = is_forwarded() ? FORWARD_BIT : 0;
            m_type_meta = reinterpret_cast<std::uintptr_t>(cls_meta) | frwd_bit;
        }

        bool is_forwarded() const noexcept
        {
            return m_type_meta & FORWARD_BIT;
        }

        void set_forwarded() noexcept
        {
            m_type_meta |= FORWARD_BIT;
        }
    private:
        std::uintptr_t m_type_meta;
        size_t         m_count;
    };
};

}}}

#endif //DIPLOMA_GC_BOX_HPP
