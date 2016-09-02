#ifndef DIPLOMA_MANAGED_POOL_CHUNK_H
#define DIPLOMA_MANAGED_POOL_CHUNK_H

#include <bitset>
#include <cstdint>
#include <memory>
#include <mutex>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range.hpp>

#include <libprecisegc/details/allocators/bitmap_pool_chunk.hpp>
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
    static const size_t CHUNK_MAXSIZE = allocators::bitmap_pool_chunk::CHUNK_MAXSIZE;
    static const size_t CHUNK_MINSIZE = 4;
private:
    typedef std::uintptr_t uintptr;
    typedef allocators::bitmap_pool_chunk plain_pool_chunk;
    typedef utils::bitset<CHUNK_MAXSIZE> bitset_t;
    typedef utils::sync_bitset<CHUNK_MAXSIZE> sync_bitset_t;
public:

    static size_t get_chunk_size(size_t cell_size)
    {
        size_t chunk_size_ub = cell_size * CHUNK_MAXSIZE;
        if (chunk_size_ub <= PAGE_SIZE) {
            return PAGE_SIZE;
        } else if (chunk_size_ub <= 2 * PAGE_SIZE) {
            return 2 * PAGE_SIZE;
        } else {
            return 4 * PAGE_SIZE;
        }
    }

    class iterator: public boost::iterator_facade<
              iterator
            , const managed_ptr
            , boost::random_access_traversal_tag
            , managed_ptr
        >
    {
    public:
        iterator() noexcept;
        iterator(const iterator&) noexcept = default;
        iterator(iterator&&) noexcept = default;

        iterator& operator=(const iterator&) noexcept = default;
        iterator& operator=(iterator&&) noexcept = default;

        const managed_ptr* operator->()
        {
            return &m_ptr;
        }
    private:
        friend class managed_pool_chunk;
        friend class boost::iterator_core_access;

        iterator(byte* ptr, memory_descriptor* descr) noexcept;

        managed_ptr dereference() const;

        void increment() noexcept;
        void decrement() noexcept;

        bool equal(const iterator& other) const noexcept;

        void advance(ptrdiff_t n);
        ptrdiff_t distance_to(const iterator& other) const;

        size_t cell_size() const;

        managed_ptr m_ptr;
    };

    typedef utils::block_ptr<managed_ptr> pointer_type;
    typedef boost::iterator_range<iterator> range_type;

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

    void unmark();

    iterator begin();
    iterator end();
    range_type get_range();

    virtual bool get_mark(byte* ptr) const override;
    virtual bool get_pin(byte* ptr) const override;

    virtual void set_mark(byte* ptr, bool mark) override;
    virtual void set_pin(byte* ptr, bool pin) override;

    virtual bool is_live(byte* ptr) const override;
    virtual void set_live(byte* ptr, bool live) override;

    virtual void sweep(byte* ptr) override;

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
    size_t m_cell_size;
    size_t m_log2_cell_size;
    uintptr m_mask;
    sync_bitset_t m_mark_bits;
    bitset_t m_pin_bits;
};

}}}

#endif //DIPLOMA_MANAGED_POOL_CHUNK_H
