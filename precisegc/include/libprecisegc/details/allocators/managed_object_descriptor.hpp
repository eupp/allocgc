#ifndef DIPLOMA_MANAGED_LARGE_OBJECT_DESCRIPTOR_HPP
#define DIPLOMA_MANAGED_LARGE_OBJECT_DESCRIPTOR_HPP

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/memory_descriptor.hpp>
#include <libprecisegc/details/gc_alloc_messaging.hpp>
#include <libprecisegc/details/constants.hpp>

namespace precisegc { namespace details { namespace allocators {

class managed_object_descriptor : public memory_descriptor, private utils::noncopyable, private utils::nonmovable
{
public:
    typedef gc_alloc_response pointer_type;

    managed_object_descriptor(size_t size);
    ~managed_object_descriptor();

    memory_descriptor* descriptor();

    byte* init_cell(byte* ptr, size_t obj_count, const gc_type_meta* type_meta);

    bool get_mark() const noexcept;
    bool get_pin() const noexcept;

    bool get_mark(byte* ptr) const override;
    bool get_pin(byte* ptr) const override;

    bool set_mark(bool mark) noexcept;
    bool set_pin(bool pin) noexcept;

    void set_mark(byte* ptr, bool mark) override;
    void set_pin(byte* ptr, bool pin) override;

    gc_lifetime_tag get_lifetime_tag(byte* ptr) const override;

    size_t cell_size() const;
    size_t cell_size(byte* ptr) const override;
    byte*  cell_start(byte* ptr) const override;

    size_t object_count(byte* ptr) const override;

    const gc_type_meta* get_type_meta(byte* ptr) const override;

    void commit(byte* ptr, bool mark) override;
    void commit(byte* ptr, bool mark, const gc_type_meta* type_meta) override;

    void trace(byte* ptr, const gc_trace_callback& cb) const override;
    void move(byte* to, byte* from, memory_descriptor* from_descr) override;
    void finalize(byte* ptr) override;
private:
    bool check_ptr(byte* ptr) const;

    size_t m_size;
    bool m_mark_bit;
    bool m_pin_bit;
    bool m_init_bit;
};

}}}

#endif //DIPLOMA_MANAGED_LARGE_OBJECT_DESCRIPTOR_HPP
