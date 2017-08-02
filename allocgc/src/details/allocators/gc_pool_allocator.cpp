#include <liballocgc/details/allocators/gc_pool_allocator.hpp>

#include <cmath>
#include <tuple>
#include <iterator>

#include <liballocgc/details/gc_facade.hpp>
#include <liballocgc/details/allocators/gc_box.hpp>
#include <liballocgc/details/allocators/memory_index.hpp>
#include <liballocgc/details/compacting/fix_ptrs.hpp>
#include <liballocgc/details/compacting/two_finger_compactor.hpp>
#include <liballocgc/details/collectors/gc_new_stack_entry.hpp>

namespace allocgc { namespace details { namespace allocators {

gc_pool_allocator::gc_pool_allocator()
    : m_bucket_policy(nullptr)
    , m_core_alloc(nullptr)
    , m_freelist(nullptr)
    , m_top(nullptr)
    , m_end(nullptr)
    , m_prev_residency(0)
{}

gc_pool_allocator::~gc_pool_allocator()
{
    for (auto it = m_descrs.begin(); it != m_descrs.end(); ) {
        it = destroy_descriptor(it);
    }
}

void gc_pool_allocator::init(const gc_bucket_policy* bucket_policy, gc_core_allocator* core_alloc)
{
    m_bucket_policy = bucket_policy;
    m_core_alloc = core_alloc;
}

gc_alloc::response gc_pool_allocator::allocate(const gc_alloc::request& rqst, size_t aligned_size)
{
    if (m_top == m_end) {
        return try_expand_and_allocate(aligned_size, rqst, 0);
    }
    return stack_allocation(aligned_size, rqst);
}

gc_alloc::response gc_pool_allocator::try_expand_and_allocate(
        size_t size,
        const gc_alloc::request& rqst,
        size_t attempt_num
) {
    using namespace collectors;

    byte*  blk;
    size_t blk_size;
    std::tie(blk, blk_size) = allocate_block(size);
    if (blk) {
        create_descriptor(blk, blk_size, size);
        m_top = blk;
        m_end = blk + blk_size;
        return stack_allocation(size, rqst);
    } else if (m_freelist) {
        return freelist_allocation(size, rqst);
    } else {
        if (attempt_num == 0) {
            gc_options opt;
            opt.kind = gc_kind::COLLECT;
            opt.gen  = 0;
            m_core_alloc->gc(opt);
        } else if (attempt_num == 1) {
            m_core_alloc->expand_heap();
        } else {
            throw gc_bad_alloc();
        }
        return try_expand_and_allocate(size, rqst, ++attempt_num);
    }
}

gc_alloc::response gc_pool_allocator::stack_allocation(size_t size, const gc_alloc::request& rqst)
{
    assert(m_top <= m_end - size);

    descriptor_t* descr = &m_descrs.back();
    byte* ptr = m_top;
    m_top += size;
    return init_cell(ptr, rqst, descr);
}

gc_alloc::response gc_pool_allocator::freelist_allocation(size_t size, const gc_alloc::request& rqst)
{
    using namespace collectors;

    assert(m_freelist);

    byte* ptr  = reinterpret_cast<byte*>(m_freelist);
    m_freelist = reinterpret_cast<byte**>(m_freelist[0]);

    assert(contains(ptr));

    memset(ptr, 0, size);
    descriptor_t* descr = static_cast<descriptor_t*>(memory_index::get_descriptor(ptr).to_gc_descriptor());
    return init_cell(ptr, rqst, descr);
}

gc_alloc::response gc_pool_allocator::init_cell(byte* box_addr, const gc_alloc::request& rqst, descriptor_t* descr) {
    assert(descr);
    assert(box_addr);

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rqst.buffer());
    stack_entry->box_handle = gc_box_handle::from_box_addr(box_addr, descr);

