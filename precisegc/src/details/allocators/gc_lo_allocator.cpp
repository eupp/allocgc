#include <libprecisegc/details/allocators/gc_lo_allocator.hpp>

#include <libprecisegc/details/compacting/fix_ptrs.hpp>

namespace precisegc { namespace details { namespace allocators {

gc_lo_allocator::gc_lo_allocator()
{ }

gc_lo_allocator::~gc_lo_allocator()
{
    for (auto it = descriptors_begin(); it != descriptors_end(); ) {
        auto next = std::next(it);
        destroy(it);
        it = next;
    }
}

gc_alloc_response gc_lo_allocator::allocate(const gc_alloc_request& rqst)
{
    size_t blk_size = get_blk_size(rqst.alloc_size());

    auto deleter = [this, blk_size] (byte* ptr) {
        std::lock_guard<mutex_t> lock(m_mutex);
        deallocate_blk(ptr, blk_size);
    };
    std::unique_ptr<byte, decltype(deleter)> blk(allocate_blk(blk_size), deleter);

    if (!blk) {
        gc_options opt;
        opt.kind = gc_kind::COLLECT;
        opt.gen  = 0;
        gc_initiation_point(initiation_point_type::HEAP_LIMIT_EXCEEDED, opt);

        blk.reset(allocate_blk(blk_size));
        if (!blk) {
            gc_core_allocator::expand_heap();
            blk.reset(allocate_blk(blk_size));
            if (!blk) {
                throw gc_bad_alloc();
            }
        }
    }

    descriptor_t* descr = get_descr(blk.get());
    new (descr) descriptor_t(rqst.alloc_size());
    byte* cell_start = get_memblk(blk.get());
    byte* obj_start = descr->init_cell(cell_start, rqst.obj_count(), rqst.type_meta());
    gc_add_to_index(align_by_page(blk.get()), m_alloc.get_blk_size(blk_size), descr);

    blk.release();

    return gc_alloc_response(obj_start, rqst.alloc_size(), gc_cell::from_cell_start(cell_start, descr));
}

gc_heap_stat gc_lo_allocator::collect(compacting::forwarding& frwd)
{
    gc_heap_stat stat;
    for (auto it = descriptors_begin(); it != descriptors_end(); ) {
        auto next = std::next(it);
        stat.mem_before_gc += it->cell_size();
        if (!it->get_mark()) {
            stat.mem_freed += it->cell_size();
            destroy(it);
        } else {
            stat.mem_all  += it->cell_size();
            stat.mem_live += it->cell_size();
        }
        if (it->get_pin()) {
            ++stat.pinned_cnt;
        }
        it = next;
    }
    return stat;
}

void gc_lo_allocator::fix(const compacting::forwarding& frwd)
{
    compacting::fix_ptrs(memory_begin(), memory_end(), frwd);
}

void gc_lo_allocator::finalize()
{
    for (auto it = descriptors_begin(); it != descriptors_end(); ++it) {
        it->set_mark(false);
        it->set_pin(false);
    }
}

void gc_lo_allocator::destroy(const descriptor_iterator& it)
{
    byte*  blk      = get_blk_by_descr(&(*it));
    size_t blk_size = get_blk_size(it->cell_size());

    byte* memblk = get_memblk(blk);
    if (it->get_lifetime_tag(memblk) == gc_lifetime_tag::GARBAGE) {
        it->finalize(memblk);
    }
    gc_remove_from_index(align_by_page(blk), m_alloc.get_blk_size(blk_size));
    it->~descriptor_t();
    deallocate_blk(blk, blk_size);
}

byte* gc_lo_allocator::allocate_blk(size_t size)
{
    std::lock_guard<mutex_t> lock(m_mutex);
    return m_alloc.allocate(size);
}

void gc_lo_allocator::deallocate_blk(byte* ptr, size_t size)
{
    m_alloc.deallocate(ptr, size);
}

gc_lo_allocator::descriptor_iterator gc_lo_allocator::descriptors_begin()
{
    return descriptor_iterator(m_alloc.begin());
}

gc_lo_allocator::descriptor_iterator gc_lo_allocator::descriptors_end()
{
    return descriptor_iterator(m_alloc.end());
}

gc_lo_allocator::memory_iterator gc_lo_allocator::memory_begin()
{
    return memory_iterator(m_alloc.begin());
}

gc_lo_allocator::memory_iterator gc_lo_allocator::memory_end()
{
    return memory_iterator(m_alloc.end());
}

}}}
