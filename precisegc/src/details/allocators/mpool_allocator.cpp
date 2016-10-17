#include <libprecisegc/details/allocators/mpool_allocator.hpp>

#include <tuple>

#include <libprecisegc/details/collectors/memory_index.hpp>

namespace precisegc { namespace details { namespace allocators {

mpool_allocator::mpool_allocator()
    : m_freelist(nullptr)
    , m_top(nullptr)
    , m_end(nullptr)
    , m_cell_idx(0)
{}

gc_alloc_response mpool_allocator::allocate(const gc_alloc_request& rqst)
{
    size_t size = rqst.alloc_size() + descriptor_t::meta_size();
    if (m_top == m_end) {
        return try_expand_and_allocate(size, rqst, true);
    }
    return stack_allocation(size, rqst);
}

gc_alloc_response mpool_allocator::try_expand_and_allocate(size_t size, const gc_alloc_request& rqst, bool call_gc)
{
    byte*  blk;
    size_t blk_size;
    std::tie(blk, blk_size) = allocate_block(size);
    if (blk) {
        create_descriptor(blk, blk_size, 0);
        m_top = blk;
        m_end = blk + blk_size;
        m_cell_idx = 0;
        return stack_allocation(size, rqst);
    } else if (m_freelist) {
        return freelist_allocation(size, rqst);
    } else {
        if (call_gc) {
            // call gc
        } else {
            throw gc_bad_alloc();
        }
        return try_expand_and_allocate(size, rqst, false);
    }
}

gc_alloc_response mpool_allocator::stack_allocation(size_t size, const gc_alloc_request& rqst)
{
    assert(m_top <= m_end - size);

    descriptor_t& descr = m_descrs.back();
    descr.set_live(m_cell_idx++, true);
    byte* ptr = m_top;
    m_top += size;
    return descr.init(ptr, rqst);
}

gc_alloc_response mpool_allocator::freelist_allocation(size_t size, const gc_alloc_request& rqst)
{
    assert(m_freelist);

    byte* ptr  = reinterpret_cast<byte*>(m_freelist);
    m_freelist = reinterpret_cast<byte**>(m_freelist[0]);

    assert(contains(ptr));

    memset(ptr, 0, size);
    descriptor_t* descr = static_cast<descriptor_t*>(collectors::memory_index::index(ptr));
    descr->set_live(ptr, true);
    return descr->init(ptr, rqst);
}

mpool_allocator::iterator_t mpool_allocator::create_descriptor(byte* blk, size_t blk_size, size_t cell_size)
{
    m_descrs.emplace_back(blk, blk_size, cell_size);
    return std::prev(m_descrs.end());
}

iterator_t mpool_allocator::destroy_descriptor(iterator_t it)
{
    sweep(*it);
    deallocate_block(it->memory(), it->size());
    return m_descrs.erase(it);
}

std::pair<byte*, size_t> mpool_allocator::allocate_block(size_t cell_size)
{
    size_t chunk_size = descriptor_t::chunk_size(cell_size);
    return std::make_pair(core_allocator::allocate(chunk_size), chunk_size);
}

void mpool_allocator::deallocate_block(byte* ptr, size_t size)
{
    core_allocator::deallocate(ptr, size);
}

bool mpool_allocator::contains(byte* ptr) const
{
    for (auto& descr: m_descrs) {
        if (descr.contains(ptr)) {
            return true;
        }
    }
    return false;
}

gc_heap_stat mpool_allocator::collect(compacting::forwarding& frwd)
{
    gc_heap_stat stat;
    shrink(stat);
    if (is_compaction_required(stat)) {
        compact(frwd, stat);
    } else {
        sweep(stat);
    }
    return stat;
}

void mpool_allocator::shrink(gc_heap_stat& stat)
{
    for (iterator_t it = m_descrs.begin(), end = m_descrs.end(); it != end; ) {
        stat.mem_used += it->size();
        if (it->unused()) {
            stat.mem_freed += it->size();
            it = destroy_descriptor(it);
        } else {
            stat.mem_residency += it->residency();
            stat.pinned_cnt += it->count_pinned();
            ++it;
        }
    }
}

void mpool_allocator::sweep(gc_heap_stat& stat)
{
    for (auto& descr: m_descrs) {
        stat.mem_freed += descr.lived_cnt() * descr.cell_size();
        sweep(descr);
    }
}

size_t mpool_allocator::sweep(descriptor_t& descr)
{
    byte* it  = descr.memory();
    byte* end = descr.memory() + descr.size();
    for (size_t i = 0; it < end; it += descr.cell_size(), ++i) {
        descr.destroy(it);
    }
}

void mpool_allocator::compact(compacting::forwarding& frwd, gc_heap_stat& stat)
{

}

}}}