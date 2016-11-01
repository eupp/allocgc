#include <libprecisegc/details/allocators/mlo_allocator.hpp>

#include <libprecisegc/details/allocators/core_allocator.hpp>
#include <libprecisegc/details/collectors/memory_index.hpp>
#include <libprecisegc/details/compacting/fix_ptrs.hpp>

namespace precisegc { namespace details { namespace allocators {

mlo_allocator::mlo_allocator()
    : m_head(get_fake_block())
{}

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
    size_t size = rqst.alloc_size() + sizeof(collectors::traceable_object_meta);
    size_t chunk_size = descriptor_t::chunk_size(size);
    size_t memblk_size = chunk_size + sizeof(control_block) + sizeof(descriptor_t);

    auto deleter = [this, memblk_size] (byte* ptr) {
        deallocate_block(ptr, memblk_size);
    };
    std::unique_ptr<byte, decltype(deleter)> memblk_owner(allocate_block(memblk_size), deleter);

    if (!memblk_owner) {
        // call gc
        memblk_owner = allocate_block(memblk_size);
        if (!memblk_owner) {
            core_allocator::expand_heap();
            memblk_owner = allocate_block(memblk_size);
            if (!memblk_owner) {
                throw gc_bad_alloc();
            }
        }
    }

    byte* memblk = memblk_owner.get();

    control_block* fake  = get_fake_block();
    control_block* cblk  = get_control_block(memblk);
    descriptor_t*  descr = get_descriptor(memblk);

    new (descr) descriptor_t(rqst.alloc_size());

    collectors::memory_index::add_to_index(memblk, chunk_size, descr);

    cblk->m_next = fake;
    cblk->m_prev = fake->m_prev;

    std::lock_guard<mutex_t> lock(m_mutex);

    fake->m_prev->m_next = cblk;
    fake->m_prev = cblk;

    if (m_head == fake) {
        m_head = cblk;
    }
    memblk_owner.release();

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

control_block* mlo_allocator::get_fake_block()
{
    return &m_fake;
}

mlo_allocator::control_block* mlo_allocator::get_control_block(byte* memblk)
{
    assert(memblk);
    return reinterpret_cast<control_block*>(memblk);
}

mlo_allocator::descriptor_t* mlo_allocator::get_descriptor(byte* memblk)
{
    assert(memblk);
    return reinterpret_cast<descriptor_t*>(memblk + sizeof(control_block));
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
    deallocate_block(ptr, size);
}

iterator mlo_allocator::begin()
{
    return iterator(m_head);
}

iterator mlo_allocator::end()
{
    return iterator(get_fake_block());
}

memory_range_type mlo_allocator::memory_range()
{
    byte* cblk = reinterpret_cast<byte*>(m_head);
    return boost::make_iterator_range(
            memory_iterator(get_mem(cblk), mlo_allocator::get_descriptor(cblk), m_head),
            memory_iterator(nullptr, nullptr, get_fake_block())
    );
}

byte* mlo_allocator::allocate_block(size_t size)
{
    return core_allocator::allocate(size);
}

void mlo_allocator::deallocate_block(byte* ptr, size_t size)
{
    core_allocator::deallocate(ptr, size);
}

mlo_allocator::iterator::iterator(control_block* cblk)
    : m_control_block(cblk)
{}

byte* mlo_allocator::iterator::memblk() const
{
    return reinterpret_cast<byte*>(m_control_block);
}

descriptor_t* mlo_allocator::iterator::get() const
{
    return get_descriptor(reinterpret_cast<byte*>(m_control_block));
}

mlo_allocator::descriptor_t& mlo_allocator::iterator::dereference() const
{
    return *get();
}

bool mlo_allocator::iterator::equal(const iterator& other) const
{
    return m_control_block == other.m_control_block;
}

void mlo_allocator::iterator::increment()
{
    m_control_block = m_control_block->m_next;
}

void mlo_allocator::iterator::decrement()
{
    m_control_block = m_control_block->m_prev;
}

descriptor_t* mlo_allocator::iterator::operator->() const
{
    return get();
}

mlo_allocator::memory_iterator::memory_iterator(byte* ptr, descriptor_t* descr, control_block* cblk)
    : managed_memory_iterator(ptr, descr)
    , m_cblk(cblk)
{
    assert(ptr   == get_mem(reinterpret_cast<byte*>(cblk)));
    assert(descr == mlo_allocator::get_descriptor(reinterpret_cast<byte*>(cblk)));
}

void mlo_allocator::memory_iterator::increment()
{
    m_cblk = get_cblk()->m_next;
    byte* cblk = reinterpret_cast<byte*>(m_cblk);
    set_ptr(get_mem(cblk));
    set_descriptor(mlo_allocator::get_descriptor(cblk));
}

void mlo_allocator::memory_iterator::decrement()
{
    m_cblk = get_cblk()->m_prev;
    byte* cblk = reinterpret_cast<byte*>(m_cblk);
    set_ptr(get_mem(cblk));
    set_descriptor(mlo_allocator::get_descriptor(cblk));
}

bool mlo_allocator::memory_iterator::equal(const memory_iterator& other) const
{
    return m_cblk == other.m_cblk;
}

control_block* mlo_allocator::memory_iterator::get_cblk()
{
    return m_cblk;
}

}}}
