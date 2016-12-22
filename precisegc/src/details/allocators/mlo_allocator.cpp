#include <libprecisegc/details/allocators/mlo_allocator.hpp>

#include <libprecisegc/details/allocators/core_allocator.hpp>
#include <libprecisegc/details/collectors/memory_index.hpp>
#include <libprecisegc/details/compacting/fix_ptrs.hpp>

namespace precisegc { namespace details { namespace allocators {

mlo_allocator::mlo_allocator()
{ }

mlo_allocator::~mlo_allocator()
{
    std::lock_guard<mutex_t> lock(m_mutex);
    for (auto it = begin(); it != end(); ) {
        auto next = std::next(it);
        destroy(it.memblk());
        it = next;
    }
}

gc_alloc_response mlo_allocator::allocate(const gc_alloc_request& rqst)
{
    size_t size = rqst.alloc_size() + sizeof(descriptor_t) + sizeof(collectors::traceable_object_meta);
    size_t memblk_size = m_alloc.align_size(size, PAGE_SIZE);

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
    collectors::memory_index::add_to_index(align_by_page(memblk.get()), m_alloc.get_blk_size(memblk_size), descr);

    memblk.release();

    return descr->init(get_mem(memblk), rqst);
}

gc_heap_stat mlo_allocator::collect(compacting::forwarding& frwd)
{
    std::lock_guard<mutex_t> lock(m_mutex);

    gc_heap_stat stat;
    for (auto it = begin(); it != end(); ) {
        auto next = std::next(it);
        stat.mem_before_gc += it->size();
        if (!it->get_mark()) {
            stat.mem_freed += it->size();
            destroy(it.memblk());
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
    std::lock_guard<mutex_t> lock(m_mutex);

    auto rng = memory_range();
    compacting::fix_ptrs(rng.begin(), rng.end(), frwd);
}

byte* mlo_allocator::get_mem(byte* memblk)
{
    assert(memblk);
    return memblk + sizeof(control_block) + sizeof(descriptor_t);
}

void mlo_allocator::destroy(byte* ptr)
{
    control_block* cblk = get_control_block(ptr);
    descriptor_t* descr = get_descriptor(ptr);
    size_t size = descr->size() + sizeof(control_block) + sizeof(descriptor_t);

    collectors::memory_index::remove_from_index(ptr, size);

    if (cblk == m_head) {
        m_head = cblk->m_next;
    }
    cblk->m_next->m_prev = cblk->m_prev;
    cblk->m_prev->m_next = cblk->m_next;

    descr->destroy(get_mem(ptr));
    descr->~descriptor_t();
    deallocate_blk(ptr, size);
}

mlo_allocator::iterator mlo_allocator::begin()
{
    return iterator(m_head);
}

mlo_allocator::iterator mlo_allocator::end()
{
    return iterator(get_fake_block());
}

mlo_allocator::memory_range_type mlo_allocator::memory_range()
{
    byte* cblk = reinterpret_cast<byte*>(m_head);
    return boost::make_iterator_range(
            memory_iterator(get_mem(cblk), mlo_allocator::get_descriptor(cblk), m_head),
            memory_iterator(nullptr, nullptr, get_fake_block())
    );
}

byte* mlo_allocator::allocate_blk(size_t size)
{
    return m_alloc.allocate(size);
}

void mlo_allocator::deallocate_blk(byte* ptr, size_t size)
{
    m_alloc.deallocate(ptr, size);
}

mlo_allocator::iterator::iterator(control_block* cblk) noexcept
    : m_control_block(cblk)
{}

byte* mlo_allocator::iterator::memblk() const
{
    return reinterpret_cast<byte*>(m_control_block);
}

mlo_allocator::descriptor_t* mlo_allocator::iterator::get() const
{
    return get_descriptor(reinterpret_cast<byte*>(m_control_block));
}

mlo_allocator::descriptor_t& mlo_allocator::iterator::dereference() const
{
    return *get();
}

bool mlo_allocator::iterator::equal(const iterator& other) const noexcept
{
    return m_control_block == other.m_control_block;
}

void mlo_allocator::iterator::increment() noexcept
{
    m_control_block = m_control_block->m_next;
}

void mlo_allocator::iterator::decrement() noexcept
{
    m_control_block = m_control_block->m_prev;
}

mlo_allocator::descriptor_t* mlo_allocator::iterator::operator->() const
{
    return get();
}

mlo_allocator::memory_iterator::memory_iterator()
    : managed_memory_iterator(nullptr, nullptr)
    , m_cblk(nullptr)
{}

mlo_allocator::memory_iterator::memory_iterator(byte* ptr, descriptor_t* descr, control_block* cblk)
    : managed_memory_iterator(ptr, descr)
    , m_cblk(cblk)
{
    assert(ptr   == get_mem(reinterpret_cast<byte*>(cblk)));
    assert(descr == mlo_allocator::get_descriptor(reinterpret_cast<byte*>(cblk)));
}

void mlo_allocator::memory_iterator::increment() noexcept
{
    m_cblk = get_cblk()->m_next;
    byte* cblk = reinterpret_cast<byte*>(m_cblk);
    set_ptr(get_mem(cblk));
    set_descriptor(mlo_allocator::get_descriptor(cblk));
}

void mlo_allocator::memory_iterator::decrement() noexcept
{
    m_cblk = get_cblk()->m_prev;
    byte* cblk = reinterpret_cast<byte*>(m_cblk);
    set_ptr(get_mem(cblk));
    set_descriptor(mlo_allocator::get_descriptor(cblk));
}

bool mlo_allocator::memory_iterator::equal(const memory_iterator& other) const noexcept
{
    return m_cblk == other.m_cblk;
}

mlo_allocator::control_block* mlo_allocator::memory_iterator::get_cblk()
{
    return m_cblk;
}

}}}
