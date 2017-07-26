#ifndef ALLOCGC_CELL_DESCRIPTOR_H
#define ALLOCGC_CELL_DESCRIPTOR_H

#include <liballocgc/details/gc_interface.hpp>
#include <liballocgc/gc_type_meta.hpp>
#include <liballocgc/gc_common.hpp>

namespace allocgc { namespace details { namespace allocators {

class gc_memory_descriptor
{
public:
    typedef byte* box_id;

    virtual ~gc_memory_descriptor() {}

    virtual box_id get_id(byte* ptr) const = 0;

    virtual bool get_mark(box_id id) const = 0;
    virtual bool get_pin(box_id id) const = 0;

    virtual void set_mark(box_id id, bool mark) = 0;
    virtual void set_pin(box_id id, bool pin) = 0;

    virtual bool mark(box_id id) = 0;

    virtual bool is_init(box_id id) const = 0;

    virtual gc_lifetime_tag get_lifetime_tag(box_id id) const = 0;

    virtual byte*  box_addr(box_id id) const = 0;
    virtual size_t box_size(box_id id) const = 0;

    virtual size_t object_count(box_id id) const = 0;

    virtual const gc_type_meta* get_type_meta(box_id id) const = 0;

    virtual void commit(box_id id) = 0;
    virtual void commit(box_id id, const gc_type_meta* type_meta) = 0;

    virtual void trace(box_id id, const gc_trace_callback& cb) const = 0;
    virtual void finalize(box_id id) = 0;
//    virtual void move(byte* to, byte* from, gc_memory_descriptor* from_descr) = 0;
};

}}}

#endif //ALLOCGC_CELL_DESCRIPTOR_H
