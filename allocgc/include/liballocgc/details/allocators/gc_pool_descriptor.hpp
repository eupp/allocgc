#ifndef ALLOCGC_GC_POOL_DESCRIPTOR_HPP
#define ALLOCGC_GC_POOL_DESCRIPTOR_HPP

#include <cassert>
#include <bitset>
#include <cstdint>
#include <memory>
#include <mutex>

#include <boost/intrusive/slist_hook.hpp>

#include <liballocgc/gc_alloc.hpp>
#include <liballocgc/details/utils/bitmap.hpp>
#include <liballocgc/details/utils/utility.hpp>
#include <liballocgc/details/constants.hpp>
#include <liballocgc/details/allocators/gc_box_handle.hpp>
#include <liballocgc/details/allocators/gc_bucket_policy.hpp>
#include <liballocgc/details/allocators/gc_memory_descriptor.hpp>

namespace allocgc { namespace details { namespace allocators {

class gc_pool_descriptor_base : public gc_memory_descriptor, public boost::intrusive::slist_base_hook<>
{
public:
    gc_pool_descriptor_base(byte* chunk, size_t size, size_t cell_size, const gc_bucket_policy& bucket_policy)
        : m_memory(chunk)
        , m_size(size)
        , m_cell_size(cell_size)
        , m_offset_tbl(bucket_policy.offsets_table(cell_size))
    {}

    inline byte* memory() const
    {
        return m_memory;
    }

    inline size_t size() const
    {
        return m_size;
    }

    inline size_t cell_size() const
    {
        return m_cell_size;
    }

    bool contains(byte* ptr) const
    {
        byte* mem_begin = memory();
        byte* mem_end   = memory() + size();
        return (mem_begin <= ptr) && (ptr < mem_end);
    }

    size_t mem_used()
    {
        return 0;
    }

    virtual bool is_unmarked() const = 0;
    virtual void unmark() = 0;

    virtual byte* init_cell(byte* ptr, size_t obj_count, const gc_type_meta* type_meta) = 0;
protected:
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

private:
    typedef gc_bucket_policy::offset_table offset_table;

    byte*         m_memory;
    size_t        m_size;
    size_t        m_cell_size;
    offset_table  m_offset_tbl;
};

class gc_pool_descriptor : public gc_pool_descriptor_base, private utils::noncopyable, private utils::nonmovable
{
    static const size_t CHUNK_MAXSIZE = GC_POOL_CHUNK_OBJECTS_COUNT;

    typedef utils::bitmap bitmap;
    typedef utils::atomic_bitmap atomic_bitmap;
public:
    gc_pool_descriptor(byte* chunk, size_t size, size_t cell_size, const gc_bucket_policy& bucket_policy)
        : gc_pool_descriptor_base(chunk, size, cell_size, bucket_policy)
        , m_init_bits(CHUNK_MAXSIZE)
        , m_pin_bits(CHUNK_MAXSIZE)
        , m_mark_bits(CHUNK_MAXSIZE)
    {}

    ~gc_pool_descriptor() {}

    size_t count_lived() const
    {
        return m_mark_bits.count();
    }

    size_t count_pinned() const
    {
        return m_pin_bits.count();
    }

    double residency() const
    {
        return static_cast<double>(m_mark_bits.count()) / m_mark_bits.size();
    }

    inline bool is_unmarked() const
    {
        return m_mark_bits.none();
    }

    inline void unmark()
    {
        m_mark_bits.reset_all();
        m_pin_bits.reset_all();
    }

    byte* init_cell(byte* ptr, size_t obj_count, const gc_type_meta* type_meta)
    {
        assert(contains(ptr));
        assert(ptr == box_addr(ptr));
        return gc_box::create(ptr, obj_count, type_meta);
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

    box_id get_id(byte* ptr) const override
    {
        assert(contains(ptr));
        return ptr;
    }

    bool is_init(box_id id) const override
    {
        return m_init_bits.get(calc_box_idx(id));
    }

    bool get_mark(box_id id) const override
    {
        assert(contains(id));
        assert(is_correct_id(id));
        return m_mark_bits.get(calc_box_idx(id));
    }

    bool get_pin(box_id id) const override
    {
        assert(contains(id));
        assert(is_correct_id(id));
        return m_pin_bits.get(calc_box_idx(id));
    }

    void set_mark(box_id id, bool mark) override
    {
        assert(contains(id));
        assert(is_correct_id(id));
        m_mark_bits.set(calc_box_idx(id), mark);
    }

    void set_pin(box_id id, bool pin) override
    {
        assert(contains(id));
        assert(is_correct_id(id));
        m_pin_bits.set(calc_box_idx(id), pin);
    }

    bool mark(box_id id) override
    {
        assert(contains(id));
        assert(is_correct_id(id));
        return m_mark_bits.test_and_set(calc_box_idx(id));
    }

    gc_lifetime_tag get_lifetime_tag(box_id id) const override
    {
        assert(contains(id));
        assert(is_correct_id(id));
        size_t idx = calc_box_idx(id);
        return get_lifetime_tag_by_bits(get_mark(idx), is_init(idx));
    }

    size_t box_size(box_id id) const override
    {
        assert(contains(id));
        assert(is_correct_id(id));
        return cell_size();
    }

    byte* box_addr(box_id id) const override
    {
        assert(contains(id));
        assert(is_correct_id(id));
        return calc_box_addr(id);
    }

    size_t object_count(box_id id) const override
    {
        assert(contains(id));
        assert(is_correct_id(id));
        return gc_box::get_obj_count(calc_box_addr(id));
    }

    const gc_type_meta* get_type_meta(box_id id) const override
    {
        assert(contains(id));
        assert(is_correct_id(id));
        return gc_box::get_type_meta(calc_box_addr(id));
    }

    void commit(box_id id) override
    {
        assert(contains(id));
        assert(is_correct_id(id));
        set_init(calc_box_idx(id), true);
    }

    void commit(box_id id, const gc_type_meta* type_meta) override
    {
        assert(type_meta);
        assert(is_correct_id(id));
        gc_box::set_type_meta(box_addr(id), type_meta);
        set_init(calc_box_idx(id), true);
    }

    void trace(box_id id, const gc_trace_callback& cb) const override
    {
        assert(is_correct_id(id));
        gc_box::trace(calc_box_addr(id), cb);
    }

    void finalize(size_t idx)
    {
        byte* box_addr = memory() + idx * cell_size();
        assert(get_lifetime_tag(box_addr) == gc_lifetime_tag::GARBAGE);
        gc_box::destroy(box_addr);
        set_init(idx, false);
    }

    void finalize(box_id id) override
    {
        assert(is_correct_id(id));
        assert(get_lifetime_tag(id) == gc_lifetime_tag::GARBAGE);
        gc_box::destroy(calc_box_addr(id));
        set_init(calc_box_idx(id), false);
    }
private:
    inline void set_init(size_t idx, bool init)
    {
        m_init_bits.set(idx, init);
    }

    bool is_correct_id(box_id id) const
    {
        return id == get_id(id);
    }


    bitmap        m_init_bits;
    bitmap        m_pin_bits;
    atomic_bitmap m_mark_bits;
};

}}}

#endif // ALLOCGC_GC_POOL_DESCRIPTOR_HPP
