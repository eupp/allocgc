#ifndef DIPLOMA_MANAGED_POOL_CHUNK_H
#define DIPLOMA_MANAGED_POOL_CHUNK_H

#include <bitset>
#include <cstdint>
#include <memory>
#include <mutex>

#include "allocators/plain_pool_chunk.h"
#include "allocators/iterator_range.h"
#include "allocators/lockable_range.h"
#include "allocators/constants.h"
#include "iterator_facade.h"
#include "iterator_access.h"
#include "managed_memory_descriptor.h"
#include "managed_ptr.h"
#include "mutex.h"
#include "util.h"

namespace precisegc { namespace details {

class managed_pool_chunk : public managed_memory_descriptor, private noncopyable
{
public:
    static const size_t CHUNK_MAXSIZE = PAGE_SIZE / MIN_CELL_SIZE;
    static const size_t CHUNK_MINSIZE = 4;
private:
    typedef std::uintptr_t uintptr;
    typedef allocators::plain_pool_chunk plain_pool_chunk;
    typedef managed_memory_descriptor::mutex_type mutex_type;
    typedef managed_memory_descriptor::lock_type lock_type;
    typedef std::bitset<CHUNK_MAXSIZE> bitset_t;
public:

    class iterator: public iterator_facade<iterator, std::bidirectional_iterator_tag, managed_cell_ptr>
    {
        class proxy
        {
        public:
            proxy(managed_cell_ptr&& ptr)
                : m_ptr(std::move(ptr))
            {}

            managed_cell_ptr* operator->() noexcept
            {
                return &m_ptr;
            }
        private:
            managed_cell_ptr m_ptr;
        };
    public:

        typedef managed_cell_ptr value_type;
        typedef managed_cell_ptr reference;
        typedef proxy pointer;

        iterator() noexcept;
        iterator(const iterator&) noexcept = default;
        iterator(iterator&&) noexcept = default;

        iterator& operator=(const iterator&) noexcept = default;
        iterator& operator=(iterator&&) noexcept = default;

        managed_cell_ptr operator*() const noexcept;
        proxy operator->() const noexcept;

        friend class managed_pool_chunk;
        friend class iterator_access<iterator>;
    private:
        iterator(byte* ptr, managed_pool_chunk* descr) noexcept;

        bool equal(const iterator& other) const noexcept;
        void increment() noexcept;
        void decrement() noexcept;

        byte* m_ptr;
        managed_pool_chunk* m_chunk;
    };

    typedef managed_cell_ptr pointer_type;
    typedef allocators::iterator_range<iterator> range_type;

    managed_pool_chunk(byte* chunk, size_t size, size_t cell_size);
    ~managed_pool_chunk();

    managed_cell_ptr allocate(size_t cell_size);
    void deallocate(const managed_cell_ptr& ptr, size_t cell_size);

    bool contains(const managed_cell_ptr& ptr) const noexcept;
    bool memory_available() const noexcept;
    bool empty(size_t cell_size) const noexcept;

    bool is_dead() const noexcept;
    void reset_bits();

    managed_memory_descriptor* get_descriptor();

    virtual bool get_mark(byte* ptr) override;
    virtual bool get_pin(byte* ptr) override;

    virtual void set_mark(byte* ptr, bool mark) override;
    virtual void set_pin(byte* ptr, bool pin) override;

    virtual void sweep(byte* ptr) override;

    virtual bool is_live(byte* ptr) override;

    virtual object_meta* get_cell_meta(byte* ptr) override;
    virtual byte* get_object_begin(byte* ptr) override;
    virtual byte* get_cell_begin(byte* ptr) override;

    virtual lock_type lock() override;
    virtual lock_type lock(std::defer_lock_t t) override;
    virtual lock_type lock(std::try_to_lock_t t) override;
    virtual lock_type lock(std::adopt_lock_t t) override;

    byte* get_mem() const;
    size_t get_mem_size() const;
    size_t get_cell_size() const;

    range_type get_range();

    void swap(managed_pool_chunk& other);
    friend void swap(managed_pool_chunk& a, managed_pool_chunk& b);
private:
    static uintptr calc_mask(byte* chunk, size_t chunk_size, size_t cell_size);
    static size_t calc_cell_ind(byte* ptr, size_t obj_size, byte* base_ptr, size_t size);

    size_t calc_cell_ind(byte* ptr);
    size_t get_log2_cell_size() const;

    plain_pool_chunk m_chunk;
    bitset_t m_alloc_bits;
    size_t m_cell_size;
    size_t m_log2_cell_size;
    uintptr m_mask;
    mutex_type m_mutex;
    bitset_t m_mark_bits;
    bitset_t m_pin_bits;
};

}}

#endif //DIPLOMA_MANAGED_POOL_CHUNK_H
