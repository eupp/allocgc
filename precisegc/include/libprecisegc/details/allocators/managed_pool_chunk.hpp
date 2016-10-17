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
#include <libprecisegc/details/allocators/managed_memory_iterator.hpp>
#include <libprecisegc/details/utils/bitset.hpp>
#include <libprecisegc/details/collectors/indexed_managed_object.hpp>
#include <libprecisegc/details/memory_descriptor.hpp>
#include <libprecisegc/details/gc_alloc_messaging.hpp>
#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/utils/block_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace allocators {

class managed_pool_chunk : public memory_descriptor, private utils::noncopyable, private utils::nonmovable
{
public:
    static const size_t CHUNK_MAXSIZE = MANAGED_CHUNK_OBJECTS_COUNT;
    static const size_t CHUNK_MINSIZE = 4;
private:
    typedef std::uintptr_t uintptr;
    typedef allocators::stack_chunk plain_pool_chunk;
    typedef utils::bitset<CHUNK_MAXSIZE> bitset_t;
    typedef utils::sync_bitset<CHUNK_MAXSIZE> sync_bitset_t;
    typedef freelist_allocator<null_allocator, fixsize_policy> freelist_t;

    struct object_meta
    {
        const gc_type_meta* m_tmeta;
        size_t m_obj_cnt;
    };
public:
    typedef allocators::multi_block_chunk_tag chunk_tag;
    typedef managed_memory_iterator<managed_pool_chunk> iterator;
    typedef boost::iterator_range<iterator> memory_range_type;

    static constexpr size_t meta_size()
    {
        return sizeof(object_meta);
    }

    static constexpr size_t chunk_size(size_t cell_size)
    {
        return cell_size * CHUNK_MAXSIZE;
    }

    managed_pool_chunk(byte* chunk, size_t size, size_t cell_size);

    ~managed_pool_chunk();

    byte*  memory() const;
    size_t size() const;

    memory_descriptor* get_descriptor();

    gc_alloc_response init(byte* ptr, const gc_alloc_request& rqst);
    void destroy(byte* ptr);
    void move(byte* from, byte* to);

    bool contains(byte* ptr) const;

    bool unused() const;

    size_t count_lived() const;
    size_t count_pinned() const;

    double residency() const;

    void unmark();

    iterator begin();
    iterator end();

    bool is_live(size_t idx) const;
    void set_live(size_t idx, bool live);

    bool is_live(byte* ptr) const;
    void set_live(byte* ptr, bool live);

    bool get_mark(size_t idx) const;
    bool get_pin(size_t idx) const;

    bool get_mark(byte* ptr) const override;
    bool get_pin(byte* ptr) const override;

    void set_mark(size_t idx, bool mark);
    void set_pin(size_t idx, bool pin);

    void set_mark(byte* ptr, bool mark) override;
    void set_pin(byte* ptr, bool pin) override;

    size_t cell_size() const override;
    byte*  cell_start(byte* ptr) const override;

    const gc_type_meta* get_type_meta(byte* ptr) const override;
    void  set_type_meta(byte* ptr, const gc_type_meta* tmeta) override;
private:
    static byte* get_obj(byte* cell_start);
    static object_meta* get_meta(byte* cell_start);

    static uintptr calc_mask(byte* chunk, size_t chunk_size, size_t cell_size);

    size_t calc_cell_ind(byte* ptr) const;
    size_t get_log2_cell_size() const;

    plain_pool_chunk m_chunk;
    size_t m_cell_size;
    size_t m_log2_cell_size;
    uintptr m_mask;
    sync_bitset_t m_mark_bits;
    bitset_t m_pin_bits;
    bitset_t m_live_bits;
};

}}}

#endif //DIPLOMA_MANAGED_POOL_CHUNK_H
