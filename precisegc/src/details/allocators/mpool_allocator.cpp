#include <libprecisegc/details/allocators/mpool_allocator.hpp>

#include <tuple>

#include <libprecisegc/details/compacting/two_finger_compactor.hpp>
#include <libprecisegc/details/compacting/fix_ptrs.hpp>
#include <libprecisegc/details/collectors/memory_index.hpp>

namespace precisegc { namespace details { namespace allocators {

mpool_allocator::mpool_allocator()
    : m_freelist(nullptr)
    , m_top(nullptr)
    , m_end(nullptr)
{}

mpool_allocator::~mpool_allocator()
{
    for (auto it = m_descrs.begin(); it != m_descrs.end(); ) {
        it = destroy_descriptor(it);
    }
}

gc_alloc_response mpool_allocator::allocate(const gc_alloc_request& rqst, size_t aligned_size)
{
    size_t size = aligned_size + descriptor_t::meta_size();
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
    return descr->init(ptr, rqst);
}

mpool_allocator::iterator_t mpool_allocator::create_descriptor(byte* blk, size_t blk_size, size_t cell_size)
{
    m_descrs.emplace_back(blk, blk_size, cell_size);
    auto last = std::prev(m_descrs.end());
    collectors::memory_index::add_to_index(blk, blk_size, &(*last));
    return last;
}

iterator_t mpool_allocator::destroy_descriptor(iterator_t it)
{
    sweep(*it);
    collectors::memory_index::remove_from_index(it->memory(), it->size());
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
    m_prev_residency = stat.mem_residency;
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
        stat.mem_freed += sweep(descr);
    }
}

size_t mpool_allocator::sweep(descriptor_t& descr)
{
    byte*  it   = descr.memory();
    byte*  end  = descr.memory() + descr.size();
    size_t size = descr.cell_size();

    size_t freed = 0;
    for (size_t i = 0; it < end; it += size, ++i) {
        freed += descr.destroy(it);
        insert_into_freelist(it);
    }
    return freed;
}

void mpool_allocator::insert_into_freelist(byte* ptr)
{

}

void mpool_allocator::compact(compacting::forwarding& frwd, gc_heap_stat& stat)
{
    typedef typename memory_range_type::iterator::value_type value_t;

    auto rng = memory_range();

    if (rng.begin() == rng.end()) {
        return;
    }

    assert(std::all_of(rng.begin(), rng.end(),
                       [&rng] (const value_t& p) { return p.cell_size() == rng.begin()->cell_size(); }
    ));

    // two-finger compacting

    auto to = rng.begin();
    auto from = rng.end();
    size_t cell_size = to->cell_size();

    while (from != to) {
        to = std::find_if(to, from, [](value_t cell_ptr) {
            return !cell_ptr.get_mark();
        });

        auto rev_from = std::find_if(reverse_iterator(from),
                                     reverse_iterator(to),
                                     [] (value_t cell_ptr) {
                                         return cell_ptr.get_mark() && !cell_ptr.get_pin();
                                     });

        from = rev_from.base();
        if (from != to) {
            --from;

            to->destroy();
            to->set_mark(true);

            from->move(*to);

            from->destroy();
            from->set_mark(false);

            frwd.create(from->get(), to->get());

            stat.mem_freed  += cell_size;
            stat.mem_copied += cell_size;
        }
    }
}

void mpool_allocator::fix(const compacting::forwarding& frwd)
{
    auto rng = memory_range();
    compacting::fix_ptrs(rng.begin(), rng.end(), frwd);

}

bool mpool_allocator::empty() const
{
    return m_descrs.empty();
}

bool mpool_allocator::is_compaction_required(const gc_heap_stat& stat) const
{
    return stat.mem_residency < RESIDENCY_COMPACTING_THRESHOLD
           || (stat.mem_residency < RESIDENCY_NON_COMPACTING_THRESHOLD
               && std::abs(stat.mem_residency - m_prev_residency) < RESIDENCY_EPS);
}

memory_range_type mpool_allocator::memory_range()
{
    return utils::flatten_range(m_descrs.begin(), m_descrs.end());
}

}}}