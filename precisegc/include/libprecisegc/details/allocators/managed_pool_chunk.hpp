#ifndef DIPLOMA_MANAGED_POOL_CHUNK_H
#define DIPLOMA_MANAGED_POOL_CHUNK_H

#include <bitset>
#include <cstdint>
#include <memory>
#include <mutex>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range.hpp>

#include <libprecisegc/details/allocators/stack_chunk.hpp>
#include <libprecisegc/details/allocators/freelist_allocator.hpp>
#include <libprecisegc/details/allocators/null_allocator.hpp>
#include <libprecisegc/details/allocators/managed_ptr_iterator.hpp>
#include <libprecisegc/details/utils/bitset.hpp>
#include <libprecisegc/details/managed_ptr.hpp>
#include <libprecisegc/details/memory_descriptor.hpp>
#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/utils/block_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace allocators {

class managed_pool_chunk : public memory_descriptor, private utils::noncopyable, private utils::nonmovable
{
public:
    static const size_t CHUNK_MAXSIZE = PAGE_SIZE / MIN_CELL_SIZE;;
    static const size_t CHUNK_MINSIZE = 4;
private:
    typedef std::uintptr_t uintptr;
    typedef allocators::stack_chunk plain_pool_chunk;
    typedef utils::bitset<CHUNK_MAXSIZE> bitset_t;
    typedef utils::sync_bitset<CHUNK_MAXSIZE> sync_bitset_t;
    typedef freelist_allocator<null_allocator, fixsize_policy> freelist_t;
public:
    typedef allocators::multi_block_chunk_tag chunk_tag;
    typedef managed_ptr pointer_type;
    typedef managed_ptr_iterator<managed_pool_chunk> iterator;
    typedef boost::iterator_range<iterator> range_type;

    static size_t chunk_size(size_t cell_size)
    {
        return cell_size * CHUNK_MAXSIZE;
    }

    managed_pool_chunk();
    managed_pool_chunk(byte* chunk, size_t size, size_t cell_size);

    ~managed_pool_chunk();

    pointer_type allocate(size_t size);
    void deallocate(const pointer_type& ptr, size_t size);

    bool contains(const pointer_type& ptr) const noexcept;
    bool memory_available() const noexcept;
    bool empty() const noexcept;

    byte*  get_mem() const;
    size_t get_mem_size() const;

    memory_descriptor* get_descriptor();

    size_t sweep();
    void unmark();

    iterator begin();
    iterator end();
    range_type get_range();

    virtual bool get_mark(byte* ptr) const override;
    virtual bool get_pin(byte* ptr) const override;

    virtual void set_mark(byte* ptr, bool mark) override;
    virtual void set_pin(byte* ptr, bool pin) override;

    virtual size_t cell_size() const override;

    virtual object_meta* get_cell_meta(byte* ptr) const override;
    virtual byte* get_obj_begin(byte* ptr) const override;
    virtual byte* get_cell_begin(byte* ptr) const override;
private:
    static uintptr calc_mask(byte* chunk, size_t chunk_size, size_t cell_size);

    void deallocate(byte* ptr, size_t size);

    size_t calc_cell_ind(byte* ptr) const;
    size_t get_log2_cell_size() const;

    plain_pool_chunk m_chunk;
    freelist_t m_freelist;
    size_t m_cell_size;
    size_t m_log2_cell_size;
    uintptr m_mask;
    sync_bitset_t m_mark_bits;
    bitset_t m_pin_bits;
};

}}}

#endif //DIPLOMA_MANAGED_POOL_CHUNK_H
