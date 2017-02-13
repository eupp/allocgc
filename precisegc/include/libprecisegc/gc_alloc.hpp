#ifndef DIPLOMA_GC_ALLOC_DESCRIPTOR_HPP
#define DIPLOMA_GC_ALLOC_DESCRIPTOR_HPP

#include <libprecisegc/gc_common.hpp>
#include <libprecisegc/gc_type_meta.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc {

class gc_alloc : private details::utils::noncopyable, private details::utils::nonmovable
{
public:
    gc_alloc() = delete;

    class request
    {
    public:
        inline request(size_t obj_size,
                       size_t obj_cnt,
                       const gc_type_meta* tmeta,
                       gc_buf* buf)
            : m_obj_size(obj_size)
            , m_obj_cnt(obj_cnt)
            , m_alloc_size(obj_size * obj_cnt)
            , m_type_meta(tmeta)
            , m_buf(buf)
        { }

        request() = default;

        request(const request&) noexcept = default;
        request(request&&) noexcept = default;

        request& operator=(const request&) noexcept = default;
        request& operator=(request&&) noexcept = default;

        inline size_t alloc_size() const noexcept
        {
            return m_alloc_size;
        }

        inline size_t obj_count() const noexcept
        {
            return m_obj_cnt;
        }

        inline size_t obj_size() const noexcept
        {
            return m_obj_size;
        }

        inline const gc_type_meta* type_meta() const noexcept
        {
            return m_type_meta;
        }

        inline gc_buf* buffer() const noexcept
        {
            return m_buf;
        }
    private:
        size_t              m_obj_size;
        size_t              m_obj_cnt;
        size_t              m_alloc_size;
        const gc_type_meta* m_type_meta;
        gc_buf*             m_buf;
    };


    class response
    {
    public:
        inline response(byte* obj_start,
                        byte* cell_start,
                        size_t cell_size,
                        gc_buf* buf)
            : m_obj_start(obj_start)
            , m_cell_start(cell_start)
            , m_cell_size(cell_size)
            , m_buf(buf)
        {}

        response() = default;

        response(const response&) = default;
        response(response&&) = default;

        response& operator=(const response&) = default;
        response& operator=(response&&) = default;

        inline byte* obj_start() const noexcept
        {
            return m_obj_start;
        }

        inline byte* cell_start() const noexcept
        {
            return m_cell_start;
        }

        inline size_t cell_size() const noexcept
        {
            return m_cell_size;
        }

        inline gc_buf* buffer() const noexcept
        {
            return m_buf;
        }
    private:
        byte*       m_obj_start;
        byte*       m_cell_start;
        size_t      m_cell_size;
        gc_buf*     m_buf;
    };
};

}

#endif //DIPLOMA_GC_ALLOC_DESCRIPTOR_HPP
