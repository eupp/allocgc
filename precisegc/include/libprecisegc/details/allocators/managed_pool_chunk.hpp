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
    typedef allocators::stack_chunk plain_pool_chunk;
    typedef utils::bitset<CHUNK_MAXSIZE> bitset_t;
    typedef utils::sync_bitset<CHUNK_MAXSIZE> sync_bitset_t;

    class memory_iterator:
              public managed_memory_iterator<managed_pool_chunk>
            , public boost::iterator_facade<
                      memory_iterator
                    , managed_memory_iterator<managed_pool_chunk>::proxy_t
                    , boost::random_access_traversal_tag
                    , managed_memory_iterator<managed_pool_chunk>::proxy_t
            >
    {
    public:
        memory_iterator();
        memory_iterator(byte* ptr, managed_pool_chunk* descr);

        const proxy_t* operator->()
        {
            return &m_proxy;
        }
    private:
        friend class boost::iterator_core_access;

        void increment();
        void decrement();

        bool equal(const memory_iterator& other) const;
    };
public:
    typedef allocators::multi_block_chunk_tag chunk_tag;
    typedef memory_iterator iterator;
    typedef boost::iterator_range<memory_iterator> memory_range_type;

    static constexpr size_t chunk_size(size_t cell_size)
    {
        return cell_size * CHUNK_MAXSIZE;
    }

    managed_pool_chunk(byte* chunk, size_t size, size_t cell_size);
    ~managed_pool_chunk();

    byte*  memory() const;
    size_t size() const;

    memory_descriptor* descriptor();

    gc_alloc_response init(byte* ptr, const gc_alloc_request& rqst);
    size_t destroy(byte* ptr);
    void move(byte* from, byte* to);

    bool contains(byte* ptr) const;

    bool unused() const;

    size_t count_lived() const;
    size_t count_pinned() const;

    double residency() const;

    void unmark();

    memory_range_type memory_range();

    iterator begin();
    iterator end();

    void set_initialized(byte* ptr) override;
    bool is_initialized(byte* ptr) const override;

    bool get_mark(size_t idx) const;
    bool get_pin(size_t idx) const;

    size_t cell_size() const;

    void set_mark(size_t idx, bool mark);
    void set_pin(size_t idx, bool pin);

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
    bool is_init(size_t idx) const;
    void set_init(size_t idx, bool init);

    bool is_init(byte* ptr) const;
    void set_init(byte* ptr, bool init);

    size_t calc_cell_ind(byte* ptr) const;

    collectors::traceable_object_meta* get_meta(byte* ptr) const;

    byte* m_memory;
    size_t m_size;
    size_t m_cell_size;
    sync_bitset_t m_mark_bits;
    bitset_t m_pin_bits;
    bitset_t m_init_bits;
};

}}}

#endif //DIPLOMA_MANAGED_POOL_CHUNK_H
