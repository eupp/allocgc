#ifndef DIPLOMA_MANAGED_OBJECT_H
#define DIPLOMA_MANAGED_OBJECT_H

#include "class_meta.h"

namespace precisegc { namespace details {

class object_header
{
public:

    object_header(const class_meta* meta, size_t count, void* obj_ptr)
        : m_meta(meta)
        , m_count(count)
        , m_obj_ptr(obj_ptr)
    {}

    const class_meta* get_class_meta() const noexcept
    {
        return m_meta;
    }

    size_t get_count() const noexcept
    {
        return m_count;
    }

    void* get_object_ptr() const noexcept
    {
        return m_obj_ptr;
    }

private:
    const class_meta* m_meta;
    size_t m_count;
    void* m_obj_ptr;
};

} }

#endif //DIPLOMA_MANAGED_OBJECT_H
