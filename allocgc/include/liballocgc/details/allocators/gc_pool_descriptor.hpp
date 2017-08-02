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
#include <liballocgc/details/utils/bitmap.hpp>
#include <liballocgc/details/utils/utility.hpp>
#include <liballocgc/details/constants.hpp>
#include <liballocgc/details/allocators/gc_bucket_policy.hpp>
#include <liballocgc/details/allocators/gc_memory_descriptor.hpp>

namespace allocgc { namespace details { namespace allocators {

class gc_pool_descriptor : public gc_memory_descriptor, private utils::noncopyable, private utils::nonmovable
{
public:
    static const size_t CHUNK_MAXSIZE = GC_POOL_CHUNK_OBJECTS_COUNT;

    typedef utils::bitmap bitmap;
    typedef utils::atomic_bitmap atomic_bitmap;
    typedef gc_bucket_policy::offset_table offset_table;
private:
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

    gc_pool_descriptor(byte* chunk, size_t size, size_t cell_size, const gc_bucket_policy& bucket_policy);
    ~gc_pool_descriptor();

    gc_memory_descriptor* descriptor()
    {
        return this;
    }

    byte* init_cell(byte* ptr, size_t obj_count, const gc_type_meta* type_meta)
    {
        assert(contains(ptr));
        assert(ptr == box_addr(ptr));
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

    bool mark(box_id id) override;

    bool is_init(box_id id) const override;

    gc_lifetime_tag get_lifetime_tag(box_id id) const;

    inline size_t cell_size() const
    {
        return m_cell_size;
    }

    byte*  box_addr(box_id id) const override;
    size_t box_size(box_id id) const override;

    size_t object_count(box_id id) const override;
    const gc_type_meta* get_type_meta(box_id id) const override;

    void commit(box_id id) override;
    void commit(box_id id, const gc_type_meta* type_meta) override;

    void trace(box_id id, const gc_trace_callback& cb) const override;

    void finalize(size_t idx);
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

    inline bool get_mark(size_t idx) const
    {
        return m_mark_bits.get(idx);
    }

    inline bool get_pin(size_t idx) const
    {
        return m_pin_bits.get(idx);
    }

    inline bool is_init(size_t idx) const
    {
        return m_init_bits.get(idx);
    }

    inline void set_mark(size_t idx, bool mark)
    {
        m_mark_bits.set(idx, mark);
    }

    inline void set_pin(size_t idx, bool pin)
    {
        m_pin_bits.set(idx, pin);
    }

    bool contains(byte* ptr) const;

    size_t mem_used();
private:
    inline byte* calc_box_addr(box_id id) const
    {
        return m_memory + m_cell_size * calc_box_idx(id);
    }

    inline size_t calc_box_idx(box_id id) const
    {
        assert(contains(id));
        assert(is_correct_id(id));
        return m_offset_tbl.obj_idx(id - memory());
    }

    inline void set_init(size_t idx, bool init)
    {
        m_init_bits.set(idx, init);
    }

    bool is_correct_id(box_id id) const;

    byte*         m_memory;
    size_t        m_size;
    size_t        m_cell_size;
    offset_table  m_offset_tbl;
    bitmap        m_init_bits;
    bitmap        m_pin_bits;
    atomic_bitmap m_mark_bits;
};

}}}

#endif // ALLOCGC_GC_POOL_DESCRIPTOR_HPP
