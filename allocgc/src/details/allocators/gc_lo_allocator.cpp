#include <liballocgc/details/allocators/gc_lo_allocator.hpp>

#include <liballocgc/details/allocators/memory_index.hpp>
#include <liballocgc/details/collectors/gc_new_stack_entry.hpp>
#include <liballocgc/details/compacting/fix_ptrs.hpp>

namespace allocgc { namespace details { namespace allocators {

gc_lo_allocator::gc_lo_allocator(gc_core_allocator* core_alloc)
    : m_alloc(redirection_allocator<gc_core_allocator>(core_alloc))
{ }

gc_lo_allocator::~gc_lo_allocator()
{
    for (auto it = descriptors_begin(); it != descriptors_end(); ) {
        auto next = std::next(it);
        destroy(it);
        it = next;
    }
}

gc_alloc::response gc_lo_allocator::allocate(const gc_alloc::request& rqst)
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

        m_alloc.upstream_allocator().allocator()->gc(opt);

        blk.reset(allocate_blk(blk_size));
        if (!blk) {
            m_alloc.upstream_allocator().allocator()->expand_heap();
            blk.reset(allocate_blk(blk_size));
            if (!blk) {
                throw gc_bad_alloc();
            }
        }
    }

    descriptor_t* descr = get_descr(blk.get());
    new (descr) descriptor_t(rqst.alloc_size());

    byte*  cell_start = get_memblk(blk.get());
    size_t cell_size  = get_cell_size(rqst.alloc_size());
    byte*  obj_start  = descr->init_cell(cell_start, rqst.obj_count(), rqst.type_meta());

    memory_index::index_gc_heap_memory(align_by_page(blk.get()), m_alloc.get_blk_size(blk_size), descr);

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rqst.buffer());
    stack_entry->descriptor = descr;

    blk.release();

    return gc_alloc::response(obj_start, cell_start, cell_size, rqst.buffer());
}

gc_collect_stat gc_lo_allocator::collect(compacting::forwarding& frwd)
{
    gc_collect_stat stat;
    size_t freed = 0;
    for (auto it = descriptors_begin(); it != descriptors_end(); ) {
        auto next = std::next(it);
        if (!it->get_mark()) {
            stat.mem_freed += it->cell_size();
            freed += get_cell_size(it->cell_size());
            destroy(it);
        } else {
            if (it->get_pin()) {
                ++stat.pinned_cnt;
            }
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

gc_memstat gc_lo_allocator::stats()
{
    gc_memstat stat;
    for (auto it = memory_begin(); it != memory_end(); ++it) {
        if (it->is_init()) {
            stat.mem_live += it->object_count() * it->get_type_meta()->type_size();
        }
        stat.mem_used += get_blk_size(it->cell_size());
    }
    return stat;
}

void gc_lo_allocator::destroy(const descriptor_iterator& it)
{
    byte*  blk      = get_blk_by_descr(&(*it));
    size_t blk_size = get_blk_size(it->cell_size());

    byte* memblk = get_memblk(blk);
    it->finalize(memblk);
    memory_index::deindex(align_by_page(blk), m_alloc.get_blk_size(blk_size));
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
