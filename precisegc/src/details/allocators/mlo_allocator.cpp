#include <libprecisegc/details/allocators/mlo_allocator.hpp>

#include <libprecisegc/details/allocators/core_allocator.hpp>

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
    size_t memblk_size = get_memblk_size(rqst.alloc_size());

    auto deleter = [this, memblk_size] (byte* ptr) {
        deallocate_blk(ptr, memblk_size);
    };
    std::unique_ptr<byte, decltype(deleter)> memblk(allocate_blk(memblk_size), deleter);

    if (!memblk) {
        gc_options opt;
        opt.kind = gc_kind::COLLECT;
        opt.gen  = 0;
        gc_initiation_point(initiation_point_type::HEAP_LIMIT_EXCEEDED, opt);

        memblk.reset(allocate_blk(memblk_size));
        if (!memblk) {
            core_allocator::expand_heap();
            memblk.reset(allocate_blk(memblk_size));
            if (!memblk) {
                throw gc_bad_alloc();
            }
        }
    }

    descriptor_t* descr = get_descr(memblk.get());
    new (descr) descriptor_t(rqst.alloc_size());
    byte* obj_start = gc_box::create(get_memblk(memblk.get()), rqst);
    collectors::memory_index::add_to_index(align_by_page(memblk.get()), m_alloc.get_blk_size(memblk_size), descr);

    memblk.release();

    return gc_alloc_response(obj_start, rqst.alloc_size(), descr);
}

gc_heap_stat mlo_allocator::collect(compacting::forwarding& frwd)
{
    gc_heap_stat stat;
    for (auto it = descriptors_begin(); it != descriptors_end(); ) {
        auto next = std::next(it);
        stat.mem_before_gc += it->size();
        if (!it->get_mark()) {
            stat.mem_freed += it->size();
            destroy(it);
        } else {
            stat.mem_all  += it->size();
            stat.mem_live += it->size();
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
    it->~descriptor_t();
    deallocate_blk(get_blk_by_descr(&(*it)), get_memblk_size(it->size()));
}

byte* mlo_allocator::allocate_blk(size_t size)
{
    return m_alloc.allocate(size);
}

void mlo_allocator::deallocate_blk(byte* ptr, size_t size)
{
    m_alloc.deallocate(ptr, size);
}

}}}
