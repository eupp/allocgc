#ifndef DIPLOMA_MANAGED_LARGE_OBJECT_DESCRIPTOR_HPP
#define DIPLOMA_MANAGED_LARGE_OBJECT_DESCRIPTOR_HPP

#include <libprecisegc/details/collectors/indexed_managed_object.hpp>
#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/allocators/managed_memory_iterator.hpp>
#include <libprecisegc/details/utils/block_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/memory_descriptor.hpp>
#include <libprecisegc/details/gc_alloc_messaging.hpp>
#include <libprecisegc/details/constants.hpp>

namespace precisegc { namespace details { namespace allocators {

class managed_object_descriptor : public memory_descriptor, private utils::noncopyable, private utils::nonmovable
{
public:
    typedef gc_alloc_response pointer_type;
    typedef allocators::single_block_chunk_tag chunk_tag;
    typedef managed_memory_iterator<managed_object_descriptor> iterator;

    static size_t chunk_size(size_t size)
    {
        return ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
    }

    managed_object_descriptor(size_t size);
    ~managed_object_descriptor();

    size_t size() const;

    memory_descriptor* descriptor();

    gc_alloc_response init(byte* ptr, const gc_alloc_request& rqst);
    size_t destroy(byte* ptr);

    void set_initialized(byte* ptr) override;
    bool is_initialized(byte* ptr) const override;

    bool get_mark() const noexcept;
    bool get_pin() const noexcept;

    bool set_mark(bool mark) noexcept;
    bool set_pin(bool pin) noexcept;

    bool get_mark(byte* ptr) const override;
    bool get_pin(byte* ptr) const override;

    void set_mark(byte* ptr, bool mark) override;
    void set_pin(byte* ptr, bool pin) override;

    size_t cell_size(byte* ptr) const override;
    byte*  cell_start(byte* ptr) const override;

    size_t object_count(byte* ptr) const override;
    void   set_object_count(byte* ptr, size_t cnt) const override;

    const gc_type_meta* get_type_meta(byte* ptr) const override;
    void  set_type_meta(byte* ptr, const gc_type_meta* tmeta) override;
private:
    collectors::traceable_object_meta* get_meta(byte* cell_start) const;

    bool check_ptr(byte* ptr) const;

    size_t m_size;
    bool m_mark_bit;
    bool m_pin_bit;
    bool m_init_bit;
};

}}}

#endif //DIPLOMA_MANAGED_LARGE_OBJECT_DESCRIPTOR_HPP
