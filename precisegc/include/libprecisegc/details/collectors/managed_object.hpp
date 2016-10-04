#ifndef DIPLOMA_MANAGED_OBJECT_HPP
#define DIPLOMA_MANAGED_OBJECT_HPP

#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/collectors/traceable_object_meta.hpp>
#include "dptr_storage.hpp"

namespace precisegc { namespace details { namespace collectors {

class managed_object
{
public:
    managed_object() = delete;
    managed_object(const managed_object&) = default;
    managed_object& operator=(const managed_object&) = default;

    inline static traceable_object_meta* get_meta(byte* cell_start) noexcept
    {
        return reinterpret_cast<traceable_object_meta*>(cell_start);
    }

    inline static byte* get_object(byte* cell_start) noexcept
    {
        return cell_start + sizeof(traceable_object_meta);
    }

    explicit managed_object(byte* ptr)
        : m_ptr(ptr)
    {
        assert(!dptr_storage::is_derived(m_ptr));
    }

    inline byte* object() const noexcept
    {
        return m_ptr;
    }

    inline traceable_object_meta* meta() const noexcept
    {
        return reinterpret_cast<traceable_object_meta*>(m_ptr - sizeof(traceable_object_meta));
    }

    explicit operator bool() const noexcept
    {
        return m_ptr != nullptr;
    }

    template <typename Functor>
    void trace_children(Functor&& f) const
    {
        assert(m_ptr);

        const type_meta* tmeta = meta()->get_type_meta();
        if (!tmeta) {
            return;
        }

        size_t obj_size = tmeta->type_size();
        auto offsets = tmeta->offsets();
        if (offsets.empty()) {
            return;
        }

        byte* obj = object();
        size_t obj_cnt = meta()->object_count();
        size_t offsets_size = offsets.size();
        for (size_t i = 0; i < obj_cnt; i++) {
            for (size_t j = 0; j < offsets_size; j++) {
                f(reinterpret_cast<gc_handle*>(obj + offsets[j]));
            }
            obj += obj_size;
        }
    }
private:
    byte* m_ptr;
};

}}}

#endif //DIPLOMA_MANAGED_OBJECT_HPP
