#ifndef DIPLOMA_OBJECT_META_H
#define DIPLOMA_OBJECT_META_H

#include "class_meta.h"

namespace precisegc { namespace details {

class object_meta
{
public:

    // computes pointer to object_meta by pointer to object and its size
    static object_meta* get_meta_ptr(void* ptr, size_t obj_size)
    {
        return (object_meta*) ((size_t) ptr + obj_size - sizeof(object_meta));
    }

    object_meta(const class_meta* cls_meta, size_t count, void* obj_ptr)
        : m_cls_meta(cls_meta)
        , m_count(count)
        , m_obj_ptr(obj_ptr)
    {}

    size_t size() const noexcept
    {
        return m_count * m_cls_meta->get_type_size();
    }

    const class_meta* get_class_meta() const noexcept
    {
        return m_cls_meta;
    }

    void set_class_meta(const class_meta* cls_meta) noexcept
    {
        m_cls_meta = cls_meta;
    }

    size_t get_count() const noexcept
    {
        return m_count;
    }

    void set_count(size_t count) noexcept
    {
        m_count = count;
    }

    void* get_object_ptr() const noexcept
    {
        return m_obj_ptr;
    }

    void set_object_ptr(void* ptr) noexcept
    {
        m_obj_ptr = ptr;
    }

private:
    const class_meta* m_cls_meta;
    size_t m_count;
    void* m_obj_ptr;
};

} }

#endif //DIPLOMA_OBJECT_META_H
