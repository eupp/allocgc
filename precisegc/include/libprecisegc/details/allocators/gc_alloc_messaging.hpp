#ifndef DIPLOMA_GC_ALLOC_DESCRIPTOR_HPP
#define DIPLOMA_GC_ALLOC_DESCRIPTOR_HPP

#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/gc_cell.hpp>
#include <libprecisegc/details/gc_type_meta.hpp>

namespace precisegc { namespace details { namespace allocators {

class gc_alloc_request
{
public:
    gc_alloc_request();
    gc_alloc_request(std::nullptr_t);
    gc_alloc_request(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta);

    gc_alloc_request(const gc_alloc_request&) noexcept = default;
    gc_alloc_request& operator=(const gc_alloc_request&) noexcept = default;

    size_t alloc_size() const noexcept;

    size_t obj_size() const noexcept;

    size_t obj_count() const noexcept;

    const gc_type_meta* type_meta() const noexcept;
private:
    size_t m_obj_size;
    size_t m_obj_cnt;
    size_t m_alloc_size;
    const gc_type_meta* m_type_meta;
};


class gc_alloc_response
{
public:
    gc_alloc_response();
    gc_alloc_response(std::nullptr_t);
    gc_alloc_response(byte* obj_start, size_t size, const gc_cell& cell);

    gc_alloc_response(const gc_alloc_response&) = default;
    gc_alloc_response& operator=(const gc_alloc_response&) = default;

    inline byte* obj_start() const noexcept
    {
        return m_obj_start;
    }

    inline byte* cell_start() const noexcept
    {
        return m_cell.cell_start();
    }

    inline size_t size() const noexcept
    {
        return m_size;
    }

    inline memory_descriptor* descriptor() const noexcept
    {
        return m_cell.descriptor();
    }

    bool get_mark() const;
    void set_mark(bool mark);

    bool get_pin() const;
    void set_pin(bool pin);

    void commit();
    void commit(const gc_type_meta* type_meta);
private:
    byte*   m_obj_start;
    size_t  m_size;
    gc_cell m_cell;
};

}}}

#endif //DIPLOMA_GC_ALLOC_DESCRIPTOR_HPP
