#ifndef ALLOCGC_GC_OBJECT_DESCRIPTOR_HPP
#define ALLOCGC_GC_OBJECT_DESCRIPTOR_HPP

#include <liballocgc/gc_alloc.hpp>
#include <liballocgc/details/utils/utility.hpp>
#include <liballocgc/details/allocators/gc_memory_descriptor.hpp>
#include <liballocgc/details/constants.hpp>

namespace allocgc { namespace details { namespace allocators {

class gc_object_descriptor : public gc_memory_descriptor, private utils::noncopyable, private utils::nonmovable
{
public:
    gc_object_descriptor(size_t size);
    ~gc_object_descriptor();

    gc_memory_descriptor* descriptor();

    byte* init_cell(byte* ptr, size_t obj_count, const gc_type_meta* type_meta);

    box_id get_id(byte* ptr) const override;

    bool get_mark() const noexcept;
    bool get_pin() const noexcept;

    bool get_mark(box_id id) const override;
    bool get_pin(box_id id) const override;

    bool set_mark(bool mark) noexcept;
    bool set_pin(bool pin) noexcept;

    void set_mark(box_id id, bool mark) override;
    void set_pin(box_id id, bool pin) override;

    bool is_init(box_id id) const override;

    gc_lifetime_tag get_lifetime_tag(box_id id) const override;

    byte* box_addr() const;
    byte* box_addr(box_id id) const override;

    size_t box_size() const;
    size_t box_size(box_id id) const override;

    size_t object_count(box_id id) const override;

    const gc_type_meta* get_type_meta(box_id id) const override;

    void commit(box_id id) override;
    void commit(box_id id, const gc_type_meta* type_meta) override;

    void trace(box_id id, const gc_trace_callback& cb) const override;
    void finalize(box_id id) override;
//    void move(byte* to, byte* from, gc_memory_descriptor* from_descr) override;
private:
    bool check_ptr(byte* ptr) const;

    size_t m_size;
    bool   m_mark_bit;
    bool   m_pin_bit;
    bool   m_init_bit;
};

}}}

#endif // ALLOCGC_GC_OBJECT_DESCRIPTOR_HPP
