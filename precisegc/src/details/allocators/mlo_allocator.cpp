#include <libprecisegc/details/allocators/mlo_allocator.hpp>

#include <libprecisegc/details/collectors/memory_index.hpp>
#include <libprecisegc/details/compacting/fix_ptrs.hpp>

namespace precisegc { namespace details { namespace allocators {

mlo_allocator::mlo_allocator()
{ }

mlo_allocator::~mlo_allocator()
{
    for (auto it = descriptors_begin(); it != descriptors_end(); ) {
        auto next = std::next(it);
        destroy(it);
        it = next;
    }
}

gc_alloc_response mlo_allocator::allocate(const gc_alloc_request& rqst)
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
            core_allocator::expand_heap();
            blk.reset(allocate_blk(blk_size));
            if (!blk) {
                throw gc_bad_alloc();
            }
        }
    }

    descriptor_t* descr = get_descr(blk.get());
    new (descr) descriptor_t(rqst.alloc_size());
    byte* obj_start = descr->init_cell(get_memblk(blk.get()), rqst.obj_count(), rqst.type_meta());
    collectors::memory_index::add_to_index(align_by_page(blk.get()), m_alloc.get_blk_size(blk_size), descr);

    blk.release();

    return gc_alloc_response(obj_start, rqst.alloc_size(), descr);
}

gc_heap_stat mlo_allocator::collect(compacting::forwarding& frwd)
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

void mlo_allocator::fix(const compacting::forwarding& frwd)
{
    compacting::fix_ptrs(memory_begin(), memory_end(), frwd);
}

void mlo_allocator::destroy(const descriptor_iterator& it)
{
    byte*  blk      = get_blk_by_descr(&(*it));
    size_t blk_size = get_blk_size(it->cell_size());

    collectors::memory_index::remove_from_index(align_by_page(blk), m_alloc.get_blk_size(blk_size));
    it->~descriptor_t();
    deallocate_blk(blk, blk_size);
}

byte* mlo_allocator::allocate_blk(size_t size)
{
    std::lock_guard<mutex_t> lock(m_mutex);
    return m_alloc.allocate(size);
}

void mlo_allocator::deallocate_blk(byte* ptr, size_t size)
{
    m_alloc.deallocate(ptr, size);
}

mlo_allocator::descriptor_iterator mlo_allocator::descriptors_begin()
{
    return descriptor_iterator(m_alloc.begin());
}

mlo_allocator::descriptor_iterator mlo_allocator::descriptors_end()
{
    return descriptor_iterator(m_alloc.end());
}

mlo_allocator::memory_iterator mlo_allocator::memory_begin()
{
    return memory_iterator(m_alloc.begin());
}

mlo_allocator::memory_iterator mlo_allocator::memory_end()
{
    return memory_iterator(m_alloc.end());
}

}}}
