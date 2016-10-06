#ifndef DIPLOMA_MANAGED_LARGE_OBJECT_DESCRIPTOR_HPP
#define DIPLOMA_MANAGED_LARGE_OBJECT_DESCRIPTOR_HPP

#include <libprecisegc/details/collectors/indexed_managed_object.hpp>
#include <libprecisegc/details/memory_descriptor.hpp>
#include <libprecisegc/details/gc_alloc_descriptor.hpp>
#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/allocators/managed_memory_iterator.hpp>
#include <libprecisegc/details/utils/block_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace allocators {

class managed_object_descriptor : public memory_descriptor, private utils::noncopyable, private utils::nonmovable
{
public:
    typedef gc_alloc_descriptor pointer_type;
    typedef allocators::single_block_chunk_tag chunk_tag;
    typedef managed_memory_iterator<managed_object_descriptor> iterator;

    static size_t chunk_size(size_t size)
    {
        return ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
    }

    managed_object_descriptor();
    managed_object_descriptor(byte* ptr, size_t size, size_t obj_size);
    ~managed_object_descriptor();

    pointer_type allocate(size_t size);
    void deallocate(const pointer_type& ptr, size_t size);

    bool contains(const pointer_type& ptr) const noexcept;
    bool memory_available() const noexcept;
    bool empty() const;

    byte* get_mem() const;
    size_t get_mem_size() const;

    memory_descriptor* get_descriptor();

    iterator begin();
    iterator end();

    bool get_mark(byte* ptr) const override;
    bool get_pin(byte* ptr) const override;

    void set_mark(byte* ptr, bool mark) override;
    void set_pin(byte* ptr, bool pin) override;

    size_t cell_size() const override;
    byte*  cell_start(byte* ptr) const override;

    void set_type_meta(byte* ptr, const type_meta* tmeta) override;
private:
    bool check_ptr(byte* ptr) const;

    byte* m_ptr;
    size_t m_size;
    bool m_mark_bit;
    bool m_pin_bit;
};

}}}

#endif //DIPLOMA_MANAGED_LARGE_OBJECT_DESCRIPTOR_HPP