    byte* obj_start = descr->init_cell(box_addr, rqst.obj_count(), rqst.type_meta());
    return gc_alloc::response(obj_start, rqst.buffer());
}

gc_pool_allocator::iterator_t gc_pool_allocator::create_descriptor(byte* blk, size_t blk_size, size_t cell_size)
{
    m_descrs.emplace_back(blk, blk_size, cell_size, *m_bucket_policy);
    auto last = std::prev(m_descrs.end());
    memory_index::index_gc_heap_memory(blk, blk_size, &(*last));
    return last;
}

gc_pool_allocator::iterator_t gc_pool_allocator::destroy_descriptor(iterator_t it)
{
    sweep(*it, false);
    memory_index::deindex(it->memory(), it->size());
    deallocate_block(it->memory(), it->size());
    return m_descrs.erase(it);
}

std::pair<byte*, size_t> gc_pool_allocator::allocate_block(size_t cell_size)
{
    assert(m_core_alloc);
    size_t chunk_size = m_bucket_policy->chunk_size(cell_size);
    return std::make_pair(m_core_alloc->allocate(chunk_size), chunk_size);
}

void gc_pool_allocator::deallocate_block(byte* ptr, size_t size)
{
    assert(m_core_alloc);
    m_core_alloc->deallocate(ptr, size);
}

bool gc_pool_allocator::contains(byte* ptr) const
{
    for (auto& descr: m_descrs) {
        if (descr.contains(ptr)) {
            return true;
        }
    }
    return false;
}

gc_collect_stat gc_pool_allocator::collect(compacting::forwarding& frwd)
{
    if (m_descrs.begin() == m_descrs.end()) {
        return gc_collect_stat();
    }

    gc_collect_stat stat;

    double residency = shrink(stat);

    m_top = nullptr;
    m_end = nullptr;
    m_freelist = nullptr;

    if (is_compaction_required(residency)) {
        compact(frwd, stat);
        m_prev_residency = 1.0;
    } else {
        sweep(stat);
        m_prev_residency = residency;
    }

    return stat;
}

double gc_pool_allocator::shrink(gc_collect_stat& stat)
{
    size_t mem_live = 0;
    size_t mem_occupied = 0;
    for (iterator_t it = m_descrs.begin(), end = m_descrs.end(); it != end; ) {
        stat.mem_used += it->size();
        if (it->is_unmarked()) {
            stat.mem_freed += it->size();
            it = destroy_descriptor(it);
        } else {
            mem_live     += it->cell_size() * it->count_lived();
            mem_occupied += it->size();
            stat.pinned_cnt += it->count_pinned();
            ++it;
        }
    }
    double residency = static_cast<double>(mem_live) / mem_occupied;
//    std::cerr << "prev_residency=" << m_prev_residency << "; residency=" << residency << std::endl;
    return residency;
}

void gc_pool_allocator::sweep(gc_collect_stat& stat)
{
    for (auto& descr: m_descrs) {
        stat.mem_freed += sweep(descr, true);
    }
}

size_t gc_pool_allocator::sweep(descriptor_t& descr, bool add_to_freelist)
{
    byte*  it   = descr.memory();
    byte*  end  = descr.memory() + descr.size();
    size_t size = descr.cell_size();

    size_t freed = 0;
    for (size_t i = 0; it < end; it += size, ++i) {
        if (!descr.get_mark(i)) {
            if (descr.is_init(i)) {
                #ifdef WITH_DESTRUCTORS
                    descr.finalize(i);
                #endif
                freed += size;
                if (add_to_freelist) {
                    insert_into_freelist(it);
                }
            } else if (add_to_freelist) {
                insert_into_freelist(it);
            }
        }
    }
    return freed;
}

void gc_pool_allocator::insert_into_freelist(byte* ptr)
{
    assert(contains(ptr));

    byte** next = reinterpret_cast<byte**>(ptr);
    next[0]     = reinterpret_cast<byte*>(m_freelist);
    m_freelist  = next;
}

void gc_pool_allocator::compact(compacting::forwarding& frwd, gc_collect_stat& stat)
{
//    typedef typename memory_range_type::iterator::value_type value_t;
//    typedef std::reverse_iterator<typename memory_range_type::iterator> reverse_iterator;
//
//    auto rng = memory_range();
//
//    if (rng.begin() == rng.end()) {
//        return;
//    }
//
//    assert(std::all_of(rng.begin(), rng.end(),
//                       [&rng] (const value_t& p) { return p.cell_size() == rng.begin()->cell_size(); }
//    ));
//
//    auto to = rng.begin();
//    auto from = rng.end();
//    size_t cell_size = to->cell_size();
//    size_t copied_cnt = 0;
//    while (from != to) {
//        to = std::find_if(to, from, [](value_t cell) {
//            return cell.get_lifetime_tag() == gc_lifetime_tag::FREE ||
//                   cell.get_lifetime_tag() == gc_lifetime_tag::GARBAGE;
//        });
//
//        if (to->get_lifetime_tag() == gc_lifetime_tag::GARBAGE) {
//            stat.mem_freed += cell_size;
//            #ifdef WITH_DESTRUCTORS
//                to->finalize();
//            #endif
//        }
//
//        auto rev_from = std::find_if(reverse_iterator(from),
//                                     reverse_iterator(to),
//                                     [] (value_t cell) {
//                                         return  cell.get_lifetime_tag() == gc_lifetime_tag::LIVE &&
//                                                 !cell.get_pin();
//                                     });
//
//        from = rev_from.base();
//        if (from != to) {
//            --from;
//
//            from->move(*to);
//            #ifdef WITH_DESTRUCTORS
//                from->finalize();
//            #endif
//            frwd.create(from->get(), to->get());
//            insert_into_freelist(from->get());
//
//            stat.mem_moved += cell_size;
//        }
//    }
}

void gc_pool_allocator::fix(const compacting::forwarding& frwd)
{
//    auto rng = memory_range();
//    compacting::fix_ptrs(rng.begin(), rng.end(), frwd);
}

void gc_pool_allocator::finalize()
{
    for (auto& descr: m_descrs) {
        descr.unmark();
    }
}

gc_memstat gc_pool_allocator::stats()
{
    gc_memstat stat;
    for (auto it = m_descrs.begin(), end = m_descrs.end(); it != end; ++it) {
        stat.mem_live  += it->mem_used();
        stat.mem_used  += it->size();
        stat.mem_extra += sizeof(descriptor_t);
    }
    return stat;
}

bool gc_pool_allocator::empty() const
{
    return m_descrs.empty();
}

bool gc_pool_allocator::is_compaction_required(double residency) const
{
    return false;
    if (residency < RESIDENCY_COMPACTING_THRESHOLD) {
        return true;
    }
    if (residency > RESIDENCY_NON_COMPACTING_THRESHOLD) {
        return false;
    }
    if ((m_prev_residency > 0) && std::abs(residency - m_prev_residency) < RESIDENCY_EPS) {
        return true;
    }
    return false;
}

//gc_pool_allocator::memory_range_type gc_pool_allocator::memory_range()
//{
//    return utils::flatten_range(m_descrs.begin(), m_descrs.end());
//}

}}}
