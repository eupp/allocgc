#ifndef ALLOCGC_GC_POOL_DESCRIPTOR_HPP
#define ALLOCGC_GC_POOL_DESCRIPTOR_HPP

#include <cassert>
#include <bitset>
#include <cstdint>
#include <memory>
#include <mutex>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range.hpp>

#include <liballocgc/gc_alloc.hpp>
#include <liballocgc/details/allocators/gc_box_handle.hpp>
#include <liballocgc/details/utils/bitset.hpp>
#include <liballocgc/details/utils/utility.hpp>
#include <liballocgc/details/allocators/gc_memory_descriptor.hpp>
#include <liballocgc/details/constants.hpp>

namespace allocgc { namespace details { namespace allocators {

class gc_pool_descriptor : public gc_memory_descriptor, private utils::noncopyable, private utils::nonmovable
{
public:
    static const size_t CHUNK_MAXSIZE = MANAGED_CHUNK_OBJECTS_COUNT;
private:
    typedef utils::bitset<CHUNK_MAXSIZE> bitset_t;
    typedef utils::sync_bitset<CHUNK_MAXSIZE> sync_bitset_t;

//    class memory_iterator: public boost::iterator_facade<
//              memory_iterator
//            , gc_box_handle
//            , boost::random_access_traversal_tag
//            , gc_box_handle
//        >
//    {
//    public:
//        memory_iterator()
//            : m_cell_size(0)
//        {}
//
//        memory_iterator(const memory_iterator&) = default;
//        memory_iterator& operator=(const memory_iterator&) = default;
//    private:
//        friend class gc_pool_descriptor;
//        friend class boost::iterator_core_access;
//
//        memory_iterator(byte* ptr, gc_pool_descriptor* descr, size_t box_size)
//            : m_cell(gc_box_handle::from_internal_ptr(ptr, descr))
//            , m_cell_size(box_size)
//        {
//            assert(ptr);
//            assert(descr);
//            assert(box_size > 0);
//        }
//
//        gc_box_handle dereference() const
//        {
//            return m_cell;
//        }
//
//        void increment()
//        {
//            m_cell.reset(m_cell.get() + m_cell_size);
//        }
//
//        void decrement()
//        {
//            m_cell.reset(m_cell.get() - m_cell_size);
//        }
//
//        bool equal(const memory_iterator& other) const
//        {
//            return m_cell.get() == other.m_cell.get();
//        }
//
//        gc_box_handle m_cell;
//        size_t  m_cell_size;
//    };
public:
//    typedef memory_iterator iterator;
//    typedef boost::iterator_range<memory_iterator> memory_range_type;

    static constexpr size_t chunk_size(size_t cell_size)
    {
        return cell_size * CHUNK_MAXSIZE;
    }

    gc_pool_descriptor(byte* chunk, size_t size, size_t cell_size);
    ~gc_pool_descriptor();

    gc_memory_descriptor* descriptor()
    {
        return this;
    }

    byte* init_cell(byte* ptr, size_t obj_count, const gc_type_meta* type_meta)
    {
        assert(contains(ptr));
//        assert(ptr == box_addr(ptr));
        return gc_box::create(ptr, obj_count, type_meta);
    }

    inline bool unused() const
    {
        return m_mark_bits.none();
    }

    inline void unmark()
    {
        m_mark_bits.reset_all();
        m_pin_bits.reset_all();
    }

    size_t count_lived() const
    {
        return m_mark_bits.count();
    }

    size_t count_pinned() const
    {
        return m_pin_bits.count();
    }

    double residency() const;

//    memory_range_type memory_range();

//    iterator begin();
//    iterator end();

    box_id get_id(byte* ptr) const override;

    bool get_mark(box_id id) const override;
    bool get_pin(box_id id) const override;

    void set_mark(box_id id, bool mark) override;
    void set_pin(box_id id, bool pin) override;

    bool is_init(box_id id) const override;

    gc_lifetime_tag get_lifetime_tag(box_id id) const;

    inline size_t cell_size() const
    {
        return 1ull << m_cell_size_log2;
    }

    byte*  box_addr(box_id id) const override;
    size_t box_size(box_id id) const override;

    size_t object_count(box_id id) const override;
    const gc_type_meta* get_type_meta(box_id id) const override;

    void commit(box_id id) override;
    void commit(box_id id, const gc_type_meta* type_meta) override;

    void trace(box_id id, const gc_trace_callback& cb) const override;
    void finalize(box_id id) override;

//    void move(byte* to, byte* from, gc_memory_descriptor* from_descr) override;

    inline byte* memory() const
    {
        return m_memory;
    }

    inline size_t size() const
    {
        return m_size;
    }

    bool contains(byte* ptr) const;

    size_t mem_used();
private:
    inline byte* calc_box_addr(box_id id) const
    {
        return m_memory + (id << m_cell_size_log2);
    }

    inline void set_init(size_t idx, bool init)
    {
        m_init_bits.set(idx, init);
    }

//    size_t calc_cell_ind(byte* ptr) const;

    byte*         m_memory;
    size_t        m_size;
    size_t        m_cell_size_log2;
    bitset_t      m_pin_bits;
    bitset_t      m_init_bits;
    sync_bitset_t m_mark_bits;
};

}}}

#endif // ALLOCGC_GC_POOL_DESCRIPTOR_HPP
