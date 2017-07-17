#ifndef ALLOCGC_GC_CELL_HPP
#define ALLOCGC_GC_CELL_HPP

#include <liballocgc/details/allocators/gc_box.hpp>
#include <liballocgc/details/allocators/gc_memory_descriptor.hpp>

namespace allocgc { namespace details { namespace allocators {

class gc_box_handle
{
public:
    static gc_box_handle from_box_addr(byte* box_addr, gc_memory_descriptor* descr)
    {
        return gc_box_handle(box_addr, descr);
    }

    static gc_box_handle from_internal_ptr(byte* ptr, gc_memory_descriptor* descr)
    {
        return gc_box_handle(descr->get_id(ptr), descr);
    }

    gc_box_handle()
        : m_id(nullptr)
        , m_descr(nullptr)
    {}

    gc_box_handle(const gc_box_handle&) = default;
    gc_box_handle& operator=(const gc_box_handle&) = default;

//    byte* get() const
//    {
//        return box_addr();
//    }

//    void reset(byte* box_addr)
//    {
//        m_id = box_addr;
//    }

    allocators::gc_memory_descriptor* descriptor() const
    {
        return m_descr;
    }

    bool get_mark() const
    {
        assert(is_initialized());
        return m_descr->get_mark(m_id);
    }

    bool get_pin() const
    {
        assert(is_initialized());
        return m_descr->get_pin(m_id);
    }

    void set_mark(bool mark) const
    {
        assert(is_initialized());
        m_descr->set_mark(m_id, mark);
    }

    void set_pin(bool pin) const
    {
        assert(is_initialized());
        m_descr->set_pin(m_id, pin);
    }

    bool is_init() const
    {
        assert(is_initialized());
        return m_descr->is_init(m_id);
    }

    gc_lifetime_tag get_lifetime_tag() const
    {
        assert(is_initialized());
        return m_descr->get_lifetime_tag(m_id);
    }

    byte* box_addr() const
    {
        assert(is_initialized());
        return m_descr->box_addr(m_id);
    }

    size_t box_size() const
    {
        assert(is_initialized());
        return m_descr->box_size(m_id);
    }

    size_t object_count() const
    {
        assert(is_initialized());
        return m_descr->object_count(m_id);
    }

    const gc_type_meta* get_type_meta() const
    {
        assert(is_initialized());
        return m_descr->get_type_meta(m_id);
    }

    void commit() const
    {
        assert(is_initialized());
        m_descr->commit(m_id);
    }

    void commit(const gc_type_meta* type_meta) const
    {
        assert(is_initialized());
        m_descr->commit(m_id, type_meta);
    }

    void trace(const gc_trace_callback& cb) const
    {
        assert(is_initialized());
        m_descr->trace(m_id, cb);
    }

//    void move(const gc_box_handle& to) const
//    {
//        assert(is_initialized());
//        to.m_descr->move(to.m_id, m_id, m_descr);
//    }

    void finalize() const
    {
        assert(is_initialized());
        m_descr->finalize(m_id);
    }
private:
    gc_box_handle(gc_memory_descriptor::box_id id, allocators::gc_memory_descriptor* descr)
            : m_id(id)
            , m_descr(descr)
    {
        assert(is_initialized());
    }

    bool is_initialized() const
    {
        return (m_id != nullptr) && (m_descr != nullptr);
    }

    gc_memory_descriptor::box_id    m_id;
    gc_memory_descriptor*           m_descr;
};

}}}

#endif //ALLOCGC_GC_CELL_HPP
